#include "sh4_cpu_shil.h"
#include "emitter/emitter.h"
#include "dc/mem/sh4_mem.h"
#include "dc/sh4/sh4_opcode_list.h"
#include "dc/sh4/sh4_registers.h"
#include "dc/sh4/shil/shil.h"
#include "log\logging_interface.h"
#include <assert.h>

#undef sh4op

#define SH4_REC

shil_stream* ilst;

#define sh4op(str) void  __fastcall rec_shil_##str (u32 op,u32 pc,BasicBlock* bb)

#define GetN(str) ((str>>8) & 0xf)
#define GetM(str) ((str>>4) & 0xf)
#define GetImm4(str) ((str>>0) & 0xf)
#define GetImm8(str) ((u8)(str>>0))
#define GetSImm8(str) ((s8)((str>>0) & 0xff))
#define GetImm12(str) ((str>>0) & 0xfff)
#define GetSImm12(str) (((s16)((GetImm12(str))<<4))>>3)

//#define tmu_underflow  0x0100
#define iNimp(info) rec_shil_iNimp(pc,op,info)

#define shil_interpret(str)  ilst->shil_ifb(str,pc);

Sh4RegType dyna_reg_id_r[16];
Sh4RegType dyna_reg_id_r_bank[8];

Sh4RegType dyna_reg_id_fr[16];
Sh4RegType dyna_reg_id_xf[16];

Sh4RegType dyna_reg_id_dr[8];
Sh4RegType dyna_reg_id_xd[8];

#define r dyna_reg_id_r
#define r_bank dyna_reg_id_r_bank

#define fr dyna_reg_id_fr
#define xf dyna_reg_id_xf

#define dr dyna_reg_id_dr
#define xd dyna_reg_id_xd

void rec_shil_iNimp(u32 pc,u32 op ,char * text)
{
	printf("SHIL: Internal fatal error: %s",text);

	printf("PC 0x%x , OP = 0x%x\n",pc,op);

	printf("GPR regs : \n"); //dump the useful only
	for(s32 i = 0;i < 16;i++)
		printf("r[%d] : 0x%x,",i,r[i]);
	printf("\nFP regs : \n");
	for(s32 i = 0;i < 16;i++)
		printf("fr[%d] : %f,",i,fr[i]);

	printf("\nControl regs : \n"); 
	printf("RM %d DN : %d PR %d SZ %d FR %d PR_SZ %d NIL %d\n",fpscr.RM,fpscr.DN,fpscr.PR,fpscr.SZ,fpscr.FR,fpscr.PR_SZ,fpscr.nil);
	printf("finexact %d,",fpscr.finexact);
	printf("funderflow %d,",fpscr.funderflow);
	printf("foverflow %d,",fpscr.foverflow);
	printf("fdivbyzero %d,",fpscr.fdivbyzero);
	printf("finvalidop %d,",fpscr.finvalidop);
	printf("einexact %d,",fpscr.einexact);
	printf("eunderflow %d,",fpscr.eunderflow);
	printf("eoverflow %d,",fpscr.eoverflow);
	printf("edivbyzero %d,",fpscr.edivbyzero);
	printf("einvalidop %d,",fpscr.einvalidop);
	printf("cinexact %d,",fpscr.cinexact);
	printf("cunderflow %d,",fpscr.cunderflow);
	printf("coverflow %d,",fpscr.coverflow);
	printf("cdivbyzero %d,",fpscr.cdivbyzero);
	printf("cinvalid %d,",fpscr.cinvalid);
	printf("cfpuerr %d\n",fpscr.cfpuerr);

	//while(1){}
	die("SHIL: Internal fatal error");
}
//************************ TLB/Cache ************************
//ldtlb                         
sh4op(i0000_0000_0011_1000)
{
	shil_interpret(op);
}

//ocbi @<REG_N>                 
sh4op(i0000_nnnn_1001_0011)
{
	shil_interpret(op);
} 

//ocbp @<REG_N>                 
sh4op(i0000_nnnn_1010_0011)
{
	shil_interpret(op);
} 

//ocbwb @<REG_N>                
sh4op(i0000_nnnn_1011_0011)
{
	shil_interpret(op);
} 

//pref @<REG_N>                 
sh4op(i0000_nnnn_1000_0011)
{
	u32 n = GetN(op);
	ilst->pref(r[n]);
}

//************************ Set/Get T/S ************************
//sets                          
sh4op(i0000_0000_0101_1000)
{
	shil_interpret(op);
} 

//clrs                          
sh4op(i0000_0000_0100_1000)
{
	shil_interpret(op);
} 

//sett                          
sh4op(i0000_0000_0001_1000)
{
	ilst->mov(reg_sr_T,1);
} 

//clrt                          
sh4op(i0000_0000_0000_1000)
{
	ilst->mov(reg_sr_T,0);
} 

//movt <REG_N>                  
sh4op(i0000_nnnn_0010_1001)
{
	u32 n = GetN(op);
	ilst->mov(r[n],reg_sr_T);
} 

//************************ Reg Compares ************************
//cmp/pz <REG_N>                
sh4op(i0100_nnnn_0001_0001)
{
	u32 n = GetN(op);

	ilst->cmp(r[n],(s8)0);			//singed compare
	ilst->SaveT(CC_NL);
}

//cmp/pl <REG_N>                
sh4op(i0100_nnnn_0001_0101)
{
	u32 n = GetN(op);

	ilst->cmp(r[n],(s8)0);			//singed compare
	ilst->SaveT(CC_NLE);
}

//cmp/eq #<imm>,R0              
sh4op(i1000_1000_iiii_iiii)
{
	ilst->cmp(r[0],GetSImm8(op));
	ilst->SaveT(CC_E);
}

//cmp/eq <REG_M>,<REG_N>        
sh4op(i0011_nnnn_mmmm_0000)
{
	u32 n = GetN(op);
	u32 m = GetM(op);

	ilst->cmp(r[n],r[m]);
	ilst->SaveT(CC_E);
}

//cmp/hs <REG_M>,<REG_N>        
sh4op(i0011_nnnn_mmmm_0010)
{
	u32 n = GetN(op);
	u32 m = GetM(op);

	ilst->cmp(r[n],r[m]);
	ilst->SaveT(CC_AE);
}

//cmp/ge <REG_M>,<REG_N>        
sh4op(i0011_nnnn_mmmm_0011)
{
	u32 n = GetN(op);
	u32 m = GetM(op);

	ilst->cmp(r[n],r[m]);
	ilst->SaveT(CC_GE);
}

//cmp/hi <REG_M>,<REG_N>        
sh4op(i0011_nnnn_mmmm_0110)
{
	u32 n = GetN(op);
	u32 m = GetM(op);

	ilst->cmp(r[n],r[m]);
	ilst->SaveT(CC_A);
}

//cmp/gt <REG_M>,<REG_N>        
sh4op(i0011_nnnn_mmmm_0111)
{
	u32 n = GetN(op);
	u32 m = GetM(op);

	ilst->cmp(r[n],r[m]);
	ilst->SaveT(CC_G);
}

//cmp/str <REG_M>,<REG_N>       
sh4op(i0010_nnnn_mmmm_1100)
{
	shil_interpret(op);
}

