#include "dc\sh4\sh4_registers.h"
#include "dc\mem\sh4_mem.h"
#include "blockmanager.h"

#include "windows.h"
#include "emitter/emitter.h"

//block manager : new implementation 
//ideas :
//use native locks for code overwrites
//fix the never ending crash problems
//make code simpler , so future optimisation of code lookups is possible

//Using mmu pages to lock areas on mem is the best way to keep track of writes , fully done by h/w.
//the problem w/ sh4 is that its usual to have data close to code (same page), witch makes it had..
//i decided to use a mixed model :
//
//Pages are locked , and if a page is invalidated too much (like 10 times)
//the dynarec will switch to manual invalidation for blocks that are on
//that page.Code that modifies its next bytes wont work , its not
//possible to exit from a block due to an invalidation.
//
//When in fallback mode , the block's code should check if the block is still valid , propably some cheap
//trick , like comparing the first 4 bytes of the block (cmp m32,i32) or rep cmp.
//
//The blocks are allways on the page block list no matter their mode.
//hybrid blocks (eg , they are half on a lockable page and half on a non lockable one) are considered 
//non lockable.they can be eiter manualy invalidated , either from lock code :).

//There is a global list of blocks , mailny used by the profiler and the shutdown functions.
//
//There is a block list per page , that contains all the blocks that are within that page (that counts
// those "partialy in" too).This list is used to discard blocks in case of a page write.
//
//Now , an array of lists is used to keep the block start addresses.It is indexed by a hash made from the
//address (to keep the list size small so fast lookups).Also there is a cache list that holds a pointer
//to the most commonly used block for that hash range.

//Profile resuts :
// Cache hits : 99.5% (ingame/menus)
// Average list size : 3.4 blocks
//

void FASTCALL RewriteBasicBlock(CompiledBlockInfo* cBB);

#include <vector>
#include <algorithm>

#define BLOCK_LUT_GUESS
//#define DEBUG_BLOCKLIST
//#define OPTIMISE_LUT_SORT

#define BLOCK_NONE (&BLOCK_NONE_B)
CompiledBlockInfo BLOCK_NONE_B;

u32* block_stack_pointer;

u32 full_lookups;
u32 fast_lookups;

void ret_cache_reset();

//helper list class
int compare_BlockLookups(const void * a, const void * b)
{
	CompiledBlockInfo* ba=*(CompiledBlockInfo**)a;
	CompiledBlockInfo* bb=*(CompiledBlockInfo**)b;

	return bb->lookups-ba->lookups;
}


