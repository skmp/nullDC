
#include "types.h"
#include "intc.h"
#include "tmu.h"
#include "ccn.h"
#include "sh4_Registers.h"
#include "log/logging_interface.h"

/*
	Interrupt controller emulation
	Sh4 has a very configurable intc, supporting variable priority per interrupt source
	On the IRL inputs the holly intc is connected

	The intc is very efficiently implemented using bit-vectors and lotsa caching
*/

#include "dc/mem/sh4_internal_reg.h"
#include "dc/asic/asic.h"
#include "dc/maple/maple_if.h"

#define printf_except(...)

//#define COUNT_INTERRUPT_UPDATES
//Return interrupt priority level
template<u32 IRL>
u32 GetIRLPriority()
{
	return 15-IRL;
}
//Return interrupt priority level
template<u16* reg,u32 part>
u32 GetPriority_bug()	//VC++ bugs this ;p
{
	return ((*reg)>>(4*part))&0xF;
}
template<u32 part>
u32 GetPriority_a()
{
	return ((INTC_IPRA.reg_data)>>(4*part))&0xF;
}
template<u32 part>
u32 GetPriority_b()
{
	return ((INTC_IPRB.reg_data)>>(4*part))&0xF;
}
template<u32 part>
u32 GetPriority_c()
{
	return ((INTC_IPRC.reg_data)>>(4*part))&0xF;
}
#define GIPA(p) GetPriority_a< p >
#define GIPB(p)  GetPriority_b< p >
#define GIPC(p)  GetPriority_c< p >

const InterptSourceList_Entry InterruptSourceList[]=
{
	//IRL
	{GetIRLPriority<9>,0x320},//sh4_IRL_9			= KMIID(sh4_int,0x320,0),
	{GetIRLPriority<11>,0x360},//sh4_IRL_11			= KMIID(sh4_int,0x360,1),
	{GetIRLPriority<13>,0x3A0},//sh4_IRL_13			= KMIID(sh4_int,0x3A0,2),

	//HUDI
	{GIPC(0),0x600},//sh4_HUDI_HUDI		= KMIID(sh4_int,0x600,3),  /* H-UDI underflow */

	//GPIO (missing on dc ?)
	{GIPC(3),0x620},//sh4_GPIO_GPIOI		= KMIID(sh4_int,0x620,4),

	//DMAC
	{GIPC(2),0x640},//sh4_DMAC_DMTE0		= KMIID(sh4_int,0x640,5),
	{GIPC(2),0x660},//sh4_DMAC_DMTE1		= KMIID(sh4_int,0x660,6),
	{GIPC(2),0x680},//sh4_DMAC_DMTE2		= KMIID(sh4_int,0x680,7),
	{GIPC(2),0x6A0},//sh4_DMAC_DMTE3		= KMIID(sh4_int,0x6A0,8),
	{GIPC(2),0x6C0},//sh4_DMAC_DMAE		= KMIID(sh4_int,0x6C0,9),

	//TMU
	{GIPA(3),0x400},//sh4_TMU0_TUNI0		=  KMIID(sh4_int,0x400,10), /* TMU0 underflow */
	{GIPA(2),0x420},//sh4_TMU1_TUNI1		=  KMIID(sh4_int,0x420,11), /* TMU1 underflow */
	{GIPA(1),0x440},//sh4_TMU2_TUNI2		=  KMIID(sh4_int,0x440,12), /* TMU2 underflow */
	{GIPA(1),0x460},//sh4_TMU2_TICPI2		=  KMIID(sh4_int,0x460,13),

	//RTC
	{GIPA(0),0x480},//sh4_RTC_ATI			= KMIID(sh4_int,0x480,14),
	{GIPA(0),0x4A0},//sh4_RTC_PRI			= KMIID(sh4_int,0x4A0,15),
	{GIPA(0),0x4C0},//sh4_RTC_CUI			= KMIID(sh4_int,0x4C0,16),

	//SCI
	{GIPB(1),0x4E0},//sh4_SCI1_ERI		= KMIID(sh4_int,0x4E0,17),
	{GIPB(1),0x500},//sh4_SCI1_RXI		= KMIID(sh4_int,0x500,18),
	{GIPB(1),0x520},//sh4_SCI1_TXI		= KMIID(sh4_int,0x520,19),
	{GIPB(1),0x540},//sh4_SCI1_TEI		= KMIID(sh4_int,0x540,29),

	//SCIF
	{GIPC(1),0x700},//sh4_SCIF_ERI		= KMIID(sh4_int,0x700,21),
	{GIPC(1),0x720},//sh4_SCIF_RXI		= KMIID(sh4_int,0x720,22),
	{GIPC(1),0x740},//sh4_SCIF_BRI		= KMIID(sh4_int,0x740,23),
	{GIPC(1),0x760},//sh4_SCIF_TXI		= KMIID(sh4_int,0x760,24),

	//WDT
	{GIPB(3),0x560},//sh4_WDT_ITI			= KMIID(sh4_int,0x560,25),

	//REF
	{GIPB(2),0x580},//sh4_REF_RCMI		= KMIID(sh4_int,0x580,26),
	{GIPA(2),0x5A0},//sh4_REF_ROVI		= KMIID(sh4_int,0x5A0,27),
};