//tst #<imm>,R0                 
sh4op(i1100_1000_iiii_iiii)
{
	ilst->test(r[0],GetImm8(op));
	ilst->SaveT(CC_Z);
}
//tst <REG_M>,<REG_N>           
sh4op(i0010_nnnn_mmmm_1000)
{
	u32 n = GetN(op);
	u32 m = GetM(op);

	ilst->test(r[n],r[m]);
	ilst->SaveT(CC_Z);
}
//************************ mulls! ************************ 
//mulu.w <REG_M>,<REG_N>          
sh4op(i0010_nnnn_mmmm_1110)
{
	u32 n = GetN(op);
	u32 m = GetM(op);

	ilst->mulu_16_16_32(r[n],r[m]);
}

//muls.w <REG_M>,<REG_N>          
sh4op(i0010_nnnn_mmmm_1111)
{	
	u32 n = GetN(op);
	u32 m = GetM(op);

	ilst->muls_16_16_32(r[n],r[m]);
}

//dmulu.l <REG_M>,<REG_N>       
sh4op(i0011_nnnn_mmmm_0101)
{
	u32 n = GetN(op);
	u32 m = GetM(op);
	
	ilst->mulu_32_32_64(r[n],r[m]);
}

//dmuls.l <REG_M>,<REG_N>       
sh4op(i0011_nnnn_mmmm_1101)
{
	u32 n = GetN(op);
	u32 m = GetM(op);
	
	ilst->muls_32_32_64(r[n],r[m]);
}

//mac.w @<REG_M>+,@<REG_N>+     
sh4op(i0100_nnnn_mmmm_1111)
{
	shil_interpret(op);
}

//mac.l @<REG_M>+,@<REG_N>+     
sh4op(i0000_nnnn_mmmm_1111)
{
	shil_interpret(op);
}

//mul.l <REG_M>,<REG_N>         
sh4op(i0000_nnnn_mmmm_0111)
{
	u32 n = GetN(op);
	u32 m = GetM(op);

	ilst->muls_32_32_32(r[n],r[m]);
}

//************************ Div ! ************************ 
#define MASK_N_M 0xF00F
#define MASK_N   0xF0FF
#define MASK_NONE   0xFFFF

#define DIV0U_KEY 0x0019
#define DIV0S_KEY 0x2007
#define DIV1_KEY 0x3004
#define ROTCL_KEY 0x4024

Sh4RegType div_som_reg1;
Sh4RegType div_som_reg2;
Sh4RegType div_som_reg3;

u32 MatchDiv32(u32 pc , Sh4RegType &reg1,Sh4RegType &reg2 , Sh4RegType &reg3)
{
	if (settings.dynarec.Safe)
		return 0;

	u32 v_pc=pc;
	u32 match=1;
	for (int i=0;i<32;i++)
	{
		u16 opcode=ReadMem16(v_pc);
		v_pc+=2;
		if ((opcode&MASK_N)==ROTCL_KEY)
		{
			if (reg1==NoReg)
				reg1=(Sh4RegType)GetN(opcode);
			else if (reg1!=(Sh4RegType)GetN(opcode))
				break;
			match++;
		}
		else
			break;
		
		opcode=ReadMem16(v_pc);
		v_pc+=2;
		if ((opcode&MASK_N_M)==DIV1_KEY)
		{
			if (reg2==NoReg)
				reg2=(Sh4RegType)GetM(opcode);
			else if (reg2!=(Sh4RegType)GetM(opcode))
				break;
			
			if (reg2==reg1)
				break;

			if (reg3==NoReg)
				reg3=(Sh4RegType)GetN(opcode);
			else if (reg3!=(Sh4RegType)GetN(opcode))
				break;
			
			if (reg3==reg1)
				break;

			match++;
		}
		else
			break;
	}
	
	return match;
}
bool __fastcall MatchDiv32u(u32 op,u32 pc)
{
	div_som_reg1=NoReg;
	div_som_reg2=NoReg;
	div_som_reg3=NoReg;

	u32 match=MatchDiv32(pc+2,div_som_reg1,div_som_reg2,div_som_reg3);


	//log("DIV32U matched %d%% @ 0x%X\n",match*100/65,pc);
	if (match==65)
	{
		//DIV32U was perfectly matched :)
		return true;
	}
	else //no match ...
		return false;
}

bool __fastcall MatchDiv32s(u32 op,u32 pc)
{
	u32 n = GetN(op);
	u32 m = GetM(op);

	div_som_reg1=NoReg;
	div_som_reg2=(Sh4RegType)m;
	div_som_reg3=(Sh4RegType)n;

	u32 match=MatchDiv32(pc+2,div_som_reg1,div_som_reg2,div_som_reg3);
	log("DIV32S matched %d%% @ 0x%X\n",match*100/65,pc);
	
	if (match==65)
	{
		//DIV32S was perfectly matched :)
		return true;
	}
	else //no match ...
		return false;
}
//div0u                         
sh4op(i0000_0000_0001_1001)
{	
	if (MatchDiv32u(op,pc))
	{
		//DIV32U was perfectly matched :)
		bb->flags.SynthOpcode=BLOCK_SOM_SIZE_128;
		ilst->div(div_som_reg1,div_som_reg2,div_som_reg3,FLAG_ZX|FLAG_32);
	}
	else //fallback to interpreter (16b div propably)
		shil_interpret(op);
}
//div0s <REG_M>,<REG_N>         
sh4op(i0010_nnnn_mmmm_0111)
{
	if (MatchDiv32s(op,pc))
	{
		//DIV32S was perfectly matched :)
		bb->flags.SynthOpcode=BLOCK_SOM_SIZE_128;
		ilst->div(div_som_reg1,div_som_reg2,div_som_reg3,FLAG_SX|FLAG_32);
		log("div32s %d/%d/%d\n",div_som_reg1,div_som_reg2,div_som_reg3);
		//shil_interpret(op);
	}
	else //fallback to interpreter (16b div propably)
		shil_interpret(op);

	return;
}

//div1 <REG_M>,<REG_N>          
sh4op(i0011_nnnn_mmmm_0100)
{
	//u32 n=GetN(op);
	//u32 m=GetM(op);

	shil_interpret(op);
}

//************************ Simple maths ************************ 
//addc <REG_M>,<REG_N>          
sh4op(i0011_nnnn_mmmm_1110)
{
	u32 n=GetN(op);
	u32 m=GetM(op);

	ilst->LoadT(CF);			//load T to carry flag
	ilst->adc(r[n],r[m]);		//add w/ carry
	ilst->SaveT(SaveCF);//save CF to T
}

// addv <REG_M>,<REG_N>          
sh4op(i0011_nnnn_mmmm_1111)
{
	shil_interpret(op);
}

//subc <REG_M>,<REG_N>          
sh4op(i0011_nnnn_mmmm_1010)
{
	//u32 n = GetN(op);
	//u32 m = GetM(op);
/*
	ilst->neg(r[n]);			//dest=-dest
	ilst->LoadT(CF);			//load T to carry flag
	ilst->adc(r[n],r[m]);		//add w/ carry
	ilst->SaveT(SaveCF);//save CF to T
*/
	shil_interpret(op);
}

//subv <REG_M>,<REG_N>          
sh4op(i0011_nnnn_mmmm_1011)
{
	shil_interpret(op);
}

//dt <REG_N>                    
sh4op(i0100_nnnn_0001_0000)
{
	u32 n = GetN(op);

	ilst->dec(r[n]);
	ilst->SaveT(CC_Z);
}

//negc <REG_M>,<REG_N>          
sh4op(i0110_nnnn_mmmm_1010)
{
	//u32 n = GetN(op);
	//u32 m = GetM(op);

	shil_interpret(op);
}

