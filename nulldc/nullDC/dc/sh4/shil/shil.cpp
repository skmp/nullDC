#include "shil.h"

//IL and related code
//shil opcode forms:

//[]-> optional
//operations on a single register
//op reg,[imm32]
//op [imm32],reg

//operations on 2 regs
//op reg reg [imm32]
bool shil_opcode::ReadsReg(Sh4RegType reg)
{
	bool used=false;

	if (reg==reg_fpul)
	{
		if (this->opcode==shilop_floatfpul)
			return true;
	}

	if (this->flags & FLAG_REG1)
	{
		if (opcode!=shilop_mov && opcode!=shilop_movex && opcode!=shilop_readm)
			used |= (reg1==(u32)reg) ;
	}

	if (this->flags & FLAG_REG2)
	{
		used |= (reg2==(u32)reg);
	}

	if (this->flags & FLAG_R0)
		used |= (r0==(u32)reg) ;

	if (this->flags & FLAG_GBR)
		used |= (reg_gbr==(u32)reg) ;

	if (this->flags & FLAG_MACH)
	{
		if (opcode!=shilop_mul)
			used |= ((u32)reg_mach==(u32)reg) ;
	}

	if (this->flags & FLAG_MACL)
	{
		if (opcode!=shilop_mul)
			used |= ((u32)reg_macl==(u32)reg) ;
	}

	return used;
}  
bool shil_opcode::OverwritesReg(Sh4RegType reg)
{
	return (!ReadsReg(reg)) && WritesReg(reg);
}
bool shil_opcode::UpdatesReg(Sh4RegType reg)
{
	return ReadsReg(reg) && WritesReg(reg);
}
bool shil_opcode::WritesReg(Sh4RegType reg)
{
	bool used=false;

	if (reg==reg_fpul)
	{
		if (this->opcode==shilop_ftrc)
			return true;
	}


	if (this->flags & FLAG_REG1)
	{
		if (opcode!=shilop_cmp && opcode!=shilop_test && opcode!=shilop_writem && opcode!=shilop_pref)
			used |= (reg1== (u32)reg);
	}

	if (this->flags & FLAG_REG2)
	{
		//none writes to reg 2 :)
		//used |= (reg2==reg) ;
	}

	if (this->flags & FLAG_R0)
	{
		//none writes to r0 flag :)
		//used |= (r0==reg) ;
	}

	if (this->flags & FLAG_GBR)
	{
		//none writes to gbr flag :)
		//used |= (reg_gbr==reg) ;
	}

	if (this->flags & FLAG_MACH)
	{
		//flag mach is allways writen
		used |= (reg_mach==reg) ;
	}

	if (this->flags & FLAG_MACL)
	{
		//flag macl is allways writen
		used |= (reg_macl==reg) ;
	}

	return used;
}


void shil_stream::emit(shil_opcodes op,Sh4RegType reg1,Sh4RegType  reg2,u32 imm1,u32 imm2,u32 flags)
{
	op_count++;
	shil_opcode sh_op;

	sh_op.opcode=(u16)op;
	sh_op.reg1=(u8)reg1;
	sh_op.reg2=(u8)reg2;
	sh_op.imm1=imm1;
	sh_op.imm2=imm2;
	sh_op.flags=(u16)flags;

	opcodes.push_back(sh_op);
}


void shil_stream::emit32(shil_opcodes op,u32 imm1)
{
	emit(op,NoReg,NoReg,imm1,0,FLAG_32 | FLAG_IMM1);
}

void shil_stream::emit32(shil_opcodes op,Sh4RegType reg1)
{
	emit(op,reg1,NoReg,0,0,FLAG_32 | FLAG_REG1);
}

void shil_stream::emit32(shil_opcodes op,Sh4RegType reg1,u32 imm1)
{
	emit(op,reg1,NoReg,imm1,0,FLAG_32 | FLAG_REG1| FLAG_IMM1);
}

