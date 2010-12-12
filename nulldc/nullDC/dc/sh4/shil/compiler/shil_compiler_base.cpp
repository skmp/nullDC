//#include "shil_compile_slow.h"

#include "types.h"
#include "dc\sh4\shil\shil.h"
#include <assert.h>
#include "emitter\emitter.h"

#include "dc\sh4\shil\shil_ce.h"
#include "dc\sh4\sh4_registers.h"
#include "dc\sh4\rec_v1\blockmanager.h"
#include "dc\sh4\sh4_opcode_list.h"
#include "dc\mem\sh4_mem.h"
#include "emitter\regalloc\x86_sseregalloc.h"

#include "shil_compiler_base.h"

//Current arch (19.8.07) :
//Single compilation unit, single branching with backpatching, including indirect calls
//Static reg alloc, seperate for fpu/cpu (fpu is disabled too , atm)
//uh. thats it ?

FloatRegAllocator*		fra;
IntegerRegAllocator*	ira;
vector<roml_patch> roml_patch_list;

x86_block* x86e;

x86_opcode_class SetCC[] =
{
	op_seto ,//r/m8 = 0F 90 /0
	op_setno ,//r/m8 = 0F 91 /0
	op_setb ,//r/m8 = 0F 92 /0
	op_setae ,//r/m8 = 0F 93 /0
	op_sete ,//r/m8 = 0F 94 /0
	op_setnz ,//r/m8 = 0F 95 /0
	op_setna ,//r/m8 = 0F 96 /0
	op_seta ,//r/m8 = 0F 97 /0
	op_sets ,//r/m8 = 0F 98 /0
	op_setns ,//r/m8 = 0F 99 /0
	op_setp ,//r/m8 = 0F 9A /0
	op_setnp ,//r/m8 = 0F 9B /0
	op_setl ,//r/m8 = 0F 9C /0
	op_setge ,//r/m8 = 0F 9D /0
	op_setle ,//r/m8 = 0F 9E /0
	op_setg ,//r/m8 = 0F 9F /0
};

typedef void __fastcall shil_compileFP(shil_opcode* op);

bool inited=false;

//[R|W][sz][M/F][addr]
__declspec(align(64))
void* mio_pvt[2][4][2][8];

/*
int fallbacks=0;
int native=0;
*/
u32 T_jcond_value;

u32 reg_pc_temp_value;


union _Cmp64
{
	struct
	{
		u32 l;
		u32 h;
	};
	u64 v;
};

//more helpers
//ensure mode is 32b (for floating point)
void c_Ensure32()
{
	assert(fpscr.PR==0);
}
//emit a call to c_Ensure32 
bool Ensure32()
{
	x86e->Emit(op_call,x86_ptr_imm(c_Ensure32));
	return true;
}

//Register managment related
//Get a pointer to a reg
u32* GetRegPtr(u32 reg)
{
	if (reg==reg_pc_temp)
		return &reg_pc_temp_value;

	u32* rv=Sh4_int_GetRegisterPtr((Sh4RegType)reg);
	assert(rv!=0);
	return rv;
}
u32* GetRegPtr(Sh4RegType reg)
{
	return GetRegPtr((u8)reg);
}
bool IsSSEAllocReg(u32 reg)
{
	return (reg >=fr_0 && reg<=fr_15);
}
bool IsFpuReg(u32 reg)
{
	return (reg >=fr_0 && reg<=xf_15);
}
//FPU !!! YESH
u32 IsInFReg(u32 reg)
{
	if (IsSSEAllocReg(reg))
	{
		if (fra->IsRegAllocated(reg))
			return 1;
	}
	return 0;
}

//REGISTER ALLOCATION
#define LoadReg(to,reg) ira->GetRegister(to,reg,RA_DEFAULT)
#define LoadReg_force(to,reg) ira->GetRegister(to,reg,RA_FORCE)
#define LoadReg_nodata(to,reg) ira->GetRegister(to,reg,RA_NODATA)
#define SaveReg(reg,from)	ira->SaveRegister(reg,from)

//intel sugest not to use the ItoM forms for some reason .. speed diference isnt big .. < 1%

//Common opcode handling code
//reg to reg
void fastcall op_reg_to_reg(shil_opcode* op,x86_opcode_class op_cl)
{
	assert(FLAG_32==(op->flags & 3));
	assert(0==(op->flags & (FLAG_IMM2)));
	assert(op->flags & FLAG_REG1);
	if (op->flags & FLAG_IMM1)
	{
		assert(0==(op->flags & FLAG_REG2));
		if (ira->IsRegAllocated(op->reg1))
		{
			x86_gpr_reg r1 = LoadReg(EAX,op->reg1);
			assert(r1!=EAX);
			x86e-> Emit(op_cl,r1,op->imm1);
			SaveReg(op->reg1,r1);
		}
		else
		{
			/*x86e-> _ItM_ (GetRegPtr(op->reg1),op->imm1);*/
			x86e->Emit(op_mov32,EAX,op->imm1);
			x86e->Emit(op_cl,GetRegPtr(op->reg1),EAX);
		}
	}
	else
	{
		assert(op->flags & FLAG_REG2);
		if (ira->IsRegAllocated(op->reg1))\
		{
			x86_gpr_reg r1 = LoadReg(EAX,op->reg1);
			assert(r1!=EAX);
			if (ira->IsRegAllocated(op->reg2))
			{
				x86_gpr_reg r2 = LoadReg(EAX,op->reg2);
				assert(r2!=EAX);
				x86e-> Emit(op_cl,r1,r2);
			}
			else
			{
				x86e-> Emit(op_cl,r1,GetRegPtr(op->reg2));
			}
			SaveReg(op->reg1,r1);
		}
		else
		{
			x86_gpr_reg r2 = LoadReg(EAX,op->reg2);
			x86e->Emit(op_cl,GetRegPtr(op->reg1),r2);
		}
	}
}

//imm to reg
void fastcall op_imm_to_reg(shil_opcode* op,x86_opcode_class op_cl)
{
	assert(FLAG_32==(op->flags & 3));
	assert(op->flags & FLAG_IMM1);
	assert(0==(op->flags & (FLAG_IMM2)));
	assert(op->flags & FLAG_REG1);
	assert(0==(op->flags & FLAG_REG2));
	if (ira->IsRegAllocated(op->reg1))
	{
		x86_gpr_reg r1=LoadReg(EAX,op->reg1);
		assert(r1!=EAX);
		x86e->Emit(op_cl,r1,op->imm1);
		SaveReg(op->reg1,r1);
	}
	else
		x86e->Emit(op_cl,GetRegPtr(op->reg1),op->imm1);
}
//reg
void fastcall op_reg(shil_opcode* op,x86_opcode_class op_cl)
{
	assert(FLAG_32==(op->flags & 3));
	assert(0==(op->flags & FLAG_IMM1));
	assert(0==(op->flags & (FLAG_IMM2)));
	assert(op->flags & FLAG_REG1);
	assert(0==(op->flags & FLAG_REG2));
	if (ira->IsRegAllocated(op->reg1))
	{
		x86_gpr_reg r1=LoadReg(EAX,op->reg1);
		assert(r1!=EAX);
		x86e->Emit(op_cl,r1);
		SaveReg(op->reg1,r1);
	}
	else
		x86e->Emit(op_cl,x86_ptr(GetRegPtr(op->reg1)));
}


//_vmem for dynarec ;)
//on dynarec we have 3 cases for input , mem , reg , const
//and 3 cases of output , mem , reg , xmm [fpu reg]
//this will also support 64b mem/xmm reads

//handler tables
extern _vmem_ReadMem8FP*		_vmem_RF8[0x1000];
extern _vmem_WriteMem8FP*		_vmem_WF8[0x1000];

extern _vmem_ReadMem16FP*		_vmem_RF16[0x1000];
extern _vmem_WriteMem16FP*		_vmem_WF16[0x1000];

extern _vmem_ReadMem32FP*		_vmem_RF32[0x1000];
extern _vmem_WriteMem32FP*		_vmem_WF32[0x1000];
//extern void* _vmem_MemInfo[0x10000];

bool _vmem_translate(u32 addr,unat& entry_or_fptr);

bool nvmem_GetPointer(void* &ptr,u32 addr,u32 rw,u32 sz)
{
	void* p_RWF_table;
	if (rw==0)
	{
		if (sz==FLAG_8)
			p_RWF_table=&_vmem_RF8[0];
		else if (sz==FLAG_16)
			p_RWF_table=&_vmem_RF16[0];
		else if (sz==FLAG_32)
			p_RWF_table=&_vmem_RF32[0];
		else if (sz==FLAG_64)
			p_RWF_table=&_vmem_RF32[0];
		else
			die("invalid read size");
	}
	else
	{
		if (sz==FLAG_8)
			p_RWF_table=&_vmem_WF8[0];
		else if (sz==FLAG_16)
			p_RWF_table=&_vmem_WF16[0];
		else if (sz==FLAG_32)
			p_RWF_table=&_vmem_WF32[0];
		else if (sz==FLAG_64)
			p_RWF_table=&_vmem_WF32[0];
		else
			die("invalid write size");
	}

	unat tptr = 0;

	if (_vmem_translate(addr,tptr))
	{
		ptr=((void**)p_RWF_table)[tptr >> 2];	//direct value, no need to derefernce
		return false;
	}
	//else
	{
		ptr=(void*)tptr;				//pointer, needs to be readed
		return true;
	}
}

