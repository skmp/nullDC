/*
	sh4 base core
	most of it is (very) old
	could use many cleanups, lets hope someone does them
*/

//All non fpu opcodes :)
#include "types.h"

#include <windows.h>

#include "dc/pvr/pvr_if.h"
#include "sh4_interpreter.h"
#include "dc/mem/sh4_mem.h"
#include "dc/mem/sh4_internal_reg.h"
#include "sh4_registers.h"
#include "sh4_interpreter.h"
#include "ccn.h"
#include "intc.h"
#include "tmu.h"
#include "dc/gdrom/gdrom_if.h"



#define GetN(str) ((str>>8) & 0xf)
#define GetM(str) ((str>>4) & 0xf)
#define GetImm4(str) ((str>>0) & 0xf)
#define GetImm8(str) ((str>>0) & 0xff)
#define GetSImm8(str) ((s8)((str>>0) & 0xff))
#define GetImm12(str) ((str>>0) & 0xfff)
#define GetSImm12(str) (((s16)((GetImm12(str))<<4))>>3)

#define iNimp cpu_iNimp
#define iWarn cpu_iWarn

//Read Mem macros
#define ReadMemU64(to,addr) (u64&)to=ReadMem64(addr)
#define ReadMemU32(to,addr) to=ReadMem32(addr)
#define ReadMemS32(to,addr) to=(s32)ReadMem32(addr)
#define ReadMemS16(to,addr) to=(u32)(s32)(s16)ReadMem16(addr)
#define ReadMemS8(to,addr) to=(u32)(s32)(s8)ReadMem8(addr)

//Base,offset format
#define ReadMemBOU32(to,addr,offset)	ReadMemU32(to,addr+offset)
#define ReadMemBOS16(to,addr,offset)	ReadMemS16(to,addr+offset)
#define ReadMemBOS8(to,addr,offset)		ReadMemS8(to,addr+offset)

//Write Mem Macros
#define WriteMemU64(addr,data)				WriteMem64(addr,(u64&)data)
#define WriteMemU32(addr,data)				WriteMem32(addr,(u32)data)
#define WriteMemU16(addr,data)				WriteMem16(addr,(u16)data)
#define WriteMemU8(addr,data)				WriteMem8(addr,(u8)data)

//Base,offset format
#define WriteMemBOU32(addr,offset,data)		WriteMemU32(addr+offset,data)
#define WriteMemBOU16(addr,offset,data)		WriteMemU16(addr+offset,data)
#define WriteMemBOU8(addr,offset,data)		WriteMemU8(addr+offset,data)



#define exept_tet_now {if (sh4_exept_raised) return;}
#ifdef NO_MMU
#define EXEPT_EXIT()
#else
#define EXEPT_EXIT() if (sex) return;
#endif

#define _INTERPRETER_COMPILE_

bool sh4_sleeping;

// 0xxx

void cpu_iNimp(u32 op, char* info)
{
	log("not implemented opcode : %X : ", op);
	log(info);
	log(" @ %X\nPress Any key to continue\n", pc);
	getc(stdin);

	//sh4_cpu->Stop();
}

void cpu_iWarn(u32 op, char* info)
{
	log("Check opcode : %X : ", op);
	log(info);
	log(" @ %X\n", pc);
}

#include "sh4_cpu_movs.h"
#include "sh4_cpu_loadstore.h"
#include "sh4_cpu_branch.h"
#include "sh4_cpu_arith.h"
#include "sh4_cpu_logic.h"
#include "dc\mem\mmu.h"

