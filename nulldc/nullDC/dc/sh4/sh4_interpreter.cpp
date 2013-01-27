
#include "types.h"
#include <windows.h>

#include "sh4_interpreter.h"
#include "sh4_opcode_list.h"
#include "sh4_registers.h"
#include "sh4_if.h"
#include "dc/pvr/pvr_if.h"
#include "dc/aica/aica_if.h"
#include "dmac.h"
#include "dc/gdrom/gdrom_if.h"
#include "naomi/naomi.h"
#include "intc.h"
#include "tmu.h"
#include "dc/mem/sh4_mem.h"

#include <time.h>
#include <float.h>

#define CPU_TIMESLICE	(448)
#define CPU_RATIO		(8)

//uh uh 
volatile bool  sh4_int_bCpuRun=false;

u32 exec_cycles=0;
time_t odtime=0;

u32 sh4_ex_ExeptionCode,sh4_ex_VectorAddress;
Sh4RegContext sh4_ex_SRC;


#define GetN(str) ((str>>8) & 0xf)
#define GetM(str) ((str>>4) & 0xf)
bool exept_was_dslot=false;
void sh4_int_restore_reg_cnt()
{
	//restore reg context
	LoadSh4Regs(&sh4_ex_SRC);
	//fix certain registers that may need fixing
	if (mmu_error_TT!=MMU_TT_IREAD)
	{
		//if the error was on IREAD no need for any fixing :)
		//this should realy not raise any exeptions :p
		u16 op = IReadMem16(pc);
		u32 n = GetN(op);
		u32 m = GetM(op);
		switch(OpDesc[op]->ex_fixup)
		{
		case rn_4:
			{
				r[n]+=4;
			}
			break;
		case rn_fpu_4:
			{
				//dbgbreak;
				r[n]+=4;
				//8 byte fixup if on double mode -> actualy this catches the exeption at the first reg read/write so i gota be carefull
				if (fpscr.SZ)
					r[n]+=4;
			}
			break;

		case rn_opt_1:
			{
				if (n!=m)
					r[n]+=1;
				//else
				//	dbgbreak;
			}
			break;

		case rn_opt_2:
			{
				if (n!=m)
					r[n]+=2;
				//else
				//	dbgbreak;
			}
			break;

		case rn_opt_4:
			{
				if (n!=m)
					r[n]+=4;
				//else
				//	dbgbreak;
			}
			break;

		case fix_none:
			break;
		}

	}
	if (exept_was_dslot)
		pc-=2;
	exept_was_dslot=false;
	//raise exeption
	RaiseExeption(sh4_ex_ExeptionCode,sh4_ex_VectorAddress);
	sh4_exept_raised=false;
}
void naked sh4_int_exept_hook()
{
	__asm 
	{
		call sh4_int_restore_reg_cnt;
		jmp [sh4_exept_next];
	}
}
void __fastcall sh4_int_RaiseExeption(u32 ExeptionCode,u32 VectorAddress)
{
	if (sh4_exept_raised)
	{
		log("WARNING : DOUBLE EXEPTION RAISED , IGNORING SECOND EXEPTION\n");
		dbgbreak;
		return;
	}
	sh4_exept_raised=true;
	*sh4_exept_ssp=(u32)sh4_int_exept_hook;

	sh4_ex_ExeptionCode=ExeptionCode;
	sh4_ex_VectorAddress=VectorAddress;

	//save reg context
	SaveSh4Regs(&sh4_ex_SRC);
}