//neg <REG_M>,<REG_N>           
sh4op(i0110_nnnn_mmmm_1011)
{
	u32 n = GetN(op);
	u32 m = GetM(op);

	ilst->mov(r[n],r[m]);
	ilst->neg(r[n]);
} 

//not <REG_M>,<REG_N>           
sh4op(i0110_nnnn_mmmm_0111)
{
	u32 n = GetN(op);
	u32 m = GetM(op);

	ilst->mov(r[n],r[m]);
	ilst->not(r[n]);
} 

//************************ shifts/rotates ************************
//shll <REG_N>                  
sh4op(i0100_nnnn_0000_0000)
{
	u32 n = GetN(op);

	ilst->shl(r[n],1);
	ilst->SaveT(SaveCF);
}

//shal <REG_N>                  
sh4op(i0100_nnnn_0010_0000)
{
	u32 n=GetN(op);

	ilst->shl(r[n],1);
	ilst->SaveT(SaveCF);
}

//shlr <REG_N>                  
sh4op(i0100_nnnn_0000_0001)
{
	u32 n = GetN(op);
	
	ilst->shr(r[n],1);
	ilst->SaveT(SaveCF);
}

//shar <REG_N>                  
sh4op(i0100_nnnn_0010_0001)
{
	u32 n = GetN(op);

	ilst->sar(r[n],1);
	ilst->SaveT(SaveCF);
}

//shad <REG_M>,<REG_N>          
sh4op(i0100_nnnn_mmmm_1100)
{
	shil_interpret(op);
}

//shld <REG_M>,<REG_N>          
sh4op(i0100_nnnn_mmmm_1101)
{
	shil_interpret(op);
}

//rotcl <REG_N>                 
sh4op(i0100_nnnn_0010_0100)
{
	u32 n = GetN(op);

	ilst->LoadT(CF);
	ilst->rcl(r[n]);
	ilst->SaveT(SaveCF);
}

//rotl <REG_N>                  
sh4op(i0100_nnnn_0000_0100)
{
	u32 n = GetN(op);

	ilst->rol(r[n]);
	ilst->SaveT(SaveCF);
}

//rotcr <REG_N>                 
sh4op(i0100_nnnn_0010_0101)
{
	u32 n = GetN(op);

	ilst->LoadT(CF);
	ilst->rcr(r[n]);
	ilst->SaveT(SaveCF);
}

//rotr <REG_N>                  
sh4op(i0100_nnnn_0000_0101)
{
	u32 n = GetN(op);

	ilst->ror(r[n]);
	ilst->SaveT(SaveCF);
}					
//************************ byte reorder/sign ************************
//swap.b <REG_M>,<REG_N>        
sh4op(i0110_nnnn_mmmm_1000)
{
	u32 m = GetM(op);
	u32 n = GetN(op);

	ilst->mov(r[n],r[m]);
	ilst->bswap(r[n]);
} 

//swap.w <REG_M>,<REG_N>        
sh4op(i0110_nnnn_mmmm_1001)
{
	u32 n = GetN(op);
	u32 m = GetM(op);

	ilst->mov(r[n],r[m]);
	ilst->wswap(r[n]);
} 

//extu.b <REG_M>,<REG_N>        
sh4op(i0110_nnnn_mmmm_1100)
{
	u32 n = GetN(op);
	u32 m = GetM(op);

	ilst->movzxb(r[n],r[m]);
} 

//extu.w <REG_M>,<REG_N>        
sh4op(i0110_nnnn_mmmm_1101)
{
	u32 n = GetN(op);
	u32 m = GetM(op);

	ilst->movzxw(r[n],r[m]);
} 

//exts.b <REG_M>,<REG_N>        
sh4op(i0110_nnnn_mmmm_1110)
{
	u32 n = GetN(op);
	u32 m = GetM(op);

	ilst->movsxb(r[n],r[m]);
} 


//exts.w <REG_M>,<REG_N>        
sh4op(i0110_nnnn_mmmm_1111)
{
	u32 n = GetN(op);
	u32 m = GetM(op);

	ilst->movsxw(r[n],r[m]);
} 


//xtrct <REG_M>,<REG_N>         
sh4op(i0010_nnnn_mmmm_1101)
{
	//u32 n = GetN(op);
	//u32 m = GetM(op);

	shil_interpret(op);
}

//************************ xxx.b #<imm>,@(R0,GBR) ************************
//tst.b #<imm>,@(R0,GBR)        
sh4op(i1100_1100_iiii_iiii)
{
	shil_interpret(op);
}

//and.b #<imm>,@(R0,GBR)        
sh4op(i1100_1101_iiii_iiii)
{
	shil_interpret(op);
}

//xor.b #<imm>,@(R0,GBR)        
sh4op(i1100_1110_iiii_iiii)
{
	shil_interpret(op);
}

//or.b #<imm>,@(R0,GBR)         
sh4op(i1100_1111_iiii_iiii)
{
	shil_interpret(op);
}

//tas.b @<REG_N>                
sh4op(i0100_nnnn_0001_1011)
{
	shil_interpret(op);
}

//bah
//Not implt
sh4op(iNotImplemented)
{
	shil_interpret(op);
}

sh4op(gdrom_hle_op)
{
	EMUERROR("GDROM HLE NOT SUPPORTED");
}

//all fpu emulation ops :)

//fadd <FREG_M>,<FREG_N>
sh4op(i1111_nnnn_mmmm_0000)
{//TODO : CHECK THIS PR FP
	
	if (fpscr.PR == 0)
	{
		u32 n = GetN(op);
		u32 m = GetM(op);
		ilst->fadd(fr[n],fr[m]);
		//fr[n] += fr[m];
		//CHECK_FPU_32(fr[n]);
	}
	else
	{
		shil_interpret(op);
		return;
		/*
		u32 n = (op >> 9) & 0x07;
		u32 m = (op >> 5) & 0x07;
		
		ilst->fadd(dr[n],dr[m]);
		*/
		//START64();
		//double drn=GetDR(n), drm=GetDR(m);
		//drn += drm;
		//CHECK_FPU_64(drn);
		//SetDR(n,drn);
		//END64();
	}
}

