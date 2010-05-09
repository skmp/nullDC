#pragma once
#include "types.h"
#include "codespan.h"
#include "recompiler.h"


//YAY , compiled block fun ;)

//Generic block info
//All Compiled Block structs contain this first
struct CompiledBlockInfo:CodeSpan
{
public :
	BasicBlockEP* Code;				//compiled code ptr
	
	//needed for lookups
	u32 cpu_mode_tag;
	u32 lookups;	//count of lookups for this block

	//block type
	union
	{
		struct
		{
			u32 exit_type:8;
			u32 ProtectionType:1;
		};
		u32 full;
	} block_type;

	//needed for free()/debug info
	u32 size;			//compiled code size (bytes)

	//can be avoided
	bool Discarded;

	void* x86_code_fixups;
	u32 tbp_ticks;

	//Called to Free :p yeshrly
	void Free();
	//Called when this block is suspended
	void Suspend();
	//Called when a block we reference is suspended
	void BlockWasSuspended(CompiledBlockInfo* block);
	//Called when a block adds reference to this
	void AddRef(CompiledBlockInfo* block);
	//remote pthis reference to block *warning* it was the oposite before
	void ClearBlock(CompiledBlockInfo* block);


	//Addresses to blocks
	u32 TF_next_addr;//tfalse or jmp or jmp guess
	u32 TT_next_addr;//ttrue  or rts guess

	//pointers to blocks
	CompiledBlockInfo* TF_block;
	CompiledBlockInfo* TT_block;

	//pointers to block entry points [isnt that the same as above ?-> not anymore]
	void* pTF_next_addr;//tfalse or jmp or jmp guess
	void* pTT_next_addr;//ttrue  or rts guess

	struct
	{
		u32 Offset;		//bytes after block start for rewrite
		u8 Type:2;		//Type (0-> none, 1 -> COND, 2-> Fixed, 3 -> wtf
		u8 RCFlags:6;	//rewrite code flags (values: private to handler, default = 0)
		u8 Last;		//last rewrite parameters (values: private to handler,default 0xFF)
	}Rewrite;
	//Block link info
	vector<CompiledBlockInfo*> blocks_to_clear;
};