void shil_stream::emit32(shil_opcodes op,Sh4RegType reg1,Sh4RegType  reg2)
{
	emit(op,reg1,reg2,0,0,FLAG_32 | FLAG_REG1| FLAG_REG2);
}
void shil_stream::emit32(shil_opcodes op,Sh4RegType reg1,Sh4RegType  reg2,u32 extraflags)
{
	emit(op,reg1,reg2,0,0,FLAG_32 | FLAG_REG1| FLAG_REG2 | extraflags);
}
void shil_stream::pref(Sh4RegType addr)
{
	emit32(shilop_pref,addr);
}
void shil_stream::emitReg(shil_opcodes op,Sh4RegType reg1,u32 flags)
{
	//emit
	emit(op,reg1,NoReg,0,0,flags|FLAG_REG1);
}

void shil_stream::emitRegImm(shil_opcodes op,Sh4RegType reg1,u32 imm1,u32 flags)
{
	//emit
	emit(op,reg1,NoReg,imm1,0,flags|FLAG_IMM1|FLAG_REG1);
}

void shil_stream::emitRegImmImm(shil_opcodes op,Sh4RegType reg1,u32 imm1,u32 imm2,u32 flags)
{
	//emit
	emit(op,reg1,NoReg,imm1,imm2,flags|FLAG_IMM1|FLAG_IMM2|FLAG_REG1);
}

void shil_stream::emitRegReg(shil_opcodes op,Sh4RegType reg1,Sh4RegType  reg2,u32 flags)
{
	//emit
	emit(op,reg1,reg2,0,0,flags|FLAG_REG1|FLAG_REG2);
}
void shil_stream::emitRegRegImm(shil_opcodes op,Sh4RegType reg1,Sh4RegType  reg2,u32 imm1,u32 flags)
{
	//emit
	emit(op,reg1,reg2,imm1,0,flags|FLAG_IMM1|FLAG_REG1|FLAG_REG2);
}

//******* opcode emitters ******
void shil_stream::jcond(u32 cond)
{
	emit32(shilop_jcond,cond);
}
void shil_stream::jmp()
{
	emit(shilop_jcond,NoReg,NoReg,0,0,0);
}
void shil_stream::mov(Sh4RegType to,Sh4RegType from)
{
	if (IsReg64(to) || IsReg64(from))
	{
		if (!(IsReg64(from) && IsReg64(from)))
		{
			log("SHIL ERROR\n");
		}
		emitRegReg(shilop_mov,to,from,FLAG_64);
	}
	else
	{
		emitRegReg(shilop_mov,to,from,FLAG_32);
	}
	//emit32(shilop_mov,to,from);
}
void shil_stream::mov(Sh4RegType to,u32 from)
{
	if (IsReg64(to))
	{
		log("SHIL ERROR\n");
	}
	emit32(shilop_mov,to,from);
	//emit(shilop_mov,to,from);
}

/*** Mem reads ***/
u16 GetBaseFlags(Sh4RegType base)
{
	if (base!=r0 && base!=reg_gbr )
	{
		log("SHIL ERROR\n");
		return 0;
	}

	if (base==r0)
		return FLAG_R0;
	else if (base==reg_gbr)
		return FLAG_GBR;

	return 0;
}
//readmem [const]
void shil_stream::readm8(Sh4RegType to,u32 from)
{
	emitRegImm(shilop_readm,to,from,FLAG_8|FLAG_SX);
}
void shil_stream::readm16(Sh4RegType to,u32 from)
{
	emitRegImm(shilop_readm,to,from,FLAG_16|FLAG_SX);
}
void shil_stream::readm32(Sh4RegType to,u32 from)
{
	emitRegImm(shilop_readm,to,from,FLAG_32);
}
void shil_stream::readm64(Sh4RegType to,u32 from)
{
	dbgbreak;
}

//readmem [reg]
void shil_stream::readm8(Sh4RegType to,Sh4RegType from)
{
	readm8(to,from,0);
}
void shil_stream::readm16(Sh4RegType to,Sh4RegType from)
{
	readm16(to,from,0);
}
void shil_stream::readm32(Sh4RegType to,Sh4RegType from)
{
	readm32(to,from,0);
}
void shil_stream::readm64(Sh4RegType to,Sh4RegType from)
{
	emitRegReg(shilop_readm,to,from,FLAG_64);
}