//************************ TLB/Cache ************************
//ldtlb                         
sh4op(i0000_0000_0011_1000)
{
	//u32 size[4]={1,4,64,1024};
	//u32 size_2[4]={1,1,1,1024/64};
	//log("ldtlb : ASID : %d , VPN : 0x%X ,PPN 0x%X,V %d , SZ : %d kb,PR %d, C %d , D %d , SH %d , WT %d\n"
	//	,CCN_PTEH.ASID,CCN_PTEH.VPN,
	//	CCN_PTEL.PPN,CCN_PTEL.V,size[CCN_PTEL.SZ1*2 + CCN_PTEL.SZ0],CCN_PTEL.PR,
	//	CCN_PTEL.C,CCN_PTEL.D,CCN_PTEL.SH,CCN_PTEL.WT);

	//u32 newmap=CCN_PTEH.VPN <<10;
	//u32 start=CCN_PTEL.PPN<<10;
	//log("Map 0x%X to 0x%X\n",start,newmap);
	//start >>=16;
	//newmap >>=16;
	
	//for (u32 i=0;i<4;i++)
	//	_vmem_mirror_mapping((i<<29) | (newmap & 0x1FFFFFFF),start,size_2[CCN_PTEL.SZ1*2 + CCN_PTEL.SZ0]);
	//_vmem_mirror_mapping((3<<30) | (newmap & 0x1FFFFFFF),start,size_2[CCN_PTEL.SZ1*2 + CCN_PTEL.SZ0]);

	//log("ldtlb %d/%d\n",CCN_MMUCR.URC,CCN_MMUCR.URB);
	
	UTLB_SyncUnmap(CCN_MMUCR.URC);
	UTLB[CCN_MMUCR.URC].Data=CCN_PTEL;
	UTLB[CCN_MMUCR.URC].Address=CCN_PTEH;
	bool is_sq_remap=UTLB_SyncMap(CCN_MMUCR.URC);
	#ifdef NO_MMU
		if (!is_sq_remap)
			iNimp(op, "ldtlb");
	#endif	
} 



//ocbi @<REG_N>                 
sh4op(i0000_nnnn_1001_0011)
{
	//u32 n = GetN(op);
	//log("ocbi @0x%08X \n",r[n]);
} 


//ocbp @<REG_N>                 
sh4op(i0000_nnnn_1010_0011)
{
	//u32 n = GetN(op);
	//log("ocbp @0x%08X \n",r[n]);
} 


//ocbwb @<REG_N>                
sh4op(i0000_nnnn_1011_0011)
{
	//u32 n = GetN(op);
	//log("ocbwb @0x%08X \n",r[n]);
} 

//pref @<REG_N>                 
void __fastcall do_pref(u32 addr)
{
	u32* sq = (u32*)&sq_both[addr& 0x20];

	if (!mmu_TranslateSQW(addr))
	{
		//log("Read Exeption From SQ WRITE \n");
		return;
	}

	if (((addr >> 26) & 0x7) == 4)//Area 4 !11!!
	{
		TAWriteSQ(addr,sq);
	}
	else
	{
		WriteMemBlock_nommu_ptr(addr,sq,8*4);
	}
}
sh4op(i0000_nnnn_1000_0011)
{
	u32 n = GetN(op);
	u32 Dest = r[n];

	if ((Dest & 0xFC000000) == 0xE0000000) //Store Queue
	{
		do_pref(Dest);
	}
}


//************************ Set/Get T/S ************************
//sets                          
sh4op(i0000_0000_0101_1000)
{
	sr.S = 1;
} 


//clrs                          
sh4op(i0000_0000_0100_1000)
{
	sr.S = 0;
} 

//sett                          
sh4op(i0000_0000_0001_1000)
{
	sr.T = 1;
} 

//clrt                          
sh4op(i0000_0000_0000_1000)
{
	sr.T = 0;
} 
//movt <REG_N>                  
sh4op(i0000_nnnn_0010_1001)
{
	u32 n = GetN(op);
	r[n] = sr.T;
} 
//************************ Reg Compares ************************
//cmp/pz <REG_N>                
sh4op(i0100_nnnn_0001_0001)
{
	u32 n = GetN(op);

	if (((s32)r[n]) >= 0)
		sr.T = 1;
	else
		sr.T = 0;
}

//cmp/pl <REG_N>                
sh4op(i0100_nnnn_0001_0101)
{
	//iNimp("cmp/pl <REG_N>");
	u32 n = GetN(op);
	if ((s32)r[n] > 0) 
		sr.T = 1;
	else 
		sr.T = 0;
}

