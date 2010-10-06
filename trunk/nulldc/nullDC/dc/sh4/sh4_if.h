#pragma once
#include "types.h"
#include "intc_types.h"

//sh4 exeption saved stack pointer :)
extern u32* sh4_exept_ssp;
//sh4 next opcode execute
extern u32* sh4_exept_next;
//sh4 exeption raised bool , set to true when an exeption was raised , and its not handled
extern bool sh4_exept_raised;

enum Sh4RegType
{
	//GPR , bank 0
	r0=0,
	r1=1,
	r2=2,
	r3=3,
	r4=4,
	r5=5,
	r6=6,
	r7=7,
	r8=8,
	r9=9,
	r10=10,
	r11=11,
	r12=12,
	r13=13,
	r14=14,
	r15=15,

	//GPR , bank 1
	r0_Bank=16,
	r1_Bank=17,
	r2_Bank=18,
	r3_Bank=19,
	r4_Bank=20,
	r5_Bank=21,
	r6_Bank=22,
	r7_Bank=23,

	//Misc regs
	reg_gbr=24,
	reg_ssr=25,
	reg_spc=26,
	reg_sgr=27,
	reg_dbr=28,
	reg_vbr=29,
	reg_mach=30,
	reg_macl=31,
	reg_pr=32,
	reg_fpul=33,
    reg_pc=34,
	reg_sr=35,
	reg_fpscr=36,

	//FPU gpr , bank 0
	fr_0=37,
	fr_1=fr_0+1,
	fr_2=fr_0+2,
	fr_3=fr_0+3,
	fr_4=fr_0+4,
	fr_5=fr_0+5,
	fr_6=fr_0+6,
	fr_7=fr_0+7,
	fr_8=fr_0+8,
	fr_9=fr_0+9,
	fr_10=fr_0+10,
	fr_11=fr_0+11,
	fr_12=fr_0+12,
	fr_13=fr_0+13,
	fr_14=fr_0+14,
	fr_15=fr_0+15,

	//FPU gpr , bank 1
	xf_0=fr_15+1,
	xf_1=xf_0+1,
	xf_2=xf_0+2,
	xf_3=xf_0+3,
	xf_4=xf_0+4,
	xf_5=xf_0+5,
	xf_6=xf_0+6,
	xf_7=xf_0+7,
	xf_8=xf_0+8,
	xf_9=xf_0+9,
	xf_10=xf_0+10,
	xf_11=xf_0+11,
	xf_12=xf_0+12,
	xf_13=xf_0+13,
	xf_14=xf_0+14,
	xf_15=xf_0+15,

	//special regs , used _only_ on rec
	reg_ftrv=xf_15+1,
	reg_xmtrx=reg_ftrv+1,
	//sr_T=xmtrx+1,
	//sr_Q=sr_T+1,
	//sr_S=sr_Q+1,
	//sr_M=sr_S+1,
	
	dr_0=reg_xmtrx+1,
	dr_7=dr_0+7,

	xd_0=dr_7+1,
	xd_7=xd_0+7,
	reg_pc_temp=xd_7+1,
	reg_sr_T=reg_pc_temp+1,

	sh4_reg_count,

	NoReg=-1
};
extern u8 GetSingleFromDouble(u8 dbl);

struct StatusReg
{//
	union
	{
		struct
		{
			u32 T_h		:1;//<<0
			u32 S		:1;//<<1
			u32 rsvd0	:2;//<<2
			u32 IMASK	:4;//<<4
			u32 Q		:1;//<<8
			u32 M		:1;//<<9
			u32 rsvd1	:5;//<<10
			u32 FD		:1;//<<15
			u32 rsvd2	:12;//<<16
			u32 BL		:1;//<<28
			u32 RB		:1;//<<29
			u32 MD		:1;//<<20
			u32 rsvd3	:1;//<<31
		};
		u32 m_full;
	};
	u32 T;
	INLINE u32 GetFull()
	{
		return (m_full&0x700083F2) | T;
	}

	INLINE void SetFull(u32 value)
	{
		m_full=value & 0x700083F2;
		T=value&1;
	}

};
struct fpscr_type
{
	union
	{
		u32 full;
		struct
		{
			u32 RM:2;
			u32 finexact:1;
			u32 funderflow:1;
			u32 foverflow:1;
			u32 fdivbyzero:1;
			u32 finvalidop:1;
			u32 einexact:1;
			u32 eunderflow:1;
			u32 eoverflow:1;
			u32 edivbyzero:1;
			u32 einvalidop:1;
			u32 cinexact:1;
			u32 cunderflow:1;
			u32 coverflow:1;
			u32 cdivbyzero:1;
			u32 cinvalid:1;
			u32 cfpuerr:1;
			u32 DN:1;
			u32 PR:1;
			u32 SZ:1;
			u32 FR:1;
		};
		struct
		{
			u32 nil:2+1+1+1+1+4+8+1;
			u32 PR_SZ:2;
		};
	};
};


typedef void RunFP();
typedef void StopFP();
typedef void StepFP();
typedef void SkipFP();
typedef void ResetFP(bool Manual);
typedef void InitFP();
typedef void TermFP();
typedef bool IsCpuRunningFP();
typedef u32 GetRegisterFP(Sh4RegType reg);
typedef void SetRegisterFP(Sh4RegType reg,u32 value);
typedef cResetEvent* GetArmResetEventFP();
typedef void __fastcall sh4_int_RaiseExeptionFP(u32 ExeptionCode,u32 VectorAddress);



//sh4 interface
struct sh4_if
{
	RunFP* Run;
	StopFP* Stop;
	StepFP* Step;
	SkipFP* Skip;
	ResetFP* Reset;
	InitFP* Init;
	TermFP* Term;

	TermFP* ResetCache;

	IsCpuRunningFP* IsCpuRunning;
	GetRegisterFP* GetRegister;
	SetRegisterFP* SetRegister;
	//Sh4RaiseInterruptFP* RaiseInterrupt;
	sh4_int_RaiseExeptionFP* RaiseExeption;
};

//Get an interface to sh4 interpreter
sh4_if* Get_Sh4Interpreter();
//Get an interface to sh4 dynarec
sh4_if* Get_Sh4Recompiler();
//free it
void Release_Sh4If(sh4_if* cpu);

void DissasembleOpcode(u16 opcode,u32 pc,char* Dissasm);

#define BPT_OPCODE		0x8A00

bool IsReg64(Sh4RegType reg);