//sz is 1,2,4,8
void emit_vmem_op_compat_const(x86_block* x86e,u32 rw,u32 sz,u32 addr,u32 reg)
{
	if (sz==FLAG_64)
		reg=GetSingleFromDouble((u8)reg);

	void* ptr;
	if (nvmem_GetPointer(ptr,addr,rw,sz))
	{
		/*
		this needs more checks before beeing implemented :)
		if (sh4_ram_alt!=0 && IsOnRam(addr))
		{
			ptr=sh4_ram_alt+(addr&RAM_MASK);
		}*/
		//direct access
		switch(sz)
		{
		case FLAG_8:
			if (rw==0)
			{
				ira->SaveRegister(reg,(s8*)ptr);
			}
			else
			{
				x86_reg rs= ira->GetRegister(EDX,reg,RA_DEFAULT);	//must be on 8 bit accessible reg
				if (rs>BL)
				{
					x86e->Emit(op_mov32,EDX,rs);
					rs=EDX;
				}
				x86e->Emit(op_mov8,ptr,rs);
			}
			break;

		case FLAG_16:
			if (rw==0)
			{
				ira->SaveRegister(reg,(s16*)ptr);
			}
			else
			{
				x86_reg rs= ira->GetRegister(EDX,reg,RA_DEFAULT);//any reg will do
				x86e->Emit(op_mov16,ptr,rs);
			}
			break;

		case FLAG_32:
			if (rw==0)
			{
				if (IsSSEAllocReg(reg))
				{
					fra->SaveRegister(reg,(float*)ptr);
				}
				else
				{
					ira->SaveRegister(reg,(u32*)ptr);
				}
			}
			else
			{
				if (IsSSEAllocReg(reg))
				{
					x86e->Emit(op_mov32,EAX,GetRegPtr(reg));
					x86e->Emit(op_mov32,(u32*)ptr,EAX);
				}
				else
				{
					x86_reg rs= ira->GetRegister(EAX,reg,RA_DEFAULT);//any reg will do
					x86e->Emit(op_mov32,ptr,rs);
				}
			}
			break;
		case FLAG_64:
			verify(IsFpuReg(reg));
			if (rw==0)
			{
				x86e->Emit(op_movlps,XMM0,(u32*)ptr);
				x86e->Emit(op_movlps,GetRegPtr(reg),XMM0);
			}
			else
			{
				x86e->Emit(op_movlps,XMM0,GetRegPtr(reg));
				x86e->Emit(op_movlps,(u32*)ptr,XMM0);
			}
			break;
		}
	}
	else
	{
		bool doloop=false;//sz==FLAG_64;

		do
		{
			doloop=doloop ^(sz==FLAG_64); 
			//gota call functions
			x86e->Emit(op_mov32,ECX,addr);



			if (rw==1)
			{
				if (IsSSEAllocReg(reg))
				{
					x86e->Emit(op_mov32,EDX,GetRegPtr(reg));
				}
				else
				{
					ira->GetRegister(EDX,reg,RA_FORCE);//any reg will do
				}
			}

			//call the function
			x86e->Emit(op_call,x86_ptr_imm(ptr));

			if(rw==0)
			{
				switch(sz)
				{
				case FLAG_8:
					{
					x86_reg r=ira->GetRegister(EAX,reg,RA_NODATA);
					x86e->Emit(op_movsx8to32,r,EAX);
					ira->SaveRegister(reg,r);
					}
					break;
				case FLAG_16:
					{
					x86_reg r=ira->GetRegister(EAX,reg,RA_NODATA);
					x86e->Emit(op_movsx16to32,r,EAX);
					ira->SaveRegister(reg,r);
					}
					break;
				case FLAG_32:
					if (IsSSEAllocReg(reg))
					{
						x86e->Emit(op_mov32,GetRegPtr(reg),EAX);
					}
					else
					{
						ira->SaveRegister(reg,EAX);
					}
					break;
				case FLAG_64:
					x86e->Emit(op_mov32,GetRegPtr(reg),EAX);
					break;
				}
			}

			//in case we loop -- its 64 bit read
			reg++;
			addr+=4;
		}
		while(doloop);
	}
	/*
	void* p_RWF_table=0;
	if (rw==0)
	{
		if (sz==1)
			p_RWF_table=&_vmem_RF8[0];
		else if (sz==2)
			p_RWF_table=&_vmem_RF16[0];
		else if (sz==4)
			p_RWF_table=&_vmem_RF32[0];
		else if (sz==8)
			p_RWF_table=&_vmem_RF32[0];
		else
			die("invalid read size");
	}
	else
	{
		if (sz==1)
			p_RWF_table=&_vmem_WF8[0];
		else if (sz==2)
			p_RWF_table=&_vmem_WF16[0];
		else if (sz==4)
			p_RWF_table=&_vmem_WF32[0];
		else if (sz==8)
			p_RWF_table=&_vmem_WF32[0];
		else
			die("invalid write size");
	}

	u32 upper=ra>>16;

	void * t =_vmem_MemInfo[upper];


	u32 lower=ra& 0xFFFF;

	if ((((u32)t) & 0xFFFF0000)==0)
	{
		verify(sz!=8);	//64 bit writes to registers are not possible so far :P.Hopefully will never happen
		if (rw==1)
		{
			if (sse)
			{
				//die("sse + function write is not supported");
				
				x86e->Emit(op_movd_xmm_to_r32,EDX,ro);
			}
			else
			{
				if (ro!=EDX)
					x86e->Emit(op_mov32,EDX,ro);
			}
		}

		x86e->Emit(op_mov32,ECX,ra);

		u32 entry=((u32)t)>>2;

		x86e->Emit(op_call , x86_ptr_imm(((u32**)p_RWF_table)[entry]));
		if (rw==0)
		{
			if (sz==1)
			{
				x86e->Emit(op_movsx8to32, ro,EAX);
			}
			else if (sz==2)
			{
				x86e->Emit(op_movsx16to32, ro,EAX);
			}
			else if (sz==4)
			{
				if (ro!=EAX)
					x86e->Emit(op_mov32,ro,EAX);
			}
		}
	}
	else
	{	//Direct Ram Read
		if (rw==1)
		{
			void* paddr=&((u8*)t)[lower];
			
			if (sz==1)
			{	//copy to eax :p
				if (ro!=EDX)
					x86e->Emit(op_mov32,EDX,ro);
				x86e->Emit(op_mov8 ,(u8*)paddr,EDX);
			}
			else if (sz==2)
			{	//,dx
				x86e->Emit(op_mov16 ,(u16*)paddr,ro);
			}
			else if (sz==4)
			{	//,edx
				if (sse)
				{
					x86e->Emit(op_movss,(u32*)paddr,ro);
				}
				else
					x86e->Emit(op_mov32,(u32*)paddr,ro);
			}
			else if (sz==8)
			{
				x86e->Emit(op_movlps,(u32*)paddr,ro);
			}
		}
		else
		{
			void* paddr=&((u8*)t)[lower];
			if (sz==1)
			{
				x86e->Emit(op_movsx8to32, ro,(u8*)paddr);
			}
			else if (sz==2)
			{
				x86e->Emit(op_movsx16to32, ro,(u16*)paddr);
			}
			else if (sz==4)
			{
				if (sse)
				{
					x86e->Emit(op_movss,ro,(u32*)paddr);
				}
				else
					x86e->Emit(op_mov32,ro,(u32*)paddr);
			}
			else if (sz==8)
			{
				x86e->Emit(op_movlps,ro,(u32*)paddr);
			}
		}
	}
	*/
}

#if 0
//sz : 1,2 -> sign extended , 4 fully loaded.SSE valid olny for sz=4
//reg_addr : either ECX , either allocated
void emit_vmem_read(x86_reg reg_addr,u8 reg_out,u32 sz)
{
	bool sse=IsInFReg(reg_out);
	if (sse)
		verify(sz==4);

	x86_ptr p_RF_table(0);
	x86_Label* direct=x86e->CreateLabel(false,8);
	x86_Label* end=x86e->CreateLabel(false,8);

	if (sz==1)
		p_RF_table=&_vmem_RF8[0];
	else if (sz==2)
		p_RF_table=&_vmem_RF16[0];
	else if (sz==4)
		p_RF_table=&_vmem_RF32[0];

	//x86e->Emit(op_int3);
	//copy address
	//this is done here , among w/ the and , it should be possible to fully execute it on paraler (no depency)
	x86e->Emit(op_mov32,EDX,reg_addr);
	x86e->Emit(op_mov32,EAX,reg_addr);
	//lower 16b of address
	x86e->Emit(op_and32,EDX,0xFFFF);
	//x86e->Emit(op_movzx16to32,EDX,EDX);
	//get upper 16 bits
	x86e->Emit(op_shr32,EAX,16);
	//read mem info
	//mov eax,[_vmem_MemInfo+eax*4];
	x86e->Emit(op_mov32,EAX,x86_mrm(EAX,sib_scale_4,_vmem_MemInfo));

	//test eax,0xFFFF0000;
	x86e->Emit(op_test32,EAX,0xFFFF0000);
	//jnz direct;
	x86e->Emit(op_jnz,direct);
	//--other read---
	if (reg_addr!=ECX)
		x86e->Emit(op_mov32,ECX,reg_addr);
	//Get function pointer and call it
	x86e->Emit(op_call32,x86_mrm(EAX,p_RF_table));

	//save reg
	if (!sse)
	{
		x86_reg writereg= LoadReg_nodata(EAX,reg_out);
		if (sz==1)
		{
			x86e->Emit(op_movsx8to32, writereg,EAX);
		}
		else if (sz==2)
		{
			x86e->Emit(op_movsx16to32, writereg,EAX);
		}
		else
		{
			if (writereg!=EAX)
				x86e->Emit(op_mov32, writereg,EAX);
		}
		SaveReg(reg_out,writereg);
	}
	else
	{	
		fra->SaveRegisterGPR(reg_out,EAX);
	}

	x86e->Emit(op_jmp,end);
//direct:
	x86e->MarkLabel(direct);
//	mov eax,[eax+edx];	//note : upper bits dont matter , so i do 32b read here ;) (to get read of partial register stalls)
	if (!sse)
	{
		x86_reg writereg= LoadReg_nodata(EAX,reg_out);
		if (sz==1)
		{
			x86e->Emit(op_movsx8to32, writereg,x86_mrm(EAX,EDX));
		}
		else if (sz==2)
		{
			x86e->Emit(op_movsx16to32, writereg,x86_mrm(EAX,EDX));
		}
		else
		{
			x86e->Emit(op_mov32, writereg,x86_mrm(EAX,EDX));
		}
		SaveReg(reg_out,writereg);
	}
	else
	{
		x86_reg writereg= fra->GetRegister(XMM0,reg_out,RA_NODATA);
		
		x86e->Emit(op_movss, writereg,x86_mrm(EAX,EDX));
		
		fra->SaveRegister(reg_out,writereg);
	}
	x86e->MarkLabel(end);
}
//SSE valid olny for sz=4
//reg_addr : either ECX , either allocated
void emit_vmem_write(x86_reg reg_addr,u8 reg_data,u32 sz)
{
	bool sse=IsInFReg(reg_data);
	if (sse)
	{
		verify(sz==4);
		if (fra->IsRegAllocated(reg_data))
			fra->GetRegister(XMM0,reg_data,RA_DEFAULT);
	}
	else
	{
		if (ira->IsRegAllocated(reg_data))
			ira->GetRegister(EAX,reg_data,RA_DEFAULT);
	}

	x86_ptr p_WF_table(0);
	x86_Label* direct=x86e->CreateLabel(false,8);
	x86_Label* end=x86e->CreateLabel(false,8);

	if (sz==1)
		p_WF_table=&_vmem_WF8[0];
	else if (sz==2)
		p_WF_table=&_vmem_WF16[0];
	else if (sz==4)
		p_WF_table=&_vmem_WF32[0];

	//x86e->Emit(op_int3);
	//copy address
	//this is done here , among w/ the and , it should be possible to fully execute it on paraler (no depency)
	x86e->Emit(op_mov32,EDX,reg_addr);
	x86e->Emit(op_mov32,EAX,reg_addr);
	//lower 16b of address
	x86e->Emit(op_and32,EDX,0xFFFF);
	//x86e->Emit(op_movzx16to32,EDX,EDX);
	//get upper 16 bits
	x86e->Emit(op_shr32,EAX,16);
	//read mem info
	//mov eax,[_vmem_MemInfo+eax*4];
	x86e->Emit(op_mov32,EAX,x86_mrm(EAX,sib_scale_4,_vmem_MemInfo));

	//test eax,0xFFFF0000;
	x86e->Emit(op_test32,EAX,0xFFFF0000);
	//jnz direct;
	x86e->Emit(op_jnz,direct);
	//--other read---
	if (reg_addr!=ECX)
		x86e->Emit(op_mov32,ECX,reg_addr);

	//load reg
	if (!sse)
	{
		LoadReg_force(EDX,reg_data);
	}
	else
	{	
		fra->LoadRegisterGPR(EDX,reg_data);
	}

	//Get function pointer and call it
	x86e->Emit(op_call32,x86_mrm(EAX,p_WF_table));

	x86e->Emit(op_jmp,end);
//direct:
	x86e->MarkLabel(direct);
//	mov [eax+edx],reg;	//note : upper bits dont matter , so i do 32b read here ;) (to get read of partial register stalls)
	if (!sse)
	{
		
		if (sz==1)
		{
			x86_reg readreg= LoadReg_force(ECX,reg_data);
			x86e->Emit(op_mov8, x86_mrm(EAX,EDX),readreg);
		}
		else if (sz==2)
		{
			x86_reg readreg= LoadReg(ECX,reg_data);
			x86e->Emit(op_mov16, x86_mrm(EAX,EDX),readreg);
		}
		else
		{
			x86_reg readreg= LoadReg(ECX,reg_data);
			x86e->Emit(op_mov32, x86_mrm(EAX,EDX),readreg);
		}
	}
	else
	{
		x86_reg readreg= fra->GetRegister(XMM0,reg_data,RA_DEFAULT);
		
		x86e->Emit(op_movss, x86_mrm(EAX,EDX),readreg);
	}
	x86e->MarkLabel(end);
}

#endif

//#define REG_ALLOC_COUNT			 (shil_compile_slow_settings.RegAllocCount)
//#define REG_ALLOC_X86			 (shil_compile_slow_settings.RegAllocX86)
//#define REG_ALLOC_XMM			 (shil_compile_slow_settings.RegAllocXMM)
/*
#define INLINE_MEM_READ_CONST   (shil_compile_slow_settings.InlineMemRead_const)
#define INLINE_MEM_READ			(shil_compile_slow_settings.InlineMemRead)
#define INLINE_MEM_WRITE		(shil_compile_slow_settings.InlineMemWrite)
*/




//shil compilation
//shil opcode handlers

//Default handler , should never be called
void __fastcall shil_compile_nimp(shil_opcode* op)
{
	log("*********SHIL \"%s\" not recompiled*********\n",GetShilName((shil_opcodes)op->opcode));
}