//cmp/eq #<imm>,R0              
sh4op(i1000_1000_iiii_iiii)
{
	u32 imm = (u32)(s32)(GetSImm8(op));
	if (r[0] == imm)
		sr.T =1;
	else
		sr.T =0;
}

//cmp/eq <REG_M>,<REG_N>        
sh4op(i0011_nnnn_mmmm_0000)
{
	//iNimp("cmp/eq <REG_M>,<REG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op);

	if (r[m] == r[n])
		sr.T = 1;
	else
		sr.T = 0;
}

//cmp/hs <REG_M>,<REG_N>        
sh4op(i0011_nnnn_mmmm_0010)
{//ToDo : Check Me [26/4/05]
	u32 n = GetN(op);
	u32 m = GetM(op);
	if (r[n] >= r[m])
		sr.T=1;
	else
		sr.T=0;
}

//cmp/ge <REG_M>,<REG_N>        
sh4op(i0011_nnnn_mmmm_0011)
{
	//iNimp("cmp/ge <REG_M>,<REG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op);
	if ((s32)r[n] >= (s32)r[m])
		sr.T = 1;
	else 
		sr.T = 0;
}

//cmp/hi <REG_M>,<REG_N>        
sh4op(i0011_nnnn_mmmm_0110)
{
	u32 n = GetN(op);
	u32 m = GetM(op);

	if (r[n] > r[m])
		sr.T=1;
	else
		sr.T=0;
}

//cmp/gt <REG_M>,<REG_N>        
sh4op(i0011_nnnn_mmmm_0111)
{
	//iNimp("cmp/gt <REG_M>,<REG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op);

	if (((s32)r[n]) > ((s32)r[m]))
		sr.T = 1;
	else 
		sr.T = 0;
}

//cmp/str <REG_M>,<REG_N>       
sh4op(i0010_nnnn_mmmm_1100)
{
	//iNimp("cmp/str <REG_M>,<REG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op);

	u32 temp;
	u32 HH, HL, LH, LL;

	temp = r[n] ^ r[m];

	HH=(temp&0xFF000000)>>24;
	HL=(temp&0x00FF0000)>>16;
	LH=(temp&0x0000FF00)>>8;
	LL=temp&0x000000FF;
	HH=HH&&HL&&LH&&LL;
	if (HH==0) sr.T=1;
	else sr.T=0;
}

//tst #<imm>,R0                 
sh4op(i1100_1000_iiii_iiii)
{
	//iNimp("tst #<imm>,R0");
	u32 utmp1 = r[0] & GetImm8(op);
	if (utmp1 == 0) 
		sr.T = 1;
	else 
		sr.T = 0;
}
//tst <REG_M>,<REG_N>           
sh4op(i0010_nnnn_mmmm_1000)
{//ToDo : Check This [26/4/05]
	u32 n = GetN(op);
	u32 m = GetM(op);

	if ((r[n] & r[m])!=0)
		sr.T=0;
	else
		sr.T=1;

}
//************************ mulls! ************************ 
//mulu.w <REG_M>,<REG_N>          
sh4op(i0010_nnnn_mmmm_1110)
{
	//iNimp("mulu.w <REG_M>,<REG_N>");//check  ++
	u32 n = GetN(op);
	u32 m = GetM(op);
	mac.l=((u16)r[n])*
		((u16)r[m]);
}

//muls.w <REG_M>,<REG_N>          
sh4op(i0010_nnnn_mmmm_1111)
{
	//iNimp("muls <REG_M>,<REG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op);

	mac.l = (u32)(((s16)(u16)r[n]) * ((s16)(u16)r[m]));
}
//dmulu.l <REG_M>,<REG_N>       
sh4op(i0011_nnnn_mmmm_0101)
{
	//iNimp("dmulu.l <REG_M>,<REG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op);

	mac.full = (u64)r[n] * (u64)r[m];
}

