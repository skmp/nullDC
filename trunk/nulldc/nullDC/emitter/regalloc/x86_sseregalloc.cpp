/*
	This is actually not used. Never was
*/
#include "x86_sseregalloc.h"
#include <assert.h>

#define REG_ALLOC_COUNT (0)
//xmm0 is reserved for math/temp
x86_sse_reg reg_to_alloc_xmm[7]=
{
	XMM7,
	XMM6,
	XMM5,
	XMM4,
	XMM3,
	XMM2,
	XMM1,
};
u32 alb=0;
u32 nalb=0;

struct fprinfo
{
	x86_sse_reg reg;
	bool Loaded;
	bool WritenBack;
};


class SimpleSSERegAlloc:public FloatRegAllocator
{
	struct sort_temp
	{
		int cnt;
		int reg;
		bool no_load;
	};

	//ebx, ebp, esi, and edi are preserved

	//Yay bubble sort
	void bubble_sort(sort_temp numbers[] , int array_size)
	{
		int i, j;
		sort_temp temp;
		for (i = (array_size - 1); i >= 0; i--)
		{
			for (j = 1; j <= i; j++)
			{
				if (numbers[j-1].cnt < numbers[j].cnt)
				{
					temp = numbers[j-1];
					numbers[j-1] = numbers[j];
					numbers[j] = temp;
				}
			}
		}
	}



	x86_block* x86e;
	fprinfo reginf[16];
	

	fprinfo* GetInfo(u32 reg)
	{
		reg-=fr_0;
		if (reg<16)
		{
			if (reginf[reg].reg!=ERROR_REG)
			{
				return &reginf[reg];
			}
		}
		return 0;
	}
	void ensure_valid(u32 reg)
	{
		if (reg>=fr_0 && reg<=fr_15)
			__noop;
		else
			__debugbreak(); 
	}
	bool DoAlloc;
	
	//methods needed
	//
	//DoAllocation		: do allocation on the block
	virtual void DoAllocation(BasicBlock* block,x86_block* x86e)
	{
		this->x86e=x86e;
		DoAlloc= (block->flags.FpuMode==0);
		
		sort_temp used[16];
		for (int i=0;i<16;i++)
		{
			used[i].cnt=0;
			used[i].reg=r0+i;
			used[i].no_load=false;
			reginf[i].reg=ERROR_REG;
			reginf[i].Loaded=false;
			reginf[i].WritenBack=false;
		}

		if (DoAlloc)
		{
			u32 op_count=block->ilst.op_count;
			shil_opcode* curop;
			
			for (u32 j=0;j<op_count;j++)
			{
				curop=&block->ilst.opcodes[j];
				for (int i = 0;i<16;i++)
				{
					Sh4RegType reg = (Sh4RegType) (fr_0+i);

					if ((curop->WritesReg(reg)==true) && (curop->ReadsReg(reg)==false) && (used[i].cnt==0) )
					{
						used[i].no_load=true;
					}
					//both reads and writes , give it one more ;P
					if ( curop->UpdatesReg(reg) )
						used[i].cnt+=12;	//3+mem rw latency (9)
					else if (curop->ReadsReg(reg))
						used[i].cnt+=6;		//3 + mem latecny  (3)
					else if (curop->WritesReg(reg))
						used[i].cnt+=9;		//3 + mem w latency (6)
				}
			}

			bubble_sort(used,16);
			u32 i;
			for (i=0;i<REG_ALLOC_COUNT;i++)
			{
				//reg alloc minimum cost is 1 read + 1 write = 6+9=15, so <14
				if (used[i].cnt<14)
					break;
				reginf[used[i].reg].reg=reg_to_alloc_xmm[i];
				if (used[i].no_load)
				{
					reginf[used[i].reg].Loaded = true;
					reginf[used[i].reg].WritenBack = true;
				}
			}
			if (i)
			{
				alb++;
				//log("Allocaded %d xmm regs, %d%%\n",i,alb*100/(alb+nalb));
				//if (getchar()=='n')
				//	memset(reginf,0xFF,sizeof(reginf));
			}
			else
				nalb++;
			//
		}
	}
	//BeforeEmit		: generate any code needed before the main emittion begins (other register allocators may have emited code tho)
	virtual void BeforeEmit()
	{
		for (int i=0;i<16;i++)
		{
			if (IsRegAllocated(i+fr_0))
			{
				GetRegister(XMM0,i+fr_0,RA_DEFAULT);
			}
		}
	}
	//BeforeTrail		: generate any code needed after the main emittion has ended (other register allocators may emit code after that tho)
	virtual void BeforeTrail()
	{
		FlushRegCache();
	}
	//AfterTrail		: generate code after the native block end (after the ret) , can be used to emit helper functions (other register allocators may emit code after that tho)
	virtual void AfterTrail()
	{

	}
	//IsRegAllocated	: *couh* yea .. :P
	virtual bool IsRegAllocated(u32 sh4_reg)
	{
		ensure_valid(sh4_reg);
		return GetInfo(sh4_reg)!=0;
	}
	//Carefull w/ register state , we may need to implement state push/pop
	//GetRegister		: Get the register , needs flag for mode
	virtual x86_sse_reg GetRegister(x86_sse_reg d_reg,u32 reg,u32 mode)
	{
		ensure_valid(reg);
		if (IsRegAllocated(reg))
		{
			fprinfo* r1=  GetInfo(reg);
			if (r1->Loaded==false)
			{
				if ((mode & RA_NODATA)==0)
				{
					x86e->Emit(op_movss,r1->reg,GetRegPtr(reg));
					r1->WritenBack=true;//data on reg is same w/ data on mem
				}
				else
					r1->WritenBack=false;//data on reg is not same w/ data on mem
			}
			
			//we reg is now on sse reg :)
			r1->Loaded=true;

			if (mode & RA_FORCE)
			{
				if ((mode & RA_NODATA)==0)
					x86e->Emit(op_movss,d_reg,r1->reg);
				return d_reg;
			}
			else
			{
				return r1->reg;
			}

		}
		else
		{
			if ((mode & RA_NODATA)==0)
				x86e->Emit(op_movss,d_reg,GetRegPtr(reg));
			return d_reg;
		}
//		__debugbreak(); 
		//return XMM_Error;
	}
	//Save registers
	virtual void SaveRegister(u32 reg,x86_sse_reg from)
	{
		ensure_valid(reg);
		if (IsRegAllocated(reg))
		{
			fprinfo* r1=  GetInfo(reg);
			r1->Loaded=true;
			r1->WritenBack=false;
			
			if (r1->reg!=from)
				x86e->Emit(op_movss,r1->reg,from);
		}
		else
			x86e->Emit(op_movss,GetRegPtr(reg),from);
	}
	
