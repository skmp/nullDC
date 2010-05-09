#pragma once
#include "recompiler.h"
#include "compiledblock.h"
#include "dc/sh4/shil/shil.h"

class BasicBlock: public CodeRegion
{
	private:
		void SetCompiledBlockInfo(CompiledBlockInfo* cBl);

	public :

	vector<u32> locked;
	BasicBlock(CodeRegion& cregion):CodeRegion(cregion)
	{
		flags.full=0;
		cBB=0;
		TF_next_addr=0xFFFFFFFF;
		TT_next_addr=0xFFFFFFFF;
	}

	BasicBlock()
	{
		start=0;
		end=0;
		flags.full=0;
		cycles=0;
		cBB=0;
		TF_next_addr=0xFFFFFFFF;
		TT_next_addr=0xFFFFFFFF;
	}

	bool IsMemLocked(u32 adr);

	
	//u32 start;
	//end pc
	//u32 end;

	//flags
	union
	{
		u32 full;
		struct 
		{
			//Exit type
			#define BLOCK_EXITTYPE_DYNAMIC			(0)		//link end
			#define BLOCK_EXITTYPE_FIXED			(1)		//call TF_next_addr
			#define BLOCK_EXITTYPE_COND				(2)		//T==0 -> TT , else TF
			#define BLOCK_EXITTYPE_FIXED_CALL		(3)		//Set rv to known for rts handling
			#define BLOCK_EXITTYPE_DYNAMIC_CALL		(4)		//Set rv to known for rts handling
			#define BLOCK_EXITTYPE_RET				(5)		//Ends w/ ret ;)
			#define BLOCK_EXITTYPE_FIXED_CSC		(6)		//Fixed , but we cant link due to cpu state change :)
			#define BLOCK_EXITTYPE_RES_2			(7)		//Reserved
			u32 ExitType:3;
			
			//Fpu mode type
			#define BLOCK_FPUMODE_32_S32	(0)	//32 bit math , 32 bit reads/writes
			#define BLOCK_FPUMODE_64_S32	(1)	//64 bit math , 32 bit reads/writes 
			#define BLOCK_FPUMODE_32_S64	(2)	//32 bit math , 64 bit read/writes
			#define BLOCK_FPUMODE_INVALID	(3)	//this mode is invalid
			u32 FpuMode:2;

			//Set if it contains any vector opcodes
			u32 FpuIsVector:1;

			//Protection Type flag
			#define BLOCK_PROTECTIONTYPE_LOCK	(0)	//block checks are done by locking memory (no extra code needed)
			#define BLOCK_PROTECTIONTYPE_MANUAL	(1)	//block checks if it's valid itself
			u32 ProtectionType:1;

			//Analyse flags :)
			//I realy need to find a better place for these
			#define BLOCK_SOM_NONE		(0)			//NONE
			#define BLOCK_SOM_SIZE_128	(1)			//DIV32U[Q|R]/DIV32S[Q|R]
			#define BLOCK_SOM_RESERVED1	(2)			//RESERVED
			#define BLOCK_SOM_RESERVED2	(3)			//RESERVED
			u32 SynthOpcode:2;

			u32 EndAnalyse:1;

			//Set if it has delayslot
			u32 HasDelaySlot:1;
		};
	}flags;	//compiled block flags :)

	//u32 cycles;

	shil_stream ilst;

	bool Compile();
	void CalculateLockFlags();

	CompiledBlockInfo* cBB;

	u32 TF_next_addr;//tfalse or jmp
	u32 TT_next_addr;//ttrue  or rts guess
};




void __fastcall basic_block_AddRef(CompiledBlockInfo* p_this,CompiledBlockInfo* block);
void __fastcall basic_block_BlockWasSuspended(CompiledBlockInfo* p_this,CompiledBlockInfo* block);
void __fastcall basic_block_ClearBlock(CompiledBlockInfo* p_this,CompiledBlockInfo* block);
void __fastcall basic_block_Suspend(CompiledBlockInfo* p_this);
void __fastcall basic_block_Free(CompiledBlockInfo* p_this);

struct ret_cache_entry
{
	u32 addr;
	CompiledBlockInfo* cBB;
};
extern ret_cache_entry* ret_cache_base;
void ret_cache_reset();


#define RET_CACHE_COUNT 32
#define RET_CACHE_ITEM_SIZE 8

#define RET_CACHE_SZ (RET_CACHE_COUNT*RET_CACHE_ITEM_SIZE)

CompiledBlockInfo*  __fastcall CompileBasicBlock(u32 pc);