//dmuls.l <REG_M>,<REG_N>       
sh4op(i0011_nnnn_mmmm_1101)
{
	//iNimp("dmuls.l <REG_M>,<REG_N>");//check ++
	u32 n = GetN(op);
	u32 m = GetM(op);

	mac.full = (s64)(s32)r[n] * (s64)(s32)r[m];
}


//mac.w @<REG_M>+,@<REG_N>+     
sh4op(i0100_nnnn_mmmm_1111)
{
	u32 n = GetN(op);
	u32 m = GetM(op);
	if (sr.S==1)
	{
		log("mac.w @<REG_M>+,@<REG_N>+ : s=%d\n",sr.S);
	}
	else
	{
		s32 rm,rn;
		
		rn = (s32)(s16)ReadMem16(r[n]);
		r[n]+=2;
		//if (n==m)
		//{
		//	r[n]+=2;
		//	r[m]+=2;
		//}
		rm = (s32)(s16)ReadMem16(r[m]);
		verify(sh4_exept_raised==false);
		
		r[m]+=2;

		s32 mul=rm * rn;
		mac.full+=(s64)mul;
	}
}		
//mac.l @<REG_M>+,@<REG_N>+     
sh4op(i0000_nnnn_mmmm_1111)
{
	//iNimp("mac.l @<REG_M>+,@<REG_N>+");
	u32 n = GetN(op);
	u32 m = GetM(op);
	s32 rm, rn;

	verify(sr.S==0);

	ReadMemS32(rm,r[m]);
	r[m] += 4;
	ReadMemS32(rn,r[n]);
	r[n] += 4;
	
	verify(sh4_exept_raised==false);

	mac.full += (s64)rm * (s64)rn;
	
	//log("%I64u %I64u | %d %d | %d %d\n",mac,mul,macl,mach,rm,rn);
}

//mul.l <REG_M>,<REG_N>         
sh4op(i0000_nnnn_mmmm_0111)
{
	u32 n = GetN(op);
	u32 m = GetM(op);
	mac.l = (u32)((((s32)r[n]) * ((s32)r[m])));
}
//************************ Div ! ************************ 
void FASTCALL sh4_div0u()
{
	sr.Q = 0;
	sr.M = 0;
	sr.T = 0;
}
//div0u                         
sh4op(i0000_0000_0001_1001)
{
	sh4_div0u();
}
void FASTCALL sh4_div0s(u32 rn,u32 rm)
{
	sr.Q=rn>>31;
	sr.M=rm>>31;
	sr.T=sr.M^sr.Q;
}
//div0s <REG_M>,<REG_N>         
sh4op(i0010_nnnn_mmmm_0111)
{
	u32 n = GetN(op);
	u32 m = GetM(op);

	sh4_div0s(r[n],r[m]);
}

u32 FASTCALL sh4_div1(u32 rn,u32 rm)
{
	unsigned long tmp0, tmp2;
	unsigned char old_q, tmp1;

	old_q = sr.Q;
	sr.Q = (u8)((0x80000000 & rn) !=0);

	rn <<= 1;
	rn |= sr.T;

	tmp0 = rn;	// this need only be done once here ..
	tmp2 = rm;

	if( 0 == old_q )
	{	
		if( 0 == sr.M )
		{
			rn -= tmp2;
			tmp1	= (rn>tmp0);
			sr.Q	= (sr.Q==0) ? tmp1 : (u8)(tmp1==0) ;
		}
		else
		{
			rn += tmp2;
			tmp1	=(rn<tmp0);
			sr.Q	= (sr.Q==0) ? (u8)(tmp1==0) : tmp1 ;
		}
	}	
	else
	{	
		if( 0 == sr.M )
		{
			rn += tmp2;
			tmp1	=(rn<tmp0);
			sr.Q	= (sr.Q==0) ? tmp1 : (u8)(tmp1==0) ;
		}	
		else
		{
			rn -= tmp2;
			tmp1	=(rn>tmp0);
			sr.Q	= (sr.Q==0) ? (u8)(tmp1==0) : tmp1 ;
		}
	}
	sr.T = (sr.Q==sr.M);

	return rn;
	/*
	u32 op1=r[m];
	u32 op2=(u32)(s32)r[n];
	u32 oldq=sr.Q;
	sr.Q=(r[n]>>31)&1;
	op2=(op2<<1)| (sr.T);

	u32 of=0;
	if (oldq==sr.M)
	{
		op2=op2-op1;
		if (underflow)
			of=1;
	}
	else	
	{
		op2=op2+op1;
		if (overflow)
			of=1;
	}

	sr.Q=(sr.Q^sr.M)^of;
	sr.T=1^(sr.Q ^ sr.M);
	r[n]=(u32)op2;*/
	//log("Q %d , S %d , T %d , r[n] %d, r[m] %d\n",sr.Q,sr.S,sr.T,r[n],r[m]);
}
//div1 <REG_M>,<REG_N>          
sh4op(i0011_nnnn_mmmm_0100)
{//ToDo : Check This [26/4/05]

	u32 n=GetN(op);	//dividend 'sign'
	u32 m=GetM(op);	//divisor

	r[n]=sh4_div1(r[n],r[m]);
}