//readmem base[offset]
void shil_stream::readm8(Sh4RegType to,Sh4RegType base,Sh4RegType offset)
{
	emitRegReg(shilop_readm,to,offset,FLAG_8|FLAG_SX|GetBaseFlags(base));
}
void shil_stream::readm16(Sh4RegType to,Sh4RegType base,Sh4RegType offset)
{
	emitRegReg(shilop_readm,to,offset,FLAG_16|FLAG_SX|GetBaseFlags(base));
}
void shil_stream::readm32(Sh4RegType to,Sh4RegType base,Sh4RegType offset)
{
	emitRegReg(shilop_readm,to,offset,FLAG_32|GetBaseFlags(base));
}
void shil_stream::readm64(Sh4RegType to,Sh4RegType base,Sh4RegType offset)
{
	emitRegReg(shilop_readm,to,offset,FLAG_64|GetBaseFlags(base));
}


//readmem base[const]
void shil_stream::readm8(Sh4RegType to,Sh4RegType base,u32 offset)
{
	emitRegRegImm(shilop_readm,to,base,offset,FLAG_8|FLAG_SX);
}
void shil_stream::readm16(Sh4RegType to,Sh4RegType base,u32 offset)
{
	emitRegRegImm(shilop_readm,to,base,offset,FLAG_16|FLAG_SX);
}
void shil_stream::readm32(Sh4RegType to,Sh4RegType base,u32 offset)
{
	emitRegRegImm(shilop_readm,to,base,offset,FLAG_32);
}


/*** Mem writes ***/

//writemem [const]
void shil_stream::writem8(Sh4RegType from,u32 to)
{
	emitRegImm(shilop_writem,from,to,FLAG_8);
}
void shil_stream::writem16(Sh4RegType from,u32 to)
{
	emitRegImm(shilop_writem,from,to,FLAG_16);
}
void shil_stream::writem32(Sh4RegType from,u32 to)
{
	emitRegImm(shilop_writem,from,to,FLAG_32);
}
void shil_stream::writem64(Sh4RegType from,u32 to)
{
	//writem8(from,to,0,sx);
	dbgbreak;
}

//writemem [reg]
void shil_stream::writem8(Sh4RegType from,Sh4RegType to)
{
//	emit(shilop_add,0,0,0);
	writem8(from,to,0);
}
void shil_stream::writem16(Sh4RegType from,Sh4RegType to)
{
	writem16(from,to,0);
}
void shil_stream::writem32(Sh4RegType from,Sh4RegType to)
{
	writem32(from,to,0);
}
void shil_stream::writem64(Sh4RegType from,Sh4RegType to)
{
	emitRegReg(shilop_writem,from,to,FLAG_64);
}

//writemem reg[reg]
void shil_stream::writem8(Sh4RegType from,Sh4RegType base,Sh4RegType offset)
{
	emitRegReg(shilop_writem,from,offset,FLAG_8|GetBaseFlags(base));
}
void shil_stream::writem16(Sh4RegType from,Sh4RegType base,Sh4RegType offset)
{
	emitRegReg(shilop_writem,from,offset,FLAG_16|GetBaseFlags(base));
}
void shil_stream::writem32(Sh4RegType from,Sh4RegType base,Sh4RegType offset)
{
	emitRegReg(shilop_writem,from,offset,FLAG_32|GetBaseFlags(base));
}
void shil_stream::writem64(Sh4RegType from,Sh4RegType base,Sh4RegType offset)
{//r0+reg
	emitRegReg(shilop_writem,from,offset,FLAG_64|GetBaseFlags(base));
}

//writemem reg[const]
void shil_stream::writem8(Sh4RegType from,Sh4RegType base,u32 offset)
{
	emitRegRegImm(shilop_writem,from,base,offset,FLAG_8);
}
void shil_stream::writem16(Sh4RegType from,Sh4RegType base,u32 offset)
{
	emitRegRegImm(shilop_writem,from,base,offset,FLAG_16);
}
void shil_stream::writem32(Sh4RegType from,Sh4RegType base,u32 offset)
{
	emitRegRegImm(shilop_writem,from,base,offset,FLAG_32);
}

//compares
void shil_stream::cmp(Sh4RegType to,Sh4RegType from)
{
	emit32(shilop_cmp,to,from);
}

void shil_stream::cmp(Sh4RegType to,s8 from)
{
	emit32(shilop_cmp,to,from);
}