//fsub <FREG_M>,<FREG_N>   
sh4op(i1111_nnnn_mmmm_0001)
{
	if (fpscr.PR == 0)
	{
		u32 n = GetN(op);
		u32 m = GetM(op);

		ilst->fsub(fr[n],fr[m]);
		//fr[n] -= fr[m];
		//CHECK_FPU_32(fr[n]);
	}
	else
	{
		shil_interpret(op);
		return;
		//u32 n = (op >> 9) & 0x07;
		//u32 m = (op >> 5) & 0x07;
		//
		//ilst->fsub(dr[n],dr[m]);
		//START64();
		//double drn=GetDR(n), drm=GetDR(m);
		//drn-=drm;
		//dr[n] -= dr[m];
		//SetDR(n,drn);
		//END64();
	}
}																								
//fmul <FREG_M>,<FREG_N>   
sh4op(i1111_nnnn_mmmm_0010)
{
	if (fpscr.PR == 0)
	{
		u32 n = GetN(op);
		u32 m = GetM(op);
		ilst->fmul(fr[n],fr[m]);
	}
	else
	{
		shil_interpret(op);
		return;
		//u32 n = (op >> 9) & 0x07;
		//u32 m = (op >> 5) & 0x07;
		//START64();
		//double drn=GetDR(n), drm=GetDR(m);
		//drn*=drm;
		//dr[n] *= dr[m];
		//SetDR(n,drn);
		//END64();
		//ilst->fmul(dr[n],dr[m]);
	}
}
//fdiv <FREG_M>,<FREG_N>   
sh4op(i1111_nnnn_mmmm_0011)
{
	if (fpscr.PR == 0)
	{
		u32 n = GetN(op);
		u32 m = GetM(op);
		ilst->fdiv(fr[n],fr[m]);
	}
	else
	{
		shil_interpret(op);
		return;
		//u32 n = (op >> 9) & 0x07;
		//u32 m = (op >> 5) & 0x07;
		//START64();
		//double drn=GetDR(n), drm=GetDR(m);
		//drn/=drm;
		//SetDR(n,drn);
		//END64();
		//ilst->fdiv(dr[n],dr[m]);
	}
}
//fcmp/eq <FREG_M>,<FREG_N>
sh4op(i1111_nnnn_mmmm_0100)
{
	if (fpscr.PR == 0)
	{
		u32 n = GetN(op);
		u32 m = GetM(op);

		//sr.T = (fr[m] == fr[n]) ? 1 : 0;
		ilst->fcmp(fr[n],fr[m]);
		ilst->SaveT(CC_FPU_E);
	}
	else
	{
		shil_interpret(op);
		return;
		//u32 n = (op >> 9) & 0x07;
		//u32 m = (op >> 5) & 0x07;
		//ilst->cmp(dr[n],dr[m]);
		//START64();
		//sr.T = (GetDR(m) == GetDR(n)) ? 1 : 0;
		//END64();	
	}
}
//fcmp/gt <FREG_M>,<FREG_N>
sh4op(i1111_nnnn_mmmm_0101)
{
	if (fpscr.PR == 0)
	{
		u32 n = GetN(op);
		u32 m = GetM(op);

		/*if (fr[n] > fr[m])
			sr.T = 1;
		else
			sr.T = 0;*/
		ilst->fcmp(fr[n],fr[m]);
		ilst->SaveT(CC_NBE);
	}
	else
	{
		shil_interpret(op);
		return;
		//u32 n = (op >> 9) & 0x07;
		//u32 m = (op >> 5) & 0x07;
		
		/*START64();
		if (GetDR(n) > GetDR(m))
			sr.T = 1;
		else
			sr.T = 0;
		END64();*/

		//ilst->cmp(dr[n],dr[m]);
	}
}
//fmov.s @(R0,<REG_M>),<FREG_N>
sh4op(i1111_nnnn_mmmm_0110)
{
	if (fpscr.SZ == 0)
	{
		u32 n = GetN(op);
		u32 m = GetM(op);

		//fr_hex[n] = ReadMem32(r[m] + r[0]);
		ilst->readm32(fr[n],r[0],r[m]);
	}
	else
	{
		//shil_interpret(op);
		//return;
		u32 n = (op >> 8) & 0x0E;
		u32 m = GetM(op);
		if (((op >> 8) & 0x1) == 0)
		{
			//fr_hex[n] = ReadMem32(r[m] + r[0]);
			//fr_hex[n + 1] = ReadMem32(r[m] + r[0] + 4);
			ilst->readm64(dr[n>>1],r[0],r[m]);
		}
		else
		{
			//xf_hex[n] = ReadMem32(r[m] + r[0]);
			//xf_hex[n + 1] = ReadMem32(r[m] + r[0] + 4);
			ilst->readm64(xd[n>>1],r[0],r[m]);
		}
	}
}


//fmov.s <FREG_M>,@(R0,<REG_N>)
sh4op(i1111_nnnn_mmmm_0111)
{//used
	if (fpscr.SZ == 0)
	{
		u32 n = GetN(op);
		u32 m = GetM(op);

		//WriteMem32(r[0] + r[n], fr_hex[m]);
		ilst->writem32(fr[m],r[0],r[n]);
	}
	else
	{
		//shil_interpret(op);
		//return;
		u32 n = GetN(op);
		u32 m = (op >> 4) & 0x0E;
		if (((op >> 4) & 0x1) == 0)
		{
			//WriteMem32(r[n] + r[0], fr_hex[m]);
			//WriteMem32(r[n] + r[0] + 4, fr_hex[m + 1]);
			ilst->writem64(dr[m>>1],r[0],r[n]);
		}
		else
		{
			//WriteMem32(r[n] + r[0], xf_hex[m]);
			//WriteMem32(r[n] + r[0] + 4, xf_hex[m + 1]);
			ilst->writem64(xd[m>>1],r[0],r[n]);
		}
	}
}


//fmov.s @<REG_M>,<FREG_N> 
sh4op(i1111_nnnn_mmmm_1000)
{//used
	if (fpscr.SZ == 0)
	{
		u32 n = GetN(op);
		u32 m = GetM(op);
		//fr_hex[n] = ReadMem32(r[m]);
		ilst->readm32(fr[n],r[m]);
	}
	else
	{
		//shil_interpret(op);
		//return;
		u32 n = GetN(op);
		u32 m = GetM(op);
		if ((n & 0x1) == 0)
		{
			//fr_hex[n] = ReadMem32(r[m]);
			//fr_hex[n + 1] = ReadMem32(r[m] + 4);
			ilst->readm64(dr[n>>1],r[m]);
			//ilst->readm32(fr[n&~1],r[m]);
			//ilst->readm32(fr[n|1],r[m],4);
		}
		else
		{
			//xf_hex[n] = ReadMem32(r[m]);
			//xf_hex[n + 1] = ReadMem32(r[m] + 4);
			ilst->readm64(xd[n>>1],r[m]);
			//ilst->readm32(xf[n&~1],r[m]);
			//ilst->readm32(xf[n|1],r[m],4);
		}
	}
}


//fmov.s @<REG_M>+,<FREG_N>
sh4op(i1111_nnnn_mmmm_1001)
{
	if (fpscr.SZ == 0)
	{
		u32 n = GetN(op);
		u32 m = GetM(op);

		//fr_hex[n] = ReadMem32(r[m]);
		ilst->readm32(fr[n],r[m]);
		ilst->add(r[m],4);
		//r[m] += 4;
	}
	else 
	{
		//shil_interpret(op);
		//return;
		u32 n = GetN(op);
		u32 m = GetM(op);
		if ((n & 0x1) == 0)
		{
			//fr_hex[n] = ReadMem32(r[m]);
			//fr_hex[n + 1] = ReadMem32(r[m]+ 4);

			ilst->readm64(dr[n>>1],r[m]);
			//ilst->readm32(fr[n&~1],r[m]);
			//ilst->readm32(fr[n|1],r[m],4);
		}
		else
		{
			//xf_hex[n] = ReadMem32(r[m] );
			//xf_hex[n + 1] = ReadMem32(r[m]+ 4);
			ilst->readm64(xd[n>>1],r[m]);
			//ilst->readm32(xf[n&~1],r[m]);
			//ilst->readm32(xf[n|1],r[m],4);
		}
		ilst->add(r[m],8);
		//r[m] += 8;
	}
}


