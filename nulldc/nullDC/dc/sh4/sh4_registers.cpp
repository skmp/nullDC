#include "Types.h"
#include "sh4_registers.h"
#include "intc.h"

__declspec(align(64)) u32 r[16];		//64 bytes : 1 cache line
__declspec(align(64)) f32 fr[16];		//64 bytes : 1 cache line
__declspec(align(64)) f32 xf[16];		//64 bytes : 1 cache line

__declspec(align(64)) u32 r_bank[8];

u32 gbr,ssr,spc,sgr,dbr,vbr;
u32 pr,fpul;
mac_type mac;
u32 pc;

StatusReg sr;

fpscr_type fpscr;



const u32 k_sin_table_size = 0x10000+0x4000;//+0x4000 to avoid having to warp around twice on cos
__declspec(align(32)) f32 sin_table[0x10000+0x4000];	//+0x4000 to avoid having to warp around twice on cos


u32*  xf_hex=(u32*)xf,*fr_hex=(u32*)fr;

StatusReg old_sr;
fpscr_type old_fpscr;

#define SAVE_REG(name) memcpy(&to->name,&name,sizeof(name))
#define LOAD_REG(name) memcpy(&name,&from->name,sizeof(name))

#define SAVE_REG_A(name) memcpy(to->name,name,sizeof(name))
#define LOAD_REG_A(name) memcpy(name,from->name,sizeof(name))

void SaveSh4Regs(Sh4RegContext* to)
{
	SAVE_REG_A(r);
	SAVE_REG_A(r_bank);

	SAVE_REG(gbr);
	SAVE_REG(ssr);
	SAVE_REG(spc);
	SAVE_REG(sgr);
	SAVE_REG(dbr);
	SAVE_REG(vbr);


	SAVE_REG(mac);
	SAVE_REG(pr);
	SAVE_REG(fpul);
	SAVE_REG(pc);

	SAVE_REG(sr);
	SAVE_REG(fpscr);

	SAVE_REG_A(xf);
	SAVE_REG_A(fr);

	SAVE_REG(old_sr);
	SAVE_REG(old_fpscr);
}
void LoadSh4Regs(Sh4RegContext* from)
{
	LOAD_REG_A(r);
	LOAD_REG_A(r_bank);

	LOAD_REG(gbr);
	LOAD_REG(ssr);
	LOAD_REG(spc);
	LOAD_REG(sgr);
	LOAD_REG(dbr);
	LOAD_REG(vbr);


	LOAD_REG(mac);
	LOAD_REG(pr);
	LOAD_REG(fpul);
	LOAD_REG(pc);

	LOAD_REG(sr);
	LOAD_REG(fpscr);

	LOAD_REG_A(xf);
	LOAD_REG_A(fr);

	LOAD_REG(old_sr);
	LOAD_REG(old_fpscr);
}

INLINE void ChangeGPR()
{
	u32 temp[8];
	for (int i=0;i<8;i++)
	{
		temp[i]=r[i];
		r[i]=r_bank[i];
		r_bank[i]=temp[i];
	}
}

INLINE void ChangeFP()
{
	u32 temp[16];
	for (int i=0;i<16;i++)
	{
		temp[i]=fr_hex[i];
		fr_hex[i]=xf_hex[i];
		xf_hex[i]=temp[i];
	}
}

//called when sr is changed and we must check for reg banks ect.. , returns true if interrupts got 
bool UpdateSR()
{
	if (sr.MD)
	{
		if (old_sr.RB !=sr.RB)
			ChangeGPR();//bank change
	}
	else
	{
		if (sr.RB)
		{
			log("UpdateSR MD=0;RB=1 , this must not happen\n");
			sr.RB =0;//error - must allways be 0
			if (old_sr.RB)
				ChangeGPR();//switch
		}
		else
		{
			if (old_sr.RB)
				ChangeGPR();//switch
		}
	}
/*
	if ((old_sr.IMASK!=0xF) && (sr.IMASK==0xF))
	{
		//log("Interrupts disabled  , pc=0x%X\n",pc);
	}

	if ((old_sr.IMASK==0xF) && (sr.IMASK!=0xF))
	{
		//log("Interrupts enabled  , pc=0x%X\n",pc);
	}
	
	bool rv=old_sr.IMASK > sr.IMASK;
	rv|=old_sr.BL==1 && sr.BL==0; 
	if (sr.IMASK==0xF)
		rv=false;
*/
	old_sr.m_full=sr.m_full;
	
	return SRdecode();
}

