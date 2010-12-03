#include "shil_ce.h"
#include "dc/mem/sh4_mem.h"
#include "log/logging_interface.h"

u64 total_ops_removed=0;
void CompileBasicBlock_slow_c(BasicBlock* block,u32 pass);
u32 shil_optimise_pass_btp_main(BasicBlock* bb);

#define shilh(name) bool __fastcall shil_ce_##name(shil_opcode* op,BasicBlock* bb,shil_stream* il)

typedef shilh(FP);

shilh(nimp)
{
	die("Fatal error : Dynarec CE NIMP!");//lol ?
	return false;
}

shil_ce_FP* shil_ce_lut[shilop_count]=
{
	shil_ce_nimp,shil_ce_nimp,shil_ce_nimp,shil_ce_nimp,
	shil_ce_nimp,shil_ce_nimp,shil_ce_nimp,shil_ce_nimp,
	shil_ce_nimp,shil_ce_nimp,shil_ce_nimp,shil_ce_nimp,
	shil_ce_nimp,shil_ce_nimp,shil_ce_nimp,shil_ce_nimp,
	shil_ce_nimp,shil_ce_nimp,shil_ce_nimp,shil_ce_nimp,
	shil_ce_nimp,shil_ce_nimp,shil_ce_nimp,shil_ce_nimp,
	shil_ce_nimp,shil_ce_nimp,shil_ce_nimp,shil_ce_nimp,
	shil_ce_nimp,shil_ce_nimp,shil_ce_nimp,shil_ce_nimp,
	shil_ce_nimp,shil_ce_nimp,shil_ce_nimp,shil_ce_nimp,
	shil_ce_nimp,shil_ce_nimp,shil_ce_nimp,shil_ce_nimp,
	shil_ce_nimp,shil_ce_nimp,shil_ce_nimp,shil_ce_nimp,
	shil_ce_nimp
};
void SetShilHanlder(shil_opcodes op,shil_ce_FP* ha)
{
	if (op>(shilop_count-1))
	{
		log("SHIL COMPILER ERROR\n");
	}
	if (shil_ce_lut[op]!=shil_ce_nimp)
	{
		log("SHIL COMPILER ERROR [hash table overwrite]\n");
	}

	shil_ce_lut[op]=ha;
}
shilh(pref);
shilh(adc);
shilh(add);
shilh(and);
shilh(cmp);
shilh(fabs);
shilh(fadd);
shilh(fdiv);
shilh(fmac);

shilh(fmul);
shilh(fneg);
shilh(fsub);
shilh(LoadT);
shilh(mov);
shilh(movex);
shilh(neg);
shilh(not);

shilh(or);
shilh(rcl);
shilh(rcr);
shilh(readm);
shilh(rol);
shilh(ror);
shilh(sar);
shilh(SaveT);

shilh(shil_ifb);
shilh(shl);
shilh(shr);
shilh(sub);
shilh(swap);
shilh(test);
shilh(writem);
shilh(xor);
shilh(jcond);
shilh(jmp);
shilh(mul);

shilh(ftrv);
shilh(fsqrt);
shilh(fipr);
shilh(floatfpul);
shilh(ftrc);
shilh(fsca);
shilh(fsrra);
shilh(div32);
shilh(fcmp);

bool Inited_ce_pass=false;
struct RegData
{
	u32 RegValue;//Current reg value (if IsConstant is true)
	u32 RB_value;//last value writen back onto the register (exists olny of IsRB is true)

	bool IsConstant;
	bool IsRB;
};