//dynamicaly built
//Maps siid -> EventID
__declspec(align(64)) u16 InterruptEnvId[32]=
{
	0
};
//dynamicaly built
//Maps piid -> 1<<siid
__declspec(align(64)) u32 InterruptBit[32] = 
{
	0
};
__declspec(align(64)) u32 InterruptLevelBit[16]=
{
	0
};
void FASTCALL RaiseInterrupt_(InterruptID intr);
bool fastcall Do_Interrupt(u32 intEvn);
bool Do_Exeption(u32 lvl, u32 expEvn, u32 CallVect);
int Check_Ints();
bool HandleSH4_exept(InterruptID expt);


#define IPr_LVL6  0x6
#define IPr_LVL4  0x4
#define IPr_LVL2  0x2

extern bool sh4_sleeping;

//bool InterruptsArePending=true;

INTC_ICR_type  INTC_ICR;
INTC_IPRA_type INTC_IPRA;
INTC_IPRB_type INTC_IPRB;
INTC_IPRC_type INTC_IPRC;

//InterruptID intr_l;

u32 interrupt_vpend;
u32 interrupt_vmask;	//-1 -> ingore that kthx.0x0FFFffff allows all interrupts ( virtual interrupts are allways masked
u32 decoded_srimask;	//-1 kills all interrupts,including rebuild ones.

//bit 0 ~ 27 : interrupt source 27:0. 0 = lowest level, 27 = highest level.
//bit 28~31  : virtual interrupt sources.These have to do with the emu

//28:31 = 0 -> nothing (disabled)
//28:31 = 1 -> rebuild interrupt list		, retest interrupts
//28:31 = * -> undefined

//Rebuild sorted interrupt id table (priorities where updated)
void RequestSIIDRebuild() 
{ 
	interrupt_vpend|=2<<28;
}
bool SRdecode() 
{ 
	u32 imsk=sr.IMASK;
	decoded_srimask=InterruptLevelBit[imsk];

	if (sr.BL)
		decoded_srimask=0x0FFFFFFF;

	return (interrupt_vpend&interrupt_vmask)>decoded_srimask;
}