class BlockList:public vector<CompiledBlockInfo*>
{
public :
	size_t ItemCount;
	BlockList():vector<CompiledBlockInfo*>()
	{
		ItemCount=0;
	}
#ifdef DEBUG_BLOCKLIST
	void Test()
	{
	#ifdef DEBUG_BLOCKLIST
		verify(ItemCount>=0);
		verify(ItemCount<=size());

		for (size_t i=0;i<ItemCount;i++)
					verify(_Myfirst[i]!=BLOCK_NONE && _Myfirst[i]->Discarded==false);
		for (size_t i=ItemCount;i<size();i++)
					verify(_Myfirst[i]==BLOCK_NONE);
	#endif
	}
#else
	#define Test()
#endif
	u32 Add(CompiledBlockInfo* block)
	{
		Test();
		if (ItemCount==size())
		{	
			ItemCount++;
			push_back(block);
			Test();
			return (u32)ItemCount-1;
		}
		else
		{
			#ifdef DEBUG_BLOCKLIST
			verify(_Myfirst[ItemCount]==BLOCK_NONE);
			#endif
			_Myfirst[ItemCount]=block;
			ItemCount++;
			Test();
			return ItemCount-1;
		}
	}
	void Remove(CompiledBlockInfo* block)
	{
		Test();
		if (ItemCount==0)
			return;
		
		for (u32 i=0;i<ItemCount;i++)
		{
			if (_Myfirst[i]==block)
			{
				ItemCount--;
				if (ItemCount==0)
				{
					#ifdef DEBUG_BLOCKLIST
					verify(_Myfirst[0]==block && i==0);
					#endif
					_Myfirst[i]=BLOCK_NONE;
					CheckEmpty();
				}
				else if (ItemCount!=i)
				{
					#ifdef DEBUG_BLOCKLIST
					verify(_Myfirst[ItemCount]!=BLOCK_NONE);
					verify(ItemCount<size());
					verify(i<ItemCount);
					#endif
					_Myfirst[i]=_Myfirst[ItemCount];
					_Myfirst[ItemCount]=BLOCK_NONE;
				}
				else
				{
					#ifdef DEBUG_BLOCKLIST
					verify(ItemCount<size());
					verify(i==ItemCount);
					#endif
					_Myfirst[i]=BLOCK_NONE;
				}

				Test();
				return;
			}
		}
		Test();
		die("Invalid path");
	}
	CompiledBlockInfo* Find(u32 address,u32 cpu_mode)
	{
		Test();
		for (u32 i=0;i<ItemCount;i++)
		{
			if ((_Myfirst[i]->start == address) &&
				(_Myfirst[i]->cpu_mode_tag == cpu_mode)
				)
			{
				return _Myfirst[i];
			}
		}
		return 0;
	}
	
	//check if the list is empty (full of BLOCK_NONE's) , if so , clear it
	void CheckEmpty()
	{
		Test();
		if (ItemCount!=0)
			return;
		#ifdef DEBUG_BLOCKLIST
		u32 sz=size();
		for (u32 i=0;i<sz;i++)
		{

			if (_Myfirst[i]!=BLOCK_NONE)
			{
				log("BlockList::CheckEmptyList fatal error , ItemCount!=RealItemCount\n");
				__debugbreak(); 
				return;
			}

		}
		#endif
		clear();
	}

	//optimise blocklist for best lookup times
	void Optimise()
	{
		if (ItemCount < 2)
			return;

		if (size())
		{
			//using a specialised routine is gona be faster .. bah
			qsort(_Myfirst, ItemCount, sizeof(CompiledBlockInfo*), compare_BlockLookups);
			//sort(begin(), end());
			/*u32 max=_Myfirst[0]->lookups/100;
			//if (max==0)
			max++;
			for (u32 i=0;i<size();i++)
				_Myfirst[i]->lookups/=max;*/
		}
	}

	void clear()
	{
		vector<CompiledBlockInfo*>::clear();
		ItemCount=0;
	}
};
//page info
pginfo PageInfo[RAM_SIZE/PAGE_SIZE];

//block managment
BlockList all_block_list;

//block discard related vars
BlockList BlockPageLists[RAM_SIZE/PAGE_SIZE];
BlockList SuspendedBlocks;

//block lookup vars
#define GetLookupHash(addr) ((addr>>2)&LOOKUP_HASH_MASK)
BlockList					BlockLookupLists[LOOKUP_HASH_SIZE];

#ifdef BLOCK_LUT_GUESS
//even after optimising blocks , guesses give a good speedup :)
CompiledBlockInfo*			BlockLookupGuess[LOOKUP_HASH_SIZE];
#endif

u32 bm_locked_block_count=0;
u32 bm_manual_block_count=0;

u8* DynarecCache;
u32 DynarecCacheUsed;
u32 DynarecCacheSize;

//implemented later
void FreeBlock(CompiledBlockInfo* block);
void init_memalloc(u32 size);
void reset_memalloc();