RegData shil_ce_gpr[sh4_reg_count];
vector<u32> static_reads;
bool shil_ce_is_locked=true;
void Init_ce()
{
	memset(shil_ce_gpr,0,sizeof(shil_ce_gpr));
	
	if (Inited_ce_pass)
		return;

	Inited_ce_pass=true;
	//
	SetShilHanlder(shilop_pref,shil_ce_pref);
	SetShilHanlder(shilop_adc,shil_ce_adc);
	SetShilHanlder(shilop_add,shil_ce_add);
	SetShilHanlder(shilop_and,shil_ce_and);
	SetShilHanlder(shilop_cmp,shil_ce_cmp);
	SetShilHanlder(shilop_fabs,shil_ce_fabs);
	SetShilHanlder(shilop_fadd,shil_ce_fadd);
	SetShilHanlder(shilop_fdiv,shil_ce_fdiv);
	SetShilHanlder(shilop_fmac,shil_ce_fmac);

	SetShilHanlder(shilop_fmul,shil_ce_fmul);
	SetShilHanlder(shilop_fneg,shil_ce_fneg);
	SetShilHanlder(shilop_fsub,shil_ce_fsub);
	SetShilHanlder(shilop_LoadT,shil_ce_LoadT);
	SetShilHanlder(shilop_mov,shil_ce_mov);
	SetShilHanlder(shilop_movex,shil_ce_movex);
	SetShilHanlder(shilop_neg,shil_ce_neg);
	SetShilHanlder(shilop_not,shil_ce_not);

	SetShilHanlder(shilop_or,shil_ce_or);
	SetShilHanlder(shilop_rcl,shil_ce_rcl);
	SetShilHanlder(shilop_rcr,shil_ce_rcr);
	SetShilHanlder(shilop_readm,shil_ce_readm);
	SetShilHanlder(shilop_rol,shil_ce_rol);
	SetShilHanlder(shilop_ror,shil_ce_ror);
	SetShilHanlder(shilop_sar,shil_ce_sar);
	SetShilHanlder(shilop_SaveT,shil_ce_SaveT);

	SetShilHanlder(shilop_ifb,shil_ce_shil_ifb);
	SetShilHanlder(shilop_shl,shil_ce_shl);
	SetShilHanlder(shilop_shr,shil_ce_shr);
	SetShilHanlder(shilop_sub,shil_ce_sub);
	SetShilHanlder(shilop_swap,shil_ce_swap);
	SetShilHanlder(shilop_test,shil_ce_test);
	SetShilHanlder(shilop_writem,shil_ce_writem);
	SetShilHanlder(shilop_xor,shil_ce_xor);
	SetShilHanlder(shilop_jcond,shil_ce_jcond);
	SetShilHanlder(shilop_jmp,shil_ce_jmp);
	SetShilHanlder(shilop_mul,shil_ce_mul);

	SetShilHanlder(shilop_ftrv,shil_ce_ftrv);
	SetShilHanlder(shilop_fsqrt,shil_ce_fsqrt);
	SetShilHanlder(shilop_fipr,shil_ce_fipr);
	SetShilHanlder(shilop_floatfpul,shil_ce_floatfpul);
	SetShilHanlder(shilop_ftrc,shil_ce_ftrc);
	SetShilHanlder(shilop_fsca,shil_ce_fsca);
	SetShilHanlder(shilop_fsrra,shil_ce_fsrra);
	SetShilHanlder(shilop_div32,shil_ce_div32);
	SetShilHanlder(shilop_fcmp,shil_ce_fcmp);
}

void ce_die(char* reason)
{
	if (reason)
		log("C.E. pass : die [%s]\n",reason);
	else
		log("C.E. pass : die\n");

	__debugbreak();
}
bool ce_CanBeConst(u32 reg)
{
	return (reg<16) || (reg==reg_pc_temp)|| (reg==reg_macl) || (reg==reg_mach);
}

bool ce_IsConst(u32 reg)
{
	if (ce_CanBeConst(reg))
		return shil_ce_gpr[reg].IsConstant;
	else
		return false;
}

u32 ce_GetConst(u32 reg)
{
	if (ce_IsConst(reg))
		return shil_ce_gpr[reg].RegValue;
	else
		ce_die("ce_GetConst : can't get const when reg is not const");
	return 0;
}
void ce_SetConst(u32 reg,u32 value)
{
	if (ce_IsConst(reg))
	{
		//shil_ce_gpr[reg].RegValue=value;
		if (shil_ce_gpr[reg].RegValue!=value)
		{
			shil_ce_gpr[reg].IsRB=false;
			shil_ce_gpr[reg].RegValue=value;
		}
	}
	else
		ce_die("ce_SetConst : can't set const when reg is not const");
}
void ce_MakeConst(u32 reg,u32 value)
{
	if (ce_CanBeConst(reg))
	{
		if (shil_ce_gpr[reg].IsConstant==false)
		{
			shil_ce_gpr[reg].IsConstant=true;
			shil_ce_gpr[reg].RegValue=value;
			shil_ce_gpr[reg].IsRB=false;
		}
		else
		{
			ce_die("ce_MakeConst : reg is allready const");
			/*
			if (shil_ce_gpr[reg].RegValue!=value)
			{
				shil_ce_gpr[reg].IsRB=false;
				shil_ce_gpr[reg].RegValue=value;
			}
			*/
		}
	}
	else
		ce_die("ce_MakeConst : can't create const when reg can't be const.tracked");
}
void ce_KillConst(u32 reg)
{
	if (ce_IsConst(reg))
	{
		shil_ce_gpr[reg].IsConstant=false;
		shil_ce_gpr[reg].IsRB=false;
	}
}
bool ce_FindExistingConst(u32 value,u8* reg_num)
{
	//return false;
	for (u8 i=0;i<sh4_reg_count;i++)
	{
		if (ce_IsConst(i))
		{
			if ((shil_ce_gpr[i].IsRB==true) && shil_ce_gpr[i].RB_value==value)
			{
				*reg_num=i;
				return true;
			}
		}
	}
	return false;
}

