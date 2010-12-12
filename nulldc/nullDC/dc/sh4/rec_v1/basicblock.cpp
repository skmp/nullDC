#include "basicblock.h"
#include "dc/sh4/shil/Compiler/shil_compiler_base.h"
#include "dc/mem/sh4_mem.h"
#include "dc/sh4/sh4_registers.h"
#include "emitter/emitter.h"
#include "emitter/regalloc/x86_sseregalloc.h"
#include <memory.h>
#include "recompiler.h"
#include "dc/sh4/sh4_interpreter.h"
#include "dc/sh4/rec_v1/blockmanager.h"
#include "recompiler.h"
#include "analyser.h"
#include "dc/sh4/shil/shil_ce.h"

void FASTCALL RewriteBasicBlock(CompiledBlockInfo* cBB);

const x86_opcode_class JumpCC[] =
{
	op_jo ,//r/m8 = 0F 90 /0
	op_jno ,//r/m8 = 0F 91 /0

	op_jb ,//r/m8 = 0F 92 /0
	op_jnb ,//r/m8 = 0F 93 /0

	op_je ,//r/m8 = 0F 94 /0
	op_jne ,//r/m8 = 0F 95 /0

	op_jbe ,//r/m8 = 0F 96 /0
	op_jnbe ,//r/m8 = 0F 97 /0

	op_js ,//r/m8 = 0F 98 /0
	op_jns ,//r/m8 = 0F 99 /0

	op_jp ,//r/m8 = 0F 9A /0
	op_jnp ,//r/m8 = 0F 9B /0

	op_jl ,//r/m8 = 0F 9C /0
	op_jnl ,//r/m8 = 0F 9D /0

	op_jle ,//r/m8 = 0F 9E /0
	op_jnle ,//r/m8 = 0F 9F /0
};
//needed declarations
void bb_link_compile_inject_TF_stub(CompiledBlockInfo* ptr);
void bb_link_compile_inject_TT_stub(CompiledBlockInfo* ptr);
void RewriteBasicBlockCond(CompiledBlockInfo* cBB);


//Basic Block
bool BasicBlock::IsMemLocked(u32 adr)
{
	if (IsOnRam(adr)==false)
		return false;

	if (flags.ProtectionType==BLOCK_PROTECTIONTYPE_MANUAL)
		return false;

	//if block isnt on ram , then there's no way to tell if its locked (well , bios mem is allways locked :p)
	if (OnRam()==false)
		return false;

	verify(page_start()<=page_end());

	u32 adrP=GetRamPageFromAddress(adr);

	return (page_start() <=adrP) && (adrP<=page_end());
}

void BasicBlock::SetCompiledBlockInfo(CompiledBlockInfo* cBl)
{
	cBB= cBl;
	
	cBB->block_type.ProtectionType=flags.ProtectionType;

	cBB->start=start;
	cBB->end=end;
	cBB->cpu_mode_tag=flags.FpuMode;
	cBB->lookups=0;
	cBB->Discarded=false;
	cBB->tbp_ticks=0;

	cBB->TF_next_addr=TF_next_addr;
	cBB->TT_next_addr=TT_next_addr;

	cBB->TF_block=cBB->TT_block=0;
}
//BasicBlock compiler :D