//fmov.s <FREG_M>,@<REG_N>
sh4op(i1111_nnnn_mmmm_1010)
{
	if (fpscr.SZ == 0)
	{
		u32 n = GetN(op);
		u32 m = GetM(op);
		//WriteMem32(r[n], fr_hex[m]);
		ilst->writem32(fr[m],r[n]);
	}
	else
	{
		//shil_interpret(op);
		//return;
		
		u32 n = GetN(op);
		u32 m = (op >> 4) & 0x0E;

		if (((op >> 4) & 0x1) == 0)
		{
			//WriteMem32(r[n], fr_hex[m]);
			//WriteMem32(r[n] + 4, fr_hex[m + 1]);
			ilst->writem64(dr[m>>1],r[n]);
		}
		else
		{
			//WriteMem32(r[n], xf_hex[m]);
			//WriteMem32(r[n] + 4, xf_hex[m + 1]);
			ilst->writem64(xd[m>>1],r[n]);
		}
	}
}

//fmov.s <FREG_M>,@-<REG_N>
sh4op(i1111_nnnn_mmmm_1011)
{//used
	if (fpscr.SZ == 0)
	{
		//iNimp("fmov.s <FREG_M>,@-<REG_N>");
		u32 n = GetN(op);
		u32 m = GetM(op);

		//r[n] -= 4;
		ilst->sub(r[n],4);
		ilst->writem32(fr[m],r[n]);
		//WriteMem32(r[n], fr_hex[m]);
	}
	else
	{
		//shil_interpret(op);
		//return;
	
		u32 n = GetN(op);
		u32 m = (op >> 4) & 0x0E;

		//r[n] -= 8;
		if (((op >> 4) & 0x1) == 0)
		{ 
			//WriteMem32(r[n] , fr_hex[m]);
			//WriteMem32(r[n]+ 4, fr_hex[m + 1]);
			ilst->sub(r[n],8);
			ilst->writem64(dr[m>>1],r[n]);
			
		}
		else
		{
			//WriteMem32(r[n] , xf_hex[m]);
			//WriteMem32(r[n]+ 4, xf_hex[m + 1]);
			ilst->sub(r[n],8);
			ilst->writem64(xd[m>>1],r[n]);
		}
	}
}


//fmov <FREG_M>,<FREG_N>   
sh4op(i1111_nnnn_mmmm_1100)
{//TODO : checkthis
	if (fpscr.SZ == 0)
	{
		u32 n = GetN(op);
		u32 m = GetM(op);
		//fr[n] = fr[m];
		ilst->mov(fr[n],fr[m]);
	}
	else
	{
		u32 n = (op >> 9) & 0x7;
		u32 m = (op >> 5) & 0x7;

		switch ((op >> 4) & 0x11)
		{
			case 0x00:
				//dr[n] = dr[m];
				//fr_hex[n] = fr_hex[m];
				//fr_hex[n + 1] = fr_hex[m + 1];
				ilst->mov(dr[n],dr[m]);
				break;
			case 0x01:
				//dr[n] = xf[m];
				//fr_hex[n] = xf_hex[m];
				//fr_hex[n + 1] = xf_hex[m + 1];
				ilst->mov(dr[n],xd[m]);
				break;
			case 0x10:
				//xf[n] = dr[m];
				//xf_hex[n] = fr_hex[m];
				//xf_hex[n + 1] = fr_hex[m + 1];
				ilst->mov(xd[n],dr[m]);
				break;
			case 0x11:
				//xf[n] = xf[m];
				//xf_hex[n] = xf_hex[m];
				//xf_hex[n + 1] = xf_hex[m + 1];
				ilst->mov(xd[n],xd[m]);
				break;
		}
	}
}


//fabs <FREG_N>            
sh4op(i1111_nnnn_0101_1101)
{
	int n=GetN(op);
	
	if (fpscr.PR ==0)
	{
		//fr_hex[n]&=0x7FFFFFFF;
		ilst->fabs(fr[n]);
	}
	else
	{
		//fr_hex[(n&0xE)+1]&=0x7FFFFFFF;
		ilst->fabs(dr[n>>1]);
	}

}

//FSCA FPUL, DRn//F0FD//1111_nnn0_1111_1101
sh4op(i1111_nnn0_1111_1101)
{
	if (fpscr.PR==0)
	{
		int n=GetN(op) & 0xE;
		ilst->fsca(fr[n]);
		//shil_interpret(op);
	}
	else
		iNimp("FSCA : Double precision mode");

}

//FSRRA //1111_nnnn_0111_1101
sh4op(i1111_nnnn_0111_1101)
{

	// What about double precision?
	if (fpscr.PR==0)
	{
		u32 n = GetN(op);
		ilst->fsrra(fr[n]);
		//shil_interpret(op);
	}
	else
		iNimp("FSRRA : Double precision mode");		

}

//fcnvds <DR_N>,FPUL       
sh4op(i1111_nnnn_1011_1101)
{
	if (fpscr.PR == 1)
	{
		shil_interpret(op);
		return;
	}
	else
	{
		iNimp("fcnvds <DR_N>,FPUL,m=0");
	}
}


//fcnvsd FPUL,<DR_N>       
sh4op(i1111_nnnn_1010_1101)
{

	if (fpscr.PR == 1)
	{
		shil_interpret(op);
		return;
	}
	else
	{
		iNimp("fcnvsd FPUL,<DR_N>,m=0");
	}
}
 
//fipr <FV_M>,<FV_N>            
sh4op(i1111_nnmm_1110_1101)
{
	int n=GetN(op)&0xC;
	int m=(GetN(op)&0x3)<<2;
	ilst->fipr(fr[n],fr[m]);
	bb->flags.FpuIsVector=true;
}


//fldi0 <FREG_N>           
sh4op(i1111_nnnn_1000_1101)
{
	if (fpscr.PR==0)
	{
		//iNimp("fldi0 <FREG_N>");
		u32 n = GetN(op);

		//fr[n] = 0.0f;
		ilst->mov(fr[n],0);//0.0f is 0x0 , right ?
	}
	else
	{
		iNimp("fldi0 <Dreg_N>");
	}
}


//fldi1 <FREG_N>           
sh4op(i1111_nnnn_1001_1101)
{
	if (fpscr.PR==0)
	{
		//iNimp("fldi1 <FREG_N>");
		u32 n = GetN(op);

		//fr[n] = 1.0f;
		ilst->mov(fr[n],0x3f800000);//1.0f is 0x3f800000 , right ?
	}
	else
	{
		iNimp("fldi1 <Dreg_N>");
	}
}


//flds <FREG_N>,FPUL       
sh4op(i1111_nnnn_0001_1101)
{
	//this seems to be a valid opcode even if double precicion is activated 
	//carrier requires this
	//u32 n = GetN(op);
	//fpul = fr_hex[n];
	ilst->mov(reg_fpul,fr[GetN(op)]);
}


//float FPUL,<FREG_N>      
sh4op(i1111_nnnn_0010_1101)
{//TODO : CHECK THIS (FP)

	if (fpscr.PR == 0)
	{
		u32 n = GetN(op);
		//fr[n] = (float)(int)fpul;
		ilst->floatfpul(fr[n]);
		//shil_interpret(op);
	}
	else
	{
		shil_interpret(op);
	}
}

//ftrc <FREG_N>, FPUL      
sh4op(i1111_nnnn_0011_1101)
{
	if (fpscr.PR == 0)
	{
		u32 n = GetN(op);
		//fpul = (u32)(s32)fr[n];
		ilst->ftrc(fr[n]);
		//shil_interpret(op);
	}
	else
	{
		shil_interpret(op);
	}
}