void ce_WriteBack(u32 reg,shil_stream* il)
{
	if (ce_IsConst(reg))
	{
		if ((shil_ce_gpr[reg].IsRB==false) || (shil_ce_gpr[reg].RegValue!=shil_ce_gpr[reg].RB_value))
		{
			/*u8 aliased_reg;
			u32 rv=ce_GetConst(reg);
			 // not realy usable til temporal reg alloc is implemented or smth :)
			if (ce_FindExistingConst(rv,&aliased_reg))
			{
				il->mov((Sh4RegType)reg,(Sh4RegType)aliased_reg);
			}
			else*/
			{
				il->mov((Sh4RegType)reg,ce_GetConst(reg));
			}
			shil_ce_gpr[reg].RB_value=ce_GetConst(reg);
			shil_ce_gpr[reg].IsRB=true;
		}
	}
}

void ce_WriteBack_all(shil_stream* il)
{
	for (u8 i=0;i<sh4_reg_count;i++)
		ce_WriteBack(i,il);
}

void ce_WriteBack_aks(u32 reg,shil_stream* il)
{
	ce_WriteBack(reg,il);
	ce_KillConst(reg);
}
void ce_WriteBack_aks_all(shil_stream* il)
{
	for (u8 i=0;i<sh4_reg_count;i++)
		ce_WriteBack_aks(i,il);
}
bool ce_re_run;

void shil_ce_add_static(u32 addr,u32 sz)
{
	static_reads.push_back(addr);
}
bool shil_ce_check_static(u32 addr,u32 sz)
{
	for (size_t i=0;i<static_reads.size();i++)
	{
		if (static_reads[i]==addr)
			return true;
	}
	return false;
}
bool is_writem_safe(BasicBlock* bb, shil_opcode* op);

//Optimisation pass mainloop
u32 shil_optimise_pass_ce_main(BasicBlock* bb)
{
	bool rv=false;
	ce_re_run=false;
	u32 opt=0;

	Init_ce();

	shil_stream il;

	size_t old_Size=bb->ilst.opcodes.size();

	u32 unsafe_pos=0xFFFFFFFF;
	if (settings.dynarec.Safe)
	{
		shil_ce_is_locked=true;		//block is safe til a write mem opcode that can modify it
		for (size_t i=0;i<bb->ilst.opcodes.size();i++)
		{
			shil_opcode* op=&bb->ilst.opcodes[i];

			if (op->opcode==shilop_writem && !is_writem_safe(bb,op))
			{
				log("Block %08X : disabling read-const @ %d/%d\n",bb->start,i,bb->ilst.opcodes.size());
				unsafe_pos=i;
				break;
			}
		}
	}


	bool old_re_run=ce_re_run;
	//il.opcodes.reserve(bb->ilst.opcodes.size());

	for (size_t i=0;i<bb->ilst.opcodes.size();i++)
	{
		shil_opcode* op=&bb->ilst.opcodes[i];

		if (i==unsafe_pos)
			shil_ce_is_locked=false;
		old_re_run=ce_re_run;
		if (shil_ce_lut[op->opcode](op,bb,&il)==false)
			il.opcodes.push_back(*op);//emit the old opcode
		else
		{
			opt++;
			rv=true;
		}
	}

	if (old_re_run!=ce_re_run)
		ce_re_run=false;

	if (rv)
	{
		bb->ilst.opcodes.clear();
		bb->ilst.opcodes.reserve(il.opcodes.size());

		for (size_t i=0;i<il.opcodes.size();i++)
			bb->ilst.opcodes.push_back(il.opcodes[i]);
		
		//no need to write back reg_pc_temp , it not used after block [its olny a temp reg]:)
		for (u8 i=0;i<16;i++)
		{
			ce_WriteBack_aks(i,&bb->ilst);
		}
		ce_WriteBack_aks(reg_mach,&bb->ilst);
		ce_WriteBack_aks(reg_macl,&bb->ilst);
		
		bb->ilst.op_count=(u32)bb->ilst.opcodes.size();
	}

	if (old_Size!=bb->ilst.opcodes.size())
		ce_re_run = true;

	return opt;
}