void shil_stream::test(Sh4RegType to,Sh4RegType from)
{
	emit32(shilop_test,to,from);
}
void shil_stream::test(Sh4RegType to,u8 from)
{
	emit32(shilop_test,to,from);
}

void shil_stream::SaveT(cmd_cond cond)
{
	emit32(shilop_SaveT,(u32)cond);
}
void shil_stream::LoadT(x86_flags to)
{
	emit32(shilop_LoadT,(u32)to);
}


void shil_stream::dec(Sh4RegType to)
{
	sub(to,1);
}
void shil_stream::inc(Sh4RegType to)
{
	add(to,1);
}
void shil_stream::neg(Sh4RegType to)
{
	emit32(shilop_neg,to);

}
void shil_stream::not(Sh4RegType to)
{
	emit32(shilop_not,to);
}

//bitwise ops
void shil_stream::and(Sh4RegType to,Sh4RegType from)
{
	emit32(shilop_and,to,from);
}
void shil_stream::and(Sh4RegType to,u32 from)
{
	emit32(shilop_and,to,from);
}
void shil_stream::or(Sh4RegType to,Sh4RegType from)
{
	emit32(shilop_or,to,from);
}
void shil_stream::or(Sh4RegType to,u32 from)
{
	emit32(shilop_or,to,from);
}
void shil_stream::xor(Sh4RegType to,Sh4RegType from)
{
	emit32(shilop_xor,to,from);
}
void shil_stream::xor(Sh4RegType to,u32 from)
{
	emit32(shilop_xor,to,from);
}

//logical shifts
void shil_stream::shl(Sh4RegType to,u8 count)
{
	emit32(shilop_shl,to,count);
}
void shil_stream::shr(Sh4RegType to,u8 count)
{
	emit32(shilop_shr,to,count);
}

//arithmetic shifts
void shil_stream::sal(Sh4RegType to,u8 count)
{//<- is this used ?
	shl(to,count);
}
void shil_stream::sar(Sh4RegType to,u8 count)
{
	emit32(shilop_sar,to,count);
}

//rotate

void shil_stream::rcl(Sh4RegType to)
{
	emit32(shilop_rcl,to);
}
void shil_stream::rcr(Sh4RegType to)
{
	emit32(shilop_rcr,to);
}
void shil_stream::rol(Sh4RegType to)
{
	emit32(shilop_rol,to);
}
void shil_stream::ror(Sh4RegType to)
{
	emit32(shilop_ror,to);
}

//swaps
void shil_stream::bswap(Sh4RegType to)
{
	emitReg(shilop_swap,to,FLAG_8);
}
void shil_stream::wswap(Sh4RegType to)
{
	emitReg(shilop_swap,to,FLAG_16);
}

//extends
//signed
void shil_stream::movsxb(Sh4RegType to,Sh4RegType from)
{
	emitRegReg(shilop_movex,to,from,FLAG_SX | FLAG_8);
}
void shil_stream::movsxw(Sh4RegType to,Sh4RegType from)
{
	emitRegReg(shilop_movex,to,from,FLAG_SX | FLAG_16);
}
//unsigned
void shil_stream::movzxb(Sh4RegType to,Sh4RegType from)
{
	emitRegReg(shilop_movex,to,from,FLAG_ZX | FLAG_8);
}
void shil_stream::movzxw(Sh4RegType to,Sh4RegType from)
{
	emitRegReg(shilop_movex,to,from,FLAG_ZX | FLAG_16);
}

//maths (integer)
void shil_stream::adc(Sh4RegType to,Sh4RegType from)
{
	emit32(shilop_adc,to,from);
}

void shil_stream::add(Sh4RegType to,Sh4RegType from)
{
	emit32(shilop_add,to,from);
}
void shil_stream::add(Sh4RegType to,u32 from)
{
	emit32(shilop_add,to,from);
}
void shil_stream::sub(Sh4RegType to,Sh4RegType from)
{
	emit32(shilop_sub,to,from);
}
void shil_stream::sub(Sh4RegType to,u32 from)
{
	emit32(shilop_sub,to,from);
}