//interface
void Sh4_int_Run()
{
	sh4_int_bCpuRun=true;

	__asm
	{
		//save regs used
		push esi;

		//for exeption rollback
		mov sh4_exept_ssp,esp;		//esp wont change after that :)
		sub sh4_exept_ssp,4;		//point to next stack item :)
		mov sh4_exept_next,offset i_exept_rp;

		//init vars
		mov esi,CPU_TIMESLICE;  //cycle count = max

		//loop start
i_mainloop:

		//run a single opcode -- doesn't use _any_ stack space :D
		{
i_run_opcode:
			mov ecx , pc;			//param #1 for readmem16
			call IReadMem16;		//ax has opcode to execute now
			movzx eax,ax;			//zero extend to 32b

			mov ecx,eax;			//ecx=opcode (param 1 to opcode handler)
			call OpPtr[eax*4];		//call opcode handler
			add pc,2;				//pc+=2 -> goto next opcode


			sub esi,CPU_RATIO;		//remove cycles from cycle count
			jns i_run_opcode;		//jump not (esi>0) , inner loop til timeslice is executed
		}

		//exeption rollback point
		//if an exception happened, resume execution here
i_exept_rp:
		//update system and run a new timeslice
		xor eax,eax;			//zero eax [used later]

		//Calculate next timeslice
		sub esi,exec_cycles;	//Add delayslot cycles
		add esi,CPU_TIMESLICE;

		mov exec_cycles,eax;	//zero out delayslot cycles

		//Call update system (cycle cnt is fixed to 448)
		call UpdateSystem;

		//if cpu still on go for one more brust of opcodes :)
		cmp  sh4_int_bCpuRun,0;
		jne i_mainloop;

i_exit_mainloop:
		//restore regs used
		pop esi;
	}


	sh4_int_bCpuRun=false;
}

void Sh4_int_Stop()
{
	if (sh4_int_bCpuRun)
	{
		sh4_int_bCpuRun=false;
	}
}

void Sh4_int_Step() 
{
	if (sh4_int_bCpuRun)
	{
		log("Sh4 Is running , can't step\n");
	}
	else
	{
		u32 op=ReadMem16(pc);
		ExecuteOpcode(op);
		pc+=2;
	}
}

void Sh4_int_Skip() 
{
	if (sh4_int_bCpuRun)
	{
		log("Sh4 Is running , can't Skip\n");
	}
	else
	{
		pc+=2;
	}
}

void Sh4_int_Reset(bool Manual) 
{
	if (sh4_int_bCpuRun)
	{
		log("Sh4 Is running , can't Reset\n");
	}
	else
	{
		pc = 0xA0000000;

		memset(r,0,sizeof(r));
		memset(r_bank,0,sizeof(r_bank));

		gbr=ssr=spc=sgr=dbr=vbr=0;
		mac.h=mac.l=pr=fpul=0;

		sr.SetFull(0x700000F0);
		old_sr=sr;
		UpdateSR();

		fpscr.full = 0x0004001;
		old_fpscr=fpscr;
		UpdateFPSCR();
		
		//Any more registers have default value ?
		log("Sh4 Reset\n");
		patchRB=0;
	}
}

void Sh4_int_Init() 
{
	BuildOpcodeTables();
	GenerateSinCos();
	log("Sh4 Init\n");
}

void Sh4_int_Term() 
{
	Sh4_int_Stop();
	CleanupSinCos();
	log("Sh4 Term\n");
}

bool Sh4_int_IsCpuRunning() 
{
	return sh4_int_bCpuRun;
}

u32 Sh4_int_GetRegister(Sh4RegType reg)
{
	if ((reg>=r0) && (reg<=r15))
	{
		return r[reg-r0];
	}
	else if ((reg>=r0_Bank) && (reg<=r7_Bank))
	{
		return r_bank[reg-r0_Bank];
	}
	else if ((reg>=fr_0) && (reg<=fr_15))
	{
		return fr_hex[reg-fr_0];
	}
	else if ((reg>=xf_0) && (reg<=xf_15))
	{
		return xf_hex[reg-xf_0];
	}
	else
	{
		switch(reg)
		{
		case reg_gbr :
			return gbr;
			break;
		case reg_vbr :
			return vbr;
			break;

		case reg_ssr :
			return ssr;
			break;

		case reg_spc :
			return spc;
			break;

		case reg_sgr :
			return sgr;
			break;

		case reg_dbr :
			return dbr;
			break;

		case reg_mach :
			return mac.h;
			break;

		case reg_macl :
			return mac.l;
			break;

		case reg_pr :
			return pr;
			break;

		case reg_fpul :
			return fpul;
			break;


		case reg_pc :
			return pc;
			break;

		case reg_sr :
			return sr.GetFull();
			break;
		case reg_fpscr :
			return fpscr.full;
			break;


		default:
			EMUERROR2("unknown register Id %d",reg);
			return 0;
			break;
		}
	}
}