//mov32/64 reg,reg/imm
void __fastcall shil_compile_mov(shil_opcode* op)
{
	u32 size=op->flags&3;
	assert(op->flags & FLAG_REG1);//reg1 has to be used on mov :)
	
	if (op->reg1==op->reg2)
			return;

	if (size==FLAG_32)
	{
		//sse_WBF(op->reg2);//write back possibly readed reg
		//OP_RegToReg_simple(MOV32);
		//sse_RLF(op->reg1);//reload writen reg
		#define mov_flag_GRP_1 1
		#define mov_flag_GRP_2 2
		#define mov_flag_XMM_1 4
		#define mov_flag_XMM_2 8
		#define mov_flag_M32_1 0
		#define mov_flag_M32_2 0
		#define mov_flag_imm_2 16

		u32 flags = 0;

		if (op->flags & FLAG_IMM1)
			flags|=mov_flag_imm_2;

		if (IsInFReg(op->reg1))
			flags|=mov_flag_XMM_1;
		
		if (((op->flags & FLAG_IMM1)==0) && IsInFReg(op->reg2))
			flags|=mov_flag_XMM_2;

		if (IsSSEAllocReg(op->reg1)==false && ira->IsRegAllocated(op->reg1))
			flags|=mov_flag_GRP_1;

		if (((op->flags & FLAG_IMM1)==0) && IsSSEAllocReg(op->reg2)==false && ira->IsRegAllocated(op->reg2))
			flags|=mov_flag_GRP_2;

		#define XMMtoXMM (mov_flag_XMM_1 | mov_flag_XMM_2)
		#define XMMtoGPR (mov_flag_GRP_1 | mov_flag_XMM_2)
		#define XMMtoM32 (mov_flag_M32_1 | mov_flag_XMM_2)
		
		#define GPRtoXMM (mov_flag_XMM_1 | mov_flag_GRP_2)
		#define GPRtoGPR (mov_flag_GRP_1 | mov_flag_GRP_2)
		#define GPRtoM32 (mov_flag_M32_1 | mov_flag_GRP_2)

		#define M32toXMM (mov_flag_XMM_1 | mov_flag_M32_2)
		#define M32toGPR (mov_flag_GRP_1 | mov_flag_M32_2)
		#define M32toM32 (mov_flag_M32_1 | mov_flag_M32_2)

		#define IMMtoXMM (mov_flag_XMM_1 | mov_flag_imm_2)
		#define IMMtoGPR (mov_flag_GRP_1 | mov_flag_imm_2)
		#define IMMtoM32 (mov_flag_M32_1 | mov_flag_imm_2)
		
		x86_sse_reg sse1=ERROR_REG;
		x86_sse_reg sse2=ERROR_REG;
		if (flags & mov_flag_XMM_1)
		{
			sse1=fra->GetRegister(XMM0,op->reg1,RA_NODATA);
			assert(sse1!=XMM0);
		}

		if (flags & mov_flag_XMM_2)
		{
			sse2=fra->GetRegister(XMM0,op->reg2,RA_DEFAULT);
			assert(sse2!=XMM0);
		}

		x86_gpr_reg gpr1=ERROR_REG;
		x86_gpr_reg gpr2=ERROR_REG;

		if (flags & mov_flag_GRP_1)
		{
			gpr1=ira->GetRegister(EAX,op->reg1,RA_NODATA);
			assert(gpr1!=EAX);
		}

		if (flags & mov_flag_GRP_2)
		{
			gpr2=ira->GetRegister(EAX,op->reg2,RA_DEFAULT);
			assert(gpr2!=EAX);
		}


		switch(flags)
		{
		case XMMtoXMM:
			{
				x86e->Emit(op_movss,sse1,sse2);
				fra->SaveRegister(op->reg1,sse1);
			}
			break;
		case XMMtoGPR:
			{
				//write back to mem location
				x86e->Emit(op_movss,GetRegPtr(op->reg1),sse2);
				//mark that the register has to be reloaded from there
				ira->ReloadRegister(op->reg1);
			}
			break;
		case XMMtoM32:
			{
				//copy to mem location
				x86e->Emit(op_movss,GetRegPtr(op->reg1),sse2);
			}
			break;

		case GPRtoXMM:		
			{
				//write back to ram
				x86e->Emit(op_mov32,GetRegPtr(op->reg1),gpr2);
				//mark reload on next use
				fra->ReloadRegister(op->reg1);
			}
			break;
		case GPRtoGPR:
			{
				x86e->Emit(op_mov32,gpr1,gpr2);
				ira->SaveRegister(op->reg1,gpr1);
			}
			break;
		case GPRtoM32:
			{
				//copy to ram
				x86e->Emit(op_mov32,GetRegPtr(op->reg1),gpr2);
			}
			break;
		case M32toXMM:
			{
				x86e->Emit(op_movss,sse1,GetRegPtr(op->reg2));
				fra->SaveRegister(op->reg1,sse1);
			}
			break;
		case M32toGPR:
			{
				x86e->Emit(op_mov32,gpr1,GetRegPtr(op->reg2));
				ira->SaveRegister(op->reg1,gpr1);
			}
			break;
		case M32toM32:
			{
				x86e->Emit(op_mov32,EAX,GetRegPtr(op->reg2));
				x86e->Emit(op_mov32,GetRegPtr(op->reg1),EAX);
			}
			break;
		case IMMtoXMM:
			{
				//log("impossible mov IMMtoXMM [%X]\n",flags);
				//__asm int 3;
				//write back to ram
				x86e->Emit(op_mov32,GetRegPtr(op->reg1),op->imm1);
				//mark reload on next use
				fra->ReloadRegister(op->reg1);
			}
			break;

		case IMMtoGPR:
			{
				x86e->Emit(op_mov32,gpr1,op->imm1);
				ira->SaveRegister(op->reg1,gpr1);
			}
			break;

		case IMMtoM32:
			{
				x86e->Emit(op_mov32,GetRegPtr(op->reg1),op->imm1);
			}
			break;

		default:
			log("unknown mov %X\n",flags);
			__asm int 3;
			break;
		}
	}
	else
	{
		assert(size==FLAG_64);//32 or 64 b
		assert(0==(op->flags & (FLAG_IMM1|FLAG_IMM2)));//no imm can be used
		//log("mov64 not supported\n");
		u8 dest=GetSingleFromDouble((u8)op->reg1);
		u8 source=GetSingleFromDouble((u8)op->reg2);

		//x86e->Emit(op_mov32,EAX,GetRegPtr(source));
		//x86e->Emit(op_mov32,ECX,GetRegPtr(source+1));
		x86e->Emit(op_movlps,XMM0,GetRegPtr(source));

		//x86e->Emit(op_mov32,GetRegPtr(dest),EAX);
		//x86e->Emit(op_mov32,GetRegPtr(dest+1),ECX);
		x86e->Emit(op_movlps,GetRegPtr(dest),XMM0);
	}
}


//movex8/16 reg32,zx/sx reg8/16
void __fastcall shil_compile_movex(shil_opcode* op)
{
	u32 size=op->flags&3;
	assert(op->flags & (FLAG_REG1|FLAG_REG2));	//reg1 , reg2 has to be used on movex :)
	assert((size!=FLAG_8)||(size!=FLAG_16));	//olny 8 or 16 bits can be extended

	if (size==FLAG_8)
	{//8 bit
		if (op->flags & FLAG_SX)
		{//SX 8
			x86_gpr_reg r2= LoadReg_force(EAX,op->reg2);
			x86_gpr_reg r1= LoadReg_nodata(ECX,op->reg1);//if same reg (so data is needed) that is done by the above op
			x86e->Emit(op_movsx8to32, r1,r2);
			SaveReg(op->reg1,r1);
		}
		else
		{//ZX 8
			x86_gpr_reg r2= LoadReg_force(EAX,op->reg2);
			x86_gpr_reg r1= LoadReg_nodata(ECX,op->reg1);//if same reg (so data is needed) that is done by the above op
			x86e->Emit(op_movzx8to32, r1,r2);
			SaveReg(op->reg1,r1);
		}
	}
	else
	{//16 bit
		if (op->flags & FLAG_SX)
		{//SX 16
			x86_gpr_reg r1;
			if (op->reg1!=op->reg2)
				r1= LoadReg_nodata(ECX,op->reg1);	//get a spare reg , or the allocated one. Data will be overwriten
			else
				r1= LoadReg(ECX,op->reg1);	//get or alocate reg 1 , load data b/c it's gona be used

			if (ira->IsRegAllocated(op->reg2))
			{
				x86_gpr_reg r2= LoadReg(EAX,op->reg2);
				assert(r2!=EAX);//reg 2 must be allocated
				x86e->Emit(op_movsx16to32, r1,r2);
			}
			else
			{
				x86e->Emit(op_movsx16to32, r1,(u16*)GetRegPtr(op->reg2));
			}
			SaveReg(op->reg1,r1);	//ensure it is saved
		}
		else
		{//ZX 16
			x86_gpr_reg r1;
			if (op->reg1!=op->reg2)
				r1= LoadReg_nodata(ECX,op->reg1);	//get a spare reg , or the allocated one. Data will be overwriten
			else
				r1= LoadReg(ECX,op->reg1);	//get or alocate reg 1 , load data b/c it's gona be used

			if (ira->IsRegAllocated(op->reg2))
			{
				x86_gpr_reg r2= LoadReg(EAX,op->reg2);
				assert(r2!=EAX);//reg 2 must be allocated
				x86e->Emit(op_movzx16to32, r1,r2);
			}
			else
			{
				x86e->Emit(op_movzx16to32, r1,(u16*)GetRegPtr(op->reg2));
			}
			SaveReg(op->reg1,r1);	//ensure it is saved
		}
	}
}
//shil_ifb opcode
//calls interpreter to emulate the opcode
void __fastcall shil_compile_shil_ifb(shil_opcode* op)
{

	//if opcode needs pc , save it
	if (OpTyp[op->imm1] !=Normal)
		SaveReg(reg_pc,op->imm2);
	
	ira->FlushRegCache();
	fra->FlushRegCache();
	
	x86e->Emit(op_mov32,ECX,op->imm1);
	x86e->Emit(op_call,x86_ptr_imm(OpPtr[op->imm1]));
}

