/*
	sh4 base core
	most of it is (very) old
	could use many cleanups, lets hope someone does them
*/

//Specialised version for recompiler ;)
#ifdef SH4_REC
#include "sh4_cpu_branch_rec.h"
#else

//braf <REG_N>                  
sh4op(i0000_nnnn_0010_0011)
{
	u32 n = GetN(op);
	u32 newpc = r[n] + pc + 2;//pc +2 is done after
	ExecuteDelayslot();	//WARN : r[n] can change here
	pc = newpc;
} 
//bsrf <REG_N>                  
 sh4op(i0000_nnnn_0000_0011)
{
	u32 n = GetN(op);
	u32 newpc = r[n] + pc +2;//pc +2 is done after
	pr = pc + 4;		   //after delayslot
	ExecuteDelayslot();	//WARN : pr and r[n] can change here
	pc = newpc;
} 


 //rte                           
 sh4op(i0000_0000_0010_1011)
{
	//iNimp("rte");
	//sr.SetFull(ssr);
	u32 newpc = spc;//+2 is added after instruction
	bool intrr=ExecuteDelayslot_RTE();
	pc = newpc -2;

	if (intrr)
	{
		//FIXME olny if interrupts got on .. :P
		pc+=2;
		UpdateINTC();
		pc-=2;
	}
} 


//rts                           
 sh4op(i0000_0000_0000_1011)
{
	u32 newpc=pr;//+2 is added after instruction
	ExecuteDelayslot();	//WARN : pr can change here
	pc=newpc-2;
} 


// bf <bdisp8>                   
 sh4op(i1000_1011_iiii_iiii)
{//ToDo : Check Me [26/4/05]  | Check DELAY SLOT [28/1/06]
	if (sr.T==0)
	{
		//direct jump
		pc = (u32)((GetSImm8(op))*2 + 4 + pc );
		pc-=2;
	}
}


// bf.s <bdisp8>                 
 sh4op(i1000_1111_iiii_iiii)
{
	if (sr.T==0)
	{
		//delay 1 instruction
		u32 newpc=(u32) ( (GetSImm8(op)<<1) + pc+2);//pc+2 is done after
		ExecuteDelayslot();
		pc = newpc;
	}
}


// bt <bdisp8>                   
 sh4op(i1000_1001_iiii_iiii)
{
	if (sr.T==1)
	{
		//direct jump
		pc = (u32) ( (GetSImm8(op)<<1) + pc+2);
	}
}


// bt.s <bdisp8>                 
 sh4op(i1000_1101_iiii_iiii)
{
	if (sr.T == 1)
	{
		//delay 1 instruction
		u32 newpc=(u32) ( (GetSImm8(op)<<1) + pc+2);//pc+2 is done after
		ExecuteDelayslot();
		pc = newpc;
	}
}





// bra <bdisp12>
sh4op(i1010_iiii_iiii_iiii)
{//ToDo : Check Me [26/4/05] | Check ExecuteDelayslot [28/1/06] 
	//delay 1 jump imm12
	u32 newpc =(u32) ((  ((s16)((GetImm12(op))<<4)) >>3)  + pc + 4);//(s16<<4,>>4(-1*2))
	ExecuteDelayslot();
	pc=newpc-2;
}
// bsr <bdisp12>
sh4op(i1011_iiii_iiii_iiii)
{//ToDo : Check Me [26/4/05] | Check new delay slot code [28/1/06]
	//iNimp("bsr <bdisp12>");
	u32 disp = GetImm12(op);
	pr = pc + 4;
	//delay 1 opcode
	u32 newpc = (u32)((((s16)(disp<<4)) >> 3) + pc + 4);

	ExecuteDelayslot();
	pc=newpc-2;
}

// trapa #<imm>                  
sh4op(i1100_0011_iiii_iiii)
{
	//log("trapa 0x%X\n",(GetImm8(op) << 2));
	CCN_TRA = (GetImm8(op) << 2);
	Do_Exeption(0,0x160,0x100);
}
//jmp @<REG_N>                  
 sh4op(i0100_nnnn_0010_1011)
{   //ToDo : Check Me [26/4/05] | Check new delay slot code [28/1/06]
	u32 n = GetN(op);
	//delay 1 instruction
	u32 newpc=r[n];
	ExecuteDelayslot();
	pc=newpc-2;//+2 is done after
}


//jsr @<REG_N>                  
 sh4op(i0100_nnnn_0000_1011)
{//ToDo : Check This [26/4/05] | Check new delay slot code [28/1/06]
	u32 n = GetN(op);
	
	pr = pc + 4;
	//delay one
	u32 newpc= r[n];
	ExecuteDelayslot();	//WARN : pr can change here

	pc=newpc-2;
}





//sleep                         
 sh4op(i0000_0000_0001_1011)
{
	//iNimp("Sleep");
	//just wait for an Interrupt
	//while on sleep the precition of Interrupt timing is not the same as when cpu is running :)
	sh4_sleeping=true;
	int i=0,s=1;

	pc+=2;//so that Interrupt return is on next opcode
	while (!UpdateSystem())//448
	{
		if (i++>1000)
		{
			s=0;
			break;
		}
	}
	//if not Interrupted , we must rexecute the sleep
	if (s==0)
		pc-=2;// re execute sleep
	
	pc-=2;//+2 is applied after opcode


	sh4_sleeping=false;
} 

//TODO : Fix this
#include "gui/base.h"
sh4op(sh4_bpt_op)
{
	sh4_cpu->Stop();
	pc-=2;//pew pew
}
#endif