void shil_optimise_pass_ce_driver(BasicBlock* bb)
{
	if (settings.dynarec.CPpass==0)
		return;

	shil_ce_is_locked=true;
	static_reads.clear();

	ce_re_run=true;
	u32 rv=0;
	u32 pass=0;
	size_t old_Size=bb->ilst.opcodes.size();

	
	//CompileBasicBlock_slow_c(bb,0);
	while(ce_re_run)
	{
		rv+=shil_optimise_pass_ce_main(bb);
		pass++;
		if (pass>10)
			break;
		//CompileBasicBlock_slow_c(bb,pass);
	}

	total_ops_removed+=old_Size-bb->ilst.opcodes.size();
	shil_optimise_pass_btp_main(bb);

	static_reads.clear();
	//if (rv)
	//	log("Optimised block 0x%X , %d opts : %d passes ,delta=%d, total removed %d \n",bb->start,rv,pass,old_Size-bb->ilst.opcodes.size(),total_ops_removed);

}
//default thing to do :p
void DefHanlder(shil_opcode* op,BasicBlock* bb,shil_stream* il)
{
	for (u8 i=0;i<16;i++)
	{
		if (ce_IsConst(i))
		{
			if (op->ReadsReg((Sh4RegType)i))
			{
				ce_WriteBack(i,il);
			}
			if (op->WritesReg((Sh4RegType)i))
			{
				ce_KillConst(i);
			}
		}
	}
}
//optimisation hanlders ;)
#define NormBinaryOp(oppstrrr)\
if ((op->flags & FLAG_REG2) && (ce_IsConst(op->reg2)))\
	{\
		op->imm1=ce_GetConst(op->reg2);\
		op->flags|=FLAG_IMM1;\
		op->flags&=~FLAG_REG2;\
		ce_re_run=true;\
	}\
	\
	if (ce_IsConst(op->reg1))\
	{\
		if ((op->flags & FLAG_IMM1))\
		{\
			ce_SetConst(op->reg1,ce_GetConst(op->reg1) oppstrrr op->imm1);\
			ce_re_run=true;\
			return true;\
		}\
		else\
		{\
			ce_WriteBack_aks(op->reg1,il);\
		}\
	}\
	else\
	{\
	}\

	//ce_WriteBack_aks(op->reg1,il);
	//	ce_WriteBack(op->reg2,il);
		//reg1 has to be writen back , and its no more const 
		//-> its not const to start with :p not realy needed here
		//----*----\
		//reg2 has to be writen back , but it will remain constant ;)
		//-> not needed , if const , the reg1,imm1 form is used
shilh(pref)
{
	if ((op->flags & FLAG_REG1) && (ce_IsConst(op->reg1)))
	{
		op->imm1=ce_GetConst(op->reg1);
		op->flags|=FLAG_IMM1;
		op->flags&=~FLAG_REG1;
	}
	DefHanlder(op,bb,il);
	return false;
}
shilh(adc)
{
	DefHanlder(op,bb,il);
	return false;
}
shilh(add)
{
	//DefHanlder(op,bb,il);
	NormBinaryOp(+);
	return false;
}
shilh(and)
{
	//DefHanlder(op,bb,il);
	NormBinaryOp(&);
	return false;
}
shilh(cmp)
{
	DefHanlder(op,bb,il);
	return false;
}