void Sh4_int_SetRegister(Sh4RegType reg,u32 regdata)
{
	if (reg<=r15)
	{
		r[reg]=regdata;
	}
	else if (reg<=r7_Bank)
	{
		r_bank[reg-16]=regdata;
	}
	else
	{
		switch(reg)
		{
		case reg_gbr :
			gbr=regdata;
			break;

		case reg_ssr :
			ssr=regdata;
			break;

		case reg_spc :
			spc=regdata;
			break;

		case reg_sgr :
			sgr=regdata;
			break;

		case reg_dbr :
			dbr=regdata;
			break;

		case reg_mach :
			mac.h=regdata;
			break;

		case reg_macl :
			mac.l=regdata;
			break;

		case reg_pr :
			pr=regdata;
			break;

		case reg_fpul :
			fpul=regdata;
			break;


		case reg_pc :
			pc=regdata;
			break;
		case reg_sr :
			sr.SetFull(regdata);
			UpdateSR();
			break;
		case reg_fpscr :
			fpscr.full=regdata;
			UpdateFPSCR();
			break;


		default:
			EMUERROR2("unknown register Id %d",reg);
			break;
		}
	}
}

//more coke .. err code

bool ExecuteDelayslot()
{
	exec_cycles+=CPU_RATIO;
	pc+=2;

	u32 op=IReadMem16(pc);

	if (sh4_exept_raised)
	{
		exept_was_dslot=true;
		return false;
	}

	//verify(sh4_exept_raised==false);
	if (op!=0)
		ExecuteOpcode(op);

	if(sh4_exept_raised)
	{
		exept_was_dslot=true;
		return false;
	}

	return true;
}
bool ExecuteDelayslot_RTE()
{
	exec_cycles+=CPU_RATIO;

	pc+=2;
	u32 op=IReadMem16(pc);
	sr.SetFull(ssr);
	bool rv=UpdateSR();
	verify(sh4_exept_raised==false);
	if (op!=0)
	ExecuteOpcode(op);
	verify(sh4_exept_raised==false);

	return rv;
}

#include "ccn.h"

void FreeSuspendedBlocks();
void DynaPrintCycles();

//General update
s32 rtc_cycles = 0;
u32 update_cnt = 0;
u32 aica_sample_cycles=0;

//typicaly, 446428 calls/second (448 cycles/call)
//fast update is 448 cycles
//medium update is 448*8=3584 cycles
//slow update is 448*16=7168  cycles

//14336 Cycles
void __fastcall VerySlowUpdate()
{
	rtc_cycles-=14336;
	if (rtc_cycles<=0)
	{
		rtc_cycles+=200*1000*1000;
		settings.dreamcast.RTC++;
	}
	//This is a patch for the DC LOOPBACK test GDROM (disables serial i/o)
	/*
	*(u16*)&mem_b.data[(0xC0196EC)& 0xFFFFFF] =9;
	*(u16*)&mem_b.data[(0xD0196D8+2)& 0xFFFFFF]=9;
	*/
	FreeSuspendedBlocks();
}
//7168 Cycles
void __fastcall SlowUpdate()
{
	#if DC_PLATFORM!=DC_PLATFORM_NAOMI
		UpdateGDRom();
	#else
		Update_naomi();
	#endif	
	if (!(update_cnt&0x10))
		VerySlowUpdate();
}
//3584 Cycles
#define AICA_SAMPLE_GCM 441
#define AICA_SAMPLE_CYCLES (SH4_MAIN_CLOCK/(44100/AICA_SAMPLE_GCM))

void aica_periodical(u32 cycl);
void maple_periodical(u32 cycl);

void __fastcall MediumUpdate()
{
	#if 0
	if(!GetAsyncKeyState(DEV_TOOL_FAST_FW_KEY))
	#endif
	{
		aica_sample_cycles+=3584*AICA_SAMPLE_GCM;

		if (aica_sample_cycles>=AICA_SAMPLE_CYCLES)
		{
			UpdateArm(512);
			UpdateAica(1);
			aica_sample_cycles-=AICA_SAMPLE_CYCLES;
		}

		aica_periodical(3584);
	}

	maple_periodical(3584);

	libExtDevice.UpdateExtDevice(3584);
	UpdateDMA();

	if (!(update_cnt&0x8))
		SlowUpdate();
}

//448 Cycles
//as of 7/2/2k8 this is fixed to 448 cycles
int __fastcall UpdateSystem()
{		
	UpdateTMU(448);
	UpdatePvr(448);

	if (!(update_cnt&0x7))
		MediumUpdate();

	update_cnt++;

	return UpdateINTC();
}