//shift based
//swap8/16 reg32
void __fastcall shil_compile_swap(shil_opcode* op)
{
	u32 size=op->flags&3;
	
	assert(0==(op->flags & (FLAG_IMM1|FLAG_IMM2)));//no imms
	assert(op->flags & FLAG_REG1);//reg1
	assert(0==(op->flags & FLAG_REG2));//reg2

	if (size==FLAG_8)
	{
		x86_gpr_reg r1 = LoadReg_force(EAX,op->reg1);
		x86e->Emit(op_xchg8,AH,AL);//ror16 ?
		SaveReg(op->reg1,r1);
	}
	else
	{
		assert(size==FLAG_16);//has to be 16 bit
		//log("Shil : wswap not implemented\n");
		
		//use rotate ?
		x86_gpr_reg r1 = LoadReg(EAX,op->reg1);
		x86e->Emit(op_ror32,r1,16);
		SaveReg(op->reg1,r1);
	}
}
//shl reg32,imm , set flags
void __fastcall shil_compile_shl(shil_opcode* op)
{
	op_imm_to_reg(op,op_shl32);
}
//shr reg32,imm , set flags
void __fastcall shil_compile_shr(shil_opcode* op)
{
	op_imm_to_reg(op,op_shr32);
}
//sar reg32,imm , set flags
void __fastcall shil_compile_sar(shil_opcode* op)
{
	op_imm_to_reg(op,op_sar32);
}
//rotates
//rcl reg32, CF is set before calling
void __fastcall shil_compile_rcl(shil_opcode* op)
{
	op_reg(op,op_rcl32);
}
//rcr reg32, CF is set before calling
void __fastcall shil_compile_rcr(shil_opcode* op)
{
	op_reg(op,op_rcr32);
}
//ror reg32
void __fastcall shil_compile_ror(shil_opcode* op)
{
	op_reg(op,op_ror32);
}
//rol reg32
void __fastcall shil_compile_rol(shil_opcode* op)
{
	op_reg(op,op_rol32);
}
//neg
void __fastcall shil_compile_neg(shil_opcode* op)
{
	op_reg(op,op_neg32);
}
//not
void __fastcall shil_compile_not(shil_opcode* op)
{
	op_reg(op,op_not32);
}
//xor reg32,reg32/imm32
void __fastcall shil_compile_xor(shil_opcode* op)
{
	op_reg_to_reg(op,op_xor32);
}
//or reg32,reg32/imm32
void __fastcall shil_compile_or(shil_opcode* op)
{
	op_reg_to_reg(op,op_or32);
}
//and reg32,reg32/imm32
void __fastcall shil_compile_and(shil_opcode* op)
{
	op_reg_to_reg(op,op_and32);
}
//readm/writem 
//Address calculation helpers
void readwrteparams1(u8 reg1,u32 imm,x86_reg* fast_nimm)
{
	if (ira->IsRegAllocated(reg1))
	{
		//lea ecx,[reg1+imm]
		x86_reg reg=LoadReg(ECX,reg1);
		assert(reg!=ECX);
		*fast_nimm=reg;
		x86e->Emit(op_lea32 ,ECX, x86_mrm(reg,x86_ptr::create(imm)));
	}
	else
	{
		//mov ecx,imm
		//add ecx,reg1
		x86e->Emit(op_mov32,ECX,imm);
		x86e->Emit(op_add32,ECX,GetRegPtr(reg1));
	}
}
void readwrteparams2(u8 reg1,u8 reg2)
{
	if (ira->IsRegAllocated(reg1))
	{
		x86_reg r1=LoadReg(ECX,reg1);
		assert(r1!=ECX);
		
		if (ira->IsRegAllocated(reg2))
		{
			//lea ecx,[reg1+reg2]
			x86_reg r2=LoadReg(ECX,reg2);
			assert(r2!=ECX);
			x86e->Emit(op_lea32,ECX,x86_mrm(r1,r2));
		}
		else
		{
			//mov ecx,reg1
			//add ecx,[reg2]
			x86e->Emit(op_mov32,ECX,r1);
			x86e->Emit(op_add32,ECX,GetRegPtr(reg2));
		}
	}
	else
	{
		if (ira->IsRegAllocated(reg2))
		{
			readwrteparams2(reg2,reg1);
		}
		else
		{
			//mov ecx,[reg1]
			//add ecx,[reg2]
			x86e->Emit(op_mov32,ECX,GetRegPtr(reg1));
			x86e->Emit(op_add32,ECX,GetRegPtr(reg2));
		}
	}
}
void readwrteparams3(u8 reg1,u8 reg2,u32 imm)
{
	if (ira->IsRegAllocated(reg1))
	{
		x86_reg r1=LoadReg(ECX,reg1);
		assert(r1!=ECX);
		
		if (ira->IsRegAllocated(reg2))
		{
			//lea ecx,[reg1+reg2]
			x86_reg r2=LoadReg(ECX,reg2);
			assert(r2!=ECX);
			x86e->Emit(op_lea32,ECX,x86_mrm(r1,r2,sib_scale_1,x86_ptr::create(imm)));
		}
		else
		{
			//lea ecx,[reg1+imm]
			//add ecx,[reg2]
			x86e->Emit(op_lea32,ECX,x86_mrm(r1,x86_ptr::create(imm)));
			x86e->Emit(op_add32,ECX,GetRegPtr(reg2));
		}
	}
	else
	{
		if (ira->IsRegAllocated(reg2))
		{
			readwrteparams3(reg2,reg1,imm);
		}
		else
		{
			//mov ecx,[reg1]
			//add ecx,[reg2]
			x86e->Emit(op_mov32,ECX,GetRegPtr(reg1));
			x86e->Emit(op_add32,ECX,imm);
			x86e->Emit(op_add32,ECX,GetRegPtr(reg2));
		}
	}
}
//Emit needed calc. asm and return register that has the address :)
x86_reg  readwrteparams(shil_opcode* op,x86_reg* fast_reg,u32* fast_offset)
{
	assert(0==(op->flags & FLAG_IMM2));
	assert(op->flags & FLAG_REG1);
	*fast_reg=ERROR_REG;
//	bool Loaded=false;

	//can use
	//mov ecx,imm
	//lea ecx[r[2]+imm]
	//lea ecx[r0/gbr+imm]
	//mov ecx,r0/gbr
	//mov ecx,r[2]
	//lea ecx[r0/gbr+r[2]*1]
	//lea ecx,[r0/gbr+r[2]*1+imm] ;)

	u32 flags=0;
#define flag_imm 1
#define flag_r2 2
#define flag_r0 4
#define flag_gbr 8

	if (op->flags & FLAG_IMM1)
	{
		if (op->imm1!=0)	//gota do that on const elimiation pass :D
			flags|=flag_imm;
	}
	if (op->flags & FLAG_REG2)
	{
		flags|=flag_r2;
	}
	if (op->flags & FLAG_R0)
	{
		flags|=flag_r0;
	}
	if (op->flags & FLAG_GBR)
	{
		flags|=flag_gbr;
	}

	verify(flags!=0);
	x86_reg reg=ERROR_REG;

	switch(flags)
	{
		//1 olny
	case flag_imm:
		x86e->Emit(op_mov32,ECX,op->imm1);
		reg=ECX;
		dbgbreak;//must never ever happen
		break;

	case flag_r2:
		reg=LoadReg(ECX,op->reg2);
		break;

	case flag_r0:
		reg=LoadReg(ECX,r0);
		break;

	case flag_gbr:
		reg=LoadReg(ECX,reg_gbr);
		break;

		//2 olny
	case flag_imm | flag_r2:
		*fast_offset=op->imm1;
		readwrteparams1((u8)op->reg2,op->imm1,fast_reg);
		reg=ECX;
		break;

	case flag_imm | flag_r0:
		*fast_offset=op->imm1;
		readwrteparams1((u8)r0,op->imm1,fast_reg);
		reg=ECX;
		break;

	case flag_imm | flag_gbr:
		*fast_offset=op->imm1;
		readwrteparams1((u8)reg_gbr,op->imm1,fast_reg);
		reg=ECX;
		break;

	case flag_r2 | flag_r0:
		readwrteparams2((u8)op->reg2,(u8)r0);
		reg=ECX;
		break;

	case flag_r2 | flag_gbr:
		readwrteparams2((u8)op->reg2,(u8)reg_gbr);
		reg=ECX;
		break;

		//3 olny
	case flag_imm | flag_r2 | flag_gbr:
		readwrteparams3((u8)op->reg2,(u8)reg_gbr,op->imm1);
		reg=ECX;
		break;

	case flag_imm | flag_r2 | flag_r0:
		readwrteparams3((u8)op->reg2,(u8)r0,op->imm1);
		reg=ECX;
		break;

	default:
		die("Unable to compute readwrteparams");
		break;
	}

	verify(reg!=ERROR_REG);
	return reg;
}