void RewriteBasicBlockFixed(CompiledBlockInfo* cBB)
{
	verify(cBB->Rewrite.Type==2);
	u8 flags=0;
	if  (cBB->TF_block)
		flags=1;

	if (cBB->Rewrite.Last==flags)
		return;

	x86_block* x86e = new x86_block();

	x86e->Init(dyna_realloc,dyna_finalize);
	x86e->do_realloc=false;
	x86e->x86_buff=(u8*)cBB->Code + cBB->Rewrite.Offset;
	x86e->x86_size=32;

	cBB->Rewrite.Last=flags;

	if  (cBB->TF_block)
	{
		x86e->Emit(op_jmp,x86_ptr_imm(cBB->TF_block->Code));
	}
	else
	{
		x86e->Emit(op_mov32,ECX,(u32)cBB);					//mov ecx , block
		x86e->Emit(op_jmp,x86_ptr_imm(bb_link_compile_inject_TF_stub));
	}
	x86e->Generate();

	delete x86e;
}
void RewriteBasicBlockCond(CompiledBlockInfo* cBB)
{
	verify(cBB->Rewrite.Type==1);
	if (cBB->Rewrite.Type==2)
	{
		RewriteBasicBlockFixed(cBB);
		return;
	}

	u8 flags=0;
	if (cBB->TT_block!=0)
		flags|=1;
	if (cBB->TF_block)
		flags|=2;

	if (cBB->Rewrite.Last==flags)
		return;

	x86_block* x86e = new x86_block();
	
	x86e->Init(dyna_realloc,dyna_finalize);
	x86e->do_realloc=false;
	x86e->x86_buff=(u8*)cBB->Code + cBB->Rewrite.Offset;
	x86e->x86_size=32;

	cBB->Rewrite.Last=flags;
	x86_opcode_class jump_op=JumpCC[cBB->Rewrite.RCFlags];
	x86_opcode_class jump_op_n=JumpCC[cBB->Rewrite.RCFlags^1];

	if (flags==1)
	{
		x86e->Emit(jump_op,x86_ptr_imm(cBB->TT_block->Code));//jne
		x86e->Emit(op_mov32,ECX,(u32)cBB);					//mov ecx , block
		x86e->Emit(op_jmp,x86_ptr_imm(bb_link_compile_inject_TF_stub));
	}
	else if  (flags==2)
	{
		x86e->Emit(jump_op_n,x86_ptr_imm(cBB->TF_block->Code));//je
		x86e->Emit(op_mov32,ECX,(u32)cBB);					//mov ecx , block
		x86e->Emit(op_jmp,x86_ptr_imm(bb_link_compile_inject_TT_stub));
	}
	else  if  (flags==3)
	{
		x86e->Emit(jump_op_n,x86_ptr_imm(cBB->TF_block->Code));//je
		x86e->Emit(op_jmp,x86_ptr_imm(cBB->TT_block->Code));
	}
	else
	{
		//x86e->Emit(op_int3);
		x86e->Emit(op_mov32,ECX,(u32)cBB);					//mov ecx , block
		x86e->Emit(jump_op_n,x86_ptr_imm(bb_link_compile_inject_TF_stub));//je
		x86e->Emit(op_jmp,x86_ptr_imm(bb_link_compile_inject_TT_stub));
	}
	x86e->Generate();

	delete x86e;
}

//Compile block and return pointer to it's code
void* __fastcall bb_link_compile_inject_TF(CompiledBlockInfo* ptr)
{
	CompiledBlockInfo* target= FindOrRecompileBlock(ptr->TF_next_addr);

	//if current block is Discared , we must not add any chain info , just jump to the new one :)
	if (ptr->Discarded==false)
	{
		//Add reference so we can undo the chain later
		target->AddRef(ptr);
		ptr->TF_block=target;
		ptr->pTF_next_addr=target->Code;
		if (ptr->Rewrite.Type)
			RewriteBasicBlock(ptr);
	}
	return target->Code;
}

void* __fastcall bb_link_compile_inject_TT(CompiledBlockInfo* ptr)
{
	CompiledBlockInfo* target= FindOrRecompileBlock(ptr->TT_next_addr);

	//if current block is Discared , we must not add any chain info , just jump to the new one :)
	if (ptr->Discarded==false)
	{
		//Add reference so we can undo the chain later
		target->AddRef(ptr);
		ptr->TT_block=target;
		ptr->pTT_next_addr=target->Code;
		if (ptr->Rewrite.Type)
			RewriteBasicBlock(ptr);
	}
	return target->Code;
} 

//call link_compile_inject_TF , and jump to code
void naked bb_link_compile_inject_TF_stub(CompiledBlockInfo* ptr)
{
	__asm
	{
		call bb_link_compile_inject_TF;
		jmp eax;
	}
}

void naked bb_link_compile_inject_TT_stub(CompiledBlockInfo* ptr)
{
	__asm
	{
		call bb_link_compile_inject_TT;
		jmp eax;
	}
}

u32 ret_cache_hits=0;
u32 ret_cache_total=0;

#define RET_CACHE_PTR_MASK_AND (0xFFFFFFFF - (RET_CACHE_SZ)  )
#define RET_CACHE_PTR_MASK_OR ( RET_CACHE_SZ*2 )
#define RET_CACHE_STACK_OFFSET_A (RET_CACHE_SZ)
#define RET_CACHE_STACK_OFFSET_B (RET_CACHE_SZ+4)
/*__declspec(align(512)) //must be 16* size
struct 
{
	u32 waste[RET_CACHE_SIZE];//force top bit to 1
	CompiledBlockInfo* ptr[RET_CACHE_SIZE];//force top bit to 1,store ptrs here
	u32 data[RET_CACHE_SIZE*2];//entrys for addr (the *2 is to make sure align is right =P)
}ret_cache;

u32* call_ret_cache_ptr=ret_cache.data;
*/