//fneg <FREG_N>            
sh4op(i1111_nnnn_0100_1101)
{
	u32 n = GetN(op);

	if (fpscr.PR ==0)
	{
		ilst->fneg(fr[n]);
	}
	else
	{
		ilst->fneg(dr[n>>1]);
	}
}


//frchg                    
sh4op(i1111_1011_1111_1101)
{
	shil_interpret(op);
	return;

	//iNimp("frchg");
 	//fpscr.FR = 1 - fpscr.FR;

	//UpdateFPSCR();
}


//fschg                    
sh4op(i1111_0011_1111_1101)
{

	shil_interpret(op);
	return;

	//iNimp("fschg");
	//fpscr.SZ = 1 - fpscr.SZ;
	//UpdateFPSCR();//*FixME* prob not needed
}

//fsqrt <FREG_N>                
sh4op(i1111_nnnn_0110_1101)
{
	if (fpscr.PR == 0)
	{
		u32 n = GetN(op);

		//fr[n] = (float)sqrt((double)fr[n]);
		//CHECK_FPU_32(fr[n]);
		ilst->fsqrt(fr[n]);
	}
	else
	{
		//Operation _can_ be done on sh4
		iNimp("fsqrt <DREG_N>");
	}
}

//fsts FPUL,<FREG_N>       
sh4op(i1111_nnnn_0000_1101)
{
	//this seems to be a valid opcode even if double precicion is activated 
	//carrier requires it
	//u32 n = GetN(op);
	ilst->mov(fr[GetN(op)],reg_fpul);

}

//fmac <FREG_0>,<FREG_M>,<FREG_N> 
sh4op(i1111_nnnn_mmmm_1110)
{
	//iNimp("fmac <FREG_0>,<FREG_M>,<FREG_N>");
	if (fpscr.PR==0)
	{
		u32 n = GetN(op);
		u32 m = GetM(op);

		//fr[n] += fr[0] * fr[m];
		ilst->fmac(fr[n],fr[m]);
		//CHECK_FPU_32(fr[n]);
	}
	else
	{
		iNimp("fmac <DREG_0>,<DREG_M>,<DREG_N>");
	}
}


//ftrv xmtrx,<FV_N>       
sh4op(i1111_nn01_1111_1101)
{
	u32 n=GetN(op)&0xC;
	ilst->ftrv(fr[n]);
	bb->flags.FpuIsVector=true;
}																				  


sh4op(icpu_nimp)
{
	shil_interpret(op);
}


//Branches

void DoDslot(u32 pc,BasicBlock* bb)
{
	u16 opcode=ReadMem16(pc+2);

	if (opcode==0 || opcode==0)
		log("0 on delayslot , ingoring it ..\n");
	else
		RecOpPtr[opcode](opcode,pc+2,bb);
		bb->flags.HasDelaySlot=true;
}

//braf <REG_N>                  
sh4op(i0000_nnnn_0010_0011)
{
	u32 n = GetN(op);
	/*
	u32 newpc = r[n] + pc + 2;//pc +2 is done after
	ExecuteDelayslot();	//WARN : r[n] can change here
	pc = newpc;
	*/

	//shil_interpret(op);
	//ilst->add(reg_pc,2);
	//return;
	bb->flags.ExitType = BLOCK_EXITTYPE_DYNAMIC;
	bb->flags.EndAnalyse=true;
	ilst->mov(reg_pc_temp,r[n]);
	ilst->add(reg_pc_temp,pc+4);
	DoDslot(pc,bb);
	ilst->mov(reg_pc,reg_pc_temp);
} 
//bsrf <REG_N>                  
 sh4op(i0000_nnnn_0000_0011)
{
	u32 n = GetN(op);
	/*
	u32 newpc = r[n] + pc +2;//pc +2 is done after
	pr = pc + 4;		   //after delayslot
	ExecuteDelayslot();	//WARN : pr and r[n] can change here
	pc = newpc;

	AddCall(pr-4,pr,pc,0);
	*/

	//shil_interpret(op);
	//ilst->add(reg_pc,2);
	//return;

	bb->flags.EndAnalyse=true;
	bb->flags.ExitType=BLOCK_EXITTYPE_DYNAMIC_CALL;
	ilst->mov(reg_pr,pc+4);
	ilst->mov(reg_pc_temp,r[n]);
	ilst->add(reg_pc_temp,pc+4);
	DoDslot(pc,bb);
	ilst->mov(reg_pc,reg_pc_temp);
	bb->TT_next_addr=pc+4;
} 


 //rte                           
 sh4op(i0000_0000_0010_1011)
{
	/*
	//iNimp("rte");
	sr.SetFull(ssr);
	u32 newpc = spc;//+2 is added after instruction
	ExecuteDelayslot();
	pc = newpc -2;
	RemoveCall(spc,1);
	UpdateSR();
	*/
	shil_interpret(op);
	ilst->add(reg_pc,2);
	
	bb->flags.EndAnalyse=true;
	bb->flags.ExitType=BLOCK_EXITTYPE_DYNAMIC;
	return;
} 


//rts                           
 sh4op(i0000_0000_0000_1011)
{
	/*
	//TODO Check new delay slot code [28/1/06]
	u32 newpc=pr;//+2 is added after instruction
	ExecuteDelayslot();	//WARN : pr can change here
	pc=newpc-2;
	RemoveCall(pr,0);
	*/
	//shil_interpret(op);
	//ilst->add(reg_pc,2);
	//return;
	bb->flags.EndAnalyse = true;
	bb->flags.ExitType= BLOCK_EXITTYPE_RET;
	ilst->mov(reg_pc_temp,reg_pr);
	DoDslot(pc,bb);
	ilst->mov(reg_pc,reg_pc_temp);
} 


// bf <bdisp8>                   
 sh4op(i1000_1011_iiii_iiii)
{//ToDo : Check Me [26/4/05]  | Check DELAY SLOT [28/1/06]
	/*
	if (sr.T==0)
	{
		//direct jump
		pc = (u32)((GetSImm8(op))*2 + 4 + pc );
		pc-=2;
	}
	*/
	
	bb->TF_next_addr=pc+2;
	bb->TT_next_addr=(u32)((GetSImm8(op))*2 + 4 + pc );

	ilst->LoadT(jcond_flag);
	
	bb->flags.EndAnalyse=true;
	bb->flags.ExitType=BLOCK_EXITTYPE_COND;
}


// bf.s <bdisp8>                 
 sh4op(i1000_1111_iiii_iiii)
{//TODO : Check This [26/4/05] | Check DELAY SLOT [28/1/06]
	/*
	if (sr.T==0)
	{
		//delay 1 instruction
		u32 newpc=(u32) ( (GetSImm8(op)<<1) + pc+2);//pc+2 is done after
		ExecuteDelayslot();
		pc = newpc;
	}
	*/
	//shil_interpret(op);
	//return;
	/*shil_interpret(op);
	ilst->add(reg_pc,2);
	return;*/

	bb->TF_next_addr=pc+4;
	bb->TT_next_addr=(u32)((GetSImm8(op))*2 + 4 + pc );

	ilst->LoadT(jcond_flag);
	DoDslot(pc,bb);
	bb->flags.EndAnalyse = true;
	bb->flags.ExitType = BLOCK_EXITTYPE_COND;
}