//************************ Simple maths ************************ 
//addc <REG_M>,<REG_N>          
sh4op(i0011_nnnn_mmmm_1110)
{//ToDo : Check This [26/4/05]
	//iNimp(op,"addc <REG_M>,<REG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op);
	u32 tmp1 = r[n] + r[m];
	u32 tmp0 = r[n];

	r[n] = tmp1 + sr.T;

	if (tmp0 > tmp1)
		sr.T = 1;
	else
		sr.T = 0;
	
	if (tmp1 > r[n])
		sr.T = 1;
}

// addv <REG_M>,<REG_N>          
sh4op(i0011_nnnn_mmmm_1111)
{
	//iNimp(op, "addv <REG_M>,<REG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op);
	//s64 br=(s64)(s32)r[n]+(s64)(s32)r[m];
	u32 rm=r[m];
	u32 rn=r[n];
	u8 tmp;

	__asm
	{
		mov eax,rm;
		mov ecx,rn;
		add eax,ecx;
		seto tmp;//cond = (@cf ==> flip(F(o)[msb]))
		movzx ebx,tmp;
		mov [sr.T],ebx;
	};

	r[n]=rn;
	/*
	if (br >=0x80000000)
		sr.T=1;
	else if (br < (s64) (0xFFFFFFFF80000000u))
		sr.T=1;
	else
		sr.T=0;
	/*if (br>>32)
		sr.T=1;
	else
		sr.T=0;*/
	
	//r[n]+=r[m];
}

//subc <REG_M>,<REG_N>          
sh4op(i0011_nnnn_mmmm_1010)
{//ToDo : Check This [26/4/05]
	//iNimp(op,"subc <REG_M>,<REG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op);

	u32 tmp1 = (u32)(((s32)r[n]) - ((s32)r[m]));
	u32 tmp0 = r[n];
	r[n] = tmp1 - sr.T;

	if (tmp0 < tmp1)
		sr.T=1;
	else
		sr.T=0;
	
	if (tmp1 < r[n])
		sr.T=1;
}

//subv <REG_M>,<REG_N>          
sh4op(i0011_nnnn_mmmm_1011)
{
	//iNimp(op, "subv <REG_M>,<REG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op);
	/*s64 br=(s64)(s32)r[n]-(s64)(s32)r[m];
	
	if (br >=0x80000000)
		sr.T=1;
	else if (br < (s64) (0xFFFFFFFF80000000u))
		sr.T=1;
	else
		sr.T=0;*/
	/*if (br>>32)
		sr.T=1;
	else
		sr.T=0;*/
	
	//r[n]-=r[m];
	u32 rm=r[m];
	u32 rn=r[n];
	u8 tmp;

	__asm 
	{
		mov eax,rm;
		mov ecx,rn;
		sub eax,ecx;
		seto tmp;//cond = (@cf ==> flip(F(o)[msb]))
		movzx ebx,tmp;
		mov [sr.T],ebx;
	};

	r[n]=rn;
}
//dt <REG_N>                    
sh4op(i0100_nnnn_0001_0000)
{
	u32 n = GetN(op);
	r[n]-=1;
	if (r[n] == 0)
		sr.T=1;
	else
		sr.T=0;
}

