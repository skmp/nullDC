#pragma once

#include "types.h"

//Init/Res/Term
void ccn_Init();
void ccn_Reset(bool Manual);
void ccn_Term();

union CCN_PTEH_type
{
	struct
	{
		u32 ASID:8; //0-7 ASID
		u32 res:2;  //8,9 reserved
		u32 VPN:22; //10-31 VPN
	};
	u32 reg_data;
};

union CCN_PTEL_type
{
	struct
	{
		u32 WT:1;
		u32 SH:1;
		u32 D :1;
		u32 C :1;
		
		u32 SZ0:1;
		u32 PR :2;
		u32 SZ1:1;

		u32 V:1;
		u32 res_0:1;
		u32 PPN:19;//PPN 10-28
		u32 res_1:3;
	};
	u32 reg_data;
};

union CCN_MMUCR_type
{
	struct
	{
		u32 AT:1;
		u32 res:1;
		u32 TI:1;
		u32 res_2:5;
		u32 SV:1;
		u32 SQMD:1;
		u32 URC:6;
		u32 URB:6;
		u32 LRUI:6;
	};
	u32 reg_data;
};

union CCN_PTEA_type
{
	struct
	{
		u32 SA:3;
		u32 TC:1;
		u32 res:28;
	};
	u32 reg_data;
};

union CCN_CCR_type
{
	struct
	{
		u32 OCE:1;
		u32 WT:1;
		u32 CB:1;
		u32 OCI:1;
		u32 res:1;
		u32 ORA:1;
		u32 res_1:1;
		u32 OIX:1;
		u32 ICE:1;
		u32 res_2:2;
		u32 ICI:1;
		u32 res_3:3;
		u32 IIX:1;
		u32 res_4:16;
	};
	u32 reg_data;
};

union CCN_QACR_type
{
	struct
	{
		u32 res:2;
		u32 Area:3;
		u32 res_1:27;
	};
	u32 reg_data;
};


//Types
extern CCN_PTEH_type CCN_PTEH;
extern CCN_PTEL_type CCN_PTEL;
extern u32 CCN_TTB;
extern u32 CCN_TEA;
extern CCN_MMUCR_type CCN_MMUCR;
extern u8 CCN_BASRA;
extern u8 CCN_BASRB;
extern CCN_CCR_type CCN_CCR;
extern u32 CCN_TRA;
extern u32 CCN_EXPEVT;
extern u32 CCN_INTEVT;
extern CCN_PTEA_type CCN_PTEA;
extern CCN_QACR_type CCN_QACR[2];
extern u32			 CCN_QACR_TR[2];