#pragma once

#include "types.h"

//Init/Res/Term
void scif_Init();
void scif_Reset(bool Manual);
void scif_Term();



//SCIF SCSMR2 0xFFE80000 0x1FE80000 16 0x0000 0x0000 Held Held Pclk
union SCSMR2_type
{
	struct
	{
		u32 CKS0:1;
		u32 CKS1:1;
		u32 res_0:1;
		u32 STOP:1;
		u32 OE_paritymode:1;
		u32 PE:1;
		u32 CHR:1;
		u32 res_1:1;
		//8
		u32 res_2:1;
		u32 res_3:1;
		u32 res_4:1;
		u32 res_5:1;
		u32 res_6:1;
		u32 res_7:1;
		u32 res_8:1;
		u32 res_9:1;
		//16
	};
	u16 full;
};

extern SCSMR2_type SCIF_SCSMR2;

//SCIF SCBRR2 0xFFE80004 0x1FE80004 8 0xFF 0xFF Held Held Pclk
extern u8 SCIF_SCBRR2;

//SCIF SCSCR2 0xFFE80008 0x1FE80008 16 0x0000 0x0000 Held Held Pclk
union SCSCR2_type
{
	struct
	{
		u32 res_0:1;
		u32 CKE1:1;
		u32 res_1:1;
		u32 REIE:1;
		u32 RE:1;
		u32 TE:1;
		u32 RIE:1;
		u32 TIE:1;
		//8
		u32 res_2:1;
		u32 res_3:1;
		u32 res_4:1;
		u32 res_5:1;
		u32 res_6:1;
		u32 res_7:1;
		u32 res_8:1;
		u32 res_9:1;
		//16
	};
	u16 full;
};
extern SCSCR2_type SCIF_SCSCR2;

//SCIF SCFTDR2 0xFFE8000C 0x1FE8000C 8 Undefined Undefined Held Held Pclk
extern u8 SCIF_SCFTDR2;

//SCIF SCFSR2 0xFFE80010 0x1FE80010 16 0x0060 0x0060 Held Held Pclk
union SCFSR2_type
{
	struct
	{
		u32 DR:1;
		u32 RDF:1;
		u32 PER:1;
		u32 FER:1;
		u32 BRK:1;
		u32 TDFE:1;
		u32 TEND:1;
		u32 ER:1;
		//8
		u32 FER0:1;
		u32 FER1:1;
		u32 FER2:1;
		u32 FER3:1;
		u32 PER0:1;
		u32 PER1:1;
		u32 PER2:1;
		u32 PER3:1;
		//16
	};
	u16 full;
};
extern SCSCR2_type SCIF_SCFSR2;

//SCIF SCFRDR2 0xFFE80014 0x1FE80014 8 Undefined Undefined Held Held Pclk
//Read OLNY
extern u8 SCIF_SCFRDR2;

//SCIF SCFCR2 0xFFE80018 0x1FE80018 16 0x0000 0x0000 Held Held Pclk
union SCFCR2_type
{
	struct
	{
		u32 LOOP:1;
		u32 RFRST:1;
		u32 TFRST:1;
		u32 MCE:1;
		u32 TTRG0:1;
		u32 TTRG1:1;
		u32 RTRG0:1;
		u32 RTRG1:1;
		//8
		u32 res_0:1;
		u32 res_1:1;
		u32 res_2:1;
		u32 res_3:1;
		u32 res_4:1;
		u32 res_5:1;
		u32 res_6:1;
		u32 res_7:1;
		//16
	};
	u16 full;
};
extern SCFCR2_type SCIF_SCFCR2;

//Read OLNY
//SCIF SCFDR2 0xFFE8001C 0x1FE8001C 16 0x0000 0x0000 Held Held Pclk
union SCFDR2_type
{
	struct
	{
		u32 R:5;
		u32 res_0:3;
		//8
		u32 T:5;
		u32 res_1:3;
		//16
	};
	u16 full;
};
extern SCFDR2_type SCIF_SCFDR2;

//SCIF SCSPTR2 0xFFE80020 0x1FE80020 16 0x0000 0x0000 Held Held Pclk
union SCSPTR2_type
{
	struct
	{
		u32 SPB2DT:1;
		u32 SPB2IO:1;
		u32 res_0:1;
		u32 res_1:1;
		u32 CTSDT:1;
		u32 CTSIO:1;
		u32 RTSDT:1;
		u32 RTSIO:1;
		//8
		u32 res_2:1;
		u32 res_3:1;
		u32 res_4:1;
		u32 res_5:1;
		u32 res_6:1;
		u32 res_7:1;
		u32 res_8:1;
		u32 res_9:1;
		//16
	};
	u16 full;
};
extern SCSPTR2_type SCIF_SCSPTR2;

//SCIF SCLSR2 0xFFE80024 0x1FE80024 16 0x0000 0x0000 Held Held Pclk
union SCLSR2_type
{
	struct
	{
		u32 ORER:1;
		u32 res_0:7;
		//8
		u32 res_1:8;
		//16
	};
	u16 full;
};
extern SCLSR2_type SCIF_SCLSR2;