//negc <REG_M>,<REG_N>          
sh4op(i0110_nnnn_mmmm_1010)
{
	//iNimp("negc <REG_M>,<REG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op);

	//r[n]=-r[m]-sr.T;
	u32 tmp=0-r[m];
	r[n]=tmp-sr.T;
	
	if (0<tmp)
		sr.T=1;
	else
		sr.T=0;

	if (tmp<r[n]) 
		sr.T=1;

	//NO , T IS *_CARRY_* , *NOT* sign :)
	//sr.T=r[n]>>31;
}


//neg <REG_M>,<REG_N>           
sh4op(i0110_nnnn_mmmm_1011)
{//ToDo : Check This [26/4/05]
	u32 n = GetN(op);
	u32 m = GetM(op);
	r[n] = -(s32)r[m];
} 

//not <REG_M>,<REG_N>           
sh4op(i0110_nnnn_mmmm_0111)
{
	//iNimp("not <REG_M>,<REG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op);

	r[n] = ~r[m];
} 


//************************ shifts/rotates ************************
//shll <REG_N>                  
sh4op(i0100_nnnn_0000_0000)
{//ToDo : Check This [26/4/05]
	u32 n = GetN(op);

	sr.T = r[n] >> 31;
	r[n] <<= 1;
}
//shal <REG_N>                  
sh4op(i0100_nnnn_0010_0000)
{
	u32 n=GetN(op);
	sr.T=r[n]>>31;
	r[n]=((s32)r[n])<<1;
}


//shlr <REG_N>                  
sh4op(i0100_nnnn_0000_0001)
{//ToDo : Check This [26/4/05]
	u32 n = GetN(op);
	sr.T = r[n] & 0x1;
	r[n] >>= 1;
}

//shar <REG_N>                  
sh4op(i0100_nnnn_0010_0001)
{//ToDo : Check This [26/4/05] x2
	//iNimp("shar <REG_N>");
	u32 n = GetN(op);

	sr.T=r[n] & 1;
	r[n]=((s32)r[n])>>1;
}

//shad <REG_M>,<REG_N>          
sh4op(i0100_nnnn_mmmm_1100)
{
	//iNimp(op,"shad <REG_M>,<REG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op);
	u32 sgn = r[m] & 0x80000000;
	if (sgn == 0)
		r[n] <<= (r[m] & 0x1F); 
	else if ((r[m] & 0x1F) == 0)
	{
		if ((r[n] & 0x80000000) == 0)
			r[n] = 0;
		else
			r[n] = 0xFFFFFFFF;
	}
	else
		r[n] = ((s32)r[n]) >> ((~r[m] & 0x1F) + 1);
}


//shld <REG_M>,<REG_N>          
sh4op(i0100_nnnn_mmmm_1101)
{//ToDo : Check This [26/4/05] x2
	//iNimp("shld <REG_M>,<REG_N>");
	//HACK : CHECKME
	/*u32 n = GetN(op);
	u32 m = GetM(op);
	s32 s;

	s =  (s32)(r[m] & 0x80000000);
	if (s == 0)
		r[n] <<= (r[m]);
	else if ((r[m] & 0x1f) == 0)
		r[n] = 0;
	else
		r[n] = (u32)r[n] >> (32-r[m]);*/ // -> bug ! 32-r[m] -> -(r[m]&0x1F)

	u32 n = GetN(op);
	u32 m = GetM(op);
	u32 sgn = r[m] & 0x80000000;
	if (sgn == 0)
		r[n] <<= (r[m] & 0x1F);
	else if ((r[m] & 0x1F) == 0)
	{
		r[n] = 0;
	}
	else
		r[n] = ((u32)r[n]) >> ((~r[m] & 0x1F) + 1);
}