shilh(LoadT)
{
	DefHanlder(op,bb,il);
	return false;
}
shilh(mov)
{
	bool rv=false;

	if ((op->flags & FLAG_REG2) &&  op->reg1==op->reg2)
	{
		ce_re_run=true;
		return true;
	}

	if ((op->flags & FLAG_REG2) &&  ce_IsConst(op->reg2))
	{
		op->flags&=~FLAG_REG2;
		op->flags|=FLAG_IMM1;
		op->imm1 = ce_GetConst(op->reg2);
		ce_re_run=true;
	}

	if (op->flags & FLAG_IMM1)
	{	//reg1=imm1
		//reg1 gets a known value
		if (ce_CanBeConst(op->reg1))
		{
			if (ce_IsConst(op->reg1))
				ce_SetConst(op->reg1,op->imm1);
			else
				ce_MakeConst(op->reg1,op->imm1);
			rv=true;
		}
		else
		{
			ce_KillConst(op->reg1);
		}
	}
	else
	{
		//reg1 gets an unknown value
		ce_WriteBack(op->reg2,il);
		ce_KillConst(op->reg1);
	}

	return rv;
}
shilh(movex)
{
	DefHanlder(op,bb,il);
	return false;
}
shilh(neg)
{
	DefHanlder(op,bb,il);
	return false;
}
shilh(not)
{
	DefHanlder(op,bb,il);
	return false;
}
shilh(or)
{
	//DefHanlder(op,bb,il);
	NormBinaryOp(|);
	return false;
}
shilh(rcl)
{
	DefHanlder(op,bb,il);
	return false;
}
shilh(rcr)
{
	DefHanlder(op,bb,il);
	return false;
}
//ReadMem 
bool GetRamReadAdr(shil_opcode* op,u32* addr)
{
	if ((op->flags & (FLAG_R0|FLAG_GBR|FLAG_REG2))==0)
	{//[imm] form
		verify((op->flags & (~(3|FLAG_SX))) == (FLAG_IMM1|FLAG_REG1));
		if (IsOnRam(op->imm1))
		{
			*addr= op->imm1;
			return true;
		}
	}
	return false;
}
bool ce_ReadWriteParams(shil_opcode* op)
{
	//return false;
	bool rv=false;

	if (op->flags & FLAG_REG2)
	{
		if (ce_IsConst(op->reg2))
		{
			u32 vl=ce_GetConst(op->reg2);
			if (op->flags & FLAG_IMM1)
				op->imm1+=vl;
			else
				op->imm1=vl;

			rv=true;
			op->flags |= FLAG_IMM1;
			op->flags&=~FLAG_REG2;
		}
	}
	if (op->flags & FLAG_R0)
	{
		if (ce_IsConst(r0))
		{
			u32 vl=ce_GetConst(r0);
			if (op->flags & FLAG_IMM1)
				op->imm1+=vl;
			else
				op->imm1=vl;

			rv=true;
			op->flags |= FLAG_IMM1;
			op->flags&=~FLAG_R0;
		}
	}
	if (op->flags & FLAG_GBR)
	{
		if (ce_IsConst(reg_gbr))
		{
			u32 vl=ce_GetConst(reg_gbr);
			if (op->flags & FLAG_IMM1)
				op->imm1+=vl;
			else
				op->imm1=vl;

			rv=true;
			op->flags |= FLAG_IMM1;
			op->flags&=~FLAG_GBR;
		}
	}
	
	return rv;
}
shilh(readm)
{
	bool rv=ce_ReadWriteParams(op);

	u32 addr;
	
	if (GetRamReadAdr(op,&addr))
	{
		u32 size=op->flags&3;
		if (shil_ce_is_locked && bb->IsMemLocked(addr) && (size!=FLAG_64))
		{
			u32 data=0;
			shil_ce_add_static(addr,size);
			if (size==FLAG_64)
			{
				Sh4RegType base=(Sh4RegType)GetSingleFromDouble((u8)op->reg1);
				il->mov(base,ReadMem32(addr));
				il->mov((Sh4RegType)(base+1),ReadMem32(addr+4));
			}
			else
			{
				if (size==0)
				{
					verify(op->flags & FLAG_SX);
					data=(u32)(s32)(s8)ReadMem8(addr);
				}
				else if (size==1)
				{
					verify(op->flags & FLAG_SX);
					data=(u32)(s32)(s16)ReadMem16(addr);
				}
				else
				{
					verify((op->flags & FLAG_SX)==0);
					data=ReadMem32(addr);
				}

				bb->locked.push_back(addr);
				bb->locked.push_back(data);

				if (ce_CanBeConst(op->reg1))
				{
					if (ce_IsConst(op->reg1))
					{
						ce_SetConst(op->reg1,data);
					}
					else
						ce_MakeConst(op->reg1,data);
				}
				else
				{	
					il->mov((Sh4RegType)op->reg1,data);
				}
			}
				//ce_die("shilh(readm) : ce_CanBeConst(op->reg1)==false");//il->mov((Sh4RegType)op->reg1,data);

			ce_re_run=true;
			return true;
		}
	}

	//even if we did optimise smth , a readback may be needed
	//since it uses opcode flags to decode what to write back , it should't cause any non needed write backs ;)
	DefHanlder(op,bb,il);

	//we did chainge smth :0
	if (rv)//we optimised something , re run the ce pass once more
		ce_re_run=true;

	return false;
}