ret_cache_entry* ret_cache_base;
//new idea : build the BRT on the stack
//
//
//......
//ESP+132 CBB ptr
//ESP+128 first address
//ESP+124 ..esp can be on these values , this space is 'NOT USED' , called functions can (And will) use it
//...
//ESP+0

CompiledBlockInfo* Curr_block;

//sp is 0 if manual discard
void CBBs_BlockSuspended(CompiledBlockInfo* block,u32* sp)
{/*
	u32* sp_inblock=block_stack_pointer-1;

	if(sp_inblock==sp)
	{
		//log("Exeption within the same block !\n");
	}
	else
	{
		if (sp!=0)
		{
			//log("Exeption possibly within the same block ; 0x%X\n",sp_inblock[-1]);
			//log("Block EP : 0x%X , sz : 0x%X\n",block->Code,block->size);
		}
	}
	*/
	if (ret_cache_base==0)
		return;
	for (int i=0;i<RET_CACHE_COUNT;i++)
	{
		if (ret_cache_base[i].cBB == block)
		{
			ret_cache_base[i].addr=0xFFFFFFFF;
			ret_cache_base[i].cBB=0;
		}
	}
}
void ret_cache_reset()
{
	if (ret_cache_base==0)
		return;
	for (int i=0;i<RET_CACHE_COUNT;i++)
	{
		ret_cache_base[i].addr=0xFFFFFFFF;
		ret_cache_base[i].cBB=0;
	}
}
void __fastcall CheckBlock(CompiledBlockInfo* block)
{
	verify(block->cpu_mode_tag==fpscr.PR_SZ);
	//verify(block->size==pc);
	verify(block->Discarded==false);
}


