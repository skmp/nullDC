#pragma once
#include "types.h"
#include "sh4_if.h"

union mac_type
{
	struct { u32 l,h; };
	u64 full;
};

__declspec(align(32)) extern f32 sin_table[0x10000+0x4000];

struct Sh4RegContext
{
	u32 r[16];
	u32 r_bank[8];

	u32 gbr,ssr,spc,sgr,dbr,vbr;
	u32 pr,fpul;
	mac_type mac;
	u32 pc;
	//u16* pc_ptr;

	StatusReg sr;

	fpscr_type fpscr;

	f32 xf[16];
	f32 fr[16];

	StatusReg old_sr;
	fpscr_type old_fpscr;
};
void GenerateSinCos();
void CleanupSinCos();
__declspec(align(64)) extern u32 r[16];
__declspec(align(64)) extern u32 r_bank[8];

extern u32 gbr,ssr,spc,sgr,dbr,vbr;
extern u32 pr,fpul;
extern mac_type mac;
extern u32 pc;

extern StatusReg sr;

extern fpscr_type fpscr;

extern __declspec(align(64)) f32 xf[16];
extern __declspec(align(64)) f32 fr[16];


extern u32*  xf_hex,*fr_hex;

void SaveSh4Regs(Sh4RegContext* to);
void LoadSh4Regs(Sh4RegContext* from);

void UpdateFPSCR();
bool UpdateSR();

#ifndef DEBUG
INLINE f64 GetDR(u32 n)
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

INLINE f64 GetXD(u32 n)
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

INLINE void SetDR(u32 n,f64 val)
{
#ifdef TRACE
	if (n>7)
		log("DR_w INDEX OVERRUN %d >7",n);
#endif
	fr_hex[(n<<1) | 1]=((u32*)(&val))[0];
	fr_hex[(n<<1) | 0]=((u32*)(&val))[1];
}

INLINE void SetXD(u32 n,f64 val)
{
#ifdef TRACE
	if (n>7)
		log("XD_w INDEX OVERRUN %d >7",n);
#endif

	xf_hex[(n<<1) | 1]=((u32*)(&val))[0];
	xf_hex[(n<<1) | 0]=((u32*)(&val))[1];
}
#else
f64 GetDR(u32 n);
f64 GetXD(u32 n);
void SetDR(u32 n,f64 val);
void SetXD(u32 n,f64 val);
#endif

extern StatusReg old_sr;
extern fpscr_type old_fpscr;

u32* Sh4_int_GetRegisterPtr(Sh4RegType reg);
void SetFloatStatusReg();