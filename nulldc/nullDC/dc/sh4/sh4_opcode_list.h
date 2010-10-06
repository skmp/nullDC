/*
	sh4 base core
	most of it is (very) old
	could use many cleanups, lets hope someone does them
*/

#pragma once
#include "types.h"
#include "sh4_interpreter.h"

class BasicBlock;
typedef void (__fastcall RecOpCallFP) (u32 op,u32 pc,BasicBlock* bb);

extern OpCallFP* OpPtr[0x10000];
extern RecOpCallFP* RecOpPtr[0x10000];
extern OpcodeType OpTyp[0x10000];

typedef void OpDissasmFP(char* out,const char* const FormatString,u32 pc,u16 opcode);

enum sh4_eu
{
	MT,
	EX,
	BR,
	LS,
	FE,
	CO,
	MA,
	sh4_eu_max
};

//exception fixup needed , added it to fix exception on opcodes that modify before exept :)
enum sh4_exept_fixup
{
	fix_none,
	rn_opt_1,	//1 if n!=m
	rn_opt_2,	//2 if n!=m
	rn_opt_4,	//4 if n!=m
	rn_4,		//always 4 from rn
	rn_fpu_4,	//4 or 8 , according to fpu size
};
struct sh4_opcodelistentry
{
	RecOpCallFP* rec_oph;
	OpCallFP* oph;
	u32 mask;
	u32 rez;
	OpcodeType type;
	OpDissasmFP* dissasm; 
	char disasm1[64];
	u8 IssueCycles;
	u8 LatencyCycles;
	sh4_eu unit;
	sh4_exept_fixup ex_fixup;
	void Dissasemble(char* strout,u32 pc , u16 params) const
	{
		dissasm(strout,&disasm1[0],pc,params);
	}

	INLINE bool SetPC() const
	{
		return (type & WritesPC)!=0;
	}

	INLINE bool NeedPC() const
	{
		return (type & ReadsPC)!=0;
	}

	INLINE bool SetSR() const
	{
		return (type & WritesSR)!=0;
	}

	INLINE bool SetFPSCR() const
	{
		return (type & WritesFPSCR)!=0;
	}
	
};

extern sh4_opcodelistentry* OpDesc[0x10000];
#define ExecuteOpcode(op) {OpPtr[op](op);}

void BuildOpcodeTables();
void DissasembleOpcode(u16 opcode,u32 pc,char* Dissasm);