//misc code & helper functions
//this should not be called from a running block , or it could crash
//Free a list of blocks
//will also clear the list
void FreeBlocks(BlockList* blocks)
{
	for (u32 i=0;i<blocks->ItemCount;i++)
	{
		if ((*blocks)[i]!=BLOCK_NONE)
		{
			FreeBlock((*blocks)[i]);
		}
	}
	blocks->clear();
}
//-1 -> a<b , 0 -> a==b, 1 -> a>b
int BlockRelocateSort(const void* p1,const void* p2)
{
	const CompiledBlockInfo* a=*(const CompiledBlockInfo**)p1;
	const CompiledBlockInfo* b=*(const CompiledBlockInfo**)p2;

	if (a==BLOCK_NONE)
		return 1;
	if (b==BLOCK_NONE)
		return -1;
	
	if (a->start>b->start)
	{
		return 1;
	}
	else if (a->start<b->start)
	{
		return -1;
	}
	else
	{
		return a->cpu_mode_tag-b->cpu_mode_tag;
	}
}

void RelocateBlocks()
{
	if (all_block_list.ItemCount==0)
		return;

	u8* temp=(u8*)malloc(DynarecCacheUsed);
	memcpy(temp,DynarecCache,DynarecCacheUsed);

	qsort(&all_block_list[0],all_block_list.ItemCount,sizeof(CompiledBlockInfo*),BlockRelocateSort);

	u8* dst_start=DynarecCache;
	for(size_t i=0;i<all_block_list.ItemCount;i++)
	{
		u8* src=((u8*)all_block_list[i]->Code)-DynarecCache+temp;
		if (dst_start!=(u8*)all_block_list[i]->Code)
		{
			memcpy(dst_start,src,all_block_list[i]->size);
			all_block_list[i]->Code=(BasicBlockEP*)dst_start;
			((x86_block_externs*)all_block_list[i]->x86_code_fixups)->Apply(dst_start);
		}
		dst_start+=all_block_list[i]->size;
	}
	free(temp);

	for(size_t i=0;i<all_block_list.ItemCount;i++)
	{
		all_block_list[i]->Rewrite.Last=0xFF;
		if (all_block_list[i]->TF_block)
			all_block_list[i]->pTF_next_addr=all_block_list[i]->TF_block->Code;
		if (all_block_list[i]->TT_block)
			all_block_list[i]->pTT_next_addr=all_block_list[i]->TT_block->Code;
		RewriteBasicBlock(all_block_list[i]);
	}
	u32 oldsz=DynarecCacheUsed;
	DynarecCacheUsed=dst_start-DynarecCache;

	log("Relocated dynarec blocks: %d KB before, %d KB new, %d bytes diff, %.2f%%\n",oldsz/1024,DynarecCacheUsed/1024,oldsz-DynarecCacheUsed,(oldsz-DynarecCacheUsed)*100/(float)DynarecCacheUsed);
}


//this should not be called from a running block , or it will crash
//Fully resets block hash/list , clears all entrys and free's any prevusly allocated blocks
extern LARGE_INTEGER total_compile;
extern u32 CompiledSRCsz;
void ResetBlocks(bool free_too=true)
{
	//this should clear all lists
	while(all_block_list.ItemCount!=0)
	{
		verify(all_block_list[0]!=BLOCK_NONE);
		SuspendBlock(all_block_list[0]);
	}
	all_block_list.CheckEmpty();
	verify(all_block_list.ItemCount==0);

	for (u32 i=0;i<LOOKUP_HASH_SIZE;i++)
	{
		verify(BlockLookupLists[i].ItemCount==0);
		BlockLookupLists[i].CheckEmpty();
		#ifdef BLOCK_LUT_GUESS
		verify(BlockLookupGuess[i] ==0 || BlockLookupGuess[i]==BLOCK_NONE);
		BlockLookupGuess[i]=BLOCK_NONE;
		#endif
	}

	for (u32 i=0;i<(RAM_SIZE/PAGE_SIZE);i++)
	{
		verify(BlockPageLists[i].ItemCount==0);
		BlockPageLists[i].CheckEmpty();
	}
	if (free_too)
		FreeBlocks(&SuspendedBlocks);
	//FreeBlocks(&all_block_list); << THIS BETTER BE EMPTY WHEN WE GET HERE
	verify(all_block_list.ItemCount==0);
	
	//Reset misc data structures (all blocks are removed , so we just need to initialise em to default !)
	reset_memalloc();
	memset(PageInfo,0,sizeof(PageInfo));
	
	bm_locked_block_count=0;
	bm_manual_block_count=0;
	total_compile.QuadPart=0;
	CompiledSRCsz=0;
}
u32 manbs,lockbs;