// bt <bdisp8>                   
 sh4op(i1000_1001_iiii_iiii)
{//TODO : Check This [26/4/05]  | Check DELAY SLOT [28/1/06]
	/*
	if (sr.T==1)
	{
		//direct jump
		pc = (u32) ( (GetSImm8(op)<<1) + pc+2);
	}
	*/


	bb->TF_next_addr=(u32)((GetSImm8(op))*2 + 4 + pc );
	bb->TT_next_addr=pc+2;

	ilst->LoadT(jcond_flag);
	
	bb->flags.EndAnalyse=true;
	bb->flags.ExitType=BLOCK_EXITTYPE_COND;
}


// bt.s <bdisp8>                 
 sh4op(i1000_1101_iiii_iiii)
{
	/*
	if (sr.T == 1)
	{//TODO : Check This [26/4/05]  | Check DELAY SLOT [28/1/06]
		//delay 1 instruction
		u32 newpc=(u32) ( (GetSImm8(op)<<1) + pc+2);//pc+2 is done after
		ExecuteDelayslot();
		pc = newpc;
	}
	*/
	/*shil_interpret(op);	
	ilst->add(reg_pc,2);
	return;*/

	bb->TF_next_addr=(u32)((GetSImm8(op))*2 + 4 + pc );
	bb->TT_next_addr=pc+4;

	ilst->LoadT(jcond_flag);
	DoDslot(pc,bb);
	
	bb->flags.EndAnalyse=true;
	bb->flags.ExitType=BLOCK_EXITTYPE_COND;
}



// bra <bdisp12>
sh4op(i1010_iiii_iiii_iiii)
{//ToDo : Check Me [26/4/05] | Check ExecuteDelayslot [28/1/06] 
	//delay 1 jump imm12
	/*
	u32 newpc =(u32) ((  ((s16)((GetImm12(op))<<4)) >>3)  + pc + 4);//(s16<<4,>>4(-1*2))
	ExecuteDelayslot();
	pc=newpc-2;
	*/

	bb->TF_next_addr=(u32) ((  ((s16)((GetImm12(op))<<4)) >>3)  + pc + 4);
	bb->flags.EndAnalyse=true;
	bb->flags.ExitType=BLOCK_EXITTYPE_FIXED;
	DoDslot(pc,bb);
}
// bsr <bdisp12>
sh4op(i1011_iiii_iiii_iiii)
{//ToDo : Check Me [26/4/05] | Check new delay slot code [28/1/06]
	/*
	//iNimp("bsr <bdisp12>");
	u32 disp = GetImm12(op);
	pr = pc + 4;
	//delay 1 opcode
	u32 newpc = (u32)((((s16)(disp<<4)) >> 3) + pc + 4);
	AddCall(pc,pr,newpc,0);	//WARN : pr can change here
	ExecuteDelayslot();
	pc=newpc-2;*/

	bb->TF_next_addr=(u32) ((  ((s16)((GetImm12(op))<<4)) >>3)  + pc + 4);
	bb->flags.EndAnalyse=true;
	bb->flags.ExitType=BLOCK_EXITTYPE_FIXED_CALL;

	ilst->mov(reg_pr,pc+4);
	DoDslot(pc,bb);
	bb->TT_next_addr=pc+4;
}

// trapa #<imm>                  
sh4op(i1100_0011_iiii_iiii)
{
	/*
	CCN_TRA = (GetImm8(op) << 2);
	Do_Exeption(0,0x160,0x100);
	*/
	shil_interpret(op);	
	ilst->add(reg_pc,2);
	bb->flags.EndAnalyse=true;
	bb->flags.ExitType=BLOCK_EXITTYPE_DYNAMIC;
	return;
}
//jmp @<REG_N>                  
 sh4op(i0100_nnnn_0010_1011)
{   //ToDo : Check Me [26/4/05] | Check new delay slot code [28/1/06]
	u32 n = GetN(op);
	/*
	//delay 1 instruction
	u32 newpc=r[n];
	ExecuteDelayslot();
	pc=newpc-2;//+2 is done after

	*/
	//shil_interpret(op);	
	//ilst->add(reg_pc,2);
	//return;
	bb->flags.EndAnalyse=true;
	bb->flags.ExitType=BLOCK_EXITTYPE_DYNAMIC;
	ilst->mov(reg_pc_temp,r[n]);
	DoDslot(pc,bb);
	ilst->mov(reg_pc,reg_pc_temp);
}


//jsr @<REG_N>                  
 sh4op(i0100_nnnn_0000_1011)
{//ToDo : Check This [26/4/05] | Check new delay slot code [28/1/06]
	u32 n = GetN(op);
	

	bb->flags.EndAnalyse=true;
	bb->flags.ExitType=BLOCK_EXITTYPE_DYNAMIC_CALL;
	ilst->mov(reg_pr,pc+4);
	ilst->mov(reg_pc_temp,r[n]);
	DoDslot(pc,bb);
	ilst->mov(reg_pc,reg_pc_temp);
	bb->TT_next_addr=pc+4;
}



//sh4_bpt_op => breakpoint opcode
sh4op(sh4_bpt_op)
{
	shil_interpret(op);
}
//sleep                         
 sh4op(i0000_0000_0001_1011)
{
	shil_interpret(op);	
	ilst->add(reg_pc,2);
	bb->flags.EndAnalyse=true;
	bb->flags.ExitType=BLOCK_EXITTYPE_DYNAMIC;
	return;
} 

bool __fastcall Scanner_FindSOM(u32 opcode,u32 pc,u32* SOM)
{ 
	if (opcode ==0x0019)
	{
		//possible div0u
		//i0000_0000_0001_1001  
		*SOM=128;
		return MatchDiv32u(opcode,pc);
	}
	else if ((opcode & 0xF00F)==0x2007)
	{
		//possible div0s
		//i0010_nnnn_mmmm_0111  
		*SOM=128;
		return MatchDiv32s(opcode,pc);
	}

	return false;
}
#define notshit
#ifdef notshit
//ok , all the opcodes to here are hand writen for the rec
//time for the compiler fun 
//>:D

#undef r
#undef r_bank

struct shil_RecRegType
{
	Sh4RegType regid;
	//SUB
	void operator-=(const u32 constv)
	{
		ilst->sub(regid,constv);
	};
	void operator-=(const shil_RecRegType& reg)
    {
		ilst->sub(regid,reg.regid);
    }

	void operator--(int wtf)
	{
		(*this)-=1;
	}
	//ADD
	void operator+=(const u32 constv)
	{
		ilst->add(regid,constv);
	};
	void operator+=(const shil_RecRegType& reg)
    {
       ilst->add(regid,reg.regid);
    }
	void operator++(int wtf)
	{
		(*this)+=1;
	}
	//MOVS
	void operator=(const u32 constv)
    {
		 ilst->mov(regid,constv);
    }
	void operator=(const s32 constv)
    {
		(*this)=(u32)constv;
    }
	void operator=(const shil_RecRegType& reg)
    {
		ilst->mov(regid,reg.regid);	
    }
	//AND
	void operator&=(const u32 constv)
	{
		ilst->and(regid,constv);
	};
	void operator&=(const shil_RecRegType& reg)
	{
		ilst->and(regid,reg.regid);
	};
	//OR
	void operator|=(const u32 constv)
	{
		ilst->or(regid,constv);
	};
	void operator|=(const shil_RecRegType& reg)
	{
		ilst->or(regid,reg.regid);
	};
	//XOR
	void operator^=(const u32 constv)
	{
		ilst->xor(regid,constv);
	};
	void operator^=(const shil_RecRegType& reg)
	{
		ilst->xor(regid,reg.regid);
	};
	//SHIFT RIGHT
	void operator>>=(const u32 constv)
	{
		ilst->shr(regid,(u8)constv);
	};