//make x86 and sh4 float status registers match ;)
u32 old_rm=0xFF;
u32 old_dn=0xFF;
void SetFloatStatusReg()
{
	if ((old_rm!=fpscr.RM) || (old_dn!=fpscr.DN) || (old_rm==0xFF )|| (old_dn==0xFF))
	{
		old_rm=fpscr.RM ;
		old_dn=fpscr.DN ;
		u32 temp=0x1f80;	//no flush to zero && round to nearest

		if (fpscr.RM==1)	//if round to 0 , set the flag
			temp|=(3<<13);

		if (fpscr.DN)		//denormals are considered 0
			temp|=(1<<15);
		_asm 
		{
			ldmxcsr temp;	//load the float status :)
		}
	}
}
//called when fpscr is changed and we must check for reg banks ect..
void UpdateFPSCR()
{
	if (fpscr.FR !=old_fpscr.FR)
		ChangeFP();//fpu bank change
	old_fpscr=fpscr;
	SetFloatStatusReg();//ensure they are on sync :)
}

void CleanupSinCos() {
 
}

void GenerateSinCos()
{
	wchar* path=GetEmuPath(L"data\\fsca-table.bin");
	FILE* tbl=_wfopen(path,L"rb");
	free(path);
	if (!tbl)
		die("fsca-table.bin is missing!");

 

	fread(sin_table,1,4*0x8000,tbl);
	fclose(tbl);

	for (int i=0;i<0x10000;i++)
	{
		if (i<0x8000)
			__noop;
		else if (i==0x8000)
			sin_table[i]=0;
		else
			sin_table[i]=-sin_table[i-0x8000];
	}
	for (int i=0x10000;i<0x14000;i++)
	{
		sin_table[i]=sin_table[(u16)i];//warp around for the last 0x4000 entries
	}
}
#ifdef DEBUG
f64 GetDR(u32 n)
{
#ifdef TRACE
	if (n>7)
		log("DR_r INDEX OVERRUN %d >7",n);
#endif
	double t;
	((u32*)(&t))[1]=fr_hex[(n<<1) | 0];
	((u32*)(&t))[0]=fr_hex[(n<<1) | 1];
	return t;
}

f64 GetXD(u32 n)
{
#ifdef TRACE
	if (n>7)
		log("XD_r INDEX OVERRUN %d >7",n);
#endif
	double t;
	((u32*)(&t))[1]=xf_hex[(n<<1) | 0];
	((u32*)(&t))[0]=xf_hex[(n<<1) | 1];
	return t;
}

void SetDR(u32 n,f64 val)
{
#ifdef TRACE
	if (n>7)
		log("DR_w INDEX OVERRUN %d >7",n);
#endif
	fr_hex[(n<<1) | 1]=((u32*)(&val))[0];
	fr_hex[(n<<1) | 0]=((u32*)(&val))[1];
}

void SetXD(u32 n,f64 val)
{
#ifdef TRACE
	if (n>7)
		log("XD_w INDEX OVERRUN %d >7",n);
#endif

	xf_hex[(n<<1) | 1]=((u32*)(&val))[0];
	xf_hex[(n<<1) | 0]=((u32*)(&val))[1];
}
#endif

u32* Sh4_int_GetRegisterPtr(Sh4RegType reg)
{
	if ((reg>=r0) && (reg<=r15))
	{
		return &r[reg-r0];
	}
	else if ((reg>=r0_Bank) && (reg<=r7_Bank))
	{
		return &r_bank[reg-r0_Bank];
	}
	else if ((reg>=fr_0) && (reg<=fr_15))
	{
		return &fr_hex[reg-fr_0];
	}
	else if ((reg>=xf_0) && (reg<=xf_15))
	{
		return &xf_hex[reg-xf_0];
	}
	else
	{
		switch(reg)
		{
		case reg_gbr :
			return &gbr;
			break;
		case reg_vbr :
			return &vbr;
			break;

		case reg_ssr :
			return &ssr;
			break;

		case reg_spc :
			return &spc;
			break;

		case reg_sgr :
			return &sgr;
			break;

		case reg_dbr :
			return &dbr;
			break;

		case reg_mach :
			return &mac.h;
			break;

		case reg_macl :
			return &mac.l;
			break;

		case reg_pr :
			return &pr;
			break;

		case reg_fpul :
			return &fpul;
			break;


		case reg_pc :
			return &pc;
			break;

		case reg_sr :
			return 0;//&sr.m_full; ?
			break;

		case reg_sr_T :
			return &sr.T;
			break;

		case reg_fpscr :
			return &fpscr.full;
			break;


		default:
			EMUERROR2("unknown register Id %d",reg);
			return 0;
			break;
		}
	}
}