void bm_GetStats(bm_stats* stats)
{
	stats->block_count=all_block_list.ItemCount;

	stats->cache_size=DynarecCacheUsed;
	stats->block_size=CompiledSRCsz;
	LARGE_INTEGER Freq;
	QueryPerformanceFrequency(&Freq);
	stats->CompileTimeMilisecs=total_compile.QuadPart*1000/(Freq.QuadPart/1000);

	stats->manual_blocks=bm_manual_block_count;
	stats->manual_block_calls_delta=manbs;manbs=0;
	stats->locked_blocks=bm_locked_block_count;
	stats->locked_block_calls_delta=lockbs;lockbs=0;

	stats->fast_lookups=fast_lookups;fast_lookups=0;
	stats->full_lookups=full_lookups;full_lookups=0;

}
bool reset_cache=false;
void __fastcall _SuspendAllBlocks();
//this should not be called from a running block , or it cloud crash
//free's suspended blocks
void FreeSuspendedBlocks()
{
	static int BBBB=0;
	BBBB++;
	if (BBBB>996666)
	{
		BBBB=0;
		RelocateBlocks();
	}
	if (reset_cache)
		_SuspendAllBlocks();
	FreeBlocks(&SuspendedBlocks);
}

//block lookup code
INLINE BlockList* GetLookupBlockList(u32 address)
{
	address&=RAM_MASK;
	return &BlockLookupLists[GetLookupHash(address)];
}



u32 luk=0;
u32 r_value=0x112;

void CompileAndRunCode();
CompiledBlockInfo* __fastcall FindBlock_full(u32 address,CompiledBlockInfo* fastblock);
//Block lookups
CompiledBlockInfo* __fastcall FindBlock_fast(u32 address)
{
#ifdef BLOCK_LUT_GUESS
	CompiledBlockInfo* fastblock;

	fastblock=BlockLookupGuess[GetLookupHash(address)];

	if ((fastblock->start==address) && 
		(fastblock->cpu_mode_tag ==fpscr.PR_SZ)
		)
	{
		fastblock->lookups++;
		return fastblock;
	}
	else
	{
		return FindBlock_full(address,fastblock);
	}
#else
	return FindBlock_full(address,BLOCK_NONE);
#endif

}
CompiledBlockInfo*  __fastcall CompileCode(u32 pc);
CompiledBlockInfo* __fastcall FindBlock_full_compile(u32 address,CompiledBlockInfo* fastblock)
{
	CompiledBlockInfo* thisblock;

	BlockList* blklist = GetLookupBlockList(address);

	//u32 listsz=(u32)blklist->size();
	//for (u32 i=0;i<listsz;i++)
	//{ 
	//	thisblock=(*blklist)[i];
	//	if (thisblock->start==address && thisblock->cpu_mode_tag==fpscr.PR_SZ)
	//	{
	//		thisblock->lookups++;
	//		return thisblock;
	//	}
	//}

#ifdef OPTIMISE_LUT_SORT
	luk++;
	if (luk==r_value)
	{
		luk=0;
		r_value=(frand() & 0x1FF) + 0x800;
		blklist->Optimise();
	}
#endif
	thisblock=blklist->Find(address,fpscr.PR_SZ);
	if (thisblock==0)
		return CompileCode(pc);

	thisblock->lookups++;
#ifdef BLOCK_LUT_GUESS
	if (fastblock->lookups<=thisblock->lookups)
	{
		BlockLookupGuess[GetLookupHash(address)]=thisblock;
	}
#endif
	return thisblock;
}
CompiledBlockInfo* __fastcall FindBlock_full(u32 address,CompiledBlockInfo* fastblock)
{
	CompiledBlockInfo* thisblock;

	BlockList* blklist = GetLookupBlockList(address);

	//u32 listsz=(u32)blklist->size();
	//for (u32 i=0;i<listsz;i++)
	//{ 
	//	thisblock=(*blklist)[i];
	//	if (thisblock->start==address && thisblock->cpu_mode_tag==fpscr.PR_SZ)
	//	{
	//		thisblock->lookups++;
	//		return thisblock;
	//	}
	//}

#ifdef OPTIMISE_LUT_SORT
	luk++;
	if (luk==r_value)
	{
		luk=0;
		r_value=(frand() & 0x1FF) + 0x800;
		blklist->Optimise();
	}
#endif
	thisblock=blklist->Find(address,fpscr.PR_SZ);
	if (thisblock==0)
		return 0;

	thisblock->lookups++;
#ifdef BLOCK_LUT_GUESS
	if (fastblock->lookups<=thisblock->lookups)
	{
		BlockLookupGuess[GetLookupHash(address)]=thisblock;
	}
#endif
	return thisblock;
}