const u32 m_unpack_sz[4]={1,2,4,8};
//Ram Only Mem Lookup
void roml(x86_reg reg,x86_Label* lbl,u32* offset_Edit,x86_reg fast_reg,u32 fast_offset)
{
	//mov ecx,reg_addr
	if (reg!=ECX)
	{
		u32 old=x86e->x86_indx;
		x86e->Emit(op_mov32,ECX,reg);
		old=x86e->x86_indx-old;
		*offset_Edit+=old;
	}
	//x86e->Emit(op_mov32,EAX,reg); <- no longer used , since i have the offset for it :)
	//cmp ecx,mask1
	if (fast_reg!=ERROR_REG)
	{
		//fast_reg has the reg before adding the imm and moving to ecx
		x86e->Emit(op_cmp32,fast_reg,0xE0000000-fast_offset);
		//log("fast reG !!!%X\n",fast_offset);
	}
	else
	{
		x86e->Emit(op_cmp32,reg,0xE0000000);
	}
	//jae full_lookup
	x86e->Emit(op_jae,lbl);
	//and ecx,mask2
	x86e->Emit(op_and32,ECX,0x1FFFFFFF);
}
const x86_opcode_class rm_table[4]={op_movsx8to32,op_movsx16to32,op_mov32,op_movlps};
void __fastcall shil_compile_readm(shil_opcode* op)
{
	u32 size=op->flags&3;

	//if constant read , and on ram area , make it a direct mem access
	//_watch_ mmu
	if (!(op->flags & (FLAG_R0|FLAG_GBR|FLAG_REG2)))
	{
		//[imm1] form
		assert(op->flags & FLAG_IMM1);
		emit_vmem_op_compat_const(x86e,0,size,op->imm1,op->reg1);
		return;
	}

	u32 old_offset=x86e->x86_indx;
	x86_reg fast_reg;
	u32 fast_reg_offset;
	x86_reg reg_addr = readwrteparams(op,&fast_reg,&fast_reg_offset);
	/*
	x86e->Emit(op_mov32,EAX,ECX);
	x86e->Emit(op_shr32,EAX,29);
	x86e->Emit(op_call32,x86_mrm(NO_REG,EAX,sib_scale_4,&mio_pvt[0][size][0][0]));

	if (size==FLAG_64)
	{
		//EAX:EDX copy
	}
	else
	{
		if (is_float)
		{
			//EAX
		}
		else
		{
			//EAX
		}
	}*/
	
	old_offset=x86e->x86_indx-old_offset;
	x86_Label* patch_point= x86e->CreateLabel(true,0);
	x86_Label* p4_handler = x86e->CreateLabel(false,0);
	//Ram Only Mem Lookup
	roml(reg_addr,p4_handler,&old_offset,fast_reg,fast_reg_offset);

	//mov to dest or temp
	u32 is_float=IsInFReg(op->reg1);
	x86_reg destreg;
	if (size==FLAG_64)
	{
		destreg=XMM0;
	}
	else
	{
		if (is_float)
		{
			destreg=EAX;
		}
		else
		{
			destreg=LoadReg_nodata(EAX,op->reg1);
		}
	}
	x86e->Emit(rm_table[size],destreg,x86_mrm(ECX,sh4_reserved_mem));
	roml_patch t;
	
	if (size==FLAG_64)
	{
		x86e->Emit(op_movlps,GetRegPtr(GetSingleFromDouble((u8)op->reg1)),XMM0);
		t.exit_point=x86e->CreateLabel(true,0);
	}
	else
	{
		t.exit_point=x86e->CreateLabel(true,0);
		if (is_float)
		{
			fra->SaveRegisterGPR(op->reg1,destreg);
		}
		else
		{
			SaveReg(op->reg1,destreg);
		}
	}

	t.p4_access=p4_handler;
	t.resume_offset=(u8)old_offset;
	t.asz=size;
	t.type=0;
	t.is_float=false;
	t.reg_addr=reg_addr;
	if (size!=FLAG_64)
	{
		t.reg_data=destreg;
	}
	else
		t.reg_data=(x86_reg)GetSingleFromDouble((u8)op->reg1);

	//emit_vmem_read(reg_addr,op->reg1,m_unpack_sz[size]);
	roml_patch_list.push_back(t);
}
const x86_opcode_class wm_table[4]={op_mov8,op_mov16,op_mov32,op_movlps};
void __fastcall shil_compile_writem(shil_opcode* op)
{
	//sse_WBF(op->reg1);//Write back possibly readed reg
	u32 size=op->flags&3;
	u32 is_float=IsInFReg(op->reg1);
	u32 was_float=is_float;

	x86_reg rsrc;
	if (size==FLAG_64)
	{
		u8 f32reg=GetSingleFromDouble((u8)op->reg1);
		x86e->Emit(op_movlps,XMM0,GetRegPtr(f32reg));
		rsrc=XMM0;
	}
	else
	{
		if (!is_float)
		{
			rsrc=LoadReg(EDX,op->reg1);
			if (size==0)
			{
				if (rsrc>BL)
				{
					x86e->Emit(op_mov32,EDX,rsrc);
					rsrc=EDX;
				}
			}
		}
		else
		{
			if (fra->IsRegAllocated(op->reg1))
			{
				rsrc=fra->GetRegister(XMM0,op->reg1,RA_DEFAULT);
			}
			else
			{
				rsrc=EDX;
				x86e->Emit(op_mov32,EDX,GetRegPtr(op->reg1));
				was_float=0;
			}
		}
	}

	//if constant read , and on ram area , make it a direct mem access
	//_watch_ mmu
	if (!(op->flags & (FLAG_R0|FLAG_GBR|FLAG_REG2)))
	{//[reg2+imm] form
		assert(op->flags & FLAG_IMM1);
		//[imm1] form
		/*if (!is_float)
		{
			emit_vmem_op_compat_const(x86e,op->imm1,rsrc,false,m_unpack_sz[size],1);
		}
		else
		{
			emit_vmem_op_compat_const(x86e,op->imm1,rsrc,true,m_unpack_sz[size],1);
		}*/
		emit_vmem_op_compat_const(x86e,1,size,op->imm1,op->reg1);
		return;
	}

	u32 old_offset=x86e->x86_indx;
	x86_reg fast_reg;
	u32 fast_reg_offset;
	x86_reg reg_addr = readwrteparams(op,&fast_reg,&fast_reg_offset);
	/*
	x86e->Emit(op_mov32,EAX,ECX);
	x86e->Emit(op_shr32,EAX,29);
	x86e->Emit(op_call32,x86_mrm(NO_REG,EAX,sib_scale_4,&mio_pvt[1][size][0][0]));
	*/
	old_offset=x86e->x86_indx-old_offset;
	
	//x86_Label* patch_point= x86e->CreateLabel(true,0);
	x86_Label* p4_handler = x86e->CreateLabel(false,0);
	//Ram Only Mem Lookup
	roml(reg_addr,p4_handler,&old_offset,fast_reg,fast_reg_offset);
	//mov [ecx],src
	if (was_float)
		x86e->Emit(op_movss,x86_mrm(ECX,sh4_reserved_mem),rsrc);
	else
	{
		x86e->Emit(wm_table[size],x86_mrm(ECX,sh4_reserved_mem),rsrc);
	}
	//if  (is_float)
	//x86e->Emit(op_jmp,p4_handler);

	roml_patch t;
	t.p4_access=p4_handler;
	t.resume_offset=(u8)old_offset;
	t.exit_point=x86e->CreateLabel(true,0);
	t.asz=size;
	t.type=1;
	t.is_float=was_float != 0;
	t.reg_addr=reg_addr;
	if (size!=FLAG_64)
	{
		t.reg_data=rsrc;
	}
	else
		t.reg_data=(x86_reg)GetSingleFromDouble((u8)op->reg1);

	roml_patch_list.push_back(t);
	
	//emit_vmem_write(reg_addr,op->reg1,m_unpack_sz[size]);
}
void* nvw_lut[4]={WriteMem8,WriteMem16,WriteMem32,WriteMem32};
void* nvr_lut[4]={ReadMem8,ReadMem16,ReadMem32,ReadMem32};
#include "dc/mem/sh4_internal_reg.h"
void apply_roml_patches()
{
	for (u32 i=0;i<roml_patch_list.size();i++)
	{
		void * function=roml_patch_list[i].type==1 ?nvw_lut[roml_patch_list[i].asz]:nvr_lut[roml_patch_list[i].asz];

		u32 offset=x86e->x86_indx;
		x86e->write8(0);
		x86e->write8(roml_patch_list[i].resume_offset);
		//log("Resume offset: %d\n",roml_patch_list[i].resume_offset);
		x86e->MarkLabel(roml_patch_list[i].p4_access);
		if (roml_patch_list[i].type==1 && (roml_patch_list[i].asz>=FLAG_32))
		{
			//check for SQ write
			x86e->Emit(op_cmp32,roml_patch_list[i].reg_addr,0xE3FFFFFF);
			
			if (roml_patch_list[i].reg_addr!=ECX)
				x86e->Emit(op_mov32,ECX,roml_patch_list[i].reg_addr);

			x86_Label* normal_write=x86e->CreateLabel(false,8);
			x86e->Emit(op_ja,normal_write);
			//x86e->Emit(op_int3);
			x86e->Emit(op_and32,ECX,0x3C);
			if (FLAG_32==roml_patch_list[i].asz)
			{
				x86e->Emit(op_mov32,x86_mrm(ECX,sq_both),roml_patch_list[i].reg_data);
			}
			else
			{
				//x86e->Emit(op_int3);
				x86e->Emit(op_movlps,x86_mrm(ECX,sq_both),XMM0);//allready readed on xmm0
			}
			x86e->Emit(op_jmp,roml_patch_list[i].exit_point);
			x86e->MarkLabel(normal_write);
			*(u8*)&x86e->x86_buff[offset]=(u8)( (u32)(x86e->x86_indx-offset-2) );
			//log("patch offset: %d\n",x86e->x86_indx-offset-2);
		}
		else
		{
			if (roml_patch_list[i].reg_addr!=ECX)
				x86e->Emit(op_mov32,ECX,roml_patch_list[i].reg_addr);
		}

		if (roml_patch_list[i].asz!=FLAG_64)
		{
			if (roml_patch_list[i].is_float)
			{
				//meh ?
				dbgbreak;
			}
			else
			{
				if (roml_patch_list[i].type==1)
				{
					//if write make sure data is on edx
					if (roml_patch_list[i].reg_data!=EDX)
						x86e->Emit(op_mov32,EDX,roml_patch_list[i].reg_data);
				}
			}

			x86e->Emit(op_call,x86_ptr_imm(function));
			if (roml_patch_list[i].type==0)
			{
				if (roml_patch_list[i].asz==0)
					x86e->Emit(op_movsx8to32,roml_patch_list[i].reg_data,EAX);
				else if (roml_patch_list[i].asz==1)
					x86e->Emit(op_movsx16to32,roml_patch_list[i].reg_data,EAX);
				else if (roml_patch_list[i].asz==2)
				{
					if (roml_patch_list[i].reg_data!=EAX)
						x86e->Emit(op_mov32,roml_patch_list[i].reg_data,EAX);
				}
			}
		}
		else
		{
			//if (roml_patch_list[i].type==0)
			//	x86e->Emit(op_int3);
			//save address once
			x86e->Emit(op_push32,ECX);
			
			x86e->Emit(op_add32,ECX,4);
			u32* target=GetRegPtr(roml_patch_list[i].reg_data);

			for (int j=1;j>=0;j--)
			{
				if (roml_patch_list[i].type==1)
				{
					//write , need data on EDX
					x86e->Emit(op_mov32,EDX,&target[j]);
				}
				x86e->Emit(op_call,x86_ptr_imm(function));
				if (roml_patch_list[i].type==0)
				{
					//read, save data from eax
					x86e->Emit(op_mov32,&target[j],EAX);
				}

				if (j==1)
					x86e->Emit(op_pop32,ECX);//get the 'low' address
			}

		}
		x86e->Emit(op_jmp,roml_patch_list[i].exit_point);
		//x86e->MarkLabel(roml_patch_list[i].p4_access);
		
		//roml_patch_list[i].reg_data;
		//emit_vmem_write(roml_patch_list[i].reg_addr,
	}
	roml_patch_list.clear();
}
//save-loadT
void __fastcall shil_compile_SaveT(shil_opcode* op)
{
	assert(op->flags & FLAG_IMM1);//imm1
	assert(0==(op->flags & (FLAG_IMM2|FLAG_REG1|FLAG_REG2)));//no imm2/r1/r2

	//x86e->Emit(SetCC[op->imm1],x86_ptr(GetRegPtr(reg_sr_T)));	 -> LOADS slower
	//strange .. anyway :p
	if (op->imm1==CC_FPU_E)
	{
		//special case
		//We want to take in account the 'unordered' case on the fpu
		x86e->Emit(op_lahf);
		x86e->Emit(op_test8,AH,0x44);
		x86e->Emit(op_setnp,EAX);
		//x86e->Emit(op_sete,EAX); old code :)
	}
	else
	{
		x86e->Emit(SetCC[op->imm1],EAX);
	}

	//meh , it just LOVES that way more :P
	x86e->Emit(op_movzx8to32, EAX,EAX);				//zero out rest of eax
	x86e->Emit(op_mov32,GetRegPtr(reg_sr_T),EAX);
	
	//x86e->Emit(op_mov8,GetRegPtr(reg_sr_T),EAX);
	//x86e->Emit(op_xor32,EAX,EAX);

}
void __fastcall shil_compile_LoadT(shil_opcode* op)
{
	assert(op->flags & FLAG_IMM1);//imm1
	assert(0==(op->flags & (FLAG_IMM2|FLAG_REG1|FLAG_REG2)));//no imm2/r1/r2
	

	assert( (op->imm1==x86_flags::CF) || (op->imm1==x86_flags::jcond_flag) );

	if (op->imm1==jcond_flag)
	{
		LoadReg_force(EAX,reg_sr_T);
		x86e->Emit(op_mov32,&T_jcond_value,EAX);//T_jcond_value;
	}
	else
	{
		//LoadReg_force(EAX,reg_sr_T);
		//x86e->Emit(op_shr32,EAX,1);//heh T bit is there now :P CF
		x86e->Emit(op_bt32,GetRegPtr(reg_sr_T),0);
	}
}
//cmp-test
void __fastcall shil_compile_cmp(shil_opcode* op)
{
	assert(FLAG_32==(op->flags & 3));
	if (op->flags & FLAG_IMM1)
	{
		assert(0==(op->flags & (FLAG_REG2|FLAG_IMM2)));
		if (ira->IsRegAllocated(op->reg1))
		{
			x86_gpr_reg r1 = LoadReg(EAX,op->reg1);
			//if (op->imm1)
				x86e->Emit(op_cmp32,r1,op->imm1);
			/*else
				x86e->Emit(op_test32,r1,r1);*/
		}
		else
		{
			x86e->Emit(op_cmp32,GetRegPtr(op->reg1),op->imm1);
		}
		//eflags is used w/ combination of SaveT
	}
	else
	{
		assert(0==(op->flags & FLAG_IMM2));
		assert(op->flags & FLAG_REG2);

		x86_gpr_reg r1 = LoadReg(EAX,op->reg1);
		if (ira->IsRegAllocated(op->reg2))
		{
			x86_gpr_reg r2 = LoadReg(ECX,op->reg2);
			x86e->Emit(op_cmp32,r1,r2);//rm,rn
		}
		else
		{
			x86e->Emit(op_cmp32,r1,GetRegPtr(op->reg2));//rm,rn
		}
		//eflags is used w/ combination of SaveT
	}
}
void __fastcall shil_compile_test(shil_opcode* op)
{
	assert(FLAG_32==(op->flags & 3));
	if (op->flags & FLAG_IMM1)
	{
		assert(0==(op->flags & (FLAG_REG2|FLAG_IMM2)));
		if (ira->IsRegAllocated(op->reg1))
		{
			x86_gpr_reg r1 = LoadReg(EAX,op->reg1);
			x86e->Emit(op_test32,r1,op->imm1);
		}
		else
		{
			x86e->Emit(op_test32,GetRegPtr(op->reg1),op->imm1);
		}
		//eflags is used w/ combination of SaveT
	}
	else
	{
		assert(0==(op->flags & FLAG_IMM2));
		assert(op->flags & FLAG_REG2);

		x86_gpr_reg r1 = LoadReg(EAX,op->reg1);
		if (ira->IsRegAllocated(op->reg2))
		{
			x86_gpr_reg r2 = LoadReg(ECX,op->reg2);
			x86e->Emit(op_test32,r1,r2);//rm,rn
		}
		else
		{
			x86e->Emit(op_test32,r1,GetRegPtr(op->reg2));//rm,rn
		}
		//eflags is used w/ combination of SaveT
	}
}

//add-sub
void __fastcall shil_compile_add(shil_opcode* op)
{
	op_reg_to_reg(op,op_add32);
}
void __fastcall shil_compile_adc(shil_opcode* op)
{
	op_reg_to_reg(op,op_adc32);
}
void __fastcall shil_compile_sub(shil_opcode* op)
{
	op_reg_to_reg(op,op_sub32);
}

//left over from older code
void __fastcall shil_compile_jcond(shil_opcode* op)
{
	log("jcond ... heh not implemented\n");
	assert(false);
}
void __fastcall shil_compile_jmp(shil_opcode* op)
{
	log("jmp ... heh not implemented\n");
}
//helpers for mul
void load_with_se16(x86_gpr_reg to,u8 from)
{
	if (ira->IsRegAllocated(from))
	{
		x86_gpr_reg r1=LoadReg(EAX,from);
		x86e->Emit(op_movsx16to32, to,r1);
	}
	else
		x86e->Emit(op_movsx16to32, to,(u16*)GetRegPtr(from));
}