void FASTCALL RewriteBasicBlockGuess_FLUT(CompiledBlockInfo* cBB)
{
	verify(cBB->Rewrite.Type==3);
	//indirect call , rewrite & link , second time(does fast look up)
	x86_block* x86e = new x86_block();

	cBB->Rewrite.RCFlags=2;
	x86e->Init(dyna_realloc,dyna_finalize);
	x86e->do_realloc=false;
	x86e->x86_buff=(u8*)cBB->Code + cBB->Rewrite.Offset;
	x86e->x86_size=64;

	
	x86e->Emit(op_jmp,x86_ptr_imm(Dynarec_Mainloop_no_update));
	
	x86e->Generate();
	delete x86e;
}
//can corrupt anything apart esp
void naked RewriteBasicBlockGuess_FLUT_stub(CompiledBlockInfo* ptr)
{
	__asm
	{
		/*mov esi,ecx;//store block ptr
		mov edi,eax;//store pc
		call RewriteBasicBlockGuess_FLUT;
		mov eax,edi;//store pc

		jmp Resolve_FLUT;
		*/
		call RewriteBasicBlockGuess_FLUT;
		jmp [Dynarec_Mainloop_no_update];
	}
}
void* FASTCALL RewriteBasicBlockGuess_TTG(CompiledBlockInfo* cBB)
{
	verify(cBB->Rewrite.Type==3);
	//indirect call , rewrite & link , first time (hardlinks to target)
	CompiledBlockInfo*	new_block=FindOrRecompileBlock(pc);

	if (cBB->Discarded)
	{
		return new_block->Code;
	}
	cBB->Rewrite.RCFlags=1;
	//Add reference so we can undo the chain later
	new_block->AddRef(cBB);
	cBB->TF_block=new_block;

	x86_block* x86e = new x86_block();

	x86e->Init(dyna_realloc,dyna_finalize);
	x86e->do_realloc=false;
	x86e->x86_buff=(u8*)cBB->Code + cBB->Rewrite.Offset;
	x86e->x86_size=64;

	
	cBB->TF_block=new_block;
	x86e->Emit(op_cmp32,EAX,pc);
	x86e->Emit(op_mov32,ECX,(u32)cBB);
	x86e->Emit(op_jne,x86_ptr_imm(RewriteBasicBlockGuess_FLUT_stub));
	x86e->Emit(op_jmp,x86_ptr_imm(new_block->Code));

	x86e->Generate();
	delete x86e;

	return new_block->Code;
}
void naked RewriteBasicBlockGuess_TTG_stub(CompiledBlockInfo* ptr)
{
	__asm
	{
		call RewriteBasicBlockGuess_TTG;
		jmp eax;
	}
}
//default behavior , calls _TTG rewrite
void FASTCALL RewriteBasicBlockGuess_NULL(CompiledBlockInfo* cBB)
{
	verify(cBB->Rewrite.Type==3);
	cBB->Rewrite.RCFlags=0;
	x86_block* x86e = new x86_block();

	x86e->Init(dyna_realloc,dyna_finalize);
	x86e->do_realloc=false;
	x86e->x86_buff=(u8*)cBB->Code + cBB->Rewrite.Offset;
	x86e->x86_size=32;
	x86e->Emit(op_mov32,ECX,(u32)cBB);
	x86e->Emit(op_jmp,x86_ptr_imm(RewriteBasicBlockGuess_TTG_stub));
	x86e->Generate();
	delete x86e;
}
void FASTCALL RewriteBasicBlock(CompiledBlockInfo* cBB)
{
	if (cBB->Rewrite.Type==1)
		RewriteBasicBlockCond(cBB);
	else if (cBB->Rewrite.Type==2)
		RewriteBasicBlockFixed(cBB);
	else if (cBB->Rewrite.Type==3)
	{
		if (cBB->Rewrite.RCFlags==0)
			RewriteBasicBlockGuess_NULL(cBB);
		else if (cBB->Rewrite.RCFlags==1)
			RewriteBasicBlockGuess_TTG(cBB);
		else if (cBB->Rewrite.RCFlags==2)
			RewriteBasicBlockGuess_FLUT(cBB);
	}
}
#ifdef RET_CACHE_PROF
void naked ret_cache_misscall()
{
	__asm jmp [Dynarec_Mainloop_no_update];
}
#endif
void ret_cache_push(CompiledBlockInfo* cBB,x86_block* x86e)
{
	//x86e->Emit(op_int3);

	x86e->Emit(op_add32,ESP,8);//add the ptr ;)
	x86e->Emit(op_and32,ESP,RET_CACHE_PTR_MASK_AND);

	//Adress
	x86e->Emit(op_mov32,x86_mrm(ESP,x86_ptr::create(RET_CACHE_STACK_OFFSET_A)),cBB->TT_next_addr);
	//Block
	x86e->Emit(op_mov32,x86_mrm(ESP,x86_ptr::create(RET_CACHE_STACK_OFFSET_B)),(u32)(cBB));
}
bool BasicBlock::Compile()
{
	FloatRegAllocator*		fra;
	IntegerRegAllocator*	ira;

	x86_block* x86e=new x86_block();
	
	x86e->Init(dyna_realloc,dyna_finalize);

	cBB=new CompiledBlockInfo();

	SetCompiledBlockInfo(cBB);

	/*
	//that is a realy nice debug helper :)
	x86e->Emit(op_mov32,&Curr_block,(u32)cBB);
	*/

	x86_Label* block_exit = x86e->CreateLabel(false,0);

	/*
	x86e->Emit(op_mov32,ECX,(u32)cBB);
	x86e->Emit(op_call,x86_ptr_imm(CheckBlock));
	*/

	x86e->Emit(op_sub32 ,&rec_cycles,cycles);
	x86e->Emit(op_js,block_exit);

	if (flags.ProtectionType==BLOCK_PROTECTIONTYPE_MANUAL)
	{
#ifdef COUNT_BLOCK_LOCKTYPE_USAGE
		x86e->Emit(op_add32,&manbs,1);
#endif
		int sz=Size();
		verify(sz!=0);

		int i=0;
		//that can be optimised a lota :p
		
		x86_Label* exit_discard_block= x86e->CreateLabel(false,0);
		x86_Label* execute_block= x86e->CreateLabel(false,8);
		verify(sz!=0);
		while(sz>=4)
		{
			u32* pmem=(u32*)GetMemPtr(start+i,4);
			x86e->Emit(op_cmp32 ,pmem,*pmem);
			
			if (sz==4)
			{
				x86e->Emit(op_je ,execute_block);
			}
			else
			{
				x86e->Emit(op_jne ,exit_discard_block);
			}
			i+=4;
			sz-=4;
		}
		if (sz>=2)
		{
			//die("lol");
			u16* pmem=(u16*)GetMemPtr(start+i,2);
			x86e->Emit(op_cmp16 ,pmem,*pmem);
			
			x86e->Emit(op_je ,execute_block);

			i+=2;
			sz-=2;
		}
		verify(sz==0);

		x86e->MarkLabel(exit_discard_block);
		x86e->Emit(op_mov32,ECX,(u32)cBB);
		x86e->Emit(op_mov32,GetRegPtr(reg_pc),start);
		x86e->Emit(op_call,x86_ptr_imm(SuspendBlock));
		x86e->Emit(op_jmp,x86_ptr_imm(Dynarec_Mainloop_no_update));
		x86e->MarkLabel(execute_block);
	}
#ifdef COUNT_BLOCK_LOCKTYPE_USAGE
	else
		x86e->Emit(op_add32,&lockbs,1);
#endif

	fra=GetFloatAllocator();
	ira=GetGPRtAllocator();
	
	ira->DoAllocation(this,x86e);
	fra->DoAllocation(this,x86e);

	ira->BeforeEmit();
	fra->BeforeEmit();

	
	shil_compiler_init(x86e,ira,fra);

	u32 list_sz=(u32)ilst.opcodes.size();
	
	u32 exit_cond_direct=16;//16 -> not possible
	for (u32 i=0;i<list_sz;i++)
	{
		shil_opcode* op=&ilst.opcodes[i];

		if ((BLOCK_EXITTYPE_COND==flags.ExitType) && i>0 && (list_sz>1) && (op[0].opcode == shilop_LoadT) && (op[0].imm1==128))	//if flag will be preserved, and we are on a LoadT jcond
		{
			//
			//folowing opcodes better be mov only 
			if (
				 //(i==(list_sz-1)) &&//&& //if current opcode is LoadT/jcond [shoul allways be...] -> checks above since its not the last opcode allways [opt passes move some stuff downhere, const wbs ..]
				(op[-1].opcode == shilop_SaveT)							  //and previous was a load to T [== the cmp was right before bt/no bts)
				)
			{
				for (u32 j=i+1;j<list_sz;j++)
				{
					//log("%d: %d\n",j,op[j-i].opcode);
					if (op[j-i].opcode!=shilop_mov)
						goto compile_normaly;
				}
				//log("Flag promotion @ %d out of %d\n",i,(list_sz-1));
				exit_cond_direct=op[-1].imm1;
				if (exit_cond_direct==CC_FPU_E)
					exit_cond_direct=CC_NP;

				//skip the LoadT jcond, work on opcodes after
				continue;
			}
		}
compile_normaly:
		shil_compile(op);
	}

	ira->BeforeTrail();
	fra->BeforeTrail();

	//end block acording to block type :)
	cBB->Rewrite.Type=0;
	cBB->Rewrite.RCFlags=0;
	cBB->Rewrite.Last=0xFF;
	cBB->block_type.exit_type=flags.ExitType;

	switch(flags.ExitType)
	{
	case BLOCK_EXITTYPE_DYNAMIC_CALL:	//same as below , sets call guess
		{
			ret_cache_push(cBB,x86e);
		}
	case BLOCK_EXITTYPE_DYNAMIC:		//not guess 
		{
			cBB->Rewrite.Type=3;
			cBB->Rewrite.RCFlags=0;
			cBB->Rewrite.Offset=x86e->x86_indx;
			x86e->Emit(op_mov32,ECX,(u32)cBB);
			x86e->Emit(op_jmp,x86_ptr_imm(RewriteBasicBlockGuess_TTG_stub));
			u32 extrasz=26-(x86e->x86_indx-cBB->Rewrite.Offset);
			for (u32 i=0;i<extrasz;i++)
				x86e->write8(0xCC);
		}
		break;
	case BLOCK_EXITTYPE_RET:			//guess
		{
#ifdef RET_CACHE_PROF
			x86e->Emit(op_add32,x86_ptr(&ret_cache_total),1);
#endif
			//cmp pr,guess
			//call_ret_cache_ptr
			//x86e->Emit(op_int3);
			
			//x86e->Emit(op_mov32 ,EAX,GetRegPtr(reg_pc));
			x86e->Emit(op_cmp32 ,EAX,x86_mrm(ESP,x86_ptr::create(RET_CACHE_STACK_OFFSET_A)));
			//je ok
#ifndef RET_CACHE_PROF
			x86e->Emit(op_jne ,x86_ptr_imm(Dynarec_Mainloop_no_update));
#else
			x86e->Emit(op_jne ,x86_ptr_imm(ret_cache_misscall));
#endif
			//ok:
			
			//x86e->Emit(op_int3);
			//Get the block ptr
			x86e->Emit(op_mov32 ,ECX,x86_mrm(ESP,x86_ptr::create(RET_CACHE_STACK_OFFSET_B)));
			x86e->Emit(op_sub32 ,ESP,8);//decrease the ptr ;)
			x86e->Emit(op_and32,ESP,RET_CACHE_PTR_MASK_AND);
			x86e->Emit(op_or32,ESP,RET_CACHE_PTR_MASK_OR);
#ifdef RET_CACHE_PROF
			x86e->Emit(op_add32,x86_ptr(&ret_cache_hits),1);
#endif
			
			//mov eax,[pcall_ret_address+codeoffset]
			x86e->Emit(op_jmp32,x86_mrm(ECX,x86_ptr::create(offsetof(CompiledBlockInfo,pTT_next_addr))));
		}
		break;
	case BLOCK_EXITTYPE_COND:			//linkable
		{
			//ok , handle COND here :)
			//mem address
			u32 TT_a=cBB->TT_next_addr;
			u32 TF_a=cBB->TF_next_addr;
			
			if (exit_cond_direct==16)
			{
				x86e->Emit(op_cmp32,&T_jcond_value,1);
				exit_cond_direct=CC_E;
			}

			if (TT_a==cBB->start)
			{
				cBB->TT_block=cBB;
			}
			else
			{
				cBB->TT_block=FindBlock(TT_a);
				if (cBB->TT_block)
					cBB->TT_block->AddRef(cBB);
			}

			if  (TF_a==cBB->start)
			{
				cBB->TF_block=cBB;
			}
			else
			{
				cBB->TF_block=FindBlock(TF_a);
				if (cBB->TF_block)
					cBB->TF_block->AddRef(cBB);
			}

			//we invert the test, jne->je etc
			//due to historical reasons, thats how the COND exit type works ;p
			exit_cond_direct^=1;

			cBB->Rewrite.Type=1;
			cBB->Rewrite.RCFlags=exit_cond_direct;
			cBB->Rewrite.Offset=x86e->x86_indx;
			
			
			x86e->Emit(op_mov32,ECX,(u32)cBB);					//mov ecx , block
			x86e->Emit(JumpCC[exit_cond_direct],x86_ptr_imm(0));//jne
			x86e->Emit(op_jmp,x86_ptr_imm(0));
			x86e->Emit(op_int3);
		} 
		break;
	case BLOCK_EXITTYPE_FIXED_CALL:		//same as below
		{
			ret_cache_push(cBB,x86e);
		}
	case BLOCK_EXITTYPE_FIXED:			//linkable
		{
			if (cBB->TF_next_addr==cBB->start)
			{
				log("Fast Link possible\n");
			}
			else
			{
				cBB->TF_block=FindBlock(cBB->TF_next_addr);
				if (cBB->TF_block)
					cBB->TF_block->AddRef(cBB);
			}

			cBB->Rewrite.Type=2;
			cBB->Rewrite.Offset=x86e->x86_indx;
			//link to next block :
			x86e->Emit(op_mov32,ECX,(u32)cBB);					//mov ecx , cBB
			x86e->Emit(op_jmp32,x86_ptr((u32*)&(cBB->pTF_next_addr)));	//mov eax , [pTF_next_addr]
		}
		break;
	case BLOCK_EXITTYPE_FIXED_CSC:		//forced lookup , possible state chainge
		{
			//x86e->Emit(op_int3);
			//We have to exit , as we gota do mode lookup :)
			//We also have to reset return cache to ensure its ok -> removed for now

			//call_ret_address=0xFFFFFFFF;
			//x86e->Emit(op_mov32 ,EBX,&call_ret_cache_ptr);
			//x86e->Emit(op_mov32,x86_mrm(EBX),0xFFFFFFFF);

			//pcall_ret_address=0;
			//Good , now return to caller :)
			x86e->Emit(op_jmp,x86_ptr_imm(Dynarec_Mainloop_no_update));
		}
		break;
	}

	ira->AfterTrail();
	fra->AfterTrail();

	x86e->MarkLabel(block_exit);
	x86e->Emit(op_add32 ,&rec_cycles,cycles);
	x86e->Emit(op_mov32 ,GetRegPtr(reg_pc),start);
	x86e->Emit(op_jmp,x86_ptr_imm(Dynarec_Mainloop_do_update));

	x86e->Emit(op_int3);

	//apply roml patches and generate needed code
	apply_roml_patches();

	void* codeptr=x86e->Generate();//NOTE, codeptr can be 0 here </NOTE>
	
	cBB->Code=(BasicBlockEP*)codeptr;
	cBB->size=x86e->x86_indx;

	//make it call the stubs , unless otherwise needed
	cBB->pTF_next_addr=bb_link_compile_inject_TF_stub;
	cBB->pTT_next_addr=bb_link_compile_inject_TT_stub;

	cBB->x86_code_fixups=x86e->GetExterns();

	delete fra;
	delete ira;
	x86e->Free();
	delete x86e;

	if (codeptr==0)
		return false; // didnt manage to generate code
	//rewrite needs valid codeptr
	if (cBB->Rewrite.Type)
		RewriteBasicBlock(cBB);

	return true;
}