BasicBlockEP* __fastcall FindCode_full(u32 address,CompiledBlockInfo* fastblock);
//Code lookups
BasicBlockEP* __fastcall FindCode_fast(u32 address)
{
#ifdef BLOCK_LUT_GUESS
	CompiledBlockInfo* fastblock;

	fastblock=BlockLookupGuess[GetLookupHash(address)];

	if ((fastblock->start==address) && 
		(fastblock->cpu_mode_tag ==fpscr.PR_SZ)
		)
	{
		fastblock->lookups++;
		return fastblock->Code;
	}
	else
	{
		return FindCode_full(address,fastblock);
	}
#else
	return FindCode_full(address,BLOCK_NONE);
#endif

}
BasicBlockEP* __fastcall FindCode_full(u32 address,CompiledBlockInfo* fastblock)
{
	CompiledBlockInfo* thisblock;

	BlockList* blklist = GetLookupBlockList(address);
#ifdef _BM_CACHE_STATS
	full_lookups++;
#endif
#ifdef OPTIMISE_LUT_SORT
	luk++;
	if (luk==r_value)
	{
		luk=0;
		r_value=(frand() & 0x1FF) + 0x800;
		blklist->Optimise();
	}
#endif
	thisblock=blklist->Find(address,fpscr.PR_SZ);
	if (thisblock==0)
		return CompileAndRunCode;

	thisblock->lookups++;
#ifdef BLOCK_LUT_GUESS
	if (fastblock->lookups<=thisblock->lookups)
	{
		BlockLookupGuess[GetLookupHash(address)]=thisblock;
	}
#endif
	return thisblock->Code;
}


CompiledBlockInfo* bm_ReverseLookup(void* code_ptr)
{
	u8* c=(u8*)code_ptr;

	
	for(size_t i=0;i<all_block_list.ItemCount;i++)
	{
		CompiledBlockInfo* b=all_block_list[i];

		if ( (u32)(c-(u8*)b->Code) < b->size)
		{
			return b;
		}
	}

	//Also look on suspended blocks, code might still run from there and use bm_RL for a block
	//that is to be discarded
	//Happens on The Grinch, it suspends a block and then a non-fastpath access is done !
	for(size_t i=0;i<SuspendedBlocks.ItemCount;i++)
	{
		CompiledBlockInfo* b=SuspendedBlocks[i];

		if ( (u32)(c-(u8*)b->Code) < b->size)
		{
			return b;
		}
	}
	
	return 0;
}