bool is_writem_safe(BasicBlock* bb, shil_opcode* op)
{
	//bool rv=ce_ReadWriteParams(op);

	u32 addr;
	if (GetRamReadAdr(op,&addr))
	{
		return !bb->IsMemLocked(addr);
		//u32 size=op->flags&3;
		/*
		if (bb->IsMemLocked(addr))
		{
			return false;	//block modifies its page. not safe.
		}
		else
		{
			return true;	//block writes somewhere outside of its page. safe.
		}*/
	}

	//any non-static write may write in the block page list, so its not safe by default
	return false;
}
shilh(writem)
{
	bool rv=ce_ReadWriteParams(op);
	u32 addr;
	//bool kill_all=false;
	if (GetRamReadAdr(op,&addr))
	{
		u32 size=op->flags&3;
		if (shil_ce_is_locked && bb->IsMemLocked(addr) && (size!=FLAG_64))
		{
			verify(!settings.dynarec.Safe);	//can't happen in safe mode
			shil_ce_is_locked=false;
			log("CE: Block will be demoted to manual for the CE pass\n");
		}
	}
	//even if we did optimise smth , a readback may be needed
	//since it uses opcode flags to decode what to write back , it should't cause any non needed write backs ;)
	DefHanlder(op,bb,il);

	//we did chainge smth :0
	if (rv)//we optimised something , re run the ce pass once more
		ce_re_run=true;
/*
	if (kill_all)
		ce_WriteBack_aks_all(il);
*/
	return false;
}
shilh(rol)
{
	DefHanlder(op,bb,il);
	return false;
}
shilh(ror)
{
	DefHanlder(op,bb,il);
	return false;
}
//uses flags from previous opcode
shilh(SaveT)
{
	DefHanlder(op,bb,il);
	return false;
}
shilh(shil_ifb)
{
	ce_WriteBack_aks_all(il);
	return false;
}
//sets T if imm ==1
shilh(sar)
{
	if (ce_IsConst(op->reg1) && (op->imm1>1))
	{
		ce_SetConst(op->reg1,(u32)(((s32)ce_GetConst(op->reg1))>>(op->imm1)));
		return true;
	}
	else
		DefHanlder(op,bb,il);
	return false;
}
//sets T if imm ==1
shilh(shl)
{
	if (ce_IsConst(op->reg1) && (op->imm1>1))
	{
		ce_SetConst(op->reg1,ce_GetConst(op->reg1)<<(op->imm1));
		return true;
	}
	else
		DefHanlder(op,bb,il);
	return false;
}
//sets T if imm ==1
shilh(shr)
{
	if (ce_IsConst(op->reg1) && (op->imm1>1))
	{
		ce_SetConst(op->reg1,ce_GetConst(op->reg1)>>(op->imm1));
		return true;
	}
	else
		DefHanlder(op,bb,il);
	return false;
}
//sets T if used as DT
shilh(sub)
{
	//NormBinaryOp(-);
	//that's all we can do for now
	//DT uses sub , and it needs flags to be ok after it
	if ((op->flags & FLAG_REG2) && (ce_IsConst(op->reg2)))
	{
		op->imm1=ce_GetConst(op->reg2);
		op->flags|=FLAG_IMM1;
		op->flags&=~FLAG_REG2;
	}
	
	DefHanlder(op,bb,il);
	return false;
}
shilh(swap)
{
	if (ce_IsConst(op->reg1))
	{
		u32 size=op->flags&3;
		if (size==FLAG_8)
		{
			u32 ov=ce_GetConst(op->reg1);
			ov=(ov & 0xFFFF0000) | ((ov&0xFF)<<8) | ((ov>>8)&0xFF);
			ce_SetConst(op->reg1,ov);
		}
		else
		{
			u32 ov=ce_GetConst(op->reg1);
			ov=(ov>>16)|(ov<<16);
			ce_SetConst(op->reg1,ov);
		}
		
		return true;
	}
	else
		DefHanlder(op,bb,il);
	return false;
}
//sets T
shilh(test)
{
	DefHanlder(op,bb,il);
	return false;
}
shilh(xor)
{
	NormBinaryOp(^);
	return false;
}
shilh(jcond)
{
	DefHanlder(op,bb,il);
	return false;
}
shilh(jmp)
{
	DefHanlder(op,bb,il);
	return false;
}
shilh(mul)
{
	DefHanlder(op,bb,il);
	return false;
}
shilh(div32)
{
	ce_WriteBack_aks(op->reg1,il);
	ce_WriteBack_aks(op->reg2,il);
	ce_WriteBack_aks((u8)op->imm1,il);
	DefHanlder(op,bb,il);
	return false;
}
shilh(fcmp)
{
	DefHanlder(op,bb,il);
	return false;
}
shilh(fabs)
{
	DefHanlder(op,bb,il);
	return false;
}
shilh(fadd)
{
	DefHanlder(op,bb,il);
	return false;
}
shilh(fdiv)
{
	DefHanlder(op,bb,il);
	return false;
}
shilh(fmac)
{
	DefHanlder(op,bb,il);
	return false;
}
shilh(fmul)
{
	DefHanlder(op,bb,il);
	return false;
}
shilh(fneg)
{
	DefHanlder(op,bb,il);
	return false;
}
shilh(fsub)
{
	DefHanlder(op,bb,il);
	return false;
}
shilh(ftrv)
{
	DefHanlder(op,bb,il);
	return false;
}
shilh(fsqrt)
{
	DefHanlder(op,bb,il);
	return false;
}
shilh(fipr)
{
	DefHanlder(op,bb,il);
	return false;
}
shilh(floatfpul)
{
	DefHanlder(op,bb,il);
	return false;
}
shilh(ftrc)
{
	DefHanlder(op,bb,il);
	return false;
}
shilh(fsca)
{
	DefHanlder(op,bb,il);
	return false;
}
shilh(fsrra)
{
	DefHanlder(op,bb,il);
	return false;
}