//
void BasicBlock::CalculateLockFlags()
{
	u32 addr=start;

	while(addr<end)
	{
		flags.ProtectionType |= GetPageInfo(addr).flags.ManualCheck;
		addr+=PAGE_SIZE;
	}
	//check the last one , it is possible to skip it on the above loop :)
	flags.ProtectionType |= GetPageInfo(end).flags.ManualCheck;
}
extern u32 CompiledSRCsz;

CompiledBlockInfo*  __fastcall CompileBasicBlock(u32 pc)
{

	CompiledBlockInfo* rv;
	BasicBlock* block=new BasicBlock();

	//scan code
	ScanCode(pc,block);
	//Fill block lock type info
	block->CalculateLockFlags();
	CompiledSRCsz+=block->Size();
	//analyse code [generate il/block type]
	AnalyseCode(block);
	//optimise
	shil_optimise_pass_ce_driver(block);
	//Compile code
	if (block->Compile())
		rv=block->cBB;
	else
		rv=0;

	delete block;
	
	return rv;
}


void FASTCALL RewriteBasicBlock(CompiledBlockInfo* cBB);
//CompiledBlockInfo Helper functions
void CompiledBlockInfo::ClearBlock(CompiledBlockInfo* block)
{
	if (TF_block==block)
	{
		TF_block=0;
		pTF_next_addr=bb_link_compile_inject_TF_stub;
		if (block_type.exit_type==BLOCK_EXITTYPE_DYNAMIC ||
			block_type.exit_type==BLOCK_EXITTYPE_DYNAMIC_CALL)
		{
			TF_next_addr=0xFFFFFFFF;
			Rewrite.RCFlags=0;
		}
		if (Rewrite.Type)
			RewriteBasicBlock(this);
	}

	if (TT_block==block)
	{
		TT_block=0;
		pTT_next_addr=bb_link_compile_inject_TT_stub;
		if (Rewrite.Type)
			RewriteBasicBlock(this);
	}
}

void CompiledBlockInfo::BlockWasSuspended(CompiledBlockInfo* block)
{
	for (u32 i=0;i<blocks_to_clear.size();i++)
	{
		if (blocks_to_clear[i]==block)
		{
			blocks_to_clear[i]=0;
		}
	}
}

void CompiledBlockInfo::AddRef(CompiledBlockInfo* block)
{
	verify(Discarded==false);
	//if we reference ourselfs we dont care ;) were suspended anyhow
	if (block !=this)
		blocks_to_clear.push_back(block);
}
void CompiledBlockInfo::Suspend()
{
	for (u32 i=0;i<blocks_to_clear.size();i++)
	{
		if (blocks_to_clear[i])
		{
			blocks_to_clear[i]->ClearBlock(this);
		}
	}
	blocks_to_clear.clear();

	if (TF_block)
		TF_block->BlockWasSuspended(this);

	if (TT_block)
		TT_block->BlockWasSuspended(this);

	//if we jump to another block , we have to re compile it :)
	Discarded=true;
}

void CompiledBlockInfo::Free()
{
		Code=0;	
		((x86_block_externs*)x86_code_fixups)->Free();
}