pginfo GetPageInfo(u32 address)
{
	if (IsOnRam(address))
	{
		return PageInfo[GetRamPageFromAddress(address)];
	}
	else
	{
		pginfo rv;
		rv.flags.full=0;
		rv.flags.ManualCheck=0;
		rv.invalidates=0;
		return rv;
	}
}
/*
void FillBlockLockInfo(BasicBlock* block)
{
	if (block->OnRam())
	{
		u32 start=block->page_start();
		u32 end=block->page_end();

		bool ManualCheck=false;
		for (u32 i=start;i<=end;i++)
		{
			ManualCheck|=PageInfo[i].flags.ManualCheck;
		}

		if (ManualCheck)
			block->flags.ProtectionType=BLOCK_PROTECTIONTYPE_MANUAL;
		else
			block->flags.ProtectionType=BLOCK_PROTECTIONTYPE_LOCK;
	}
	else//On rom , LOCK protection , never gets invalidated :)
	{
		block->flags.ProtectionType=BLOCK_PROTECTIONTYPE_LOCK;
	}
}*/

void RegisterBlock(CompiledBlockInfo* block)
{
	all_block_list.Add(block);

	u32 start=(block->start&RAM_MASK);
	u32 end=(block->end&RAM_MASK);

	if (0U != start) {//The compiler turns this into a shift in high opt levels but play safe while in debug mode;)
		start /= PAGE_SIZE;
	}

	if (0U != end) {//just in case
		end /= PAGE_SIZE;
	}

	//AddToBlockList(GetLookupBlockList(block->start),block);
	GetLookupBlockList(block->start)->Add(block);
	
	if (block->block_type.ProtectionType)
		bm_manual_block_count++;
	else
		bm_locked_block_count++;

	if (((block->start >>26)&0x7)==3)
	{	//Care about invalidates olny if on ram
		for (u32 i=start;i<=end;i++)
		{
			if (PageInfo[i].flags.ManualCheck==0)
			{	//if manual checks , we must not lock
				mem_b.LockRegion(i*PAGE_SIZE,PAGE_SIZE);
				BlockPageLists[i].Add(block);
			}
		}
	}
}

void UnRegisterBlock(CompiledBlockInfo* block)
{
	u32 start=(block->start&RAM_MASK);
	u32 end=(block->end&RAM_MASK);

	if (0U != start) { //The compiler turns this into a shift in high opt levels but play safe while in debug mode;)
		start /= PAGE_SIZE;
	}

	if (0U != end) {//just in case
		end /= PAGE_SIZE;
	}

	GetLookupBlockList(block->start)->Remove(block);

	all_block_list.Remove(block);

	#ifdef BLOCK_LUT_GUESS
	if (BlockLookupGuess[GetLookupHash(block->start)]==block)
		BlockLookupGuess[GetLookupHash(block->start)]=BLOCK_NONE;
	#endif

	if (block->block_type.ProtectionType)
		bm_manual_block_count--;
	else
		bm_locked_block_count--;

	if (block->OnRam())
	{	//Care about invalidates olny if on ram
		for (u32 i=start;i<=end;i++)
		{
			if (PageInfo[i].flags.ManualCheck==0)
			{	//if manual checks , we don't need to unlock (its not locked to start with :p)
				BlockPageLists[i].Remove(block);
				BlockPageLists[i].CheckEmpty();
				if (BlockPageLists[i].ItemCount==0)
				{
					mem_b.UnLockRegion(i*PAGE_SIZE,PAGE_SIZE);
				}
			}
		}
	}
}


