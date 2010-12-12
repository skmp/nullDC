#include "types.h"
#include <windows.h>

#include "dc/sh4/sh4_interpreter.h"
#include "dc/sh4/sh4_opcode_list.h"
#include "dc/sh4/sh4_registers.h"
#include "dc/sh4/sh4_if.h"
#include "dc/pvr/pvr_if.h"
#include "dc/aica/aica_if.h"
#include "dc/gdrom/gdrom_if.h"
#include "dc/sh4/intc.h"
#include "dc/sh4/tmu.h"
#include "dc/mem/sh4_mem.h"
#include "dc/sh4/shil/shil_ce.h"

#include "emitter/emitter.h"

#include "recompiler.h"
#include "blockmanager.h"
#include "analyser.h"

#include <time.h>
#include <float.h>

#define CPU_TIMESLICE	(BLOCKLIST_MAX_CYCLES)

//uh uh 
volatile bool  rec_sh4_int_bCpuRun=false;
cThread* rec_sh4_int_thr_handle=0;

u32 rec_exec_cycles=0;
time_t rec_odtime=0;

u32 avg_rat=0;
u32 avg_rat_cnt=0;
u32 avg_bc=0;

u32 sb_count=0;
u32 nb_count=0;
u32 rec_cycles=0;

void* Dynarec_Mainloop_no_update;
void* Dynarec_Mainloop_do_update;
void* Dynarec_Mainloop_end;

void DynaPrintLinkStart();
void DynaPrintLinkEnd();
void DumpBlockMappings();
LARGE_INTEGER total_compile;
u32 CompiledSRCsz=0;
CompiledBlockInfo*  __fastcall CompileCode(u32 pc)
{
	LARGE_INTEGER comp_time_start,comp_time_end;
	CompiledBlockInfo* cblock=0;
	//bool reset_blocks;

	QueryPerformanceCounter(&comp_time_start);
	{
		nb_count++;
		//cblock=AnalyseCodeSuperBlock(pc);
		if (cblock==0)
			cblock=CompileBasicBlock(pc);
		RegisterBlock(cblock);
	}
	QueryPerformanceCounter(&comp_time_end);
	total_compile.QuadPart+=comp_time_end.QuadPart-comp_time_start.QuadPart;
	//compile code
	//return pointer
	if (!cblock)
	{
		_SuspendAllBlocks();
		log("Compile failed -- retrying\n");
		return CompileCode(pc);//retry
	}
	
	return cblock;
}
BasicBlockEP* __fastcall CompileCodePtr()
{
	return CompileCode(pc)->Code;
}
INLINE BasicBlockEP * __fastcall GetRecompiledCodePointer(u32 pc)
{
	return FindCode(pc);
}

CompiledBlockInfo* FindOrRecompileBlock(u32 pc)
{
	CompiledBlockInfo* cblock=FindBlock(pc);
	
	if (cblock)
		return cblock;
	else
		return CompileCode(pc);
}

void naked CompileAndRunCode()
{
	__asm 
	{
		call CompileCodePtr;
		jmp eax;
	}
}

void __fastcall rec_sh4_int_RaiseExeption(u32 ExeptionCode,u32 VectorAddress)
{
}
void rec_sh4_ResetCache()
{
	SuspendAllBlocks();
}
//asm version
BasicBlockEP* __fastcall FindCode_full(u32 address,CompiledBlockInfo* fastblock);

extern u32 fast_lookups;
u32 old_esp;
u32 Dynarec_Mainloop_no_update_fast;