u32 FASTCALL sh4_rotcl(u32 rn)
{
	u32 t = sr.T;

	sr.T = rn >> 31;

	rn <<= 1;
	rn|=t;

	return rn;
}
//rotcl <REG_N>                 
sh4op(i0100_nnnn_0010_0100)
{//ToDo : Check This [26/4/05]
	//iNimp("rotcl <REG_N>");
	u32 n = GetN(op);
	
	r[n]=sh4_rotcl(r[n]);
}


//rotl <REG_N>                  
sh4op(i0100_nnnn_0000_0100)
{
	//iNimp("rotl <REG_N>");
	u32 n = GetN(op);
	//return;
	/*
	if ((r[n] & 0x80000000)!=0)
	sr.T=1;
	else
	sr.T = 0;*/

	sr.T=r[n]>>31;

	r[n] <<= 1;

	/*if (sr.T!=0)
	r[n] |= 0x00000001;
	else
	r[n] &= 0xFFFFFFFE;*/
	r[n]|=sr.T;
}

//rotcr <REG_N>                 
sh4op(i0100_nnnn_0010_0101)
{
	//iNimp("rotcr <REG_N>");
	u32 n = GetN(op);
	u32 temp;

	/*if ((R[n] & 0x00000001) == 0) 
	temp = 0;
	else 
	temp = 1;*/

	temp = r[n] & 0x1;

	r[n] >>= 1;


	/*
	if (sr.T == 1) 
	r[n] |= 0x80000000;
	else 
	r[n] &= 0x7FFFFFFF;*/
	r[n] |=sr.T<<31;

	sr.T = temp;
	/*if (temp == 1) 
	T = 1;
	else 
	T = 0;*/
}


//rotr <REG_N>                  
sh4op(i0100_nnnn_0000_0101)
{
	//iNimp("rotr <REG_N>");//check ++
	u32 n = GetN(op);
	sr.T = r[n] & 0x1;
	r[n] >>= 1;
	r[n] |= (sr.T << 31);
}					
//************************ byte reorder/sign ************************
//swap.b <REG_M>,<REG_N>        
sh4op(i0110_nnnn_mmmm_1000)
{
	//iNimp("swap.b <REG_M>,<REG_N>");
	u32 m = GetM(op);
	u32 n = GetN(op);
	r[n] = (r[m] & 0xFFFF0000) | ((r[m]&0xFF)<<8) | ((r[m]>>8)&0xFF);
} 


//swap.w <REG_M>,<REG_N>        
sh4op(i0110_nnnn_mmmm_1001)
{
	u32 n = GetN(op);
	u32 m = GetM(op);

	u16 t = (u16)(r[m]>>16);
	r[n] = (r[m] << 16) | t;
} 

//extu.b <REG_M>,<REG_N>        
sh4op(i0110_nnnn_mmmm_1100)
{
	u32 n = GetN(op);
	u32 m = GetM(op);

	r[n] = (u32)(u8)r[m];
} 

//extu.w <REG_M>,<REG_N>        
sh4op(i0110_nnnn_mmmm_1101)
{
	u32 n = GetN(op);
	u32 m = GetM(op);

	r[n] =(u32)(u16) r[m] & 0x0000FFFF;
} 

//exts.b <REG_M>,<REG_N>        
sh4op(i0110_nnnn_mmmm_1110)
{
	u32 n = GetN(op);
	u32 m = GetM(op);

	r[n] = (u32)(s32)(s8)(u8)(r[m]);
} 

//exts.w <REG_M>,<REG_N>        
sh4op(i0110_nnnn_mmmm_1111)
{
	u32 n = GetN(op);
	u32 m = GetM(op);

	r[n] = (u32)(s32)(s16)(u16)(r[m]);
} 

//xtrct <REG_M>,<REG_N>         
sh4op(i0010_nnnn_mmmm_1101)
{
	u32 n = GetN(op);
	u32 m = GetM(op);

	r[n] = ((r[n] >> 16) & 0xFFFF) | ((r[m] << 16) & 0xFFFF0000);
}