void shil_stream::muls_16_16_32(Sh4RegType reg1,Sh4RegType reg2)
{
	shil_stream::emitRegReg(shilop_mul,reg1,reg2,FLAG_MACL | FLAG_16| FLAG_SX);
}

void shil_stream::mulu_16_16_32(Sh4RegType reg1,Sh4RegType reg2)
{
	shil_stream::emitRegReg(shilop_mul,reg1,reg2,FLAG_MACL | FLAG_16| FLAG_ZX);
}

void shil_stream::muls_32_32_32(Sh4RegType reg1,Sh4RegType reg2)
{
	shil_stream::emitRegReg(shilop_mul,reg1,reg2,FLAG_MACL | FLAG_32| FLAG_SX);
}
void shil_stream::mulu_32_32_32(Sh4RegType reg1,Sh4RegType reg2)
{
	shil_stream::emitRegReg(shilop_mul,reg1,reg2,FLAG_MACL | FLAG_32| FLAG_ZX);
}

void shil_stream::muls_32_32_64(Sh4RegType reg1,Sh4RegType reg2)
{
	shil_stream::emitRegReg(shilop_mul,reg1,reg2,FLAG_MACL | FLAG_MACH | FLAG_64| FLAG_SX);
}
void shil_stream::mulu_32_32_64(Sh4RegType reg1,Sh4RegType reg2)
{
	shil_stream::emitRegReg(shilop_mul,reg1,reg2,FLAG_MACL | FLAG_MACH | FLAG_64| FLAG_ZX);
}

//floating

u16 shil_stream::GetFloatFlags(Sh4RegType reg1,Sh4RegType reg2)
{
	u32 rv=0;

	if (reg1==NoReg)
	{
		//error
	}
	//reg1 allways used
	rv|=FLAG_REG1;

	if (reg2!=NoReg)
		rv|=FLAG_REG2;

	if (IsReg64(reg1))
	{
		if (rv&FLAG_REG2)
		{
			if (!IsReg64(reg2))
			{
				//both operands need to be 64bit on float,float
				//?
			}
		}

		rv|=FLAG_64;
	}
	else
	{
		rv|=FLAG_32;
		/*if (rv&FLAG_REG2)
		{
			if (IsReg64(reg2))
			{
				//both operands need to be 64bit on float,float
				//?
				rv|=FLAG_64;
			}
		}*/
	}

	return (u16)rv;
}
void shil_stream::fadd(Sh4RegType to,Sh4RegType from)
{
	emitRegReg(shilop_fadd,to,from,GetFloatFlags(to,from));
}
void shil_stream::fsub(Sh4RegType to,Sh4RegType from)
{
	emitRegReg(shilop_fsub,to,from,GetFloatFlags(to,from));
}
void shil_stream::fmul(Sh4RegType to,Sh4RegType from)
{
	emitRegReg(shilop_fmul,to,from,GetFloatFlags(to,from));
}
void shil_stream::fmac(Sh4RegType to,Sh4RegType from)
{
	emitRegReg(shilop_fmac,to,from,GetFloatFlags(to,from));
}
void shil_stream::fdiv(Sh4RegType to,Sh4RegType from)
{
	emitRegReg(shilop_fdiv,to,from,GetFloatFlags(to,from));
}

void shil_stream::fabs(Sh4RegType to)
{
	emitReg(shilop_fabs,to,GetFloatFlags(to,NoReg));
}
void shil_stream::fneg(Sh4RegType to)
{
	emitReg(shilop_fneg,to,GetFloatFlags(to,NoReg));
}


void shil_stream::fipr(Sh4RegType to,Sh4RegType from)
{
	emitRegReg(shilop_fipr,to,from,GetFloatFlags(to,from));
}


void shil_stream::fsqrt(Sh4RegType to)
{
	emitReg(shilop_fsqrt,to,GetFloatFlags(to,NoReg));
}

void shil_stream::ftrv(Sh4RegType fv_n)
{
	emitReg(shilop_ftrv,fv_n,GetFloatFlags(fv_n,NoReg));
}
void shil_stream::floatfpul(Sh4RegType frn)
{
	emitReg(shilop_floatfpul,frn,GetFloatFlags(frn,NoReg));
}

void shil_stream::fsca(Sh4RegType frn)
{
	emitReg(shilop_fsca,frn,GetFloatFlags(frn,NoReg));
}