//suspend/ free related ;)
//called to suspend a block
//can be called from a mem invalidation
void CBBs_BlockSuspended(CompiledBlockInfo* block,u32* sp);
void __fastcall SuspendBlock_exept(CompiledBlockInfo* block,u32* sp)
{
	//remove the block from :
	//
	//not full block list , its removed from here olny when deleted (thats why its "full block" list ..)
	//page block list
	//look up block list
	//Look up guess block list
	//unregisting a block does exactly all that :)

	verify(block!=BLOCK_NONE);
	UnRegisterBlock(block);
	block->Suspend();
	CBBs_BlockSuspended(block,sp);

	//
	//add it to the "to be suspended" list
	SuspendedBlocks.Add(block);
}
void __fastcall SuspendAllBlocks()
{
	reset_cache=true;
}
void __fastcall _SuspendAllBlocks()
{
	log("Reseting Dynarec Cache...\n");
	ResetBlocks(false);
	reset_cache=false;
}
//called from a manualy invalidated block
void __fastcall SuspendBlock(CompiledBlockInfo* block)
{
	//remove the block from :
	//
	//not full block list , its removed from here olny when deleted (thats why its "full block" list ..)
	//page block list
	//look up block list
	//Look up guess block list
	//unregisting a block does exactly all that :)

	verify(block!=BLOCK_NONE);
	UnRegisterBlock(block);
	block->Suspend();
	CBBs_BlockSuspended(block,0);

	//
	//add it to the "to be suspended" list
	SuspendedBlocks.Add(block);
}
//called to free a suspended block
void FreeBlock(CompiledBlockInfo* block)
{
	//free the block
	//all_block_list.Remove(block);
	verify(block!=BLOCK_NONE);
	block->Free();
	delete block;
}

bool RamLockedWrite(u8* address,u32* sp)
{
	size_t offset=address-mem_b.data;

	if (offset<RAM_SIZE)
	{
		size_t addr_hash = (0U!=offset) ? offset/PAGE_SIZE : 0;
		BlockList* list=&BlockPageLists[addr_hash];
		if (list->ItemCount==0)
			return false;
		PageInfo[addr_hash].invalidates++;
					
		//for (size_t i=0;i<list->ItemCount;i++)
		//{
		//	if ((*list)[i])
		//	{
		while(list->ItemCount)
			SuspendBlock_exept((*list)[list->ItemCount-1],sp);
		//	}
		//}
		//list->clear();
		mem_b.UnLockRegion((u32)offset&(~(PAGE_SIZE-1)),PAGE_SIZE);

		if (PageInfo[addr_hash].invalidates>=1)
			PageInfo[addr_hash].flags.ManualCheck=1;

		return true;
	}
	else
		return false;
}

void InitBlockManager()
{
	BLOCK_NONE->start=0xFFFFFFFF;
	BLOCK_NONE->cpu_mode_tag=0xFFFFFFFF;
	BLOCK_NONE->lookups=0;
	init_memalloc(32*1024*1024);
}
void ResetBlockManager()
{
	ResetBlocks();
}
void TermBlockManager()
{
	ResetBlocks();
}

void DumpBlockMappings()
{
	FILE* out=fopen("C:\\blk.txt","wb");
	log("nullDC block manager stats : tracing %d blocks\n",all_block_list.ItemCount);

	for (u32 i=0;i<all_block_list.ItemCount;i++)
	{
		fprintf(out,"block :0x%08X\t[%d]\n{\n",all_block_list[i]->start,i);
		{
			fprintf(out,"\tCode         : 0x%p\n",all_block_list[i]->Code);
			fprintf(out,"\tLookup count : %d\n",all_block_list[i]->lookups);
			fprintf(out,"\tSh4 size     : %d\n",all_block_list[i]->Size());
			fprintf(out,"\tx86 size     : %d\n",all_block_list[i]->size);
			
			if (all_block_list[i]->TF_next_addr!=0xFFFFFFFF)
				fprintf(out,"\tLink 1 : 0x%08X\t[%d]\n",all_block_list[i]->TF_next_addr,i);

			if (all_block_list[i]->TT_next_addr!=0xFFFFFFFF)
				fprintf(out,"\tLink 2 : 0x%08X\t[%d]\n",all_block_list[i]->TT_next_addr,i);

			fprintf(out,"}\n");
		}
	}
	fclose(out);
}