//************************ xxx.b #<imm>,@(R0,GBR) ************************
//tst.b #<imm>,@(R0,GBR)        
sh4op(i1100_1100_iiii_iiii)
{
	u32 imm=GetImm8(op);

	u32 temp = (u8)ReadMem8(gbr+r[0]);

	temp &= imm;

	if (temp==0)
		sr.T=1;
	else
		sr.T=0;
}


//and.b #<imm>,@(R0,GBR)        
sh4op(i1100_1101_iiii_iiii)
{
	u8 temp = (u8)ReadMem8(gbr+r[0]);
	
	temp &= GetImm8(op);

	WriteMem8(gbr +r[0], temp);
}


//xor.b #<imm>,@(R0,GBR)        
sh4op(i1100_1110_iiii_iiii)
{
	u8 temp = (u8)ReadMem8(gbr+r[0]);

	temp ^= GetImm8(op);

	WriteMem8(gbr +r[0], temp);
}

//or.b #<imm>,@(R0,GBR)         
sh4op(i1100_1111_iiii_iiii)
{
	u8 temp = (u8)ReadMem8(gbr+r[0]);
	
	temp |= GetImm8(op);

	WriteMem8(gbr+r[0], temp);
}

//tas.b @<REG_N>                
sh4op(i0100_nnnn_0001_1011)
{
	u32 n = GetN(op);
	u8 val;

	val=(u8)ReadMem8(r[n]);

	u32 srT;
	if (val == 0)
		srT = 1;
	else
		srT = 0;

	val |= 0x80;

	WriteMem8(r[n], val);
	
	sr.T=srT;
}

//************************ Opcodes that read/write the status registers ************************
//stc SR,<REG_N>                
sh4op(i0000_nnnn_0000_0010)//0002
{
	u32 n = GetN(op);

	r[n] = sr.GetFull();
}

 //sts FPSCR,<REG_N>             
sh4op(i0000_nnnn_0110_1010)
{
	u32 n = GetN(op);
	r[n] = fpscr.full;

	UpdateFPSCR();
}

//sts.l FPSCR,@-<REG_N>         
sh4op(i0100_nnnn_0110_0010)
{
	u32 n = GetN(op);

	WriteMemBOU32(r[n],(u32)-4,fpscr.full);
	r[n] -= 4;
}

//stc.l SR,@-<REG_N>            
sh4op(i0100_nnnn_0000_0011)
{
	u32 n = GetN(op);
	
	WriteMemBOU32(r[n],(u32)-4, sr.GetFull());
	r[n] -= 4;
}

//lds.l @<REG_N>+,FPSCR         
sh4op(i0100_nnnn_0110_0110)
{
	u32 n = GetN(op);
	
	ReadMemU32(fpscr.full,r[n]);
	
	UpdateFPSCR();
	r[n] += 4;
}

//ldc.l @<REG_N>+,SR            
sh4op(i0100_nnnn_0000_0111)
{
	u32 n = GetN(op);
	
	u32 sr_t;
	ReadMemU32(sr_t,r[n]);

	sr.SetFull(sr_t);
	r[n] += 4;
	if (UpdateSR())
	{
		//FIXME olny if interrupts got on .. :P
		pc+=2;
		UpdateINTC();
		pc-=2;
	}
}

//lds <REG_N>,FPSCR             
sh4op(i0100_nnnn_0110_1010)
{
	u32 n = GetN(op);

	fpscr.full = r[n];
	UpdateFPSCR();
}

//ldc <REG_N>,SR                
sh4op(i0100_nnnn_0000_1110)
{
	u32 n = GetN(op);

	sr.SetFull(r[n]);
	if (UpdateSR())
	{
		//FIXME olny if interrupts got on .. :P
		pc+=2;
		UpdateINTC();
		pc-=2;
	}
}

//bah
//Not implt
sh4op(iNotImplemented)
{
	cpu_iNimp(op,"unknown opcode");
#ifndef NO_MMU
	if (sh4_exept_raised==false)
	{
		sh4_cpu->RaiseExeption(0x180,0x100);
	}
#endif
}

sh4op(gdrom_hle_op)
{
	EMUERROR("GDROM HLE NOT SUPPORTED");
}