//works olny for moves
//a very limited form of dce pre scan ;)
bool backscan_const(BasicBlock* bb,u8 reg,u32* rv)
{
	u32 lop=bb->ilst.op_count;
	shil_stream* ilst=&bb->ilst;

	//look if reg takes a const value
	while (lop)
	{
		lop--;
		shil_opcode* op=&ilst->opcodes[lop];
		//if move (we can take const value olny from here for now)
		if (op->opcode==shilop_mov && op->reg1==reg)
		{
			//if the reg we want became a const , were finished :D
			if (op->flags & FLAG_IMM1)
			{
				*rv=op->imm1;
				return true;
			}
			else
			{
				//if its a reg 2 reg move , we alias the old reg w/ the one that replaced it ;)
				verify(op->flags & FLAG_REG2);
				reg=(u8)op->reg2;
			}
		}
		else
		{
			//we get writen an unknown value , unable to fix it up :p
			if (op->WritesReg((Sh4RegType)reg))
				return false;
		}
	}

	//we failed to find a const on the entire block
	return false;
}

u32 shil_optimise_pass_btp_main(BasicBlock* bb)
{
	if ((bb->flags.ExitType==BLOCK_EXITTYPE_DYNAMIC) ||
		(bb->flags.ExitType==BLOCK_EXITTYPE_DYNAMIC_CALL))
	{
		u32 new_cv=0;
		if (backscan_const(bb,reg_pc,&new_cv))
		{
			//log("Block promote 0x%X , from DYNAMIC to FIXED exit 0x%X : %d\n",bb->start,new_cv,bb->flags.ExitType);
			bb->TF_next_addr=new_cv;
			if (bb->flags.ExitType==BLOCK_EXITTYPE_DYNAMIC)
				bb->flags.ExitType=BLOCK_EXITTYPE_FIXED;
			else
				bb->flags.ExitType=BLOCK_EXITTYPE_FIXED_CALL;
		}
	}

	return 1;
}