	//SHIFT LEFT
	void operator<<=(const u32 constv)
	{
		ilst->shl(regid,(u8)constv);
	};
};


//Read Mem macros
#define ReadMemU32(to,addr)					ReadMemRec(to,addr,0,4)//to=ReadMem32(addr)
#define ReadMemS16(to,addr)					ReadMemRecS(to,addr,0,2)// to=(u32)(s32)(s16)ReadMem16(addr)
#define ReadMemS8(to,addr)					ReadMemRecS(to,addr,0,1)//to=(u32)(s32)(s8)ReadMem8(addr)

//Base,offset format
#define ReadMemBOU32(to,addr,offset)		ReadMemRec(to,addr,offset,4)//ReadMemU32(to,addr+offset)
#define ReadMemBOS16(to,addr,offset)		ReadMemRecS(to,addr,offset,2)//ReadMemS16(to,addr+offset)
#define ReadMemBOS8(to,addr,offset)			ReadMemRecS(to,addr,offset,1)//ReadMemS8(to,addr+offset)

//Write Mem Macros
#define WriteMemU32(addr,data)				WriteMemRec(addr,0,data,4)//WriteMem32(addr,(u32)data)
#define WriteMemU16(addr,data)				WriteMemRec(addr,0,data,2)//WriteMem16(addr,(u16)data)
#define WriteMemU8(addr,data)				WriteMemRec(addr,0,data,1)//WriteMem8(addr,(u8)data)

//Base,offset format
#define WriteMemBOU32(addr,offset,data)		WriteMemRec(addr,offset,data,4)//WriteMemU32(addr+offset,data)
#define WriteMemBOU16(addr,offset,data)		WriteMemRec(addr,offset,data,2)//WriteMemU16(addr+offset,data)
#define WriteMemBOU8(addr,offset,data)		WriteMemRec(addr,offset,data,1)//WriteMemU8(addr+offset,data)


void ReadMemRec(shil_RecRegType &to,u32 addr,u32 offset,u32 sz)
{
	assert(sz==4);
	ilst->readm32(to.regid,addr+offset);
}

void ReadMemRec(shil_RecRegType &to,shil_RecRegType& addr,u32 offset,u32 sz)
{
	assert(sz==4);
	ilst->readm32(to.regid,addr.regid,offset);
}


void ReadMemRec(shil_RecRegType &to,shil_RecRegType& addr,shil_RecRegType& offset,u32 sz)
{
	assert(sz==4);
	ilst->readm32(to.regid,addr.regid,offset.regid);
}

//signed
void ReadMemRecS(shil_RecRegType &to,u32 addr,u32 offset,u32 sz)
{
	assert(sz!=4);
	if (sz==1)
		ilst->readm8(to.regid,addr+offset);
	else
		ilst->readm16(to.regid,addr+offset);

}
void ReadMemRecS(shil_RecRegType &to,shil_RecRegType& addr,u32 offset,u32 sz)
{
	assert(sz!=4);
	if (sz==1)
		ilst->readm8(to.regid,addr.regid,offset);
	else
		ilst->readm16(to.regid,addr.regid,offset);
}


void ReadMemRecS(shil_RecRegType &to,shil_RecRegType& addr,shil_RecRegType& offset,u32 sz)
{
	assert(sz!=4);
	if (sz==1)
		ilst->readm8(to.regid,addr.regid,offset.regid);
	else
		ilst->readm16(to.regid,addr.regid,offset.regid);
}
//WriteMem(u32 addr,u32 data,u32 sz)
void WriteMemRec(shil_RecRegType& addr,u32 offset,shil_RecRegType &data,u32 sz)
{
	if (sz==1)
		ilst->writem8(data.regid,addr.regid,offset);
	else if (sz==2)
		ilst->writem16(data.regid,addr.regid,offset);
	else
		ilst->writem32(data.regid,addr.regid,offset);
}
void WriteMemRec(shil_RecRegType& addr,shil_RecRegType& offset,shil_RecRegType &data,u32 sz)
{
	if (sz==1)
		ilst->writem8(data.regid,addr.regid,offset.regid);
	else if (sz==2)
		ilst->writem16(data.regid,addr.regid,offset.regid);
	else
		ilst->writem32(data.regid,addr.regid,offset.regid);
}




shil_RecRegType shil_rec_r[16];
shil_RecRegType shil_rec_r_bank[8];

shil_RecRegType shil_rec_gbr,shil_rec_ssr,shil_rec_spc,shil_rec_sgr,shil_rec_dbr,shil_rec_vbr;
shil_RecRegType shil_rec_pr,shil_rec_fpul;

struct 
{
	shil_RecRegType l;
	shil_RecRegType h;
}shil_rec_mac;


void shil_DynarecInit()
{
	for (int i=0;i<8;i++)
	{
		dyna_reg_id_fr[i]=(Sh4RegType)(fr_0+i);
		dyna_reg_id_xf[i]=(Sh4RegType)(xf_0+i);
		dyna_reg_id_dr[i]=(Sh4RegType)(dr_0+i);
		dyna_reg_id_xd[i]=(Sh4RegType)(xd_0+i);

		shil_rec_r[i].regid=dyna_reg_id_r[i]=(Sh4RegType)(r0+i);
		shil_rec_r_bank[i].regid=dyna_reg_id_r_bank[i]=(Sh4RegType)(r0_Bank+i);
	}

	for (int i=8;i<16;i++)
	{
		dyna_reg_id_fr[i]=(Sh4RegType)(fr_0+i);
		dyna_reg_id_xf[i]=(Sh4RegType)(xf_0+i);

		shil_rec_r[i].regid=dyna_reg_id_r[i]=(Sh4RegType)(r0+i);
	}

	shil_rec_gbr.regid=reg_gbr;
	shil_rec_ssr.regid=reg_ssr;
	shil_rec_spc.regid=reg_spc;
	shil_rec_sgr.regid=reg_sgr;
	shil_rec_dbr.regid=reg_dbr;
	shil_rec_vbr.regid=reg_vbr;

	shil_rec_mac.h.regid=reg_mach;
	shil_rec_mac.l.regid=reg_macl;
	shil_rec_pr.regid=reg_pr;
	shil_rec_fpul.regid=reg_fpul;
}
//rename shit

#define UpdateFPSCR rec_UpdateFPSCR
#define UpdateSR rec_UpdateSR

#define r		shil_rec_r
#define r_bank	shil_rec_r_bank
#define gbr		shil_rec_gbr
#define ssr		shil_rec_ssr
#define spc		shil_rec_spc
#define sgr		shil_rec_sgr
#define dbr		shil_rec_dbr
#define vbr		shil_rec_vbr
#define mac		shil_rec_mac

#define pr		shil_rec_pr
#define fpul	shil_rec_fpul

//nice names ehh ? make us sure we don't use em :)
#define sr		a_not_defined_name_has_to_be_rec_sr
#define fpscr	a_not_defined_name_has_to_be_rec_fpscr

#undef iNimp
#define iNimp(op,info) rec_shil_iNimp(pc,op,info)

#include "dc\sh4\sh4_cpu_arith.h"
#include "dc\sh4\sh4_cpu_branch.h"
#include "dc\sh4\sh4_cpu_logic.h"
#include "dc\sh4\sh4_cpu_movs.h"
#include "dc\sh4\sh4_cpu_loadstore.h"
#endif