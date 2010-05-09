#pragma once
#include "types.h"
#include "recompiler.h"
#include "compiledblock.h"

struct pginfo
{
	u32 invalidates;
	union
	{
		struct
		{
			u32 ManualCheck:1;	//bit 0 :1-> manual check , 0 -> locked check
			u32 reserved:31;	//bit 1-31: reserved
		};
		u32 full;
	} flags;
};

extern u32* block_stack_pointer;

#define PAGE_MANUALCHECK 1
pginfo GetPageInfo(u32 address);

#define FindBlock FindBlock_fast
#define FindCode FindCode_fast

CompiledBlockInfo* __fastcall FindBlock_fast(u32 address);
BasicBlockEP* __fastcall FindCode_fast(u32 address);

void RegisterBlock(CompiledBlockInfo* block);
void UnRegisterBlock(CompiledBlockInfo* block);
CompiledBlockInfo* bm_ReverseLookup(void* code_ptr);

void FreeSuspendedBlocks();

CompiledBlockInfo* FindOrRecompileBlock(u32 pc);
void __fastcall SuspendBlock(CompiledBlockInfo* block);
void __fastcall SuspendAllBlocks();
void __fastcall _SuspendAllBlocks();
CompiledBlockInfo* __fastcall FindBlock_full_compile(u32 address,CompiledBlockInfo* fastblock);

void InitBlockManager();
void ResetBlockManager();
void TermBlockManager();

extern u8* DynarecCache;
extern u32 DynarecCacheSize;

void* dyna_malloc(u32 size);
void* dyna_realloc(void*ptr,u32 oldsize,u32 newsize);
void* dyna_finalize(void* ptr,u32 oldsize,u32 newsize);
void dyna_profiler_tick(void* addr);

struct bm_stats
{
	u32 block_count;
	u32 block_size;
	u32 cache_size;
	u64 CompileTimeMilisecs;

	u32 locked_blocks;
	u32 locked_block_calls_delta;

	u32 manual_blocks;
	u32 manual_block_calls_delta;

	u32 full_lookups;
	u32 fast_lookups;
};

void bm_GetStats(bm_stats* stats);

//#define _BM_CACHE_STATS
#define LOOKUP_HASH_SIZE	0x4000
#define LOOKUP_HASH_MASK	(LOOKUP_HASH_SIZE-1)
extern CompiledBlockInfo*			BlockLookupGuess[LOOKUP_HASH_SIZE];

//#define COUNT_BLOCK_LOCKTYPE_USAGE
extern u32 manbs;
extern u32 lockbs;
