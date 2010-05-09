#pragma once
#include "emitter/emitter.h"
#include "dc/sh4/rec_v1/BasicBlock.h"

#define RALLOC_R  1
#define RALLOC_W  2
#define RALLOC_RW (RALLOC_R|RALLOC_W)

#define RA_DEFAULT 0
#define RA_NODATA  1
#define RA_FORCE   2

class IntegerRegAllocator
{
	public :
#define T x86_reg
	//DoAllocation		: do allocation on the block
	virtual void DoAllocation(BasicBlock* bb,x86_block* x86e)=0;
	//BeforeEmit		: generate any code needed before the main emittion begins (other register allocators may have emited code tho)
	virtual void BeforeEmit()=0;
	//BeforeTrail		: generate any code needed after the main emittion has ended (other register allocators may emit code after that tho)
	virtual void BeforeTrail()=0;
	//AfterTrail		: generate code after the native block end (after the ret) , can be used to emit helper functions (other register allocators may emit code after that tho)
	virtual void AfterTrail()=0;
	//IsRegAllocated	: *couh* yea .. :P
	virtual bool IsRegAllocated(u32 sh4_reg)=0;
	//Carefull w/ register state , we may need to implement state push/pop
	//GetRegister		: Get the register , needs flag for mode
	virtual T GetRegister(T d_reg,u32 sh4_reg,u32 mode)=0;
	//Save registers
	virtual void SaveRegister(u32 reg,T from)=0;
	//FlushRegister		: write reg to reg location , and dealloc it
	virtual void FlushRegister(u32 sh4_reg)=0;
	//Flush all regs
	virtual void FlushRegCache()=0;
	//WriteBackRegister	: write reg to reg location
	virtual void WriteBackRegister(u32 sh4_reg)=0;
	//ReloadRegister	: read reg from reg location , discard old result
	virtual void ReloadRegister(u32 sh4_reg)=0;

	virtual void SaveRegister(u32 reg,u32 from)=0;
	virtual void SaveRegister(u32 to,u32* from)=0;
	virtual void SaveRegister(u32 to,s16* from)=0;
	virtual void SaveRegister(u32 to,s8* from)=0;
	virtual void MarkDirty(u32 reg)=0;
#undef T
};


class FloatRegAllocator
{
public :
#define T x86_sse_reg
	//DoAllocation		: do allocation on the block
	virtual void DoAllocation(BasicBlock* bb,x86_block* x86e)=0;
	//BeforeEmit		: generate any code needed before the main emittion begins (other register allocators may have emited code tho)
	virtual void BeforeEmit()=0;
	//BeforeTrail		: generate any code needed after the main emittion has ended (other register allocators may emit code after that tho)
	virtual void BeforeTrail()=0;
	//AfterTrail		: generate code after the native block end (after the ret) , can be used to emit helper functions (other register allocators may emit code after that tho)
	virtual void AfterTrail()=0;
	//IsRegAllocated	: *couh* yea .. :P
	virtual bool IsRegAllocated(u32 sh4_reg)=0;
	//Carefull w/ register state , we may need to implement state push/pop
	//GetRegister		: Get the register , needs flag for mode
	virtual T GetRegister(T d_reg,u32 sh4_reg,u32 mode)=0;
	//Save registers
	virtual void SaveRegister(u32 reg,T from)=0;
	//FlushRegister		: write reg to reg location , and dealloc it
	virtual void FlushRegister(u32 sh4_reg)=0;
	//Flush all regs
	virtual void FlushRegCache()=0;
	//WriteBackRegister	: write reg to reg location
	virtual void WriteBackRegister(u32 sh4_reg)=0;
	//ReloadRegister	: read reg from reg location , discard old result
	virtual void ReloadRegister(u32 sh4_reg)=0;
	virtual void SaveRegister(u32 to,float* from)=0;
	virtual void FlushRegister_xmm(x86_sse_reg reg)=0;
	virtual void SaveRegisterGPR(u32 to,x86_reg from)=0;
	virtual void LoadRegisterGPR(x86_reg to,u32 from)=0;
#undef T
};
u32* GetRegPtr(u32 reg);
u32* GetRegPtr(Sh4RegType reg);
FloatRegAllocator * GetFloatAllocator();
IntegerRegAllocator* GetGPRtAllocator();