void load_with_ze16(x86_gpr_reg to,u8 from)
{
	if (ira->IsRegAllocated(from))
	{
		x86_gpr_reg r1=LoadReg(EAX,from);
		x86e->Emit(op_movzx16to32, to,r1);
	}
	else
		x86e->Emit(op_movzx16to32, to,(u16*)GetRegPtr(from));
}
//mul16/32/64 reg,reg
void __fastcall shil_compile_mul(shil_opcode* op)
{
	u32 sz=op->flags&3;

	assert(sz!=FLAG_8);//nope , can't be 16 bit..

	if (sz==FLAG_64)//mach is olny used on 64b version
		assert(op->flags & FLAG_MACH);
	else
		assert(0==(op->flags & FLAG_MACH));


	if (sz!=FLAG_64)
	{
		x86_gpr_reg r1,r2;
		if (sz==FLAG_16)
		{
			//FlushRegCache_reg(op->reg1);
			//FlushRegCache_reg(op->reg2);

			if (op->flags & FLAG_SX)
			{
				//x86e->Emit(op_movsx16to32, EAX,(u16*)GetRegPtr(op->reg1));
				load_with_se16(EAX,(u8)op->reg1);
				//x86e->Emit(op_movsx16to32, ECX,(u16*)GetRegPtr(op->reg2));
				load_with_se16(ECX,(u8)op->reg2);
			}
			else
			{
				//x86e->Emit(op_movzx16to32, EAX,(u16*)GetRegPtr(op->reg1));
				load_with_ze16(EAX,(u8)op->reg1);
				//x86e->Emit(op_movzx16to32, ECX,(u16*)GetRegPtr(op->reg2));
				load_with_ze16(ECX,(u8)op->reg2);
			}
		}
		else
		{
			//x86e->Emit(op_mov32,EAX,GetRegPtr(op->reg1));
			//x86e->Emit(op_mov32,ECX,GetRegPtr(op->reg2));
			r1=LoadReg_force(EAX,op->reg1);
			r2=LoadReg_force(ECX,op->reg2);
		}

		if (op->flags & FLAG_SX)
			x86e->Emit(op_imul32,EAX,ECX);
		else
			x86e->Emit(op_mul32,ECX);
		
		SaveReg((u8)reg_macl,EAX);
	}
	else
	{
		assert(sz==FLAG_64);
		
		ira->FlushRegister(op->reg1);
		ira->FlushRegister(op->reg2);

		x86e->Emit(op_mov32,EAX,GetRegPtr(op->reg1));

		if (op->flags & FLAG_SX)
			x86e->Emit(op_imul32,x86_ptr(GetRegPtr(op->reg2)));
		else
			x86e->Emit(op_mul32,x86_ptr(GetRegPtr(op->reg2)));

		SaveReg((u8)reg_macl,EAX);
		SaveReg((u8)reg_mach,EDX);
	}
}
void FASTCALL sh4_div0u();
void FASTCALL sh4_div0s(u32 rn,u32 rm);
u32 FASTCALL sh4_rotcl(u32 rn);
u32 FASTCALL sh4_div1(u32 rn,u32 rm);
template<bool sgn>
u64 FASTCALL shil_helper_slowdiv32(u32 r3,u32 r2, u32 r1)
{
	if (sgn)
		sh4_div0s(r2,r3);
	else
		sh4_div0u();

	for (int i=0;i<32;i++)
	{
		r1=sh4_rotcl(r1);
		r2=sh4_div1(r2,r3);
	}

	return ((u64)r3<<32)|r1;//EAX=r1, EDX=r3 
}
void __fastcall shil_compile_div32(shil_opcode* op)
{
	assert(0==(op->flags & (FLAG_IMM2)));

	//x86e->Emit(op_int3);
	u8 rQuotient=(u8)op->reg1;
	u8 rDivisor=(u8)op->reg2;
	u8 rDividend=(u8)op->imm1;
	//Q=Dend/Dsor

	//make sure that the sign extention is correct
	x86_gpr_reg Quotient=LoadReg_force(EAX,rQuotient);
	
	 if (op->flags & FLAG_SX)
		x86e->Emit(op_cdq);
	else
		x86e->Emit(op_xor32,EDX,EDX);

	x86_gpr_reg Dividend=LoadReg_force(ECX,rDividend);
	
	x86e->Emit(op_cmp32,EDX,ECX);
	
	x86_gpr_reg Divisor=LoadReg(EDX,rDivisor);
	
	x86_Label* slowdiv=x86e->CreateLabel(false,8);
	x86_Label* fastdiv=x86e->CreateLabel(false,8);
	x86_Label* exit =x86e->CreateLabel(false,8);

	//CDQ(rQuotient)!=rDividend  || rDividend!=0 ? (on input this should happen)
	x86e->Emit(op_jne,slowdiv);
	
	//make sure its not divide by 0
	x86e->Emit(op_test32,Divisor,Divisor);
	//EDX==0 ?
	x86e->Emit(op_jz,slowdiv);
	
	//all was ok, do normal divition !
	x86e->Emit(op_jmp,fastdiv);
	
	//something went wrong
	//slowdiv:
	x86e->MarkLabel(slowdiv);
	//push the 3rd param
	x86e->Emit(op_push32,EAX);
	//call the slow div (full sh4 impl)
	x86e->Emit(op_call,x86_ptr_imm( (op->flags & FLAG_SX) ? shil_helper_slowdiv32<true> : shil_helper_slowdiv32<false>));
	//goto enddiv;
	x86e->Emit(op_jmp,exit);

	//fast divition
	//fastdiv:
	x86e->MarkLabel(fastdiv);
	if (Divisor==EDX)	//we can't have it there ..
	{
		x86e->Emit(op_xchg32,ECX,EDX);
		Divisor=ECX;
		Dividend=EDX;
	}
	else
	{
		Dividend=EDX;//its the same value if its here ...
	}

	if (op->flags & FLAG_SX)
	{
		x86e->Emit(op_idiv32,Divisor);
	}
	else
	{
		x86e->Emit(op_div32,Divisor);
	}


	if (op->flags & FLAG_SX)
	{
		x86e->Emit(op_sar32 ,EAX);
	}
	else
	{
		x86e->Emit(op_shr32 ,EAX);
	}

	//set T
	//Set byte if below (CF=1)
	x86e->Emit(op_setb,x86_ptr(GetRegPtr(reg_sr_T)));


	//WARNING--JUMP--

	x86e->Emit(op_jb ,exit);

	if (ira->IsRegAllocated(rDivisor))
	{	//safe to do here b/c rDivisor was loaded to reg above (if reg cached)
		x86_gpr_reg t=LoadReg(EAX,rDivisor);
		x86e->Emit(op_sub32 ,EDX,t);
	}
	else
	{
		x86e->Emit(op_sub32 ,EDX,GetRegPtr(rDivisor));
	}

	//WARNING--JUMP--

	x86e->MarkLabel(exit);

	SaveReg(rDividend,Dividend);
	SaveReg(rQuotient,Quotient);
}




//Fpu alloc helpers
#define fa_r1r2 (1|2)
#define fa_r1m2 (1|0)
#define fa_m1r2 (0|2)
#define fa_m1m2 (0|0)

#define frs(op) (IsInFReg(op->reg1)|(IsInFReg(op->reg2)<<1))

//Fpu consts
#define pi (3.14159265f)

__declspec(align(16)) u32 ps_not_data[4]={0x80000000,0x80000000,0x80000000,0x80000000};
__declspec(align(16)) u32 ps_and_data[4]={0x7FFFFFFF,0x7FFFFFFF,0x7FFFFFFF,0x7FFFFFFF};

__declspec(align(16)) float mm_1[4]={1.0f,1.0f,1.0f,1.0f};
//__declspec(align(16)) float fsca_fpul_adj[4]={((2*pi)/65536.0f),((2*pi)/65536.0f),((2*pi)/65536.0f),((2*pi)/65536.0f)};

void sse_reg_to_reg(shil_opcode* op,x86_opcode_class op_cl)
{
	switch (frs(op))
	{
	case fa_r1r2:
		{
			x86_sse_reg r1=fra->GetRegister(XMM0,op->reg1,RA_DEFAULT);
			x86_sse_reg r2=fra->GetRegister(XMM0,op->reg2,RA_DEFAULT);
			assert(r1!=XMM0 && r2!=XMM0);
			x86e->Emit(op_cl,r1,r2);
			fra->SaveRegister(op->reg1,r1);
		}
		break;
	case fa_r1m2:
		{
			x86_sse_reg r1=fra->GetRegister(XMM0,op->reg1,RA_DEFAULT);
			assert(r1!=XMM0);
			x86e->Emit(op_cl,r1,x86_ptr(GetRegPtr(op->reg2)));
			fra->SaveRegister(op->reg1,r1);
		}
		break;
	case fa_m1r2:
		{
			x86_sse_reg r1=fra->GetRegister(XMM0,op->reg1,RA_DEFAULT);
			x86_sse_reg r2=fra->GetRegister(XMM0,op->reg2,RA_DEFAULT);
			assert(r1==XMM0);
			assert(r2!=XMM0);
			x86e->Emit(op_cl,r1,r2);
			fra->SaveRegister(op->reg1,r1);
		}
		break;
	case fa_m1m2:
		{
			x86_sse_reg r1=fra->GetRegister(XMM0,op->reg1,RA_DEFAULT);
			assert(r1==XMM0);
			x86e->Emit(op_cl,r1,x86_ptr(GetRegPtr(op->reg2)));
			fra->SaveRegister(op->reg1,r1);
		}
		break;
	}
}

//#define SSE_s(op,sseop) sse_reg_to_reg(op,sseop);
//simple opcodes
void __fastcall shil_compile_fadd(shil_opcode* op)
{
	assert(0==(op->flags & (FLAG_IMM1|FLAG_IMM2)));
	u32 sz=op->flags & 3;
	if (sz==FLAG_32)
	{
		assert(!IsReg64((Sh4RegType)op->reg1));
		assert(Ensure32());

		sse_reg_to_reg(op,op_addss);
	}
	else
	{
		assert(false);
	}
}
void __fastcall shil_compile_fsub(shil_opcode* op)
{
	assert(0==(op->flags & (FLAG_IMM1|FLAG_IMM2)));
	u32 sz=op->flags & 3;
	if (sz==FLAG_32)
	{
		assert(!IsReg64((Sh4RegType)op->reg1));
		assert(Ensure32());
		
		sse_reg_to_reg(op,op_subss);
	}
	else
	{
		assert(false);
	}
}

void __fastcall shil_compile_fmul(shil_opcode* op)
{
	assert(0==(op->flags & (FLAG_IMM1|FLAG_IMM2)));
	u32 sz=op->flags & 3;
	if (sz==FLAG_32)
	{
		assert(!IsReg64((Sh4RegType)op->reg1));
		assert(Ensure32());

		sse_reg_to_reg(op,op_mulss);
	}
	else
	{
		assert(false);
	}
}

void __fastcall shil_compile_fdiv(shil_opcode* op)
{
	assert(0==(op->flags & (FLAG_IMM1|FLAG_IMM2)));
	u32 sz=op->flags & 3;
	if (sz==FLAG_32)
	{
		assert(!IsReg64((Sh4RegType)op->reg1));
		assert(Ensure32());

		sse_reg_to_reg(op,op_divss);
	}
	else
	{
		assert(false);
	}
}

//binary opcodes
void __fastcall shil_compile_fneg(shil_opcode* op)
{
	assert(0==(op->flags & (FLAG_IMM1|FLAG_IMM2|FLAG_REG2)));
	u32 sz=op->flags & 3;
	if (sz==FLAG_32)
	{
		assert(!IsReg64 ((Sh4RegType)op->reg1));
		if (IsInFReg(op->reg1))
		{
			x86_sse_reg r1=fra->GetRegister(XMM0,op->reg1,RA_DEFAULT);
			assert(r1!=XMM0);
			x86e->Emit(op_xorps,r1,ps_not_data);
			fra->SaveRegister(op->reg1,r1);
		}
		else
		{
			x86e->Emit(op_xor32,GetRegPtr(op->reg1),0x80000000);
		}
	}
	else
	{
		assert(sz==FLAG_64);
		assert(IsReg64((Sh4RegType)op->reg1));
		u32 reg=GetSingleFromDouble((u8)op->reg1);
		x86e->Emit(op_xor32,GetRegPtr(reg+0),0x80000000);
	}
}