void fastcall VirtualInterrupt(u32 id)
{
	if (id)
	{
		u32 cnt=0;
		u32 vpend=interrupt_vpend;
		u32 vmask=interrupt_vmask;
		interrupt_vpend=0;
		interrupt_vmask=0xF0000000;
		//rebuild interrupt table
		for (u32 ilevel=0;ilevel<16;ilevel++)
		{
			for (u32 isrc=0;isrc<28;isrc++)
			{
				if (InterruptSourceList[isrc].GetPrLvl()==ilevel)
				{
					InterruptEnvId[cnt]=(u16)InterruptSourceList[isrc].IntEvnCode;
					bool p=(InterruptBit[isrc]&vpend) != 0;
					bool m=(InterruptBit[isrc]&vmask) != 0;

					InterruptBit[isrc]=1<<cnt;

					if (p)
						interrupt_vpend|=InterruptBit[isrc];

					if (m)
						interrupt_vmask|=InterruptBit[isrc];

					cnt++;
				}
			}
			InterruptLevelBit[ilevel]=(1<<cnt)-1;
		}

		SRdecode();
	}
}
//#ifdef COUNT_INTERRUPT_UPDATES
u32 no_interrupts,yes_interrupts;
//#endif
naked int UpdateINTC()
{
	__asm
	{
		mov ecx,interrupt_vpend;
		and ecx,interrupt_vmask;	//calculate enabled interrupts
		cmp ecx,decoded_srimask;	//If any of em isnt masked
		ja handle_inerrupt;			//handle em
		//else exit
		xor eax,eax;
#ifdef COUNT_INTERRUPT_UPDATES
		add no_interrupts,1;
#endif
		ret;

handle_inerrupt:
#ifdef COUNT_INTERRUPT_UPDATES
		add yes_interrupts,1;
#endif
		test ecx,0xF0000000;
		jnz  virtual_interrupt;
		bsr eax,ecx;							//find first interrupt
		movsx ecx,WORD PTR InterruptEnvId[eax*2];
		jmp Do_Interrupt;

virtual_interrupt:
		//handle virtual interrupts
		shr ecx,28;
		call VirtualInterrupt;
		jmp UpdateINTC;
	}
}

void RaiseExeption(u32 code,u32 vector)
{
	if (sr.BL!=0)
	{
		log("RaiseExeption: sr.BL==1, pc=%08X\n",pc);
		verify(sr.BL == 0);
	}
		
	spc = pc;
	ssr = sr.GetFull();
	sgr = r[15];
	CCN_EXPEVT = code;
	sr.MD = 1;
	sr.RB = 1;
	sr.BL = 1;
	pc = vbr + vector;
	UpdateSR();
	printf_except("RaiseExeption: from %08X , pc errh %08X\n",spc,pc);
}

void SetInterruptPend(InterruptID intr)
{
	u32 piid= intr & InterruptPIIDMask;
	interrupt_vpend|=InterruptBit[piid];
}
void ResetInterruptPend(InterruptID intr)
{
	u32 piid= intr & InterruptPIIDMask;
	interrupt_vpend&=~InterruptBit[piid];
}

void SetInterruptMask(InterruptID intr)
{
	u32 piid= intr & InterruptPIIDMask;
	interrupt_vmask|=InterruptBit[piid];
}
void ResetInterruptMask(InterruptID intr)
{
	u32 piid= intr & InterruptPIIDMask;
	interrupt_vmask&=~InterruptBit[piid];
}
//this is what left from the old intc .. meh .. 
//exeptions are handled here .. .. hmm are they ? :P
bool HandleSH4_exept(InterruptID expt)
{
	switch(expt)
	{

	case sh4_ex_TRAP:
		return Do_Exeption(0,0x160,0x100);

	default:
		return false;
	}
}

bool fastcall Do_Interrupt(u32 intEvn)
{
	printf_except("Interrupt : 0x%04x,0x%08x\n",intEvn,pc);
	verify(sr.BL==0);

	CCN_INTEVT = intEvn;

	ssr = sr.GetFull();
	spc = pc;
	sgr = r[15];
	sr.BL = 1;
	sr.MD = 1;
	sr.RB = 1;
	UpdateSR();

	pc = vbr + 0x600;

	return true;
}

bool Do_Exeption(u32 lvl, u32 expEvn, u32 CallVect)
{
	CCN_EXPEVT = expEvn;

	ssr = sr.GetFull();
	spc = pc+2;
	sgr = r[15];
	sr.BL = 1;
	sr.MD = 1;
	sr.RB = 1;
	UpdateSR();

	//this is from when the project was still in C#
	//left in for novely reasons ...
	//CallStackTrace.cstAddCall(sh4.pc, sh4.pc, sh4.vbr + 0x600, CallType.Interrupt);

	pc = vbr + CallVect;

	pc-=2;//fix up ;)
	return true;
}