	virtual void SaveRegister(u32 reg,float* from)
	{
		ensure_valid(reg);
		if (IsRegAllocated(reg))
		{
			fprinfo* r1=  GetInfo(reg);
			r1->Loaded=true;
			r1->WritenBack=false;
			x86e->Emit(op_movss,r1->reg,(u32*)from);
		}
		else
		{
			x86e->Emit(op_movss,XMM0,(u32*)from);
			x86e->Emit(op_movss,GetRegPtr(reg),XMM0);
		}
	}
	//FlushRegister		: write reg to reg location , and reload it on next use that needs reloading
	virtual void FlushRegister(u32 reg)
	{
		ensure_valid(reg);
		if (IsRegAllocated(reg))
		{
			WriteBackRegister(reg);
			ReloadRegister(reg);
		}
	}
	virtual void FlushRegister_xmm(x86_sse_reg reg)
	{
		for (int i=0;i<16;i++)
		{
			fprinfo* r1=  GetInfo(fr_0+i);
			if (r1!=0 && r1->reg==reg)
			{
				FlushRegister(fr_0+i);
			}
		}
	}
	virtual void FlushRegCache()
	{
		for (int i=0;i<16;i++) {
			FlushRegister(fr_0+i);
		}

		//Notes (Div22) :
		//x86e->Emit(op_emms); //Results in no-op due to previous mov r,m (if any)
		//x86e->Emit(op_sfence); //7-10% speed penalty
	}
	//WriteBackRegister	: write reg to reg location
	virtual void WriteBackRegister(u32 reg)
	{
		ensure_valid(reg);
		if (IsRegAllocated(reg))
		{
			fprinfo* r1=  GetInfo(reg);
			if (r1->Loaded)
			{
				if (r1->WritenBack==false)
				{
					x86e->Emit(op_movss,GetRegPtr(reg),r1->reg);
					r1->WritenBack=true;
				}
			}
		}
	}
	//ReloadRegister	: read reg from reg location , discard old result
	virtual void ReloadRegister(u32 reg)
	{
		ensure_valid(reg);
		if (IsRegAllocated(reg))
		{
			fprinfo* r1=  GetInfo(reg);
			r1->Loaded=false;
		}
	}
	virtual void SaveRegisterGPR(u32 to,x86_gpr_reg from)
	{
		if (IsRegAllocated(to))
		{
			if (x86_caps.sse_2)
			{
				x86_sse_reg freg=GetRegister(XMM0,to,RA_NODATA);
				assert(freg!=XMM0);
				//x86e->Emit(op_int3);
				//x86e->SSE2_MOVD_32R_to_XMM(freg,from);
				x86e->Emit(op_movd_xmm_from_r32,freg,from);
			}
			else
			{
				x86e->Emit(op_mov32,GetRegPtr(to),from);
				ReloadRegister(to);
			}
		}
		else
		{
			x86e->Emit(op_mov32,GetRegPtr(to),from);
		}
	}
	virtual void LoadRegisterGPR(x86_gpr_reg to,u32 from)
	{
		if (IsRegAllocated(from))
		{
			fprinfo* r1=  GetInfo(from);
			if ((x86_caps.sse_2) &&  (r1->Loaded==true) && (r1->WritenBack==false))
			{
				x86_sse_reg freg=GetRegister(XMM0,from,RA_DEFAULT);
				assert(freg!=XMM0);
				x86e->Emit(op_movd_xmm_to_r32,to,freg);
			}
			else
			{
				WriteBackRegister(from);
				x86e->Emit(op_mov32,to,GetRegPtr(from));
			}
		}
		else
		{
			x86e->Emit(op_mov32,to,GetRegPtr(from));
		}
	}
};

FloatRegAllocator * GetFloatAllocator()
{
	return new SimpleSSERegAlloc();
}