void __fastcall shil_compile_fabs(shil_opcode* op)
{
	assert(0==(op->flags & (FLAG_IMM1|FLAG_IMM2|FLAG_REG2)));
	u32 sz=op->flags & 3;
	if (sz==FLAG_32)
	{
		assert(!IsReg64((Sh4RegType)op->reg1));
		if (IsInFReg(op->reg1))
		{
			x86_sse_reg r1=fra->GetRegister(XMM0,op->reg1,RA_DEFAULT);
			assert(r1!=XMM0);
			x86e->Emit(op_andps,r1,ps_and_data);
			fra->SaveRegister(op->reg1,r1);
		}
		else
		{
			x86e->Emit(op_and32,GetRegPtr(op->reg1),0x7FFFFFFF);
		}
	}
	else
	{
		assert(sz==FLAG_64);
		assert(IsReg64((Sh4RegType)op->reg1));
		u32 reg=GetSingleFromDouble((u8)op->reg1);
		x86e->Emit(op_and32,GetRegPtr(reg),0x7FFFFFFF);
	}
}

//pref !
void __fastcall do_pref(u32 Dest);
void __fastcall shil_compile_pref(shil_opcode* op)
{
	assert(0==(op->flags & (FLAG_REG2|FLAG_IMM2)));
	assert(op->flags & (FLAG_REG1|FLAG_IMM1));
	
//	u32 sz=op->flags & 3;
//	assert(sz==FLAG_32);
	assert((op->flags & 3)==FLAG_32);

	if (op->flags&FLAG_REG1)
	{
		x86_reg raddr=LoadReg_force(ECX,op->reg1);
		x86e->Emit(op_mov32,EDX,ECX);
		x86e->Emit(op_and32,EDX,0xFC000000);
		x86e->Emit(op_cmp32,EDX,0xE0000000);

		x86_Label* after=x86e->CreateLabel(false,8);
		x86e->Emit(op_jne,after);
		x86e->Emit(op_call,x86_ptr_imm(do_pref));
		x86e->MarkLabel(after);
	}
	else if (op->flags&FLAG_IMM1)
	{
		if((op->imm1&0xFC000000)==0xE0000000)
		{
			x86e->Emit(op_mov32,ECX,op->imm1);
			x86e->Emit(op_call,x86_ptr_imm(do_pref));
		}
	}
}

//complex opcodes
void __fastcall shil_compile_fcmp(shil_opcode* op)
{
	assert(0==(op->flags & (FLAG_IMM1|FLAG_IMM2)));
	u32 sz=op->flags & 3;
	if (sz==FLAG_32)
	{
		assert(!IsReg64((Sh4RegType)op->reg1));
		assert(Ensure32());

		//x86e->Emit(op_movss,XMM0,GetRegPtr(op->reg1));
		x86_sse_reg fr1=fra->GetRegister(XMM0,op->reg1,RA_DEFAULT);
		
		//x86e->SSE_UCOMISS_M32_to_XMM(XMM0,GetRegPtr(op->reg2));
		if (fra->IsRegAllocated(op->reg2))
		{
			x86_sse_reg fr2=fra->GetRegister(XMM0,op->reg2,RA_DEFAULT);
			assert(fr2!=XMM0);
			x86e->Emit(op_ucomiss, fr1,fr2);
		}
		else
		{
			x86e->Emit(op_ucomiss ,fr1,GetRegPtr(op->reg2));
		}
	}
	else
	{
		assert(false);
	}
}

void __fastcall shil_compile_fmac(shil_opcode* op)
{
	assert(0==(op->flags & (FLAG_IMM1|FLAG_IMM2)));
	u32 sz=op->flags & 3;
	if (sz==FLAG_32)
	{
		assert(!IsReg64((Sh4RegType)op->reg1));
		//fr[n] += fr[0] * fr[m];
		assert(Ensure32());

		//x86e->Emit(op_movss,XMM0,GetRegPtr(fr_0));		//xmm0=fr[0]
		x86_sse_reg fr0=fra->GetRegister(XMM0,fr_0,RA_FORCE);
		assert(fr0==XMM0);
		
		//x86e->SSE_MULSS_M32_to_XMM(XMM0,GetRegPtr(op->reg2));	//xmm0*=fr[m]
		if (fra->IsRegAllocated(op->reg2))
		{
			x86_sse_reg frm=fra->GetRegister(XMM0,op->reg2,RA_DEFAULT);
			assert(frm!=XMM0);
			x86e->Emit(op_mulss ,fr0,frm);
		}
		else
		{
			x86e->Emit(op_mulss ,XMM0,GetRegPtr(op->reg2));
		}

		//x86e->SSE_ADDSS_M32_to_XMM(XMM0,GetRegPtr(op->reg1));	//xmm0+=fr[n] 
		if (fra->IsRegAllocated(op->reg1))
		{
			x86_sse_reg frn=fra->GetRegister(XMM0,op->reg1,RA_DEFAULT);
			assert(frn!=XMM0);
			x86e->Emit(op_addss ,fr0,frn);
		}
		else
		{
			x86e->Emit(op_addss ,fr0,GetRegPtr(op->reg1));
		}
		
		
		//x86e->Emit(op_movss,GetRegPtr(op->reg1),XMM0);	//fr[n]=xmm0
		fra->SaveRegister(op->reg1,fr0);
	}
	else
	{
		assert(false);
	}
}


void __fastcall shil_compile_fsqrt(shil_opcode* op)
{
	assert(0==(op->flags & (FLAG_IMM1|FLAG_IMM2|FLAG_REG2)));
	u32 sz=op->flags & 3;
	if (sz==FLAG_32)
	{
		assert(Ensure32());

		assert(!IsReg64((Sh4RegType)op->reg1));
		//RSQRT vs SQRTSS -- why rsqrt no workie ? :P -> RSQRT = 1/SQRTSS
		if (fra->IsRegAllocated(op->reg1))
		{
			x86_sse_reg fr1=fra->GetRegister(XMM0,op->reg1,RA_DEFAULT);
			assert(fr1!=XMM0);
			x86e->Emit(op_sqrtss ,fr1,fr1);
			fra->SaveRegister(op->reg1,fr1);
		}
		else
		{
			x86e->Emit(op_sqrtss ,XMM0,GetRegPtr(op->reg1));
			x86e->Emit(op_movss,GetRegPtr(op->reg1),XMM0);
		}
		
	}
	else
	{
		assert(false);
	}
}
void __fastcall shil_compile_fsrra(shil_opcode* op)
{
	assert(0==(op->flags & (FLAG_IMM1|FLAG_IMM2|FLAG_REG2)));
	u32 sz=op->flags & 3;
	if (sz==FLAG_32)
	{
		assert(Ensure32());
		assert(!IsReg64((Sh4RegType)op->reg1));
		//maby need to calculate 1/sqrt manualy ? -> yes , it seems rcp is not as accurate as needed :)
		//-> no , it wasn that , rcp=1/x , RSQRT=1/srqt tho

		//x86e->SSE_SQRTSS_M32_to_XMM(XMM1,GetRegPtr(op->reg1));	//XMM1=sqrt
		//x86e->Emit(op_movss,XMM0,(u32*)mm_1);			//XMM0=1
		//x86e->SSE_DIVSS_XMM_to_XMM(XMM0,XMM1);					//XMM0=1/sqrt
		//or
		//x86e->SSE_RSQRTSS_M32_to_XMM(XMM0,GetRegPtr(op->reg1));//XMM0=APPR(1/sqrt(fr1))
		//-> im using Approximate version , since this is an aproximate opcode on sh4 too
		//i hope x86 isnt less accurate ..

		if (fra->IsRegAllocated(op->reg1))
		{
			x86_sse_reg fr1=fra->GetRegister(XMM0,op->reg1,RA_DEFAULT);
			verify(fr1!=XMM0);
#ifdef _FAST_fssra
			x86e->SSE_RSQRTSS_XMM_to_XMM(fr1,fr1);
#else
			//fra->FlushRegister_xmm(XMM7);
			x86e->Emit(op_sqrtss ,XMM0,fr1);				//XMM0=sqrt(fr1)
			x86e->Emit(op_movss,fr1,(u32*)mm_1);			//fr1=1
			x86e->Emit(op_divss,fr1,XMM0);				//fr1=1/XMM0
#endif
			fra->SaveRegister(op->reg1,fr1);
		}
		else
		{
			#ifdef _FAST_fssra
			x86e->SSE_RSQRTSS_M32_to_XMM(XMM0,GetRegPtr(op->reg1));//XMM0=APPR(1/sqrt(fr1))
			#else
			fra->FlushRegister_xmm(XMM7);
			x86e->Emit(op_sqrtss ,XMM7,GetRegPtr(op->reg1));	//XMM7=sqrt(fr1)
			x86e->Emit(op_movss ,XMM0,(u32*)mm_1);			//XMM0=1
			x86e->Emit(op_divss ,XMM0,XMM7);					//XMM0=1/XMM7
			#endif
			x86e->Emit(op_movss ,GetRegPtr(op->reg1),XMM0);	//fr1=XMM0
		}
		
	}
	else
	{
		assert(false);
	}
}

void __fastcall shil_compile_floatfpul(shil_opcode* op)
{
	assert(0==(op->flags & (FLAG_IMM1|FLAG_IMM2|FLAG_REG2)));
	u32 sz=op->flags & 3;
	if (sz==FLAG_32)
	{
		assert(Ensure32());
		assert(!IsReg64((Sh4RegType)op->reg1));

		//TODO : This is not entietly correct , sh4 rounds too [need to set MXCSR]
		x86_sse_reg r1=fra->GetRegister(XMM0,op->reg1,RA_NODATA);
		x86e->Emit(op_cvtsi2ss ,r1,GetRegPtr(reg_fpul));
		fra->SaveRegister(op->reg1,r1);
		
	}
	else
	{
		assert(false);
	}
}
f32 sse_ftrc_saturate=0x7FFFFFBF;//1.11111111111111111111111 << 31
void __fastcall shil_compile_ftrc(shil_opcode* op)
{
	assert(0==(op->flags & (FLAG_IMM1|FLAG_IMM2|FLAG_REG2)));
	u32 sz=op->flags & 3;
	if (sz==FLAG_32)
	{
		assert(Ensure32());
		assert(!IsReg64((Sh4RegType)op->reg1));

		//TODO : This is not entietly correct , sh4 saturates too -> its correct now._CHEAP_FTRC_FIX can be defined for a cheaper, but not 100% accurate version
		//EAX=(int)saturate(fr[n])

		x86_sse_reg r1=fra->GetRegister(XMM0,op->reg1,RA_FORCE);

#ifdef _CHEAP_FTRC_FIX 
		x86e->Emit(op_minss,r1,&sse_ftrc_saturate);
#endif
		x86e->Emit(op_cvttss2si, EAX,r1);

#ifndef _CHEAP_FTRC_FIX 
		x86_gpr_reg r2=ECX;
		fra->LoadRegisterGPR(r2,op->reg1);
		assert(r2!=EAX);
		x86e->Emit(op_shr32,r2,31);			//sign -> LSB
		x86e->Emit(op_add32,r2,0x7FFFFFFF);	//0x7FFFFFFF for +, 0x80000000 for -
		x86e->Emit(op_cmp32,EAX,0x80000000);//is result indefinitive ?
		x86e->Emit(op_cmove32,EAX,r2);		//if yes, saturate		
#endif
		//fpul=EAX
		SaveReg(reg_fpul,EAX);
	}
	else
	{
		assert(false);
	}
}