//Register writes need interrupt re-testing !

void write_INTC_IPRA(u32 data)
{
	if (INTC_IPRA.reg_data!=(u16)data)
	{
		INTC_IPRA.reg_data=(u16)data;
		RequestSIIDRebuild();	//we need to rebuild the table
	}
}
void write_INTC_IPRB(u32 data)
{
	if (INTC_IPRB.reg_data!=(u16)data)
	{
		INTC_IPRB.reg_data=(u16)data;
		RequestSIIDRebuild();	//we need to rebuild the table
	}
}
void write_INTC_IPRC(u32 data)
{
	if (INTC_IPRC.reg_data!=(u16)data)
	{
		INTC_IPRC.reg_data=(u16)data;
		RequestSIIDRebuild();	//we need to rebuild the table
	}
}
//Init/Res/Term
void intc_Init()
{
	//INTC ICR 0xFFD00000 0x1FD00000 16 0x0000 0x0000 Held Held Pclk
	INTC[(INTC_ICR_addr&0xFF)>>2].flags=REG_16BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	INTC[(INTC_ICR_addr&0xFF)>>2].readFunction=0;
	INTC[(INTC_ICR_addr&0xFF)>>2].writeFunction=0;
	INTC[(INTC_ICR_addr&0xFF)>>2].data16=&INTC_ICR.reg_data;

	//INTC IPRA 0xFFD00004 0x1FD00004 16 0x0000 0x0000 Held Held Pclk
	INTC[(INTC_IPRA_addr&0xFF)>>2].flags=REG_16BIT_READWRITE | REG_READ_DATA;
	INTC[(INTC_IPRA_addr&0xFF)>>2].readFunction=0;
	INTC[(INTC_IPRA_addr&0xFF)>>2].writeFunction=write_INTC_IPRA;
	INTC[(INTC_IPRA_addr&0xFF)>>2].data16=&INTC_IPRA.reg_data;

	//INTC IPRB 0xFFD00008 0x1FD00008 16 0x0000 0x0000 Held Held Pclk
	INTC[(INTC_IPRB_addr&0xFF)>>2].flags=REG_16BIT_READWRITE | REG_READ_DATA;
	INTC[(INTC_IPRB_addr&0xFF)>>2].readFunction=0;
	INTC[(INTC_IPRB_addr&0xFF)>>2].writeFunction=write_INTC_IPRB;
	INTC[(INTC_IPRB_addr&0xFF)>>2].data16=&INTC_IPRB.reg_data;

	//INTC IPRC 0xFFD0000C 0x1FD0000C 16 0x0000 0x0000 Held Held Pclk
	INTC[(INTC_IPRC_addr&0xFF)>>2].flags=REG_16BIT_READWRITE | REG_READ_DATA;
	INTC[(INTC_IPRC_addr&0xFF)>>2].readFunction=0;
	INTC[(INTC_IPRC_addr&0xFF)>>2].writeFunction=write_INTC_IPRC;
	INTC[(INTC_IPRC_addr&0xFF)>>2].data16=&INTC_IPRC.reg_data;
}

void intc_Reset(bool Manual)
{
	INTC_ICR.reg_data = 0x0;
	INTC_IPRA.reg_data = 0x0;
	INTC_IPRB.reg_data = 0x0;
	INTC_IPRC.reg_data = 0x0;

	interrupt_vpend=0x00000000;	//rebuild & recalc
	interrupt_vmask=0xFFFFFFFF;	//no masking
	decoded_srimask=0;			//nothing is real, everything is allowed ...

	RequestSIIDRebuild();		//we have to rebuild the table.

	for (u32 i=0;i<28;i++)
		InterruptBit[i]=1<<i;
}

void intc_Term()
{

}