void naked DynaMainLoop()
{
	__asm
	{
		//save corrupted regs
		push esi;
		push edi;
		push ebx;
		push ebp;

		mov old_esp,esp;

		//Allocate the ret cache table on stack
		//format :
		
		//[table] <- 'on'=1 , 'off' = 1 , 'index'=0
		//[waste] <- 'on'=1 , 'off' = 0 , 'index'=0

		sub esp,RET_CACHE_SZ*4-1;	//allign to RET_CACHE_SZ*4 
		and esp,~(RET_CACHE_SZ*4-1);	//(1kb for 32 entry cache) <- now , we are '00'
		sub esp,RET_CACHE_SZ*2;		//<- now we are '10'

		//now store it !
		mov ret_cache_base,esp;
		add ret_cache_base,RET_CACHE_SZ;	//pointer to the table base ;)

		//Reset it !
		call ret_cache_reset;

		//misc pointers needed
		mov block_stack_pointer,esp;
		mov Dynarec_Mainloop_no_update,offset no_update;
		mov Dynarec_Mainloop_no_update_fast,offset no_update;
		mov Dynarec_Mainloop_do_update,offset do_update;
		mov Dynarec_Mainloop_end,offset end_of_mainloop;
		//Max cycle count :)
		mov rec_cycles,(CPU_TIMESLICE*9/10);

		jmp no_update;

		//some utility code :)

		//partial , faster lookup. When we know the mode hasnt changed :)

		align 16
no_update:
		/*
		//called if no update is needed
		
		call GetRecompiledCodePointer;
		*/
		
		/*
		#define LOOKUP_HASH_SIZE	0x4000
		#define LOOKUP_HASH_MASK	(LOOKUP_HASH_SIZE-1)
		#define GetLookupHash(addr) ((addr>>2)&LOOKUP_HASH_MASK)

		*/
		/*
		CompiledBlockInfo* fastblock;
		fastblock=BlockLookupGuess[GetLookupHash(address)];
		*/
		mov ecx,pc;
		mov edx,ecx;
		and edx,(LOOKUP_HASH_MASK<<2);

		mov edx,[BlockLookupGuess + edx]
		

		/*
		if ((fastblock->start==address) && 
			(fastblock->cpu_mode_tag ==fpscr.PR_SZ)
			)
		{
			fastblock->lookups++;
			return fastblock->Code;
		}*/
		
		mov eax,fpscr;
		cmp [edx],ecx;
		jne full_lookup;
		
		shr eax,19;
		and eax,0x3;
		cmp [edx+12],eax;
		jne full_lookup;
		add dword ptr[edx+16],1;
#ifdef _BM_CACHE_STATS
		add fast_lookups,1;
#endif
		jmp dword ptr[edx+8];
		/*
		else
		{
			return FindCode_full(address,fastblock);
		}*/
full_lookup:
		call FindCode_full
		jmp eax;

		align 16
do_update:
		//called if update is needed
		mov ecx,CPU_TIMESLICE;
		add rec_cycles,ecx;
		//ecx=cycles
		call UpdateSystem;

		//check for exit
		cmp rec_sh4_int_bCpuRun,0;
		jne no_update;

		//exit from function
		mov esp,old_esp;
		pop ebp;
		pop ebx;
		pop edi;
		pop esi;
end_of_mainloop:
		ret;
	}
}

//interface
void rec_Sh4_int_Run()
{
	rec_sh4_int_bCpuRun=true;
	rec_cycles=0;
	SetFloatStatusReg();
	DynaMainLoop();
}

void rec_Sh4_int_Stop()
{
	if (rec_sh4_int_bCpuRun)
	{
		rec_sh4_int_bCpuRun=false;
	}
}

void rec_Sh4_int_Step() 
{
	if (rec_sh4_int_bCpuRun)
	{
		log("recSh4 Is running , can't step\n");
	}
	else
	{
		u32 op=ReadMem16(pc);
		ExecuteOpcode(op);
		pc+=2;
	}
}

void rec_Sh4_int_Skip() 
{
	if (rec_sh4_int_bCpuRun)
	{
		log("recSh4 Is running , can't Skip\n");
	}
	else
	{
		pc+=2;
	}
}

void rec_Sh4_int_Reset(bool Manual) 
{
	if (rec_sh4_int_bCpuRun)
	{
		log("Sh4 Is running , can't Reset\n");
	}
	else
	{
		Sh4_int_Reset(Manual);

		ResetAnalyser();
		ResetBlockManager();
		
		//Any more registers have default value ?
		log("recSh4 Reset\n");
	}
}

void rec_Sh4_int_Init() 
{
	//InitHash();
	Sh4_int_Init();
	InitAnalyser();
	InitBlockManager();

	ResetAnalyser();
	ResetBlockManager();

	log("recSh4 Init\n");
}

void rec_Sh4_int_Term() 
{
	TermBlockManager();
	TermAnalyser();
	Sh4_int_Term();
	log("recSh4 Term\n");
}

bool rec_Sh4_int_IsCpuRunning() 
{
	return rec_sh4_int_bCpuRun;
}