//Mixed opcodes (sse & x87)
void __fastcall shil_compile_fsca(shil_opcode* op)
{
	assert(0==(op->flags & (FLAG_IMM1|FLAG_IMM2|FLAG_REG2)));
	u32 sz=op->flags & 3;
	if (sz==FLAG_32)
	{
		assert(Ensure32());

		assert(!IsReg64((Sh4RegType)op->reg1));
		//float real_pi=(((float)(s32)fpul)/65536)*(2*pi);
		//real_pi=(s32)fpul * ((2*pi)/65536.0f);
		
		/*
		//that is the old way :P

		x86e->Emit(op_fild32i,x86_ptr(GetRegPtr(reg_fpul)));		//st(0)=(s32)fpul
		x86e->Emit(op_fmul32f,x86_ptr(fsca_fpul_adj));			//st(0)=(s32)fpul * ((2*pi)/65536.0f)
		x86e->Emit(op_fsincos);						//st(0)=sin , st(1)=cos
		
		x86e->Emit(op_fstp32f,x86_ptr(GetRegPtr(op->reg1 +1)));	//Store cos to reg+1
		x86e->Emit(op_fstp32f,x86_ptr(GetRegPtr(op->reg1)));		//store sin to reg

		fra->ReloadRegister(op->reg1+1);
		fra->ReloadRegister(op->reg1);
		*/


		///THIS IS THE NEW way :p
		/*
		u32 pi_index=fpul&0xFFFF;
		
		fr[n | 0] = sin_table[pi_index];
		fr[n | 1] = sin_table[(16384 + pi_index) & 0xFFFF];
		*/
		//x86e->Emit(op_int3);
		x86_sse_reg r1=fra->GetRegister(XMM0,op->reg1,RA_NODATA);	//to store sin
		x86_sse_reg r2=fra->GetRegister(XMM1,op->reg1+1,RA_NODATA);	//to store cos

		verify(!ira->IsRegAllocated(reg_fpul));
		x86e->Emit(op_movzx16to32,EAX,GetRegPtr(reg_fpul));			//we do the 'and' here :p
		x86e->Emit(op_movss,r1,x86_mrm(EAX,sib_scale_4,&sin_table[0])); //r1=sin
		
		//cos(x) = sin (pi/2 + x) , we add 1/4 of 2pi (2^16/4)
		x86e->Emit(op_movss,r2,x86_mrm(EAX,sib_scale_4,&sin_table[0x4000])); //r2=cos, table has 0x4000 more for warping :)

		fra->SaveRegister(op->reg1,r1);
		fra->SaveRegister(op->reg1 + 1,r2);
	}
	else
	{
		assert(false);
	}
}
//Vector opcodes ;)
void __fastcall shil_compile_ftrv(shil_opcode* op)
{
	assert(0==(op->flags & (FLAG_IMM1|FLAG_IMM2|FLAG_REG2)));
	u32 sz=op->flags & 3;
	if (sz==FLAG_32)
	{
		fra->FlushRegister_xmm(XMM0);
		fra->FlushRegister_xmm(XMM1);
		fra->FlushRegister_xmm(XMM2);
		fra->FlushRegister_xmm(XMM3);

		fra->FlushRegister(op->reg1);
		fra->FlushRegister(op->reg1+1);
		fra->FlushRegister(op->reg1+2);
		fra->FlushRegister(op->reg1+3);

		assert(!IsReg64((Sh4RegType)op->reg1));
		assert(Ensure32());


		if (x86_caps.sse_2)
		{
			x86e->Emit(op_movaps ,XMM3,GetRegPtr(op->reg1));	//xmm0=vector

			x86e->Emit(op_pshufd ,XMM0,XMM3,0);					//xmm0={v0}
			x86e->Emit(op_pshufd ,XMM1,XMM3,0x55);				//xmm1={v1}	
			x86e->Emit(op_pshufd ,XMM2,XMM3,0xaa);				//xmm2={v2}
			x86e->Emit(op_pshufd ,XMM3,XMM3,0xff);				//xmm3={v3}
		}
		else
		{
			x86e->Emit(op_movaps ,XMM0,GetRegPtr(op->reg1));	//xmm0=vector

			x86e->Emit(op_movaps ,XMM3,XMM0);					//xmm3=vector
			x86e->Emit(op_shufps ,XMM0,XMM0,0);					//xmm0={v0}
			x86e->Emit(op_movaps ,XMM1,XMM3);					//xmm1=vector
			x86e->Emit(op_movaps ,XMM2,XMM3);					//xmm2=vector
			x86e->Emit(op_shufps ,XMM3,XMM3,0xff);				//xmm3={v3}
			x86e->Emit(op_shufps ,XMM1,XMM1,0x55);				//xmm1={v1}	
			x86e->Emit(op_shufps ,XMM2,XMM2,0xaa);				//xmm2={v2}
		}

		x86e->Emit(op_mulps ,XMM0,GetRegPtr(xf_0));			//v0*=vm0
		x86e->Emit(op_mulps ,XMM1,GetRegPtr(xf_4));			//v1*=vm1
		x86e->Emit(op_mulps ,XMM2,GetRegPtr(xf_8));			//v2*=vm2
		x86e->Emit(op_mulps ,XMM3,GetRegPtr(xf_12));		//v3*=vm3

		x86e->Emit(op_addps ,XMM0,XMM1);					//sum it all up
		x86e->Emit(op_addps ,XMM2,XMM3);
		x86e->Emit(op_addps ,XMM0,XMM2);

		x86e->Emit(op_movaps ,GetRegPtr(op->reg1),XMM0);

		fra->ReloadRegister(op->reg1);
		fra->ReloadRegister(op->reg1+1);
		fra->ReloadRegister(op->reg1+2);
		fra->ReloadRegister(op->reg1+3);
	}
	else
	{
		assert(false);
	}
}

void __fastcall shil_compile_fipr(shil_opcode* op)
{
	assert(0==(op->flags & (FLAG_IMM1|FLAG_IMM2)));
	u32 sz=op->flags & 3;
	if (sz==FLAG_32)
	{
		assert(Ensure32());

		fra->FlushRegister_xmm(XMM0);
		fra->FlushRegister_xmm(XMM1);

		fra->FlushRegister(op->reg1);
		fra->FlushRegister(op->reg1+1);
		fra->FlushRegister(op->reg1+2);
		fra->FlushRegister(op->reg1+3);

		fra->FlushRegister(op->reg2);
		fra->FlushRegister(op->reg2+1);
		fra->FlushRegister(op->reg2+2);
		fra->FlushRegister(op->reg2+3);

		if (x86_caps.sse_3)
		{
			x86e->Emit(op_movaps ,XMM0,GetRegPtr(op->reg1));
			x86e->Emit(op_mulps ,XMM0,GetRegPtr(op->reg2));
												//xmm0={a0				,a1				,a2				,a3}
			x86e->Emit(op_haddps,XMM0,XMM0);	//xmm0={a0+a1			,a2+a3			,a0+a1			,a2+a3}
			x86e->Emit(op_haddps,XMM0,XMM0);	//xmm0={(a0+a1)+(a2+a3) ,(a0+a1)+(a2+a3),(a0+a1)+(a2+a3),(a0+a1)+(a2+a3)}

			x86e->Emit(op_movss,GetRegPtr(op->reg1+3),XMM0);
		}
		else
		{
			x86e->Emit(op_movaps ,XMM0,GetRegPtr(op->reg1));
			x86e->Emit(op_mulps ,XMM0,GetRegPtr(op->reg2));
			x86e->Emit(op_movhlps ,XMM1,XMM0);
			x86e->Emit(op_addps ,XMM0,XMM1);
			x86e->Emit(op_movaps ,XMM1,XMM0);
			x86e->Emit(op_shufps ,XMM1,XMM1,1);
			x86e->Emit(op_addss ,XMM0,XMM1);
			x86e->Emit(op_movss,GetRegPtr(op->reg1+3),XMM0);
		}
		fra->ReloadRegister(op->reg1+3);
	}
	else
	{
		assert(false);
	}
}

//decoding table ;)

shil_compileFP* sclt[shilop_count]=
{
	shil_compile_nimp,shil_compile_nimp,shil_compile_nimp,shil_compile_nimp,
	shil_compile_nimp,shil_compile_nimp,shil_compile_nimp,shil_compile_nimp,
	shil_compile_nimp,shil_compile_nimp,shil_compile_nimp,shil_compile_nimp,
	shil_compile_nimp,shil_compile_nimp,shil_compile_nimp,shil_compile_nimp,
	shil_compile_nimp,shil_compile_nimp,shil_compile_nimp,shil_compile_nimp,
	shil_compile_nimp,shil_compile_nimp,shil_compile_nimp,shil_compile_nimp,
	shil_compile_nimp,shil_compile_nimp,shil_compile_nimp,shil_compile_nimp,
	shil_compile_nimp,shil_compile_nimp,shil_compile_nimp,shil_compile_nimp,
	shil_compile_nimp,shil_compile_nimp,shil_compile_nimp,shil_compile_nimp,
	shil_compile_nimp,shil_compile_nimp,shil_compile_nimp,shil_compile_nimp,
	shil_compile_nimp,shil_compile_nimp,shil_compile_nimp,shil_compile_nimp,
	shil_compile_nimp
};

void SetH(shil_opcodes op,shil_compileFP* ha)
{
	if (op>(shilop_count-1))
	{
		log("SHIL COMPILER ERROR\n");
	}
	if (sclt[op]!=shil_compile_nimp)
	{
		log("SHIL COMPILER ERROR [hash table overwrite]\n");
	}

	sclt[op]=ha;
}
bool sclt_inited=false;
void sclt_Init()
{
	//
	SetH(shilop_adc,shil_compile_adc);
	SetH(shilop_add,shil_compile_add);
	SetH(shilop_and,shil_compile_and);
	SetH(shilop_cmp,shil_compile_cmp);
	SetH(shilop_fabs,shil_compile_fabs);
	SetH(shilop_fadd,shil_compile_fadd);
	SetH(shilop_fdiv,shil_compile_fdiv);
	SetH(shilop_fmac,shil_compile_fmac);

	SetH(shilop_fmul,shil_compile_fmul);
	SetH(shilop_fneg,shil_compile_fneg);
	SetH(shilop_fsub,shil_compile_fsub);
	SetH(shilop_LoadT,shil_compile_LoadT);
	SetH(shilop_mov,shil_compile_mov);
	SetH(shilop_movex,shil_compile_movex);
	SetH(shilop_neg,shil_compile_neg);
	SetH(shilop_not,shil_compile_not);

	SetH(shilop_or,shil_compile_or);
	SetH(shilop_rcl,shil_compile_rcl);
	SetH(shilop_rcr,shil_compile_rcr);
	SetH(shilop_readm,shil_compile_readm);
	SetH(shilop_rol,shil_compile_rol);
	SetH(shilop_ror,shil_compile_ror);
	SetH(shilop_sar,shil_compile_sar);
	SetH(shilop_SaveT,shil_compile_SaveT);

	SetH(shilop_ifb,shil_compile_shil_ifb);
	SetH(shilop_shl,shil_compile_shl);
	SetH(shilop_shr,shil_compile_shr);
	SetH(shilop_sub,shil_compile_sub);
	SetH(shilop_swap,shil_compile_swap);
	SetH(shilop_test,shil_compile_test);
	SetH(shilop_writem,shil_compile_writem);
	SetH(shilop_xor,shil_compile_xor);
	SetH(shilop_jcond,shil_compile_jcond);
	SetH(shilop_jmp,shil_compile_jmp);
	SetH(shilop_mul,shil_compile_mul);

	SetH(shilop_ftrv,shil_compile_ftrv);
	SetH(shilop_fsqrt,shil_compile_fsqrt);
	SetH(shilop_fipr,shil_compile_fipr);
	SetH(shilop_floatfpul,shil_compile_floatfpul);
	SetH(shilop_ftrc,shil_compile_ftrc);
	SetH(shilop_fsca,shil_compile_fsca);
	SetH(shilop_fsrra,shil_compile_fsrra);
	SetH(shilop_div32,shil_compile_div32);
	SetH(shilop_fcmp,shil_compile_fcmp);

	SetH(shilop_pref,shil_compile_pref);

	/*
	u32 shil_nimp=shilop_count;
	for (int i=0;i<shilop_count;i++)
	{
		if (sclt[i]==shil_compile_nimp)
			shil_nimp--;
	}

	//log("lazy shil compiler stats : %d%% opcodes done\n",shil_nimp*100/shilop_count);
	*/
}


void shil_compile(shil_opcode* op)
{
	if (op->opcode>(shilop_count-1))
	{
		log("SHIL COMPILER ERROR\n");
	}
	sclt[op->opcode](op);
}

void shil_compiler_init(x86_block* block,IntegerRegAllocator* _ira,FloatRegAllocator* _fra)
{
	if (!sclt_inited)
	{
		sclt_Init();
		sclt_inited=true;
	}

	fra = _fra;
	ira = _ira;
	x86e = block;
}