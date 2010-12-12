#include "analyser.h"
#include "sh4_cpu_shil.h"
#include "blockmanager.h"

#include "dc\mem\sh4_mem.h"
#include "dc\sh4\sh4_registers.h"

#define CPU_RATIO 1
#define CPU_BASIC_BLOCK_SIZE (BLOCKLIST_MAX_CYCLES/2)
u32 execution_group_busy[sh4_eu_max];
u32 known_pl_cycles=0;

//pipeline hackmulation
void InitPipeline()
{
	known_pl_cycles=0;
	memset(execution_group_busy,0,sizeof(execution_group_busy));
}
void plSubCycles(u32 cycles)
{
	for (int i=0;i<sh4_eu_max;i++)
	{
		if (execution_group_busy[i]>0)
		{
			execution_group_busy[i]-=cycles;
			if (execution_group_busy[i]<0)
				execution_group_busy[i]=0;
		}
	}
}
void StepPipeline(u32 opcode)
{
	s32 rv;
	if (OpDesc[opcode])
	{
		rv= OpDesc[opcode]->IssueCycles;
		if (rv==0)
			rv=OpDesc[opcode]->LatencyCycles;

		s32 lc=OpDesc[opcode]->LatencyCycles;
		lc-=rv;//if it has latency>issue
		if (opcode>=0xF000 && !settings.dynarec.UnderclockFpu)
		{
			lc=0;
			rv/=3;
			if (rv==0)
				rv=1;
		}
		if (lc>0)
			execution_group_busy[OpDesc[opcode]->unit]+=lc;
		//we allways count issue cycles
		//latecny cycles are counted @ the end :)
		known_pl_cycles+=rv;
	}
	else
	{
		rv= 10;
	}
	//if (rv<2)
	//	rv=2;
	//return rv;
}
void TermPipeline()
{
	u32 mpc=0;
	for (int i=0;i<sh4_eu_max;i++)
	{
		//if (execution_group_busy[i]>mpc)
		{
			mpc+=execution_group_busy[i];
		}
	}
	known_pl_cycles+=mpc;
}

void ScanCode(u32 pc,CodeRegion* to)
{
	to->start=pc;

	InitPipeline();
	
	bool stop=false;
	u32 op_count=0;
	u32 SOM;

	while(stop==false)
	{
		op_count++;
		u32 opcode=IReadMem16(pc);
		StepPipeline(opcode);

		if (Scanner_FindSOM(opcode,pc,&SOM))
		{
			//log("Scanner : SOM %d\n",SOM);
			pc+=SOM+2;
			op_count+=SOM>>1;
			known_pl_cycles+=SOM>>1;
		}
		else
		{
			pc+=2;
			stop=(OpTyp[opcode] &  (WritesSR | WritesFPSCR | WritesPC))!=0;
			if (stop && OpTyp[opcode] & Delayslot) 
			{
				u32 opcode=ReadMem16(pc);
				StepPipeline(opcode);
				pc+=2;
			}
		}

		if (op_count>=CPU_BASIC_BLOCK_SIZE)
			stop=true;
	}

	TermPipeline();

	to->end=pc-2;

	to->cycles=known_pl_cycles*CPU_RATIO;
}

//Code Analyser
void AnalyseCode(BasicBlock* to)
{
	u32 pc=to->start;
	ilst=&to->ilst;

	to->flags.FpuMode = GET_CURRENT_FPU_MODE();

	u32 block_ops=0;
	u32 endpc;
	for(;;)
	{
		u32 opcode=IReadMem16(pc);
		block_ops++;
		RecOpPtr[opcode](opcode,pc,to);

		if (to->flags.SynthOpcode)
		{
			switch (to->flags.SynthOpcode )
			{
			case BLOCK_SOM_SIZE_128:
				//log("Syth opcode found at pc 0x%X , bytelen = 128+2 , skiping 130 bytes\n",pc);
				pc+=128;
				block_ops+=128>>1;
				break;
			
			case BLOCK_SOM_RESERVED1:
			case BLOCK_SOM_RESERVED2:
				break;
			}
			to->flags.SynthOpcode=BLOCK_SOM_NONE;
		}

		if (to->flags.EndAnalyse)
		{
			endpc=pc;
			break;
		}

		//must not happen as all calls/branches are emulated natively (338)
		//verify((OpTyp[opcode]&WritesPC)==0);

		if ((OpTyp[opcode]&(WritesSR | WritesFPSCR | WritesPC)))
		{
			//after execution , resume to recompile from pc+2
			//opcode is interpreted so pc is set , if update shit() is called , pc must remain

			endpc=pc;
			to->flags.ExitType =BLOCK_EXITTYPE_FIXED_CSC;
			//to->flags.PerformModeLookup=1; we use BLOCK_EXITTYPE_FIXED_CSC from now on 
			ilst->add(reg_pc,2);
			break;
		}

		if (block_ops>=(CPU_BASIC_BLOCK_SIZE))
		{
			to->flags.ExitType=BLOCK_EXITTYPE_FIXED;
			endpc=pc;
			to->TF_next_addr=pc+2;
			break;
		}
 
		pc+=2;
	}

	to->ilst.op_count=(u32)to->ilst.opcodes.size();
}




//Init/Reset/Term
void InitAnalyser()
{
	shil_DynarecInit();
}

void ResetAnalyser()
{
	
}

void TermAnalyser()
{
	
}