void shil_stream::fsrra(Sh4RegType frn)
{
	emitReg(shilop_fsrra,frn,GetFloatFlags(frn,NoReg));
}


void shil_stream::ftrc(Sh4RegType frn)
{
	emitReg(shilop_ftrc,frn,GetFloatFlags(frn,NoReg));
}
void shil_stream::fcmp(Sh4RegType to,Sh4RegType from)
{
	emit32(shilop_fcmp,to,from,FLAG_SETFLAGS);
}
void shil_stream::div(Sh4RegType r0,Sh4RegType r1,Sh4RegType r2,u32 flags)
{
	emit(shilop_div32,r0,r1,r2,0,FLAG_IMM1|FLAG_REG1|FLAG_REG2|flags);	//WARNING !! NEEDS FIXUP for 3rd reg type
}

void shil_stream::shil_ifb(u32 opcode,u32 pc)
{
	emit(shilop_ifb,NoReg,NoReg,opcode,pc,FLAG_IMM1|FLAG_IMM2);	//size flags ect are ignored on this opcode
}

char* shil_names[]=
{
	//mov reg,reg	[32|64]
	//mov reg,const	[32]
	"mov",

	/*** Mem reads ***/
	//readmem reg,[base+reg]	[s]			[8|16|32|64]
	//readmem reg,[base+const]	[s]			[8|16|32|64]
	"readm",

	/*** Mem writes ***/
	//writemem reg,[base+reg]	[]			[8|16|32|64]
	//writemem reg,[base+const]	[]			[8|16|32|64]
	"writem",
	
	//cmp reg,reg
	//cmp reg,imm [s] [8]
	"cmp",

	//cmp reg,reg
	//cmp reg,imm [] [8]
	"test",

	//SaveT/LoadT cond
	"SaveT",
	"LoadT",

	//bit shits
	//neg reg
	"neg",
	//not reg
	"not",

	//bitwise ops

	//and reg,reg
	//and reg,const [32]
	"and",

	//and reg,reg
	//and reg,const [32]
	"or",

	//and reg,reg
	//and reg,const [32]
	"xor",
	

	//logical shifts

	//shl reg,const [8]
	"shl",
	//shr reg,const [8]
	"shr",

	//arithmetic shifts

	//sal reg,const [8]
	//sal is same as shl
	//sar reg,const [8]
	"sar",

	//rotate

	//rcl reg,const [8]
	"rcl",
	
	//rcr reg,const [8]
	"rcr",
	
	//rol reg,const [8]
	"rol",
	
	//ror reg,const [8]
	"ror",

	//swaps
	//swap [16|32]
	"swap",

	//moves w/ extend
	//signed - unsigned
	//movex reg,reg		[s] [8|16]
	//movsxb reg,reg	 s	 8
	//movsxb reg,reg	 s	 16
	//movzxb reg,reg	 z	 8
	//movzxw reg,reg	 z	 16
	"movex",

	//maths (integer)
	//adc reg,reg
	"adc",

	//add reg,reg
	//add reg,const
	"add",

	//sub reg,reg
	//sub reg,const
	"sub",

	//floating
	//basic ops

	//fadd reg,reg [32|64]
	"fadd",
	//fsub reg,reg [32|64]
	"fsub",
	//fmul reg,reg [32|64]
	"fmul",
	//fdiv reg,reg [32|64]
	"fdiv",

	
	//fabs reg [32|64]
	"fabs",
	//fneg reg [32|64]
	"fneg",

	//pfftt
	//fmac r0,reg,reg [32|64]
	"fmac",

	//shil_ifb const , const
	"shil_ifb",
	//JCond T==imm
	"jcond",
	//Jmp
	"jmp",
	//mul [s] [16|32|64] 16*16->32 , 32*32->32 , 32*32->64
	"mul",
	
	//more fpu fun :)
	"fsqrt",
	"ftrv",
	"fipr",
	"floatfpul",
	"ftrc",

	"fsca",
	"fsrra",
	"div32",
	"fcmp"
};
char* GetShilName(shil_opcodes ops)
{
	if (ops>shilop_count)
	{
		log("SHIL ERROR\n");
	}
	return shil_names[ops];
}