//Memory allocator

void init_memalloc(u32 size)
{
	log("Dynarec cache : size is %.2fMB\n",size/1024.f/1024.f);

	DynarecCacheSize=size;
	DynarecCacheUsed=0;

	DynarecCache = (u8*)VirtualAlloc(0,DynarecCacheSize,MEM_COMMIT | MEM_RESERVE,PAGE_EXECUTE_READWRITE);
	verify(DynarecCache!=0);

	
}
void reset_memalloc()
{
	DynarecCacheUsed=0;
	memset(DynarecCache,0x90909090,DynarecCacheSize);
}
u8 dyna_tempbuffer[1024*1024];
void* dyna_malloc(u32 size)
{
	verify(size<sizeof(dyna_tempbuffer));
	return &dyna_tempbuffer;
}
void* dyna_realloc(void*ptr,u32 oldsize,u32 newsize)
{
	if (ptr==0)
		return dyna_malloc(newsize);
	verify(newsize<sizeof(dyna_tempbuffer));
	return ptr;
}
void* dyna_finalize(void* ptr,u32 oldsize,u32 newsize)
{
	if ((DynarecCacheUsed+newsize)>DynarecCacheSize)
		return 0;
	
	void* rv=&DynarecCache[DynarecCacheUsed];
	DynarecCacheUsed+=newsize;
	memcpy(rv,dyna_tempbuffer,newsize);
	return rv;
}
/*
	TBP
*/
BlockList tbp_tick_blocks;
u32 dyna_ticz=0;

int compare_tbp_ticks (const void * a, const void * b)
{
	CompiledBlockInfo* cba=*(CompiledBlockInfo**)a;
	CompiledBlockInfo* cbb=*(CompiledBlockInfo**)b;
	return cba->tbp_ticks-cbb->tbp_ticks;
}
void dyna_profiler_printf()
{
	log("%d blocks in list\n",tbp_tick_blocks.size());
	qsort(&(tbp_tick_blocks[0]), tbp_tick_blocks.ItemCount, sizeof(CompiledBlockInfo*), compare_tbp_ticks);

	u32 tticks=0;
	for (u32 i=0;i<tbp_tick_blocks.size();i++)
	{
		log("%d%%[%d] :0x%X : 0x%X %d-%d\n"
			,tbp_tick_blocks[i]->tbp_ticks*100/dyna_ticz
			,tbp_tick_blocks[i]->tbp_ticks
			,tbp_tick_blocks[i]->start
			,tbp_tick_blocks[i]->cpu_mode_tag
			,tbp_tick_blocks[i]->OpcodeCount()
			,tbp_tick_blocks[i]->size);
		tticks+=tbp_tick_blocks[i]->tbp_ticks;
		tbp_tick_blocks[i]->tbp_ticks=0;
		//if (i>10)
		//	break;
	}
	log("%d%% total\n",tticks*100/dyna_ticz);
	tbp_tick_blocks.clear();
	dyna_ticz=0;
}

void dyna_profiler_tick(void* addr)
{
	if (dyna_ticz++>200)
		dyna_profiler_printf();
	u32 nca=(u32)addr;
	for (u32 i=0;i<all_block_list.size();i++)
	{
		CompiledBlockInfo* cb=all_block_list[i];
		u32 baddr=(u32)cb->Code;

		if ((nca>=baddr) && ((nca-baddr)<cb->size))
		{
			if (tbp_tick_blocks.Find(cb->start,cb->cpu_mode_tag)==0)
				tbp_tick_blocks.Add(cb);
			
			cb->tbp_ticks++;
			return;
		}
	}
	__debugbreak(); 
	log("0x%X OMG! UNABLE TO MATCH BLOCK TEH NOES\n",addr);
}
