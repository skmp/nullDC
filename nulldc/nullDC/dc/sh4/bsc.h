#pragma once

#include "types.h"

//Init/Res/Term
void bsc_Init();
void bsc_Reset(bool Manual);
void bsc_Term();


//32 bits
//All bits exept A0MPX,MASTER,ENDIAN are editable and reseted to 0
union BCR1_type
{
	struct
	{
		u32 A56PCM:1;
		u32 res_0:1;
		u32 DRAMTP0:1;
		u32 DRAMTP1:1;
		u32 DRAMTP2:1;
		u32 A6BST0:1;
		u32 A6BST1:1;
		u32 A6BST2:1;
		//8
		u32 A5BST0:1;
		u32 A5BST1:1;
		u32 A5BST2:1;
		u32 A0BST0:1;
		u32 A0BST1:1;
		u32 A0BST2:1;
		u32 HIZCNT:1;
		u32 HIZMEM:1;
		//16
		u32 res_1:1;
		u32 MEMMPX:1;
		u32 PSHR:1;
		u32 BREQEN:1;
		u32 A4MBC:1;
		u32 A1MBC:1;
		u32 res_2:1;
		u32 res_3:1;
		//24
		u32 OPUP:1;
		u32 IPUP:1;
		u32 res_4:1;
		u32 res_5:1;
		u32 res_6:1;
		u32 A0MPX:1;	//set to 1 (area 0 is mpx)
		u32 MASTER:1;	//what is it on dreamcast ?
		u32 ENDIAN:1;	//this is 1 on dreamcast
	};

	u32 full;
};

extern BCR1_type BSC_BCR1;

//16 bit
//A0SZ0,A0SZ1 are read olny , ohters are are editable and reseted to 0
union BCR2_type
{
	struct
	{
		u32 PORTEN:1;
		u32 res_0:1;
		u32 A0SZ0:1;
		u32 A1SZ1:1;
		u32 A2SZ0:1;
		u32 A2SZ1:1;
		u32 A3SZ0:1;
		u32 A3SZ1:1;
		//8
		u32 A4SZ0:1;
		u32 A4SZ1:1;
		u32 A5SZ0:1;
		u32 A5SZ1:1;
		u32 A6SZ0:1;
		u32 A6SZ1:1;
		u32 A0SZ0_inp:1;	//read olny , what value on dc?
		u32 A0SZ1_inp:1;	//read olny , what value on dc?
		//16
	};

	u16 full;
};
extern BCR2_type BSC_BCR2;

//32 bits
union WCR1_type
{
	struct
	{
		u32 A0IW0:1;
		u32 A0IW1:1;
		u32 A0IW2:1;
		u32 res_0:1;
		u32 A1IW0:1;
		u32 A1IW1:1;
		u32 A1IW2:1;
		u32 res_1:1;
		//8
		u32 A2IW0:1;
		u32 A2IW1:1;
		u32 A2IW2:1;
		u32 res_2:1;
		u32 A3IW0:1;
		u32 A3IW1:1;
		u32 A3IW2:1;
		u32 res_3:1;
		//16
		u32 A4IW0:1;
		u32 A4IW1:1;
		u32 A4IW2:1;
		u32 res_4:1;
		u32 A5IW0:1;
		u32 A5IW1:1;
		u32 A5IW2:1;
		u32 res_5:1;
		//24
		u32 A6IW0:1;
		u32 A6IW1:1;
		u32 A6IW2:1;
		u32 res_6:1;
		u32 DMAIW0:1;
		u32 DMAIW1:1;	
		u32 DMAIW2:1;	
		u32 res_7:1;		
	};

	u32 full;
};
extern WCR1_type BSC_WCR1;

//32 bits
union WCR2_type
{
	struct
	{
		u32 A0B0:1;
		u32 A0B1:1;
		u32 A0B2:1;
		u32 A0W0:1;
		u32 A0W1:1;
		u32 A0W2:1;
		u32 A1W0:1;
		u32 A1W1:1;
		//8
		u32 A1W2:1;
		u32 A2W0:1;
		u32 A2W1:1;
		u32 A2W2:1;
		u32 res_0:1;
		u32 A3W0:1;
		u32 A3W1:1;
		u32 A3W2:1;
		//16
		u32 res_1:1;
		u32 A4W0:1;
		u32 A4W1:1;
		u32 A4W2:1;
		u32 A5B0:1;
		u32 A5B1:1;
		u32 A5B2:1;
		u32 A5W0:1;
		//24
		u32 A5W1:1;
		u32 A5W2:1;
		u32 A6B0:1;
		u32 A6B1:1;
		u32 A6B2:1;
		u32 A6W0:1;	
		u32 A6W1:1;	
		u32 A6W2:1;		
	};

	u32 full;
};

extern WCR2_type BSC_WCR2;

//32 bits
union WCR3_type
{
	struct
	{
		u32 A0H0:1;
		u32 A0H1:1;
		u32 A0S0:1;
		u32 res_0:1;
		u32 A1H0:1;	//TODO: check if this is correct, on the maual it says A1H0 .. typo in the manual ? 
		u32 A1H1:1;
		u32 A1S0:1;
		u32 res_1:1;
		//8
		u32 A2H0:1;
		u32 A2H1:1;
		u32 A2S0:1;
		u32 res_2:1;
		u32 A3H0:1;
		u32 A3H1:1;
		u32 A3S0:1;
		u32 res_3:1;
		//16
		u32 A4H0:1;
		u32 A4H1:1;
		u32 A4S0:1;
		u32 res_4:1;
		u32 A5H0:1;
		u32 A5H1:1;
		u32 A5S0:1;
		u32 res_5:1;
		//24
		u32 A6H0:1;
		u32 A6H1:1;
		u32 A6S0:1;
		u32 res_6:1;
		u32 res_7:1;
		u32 res_8:1;	
		u32 res_9:1;	
		u32 res_10:1;		
	};

	u32 full;
};


extern WCR3_type BSC_WCR3;

//32 bits
union MCR_type
{
	struct
	{
		u32 EDO_MODE:1;
		u32 RMODE:1;
		u32 RFSH:1;
		u32 AMX0:1;
		u32 AMX1:1;
		u32 AMX2:1;
		u32 AMXEXT:1;
		u32 SZ0:1;
		//8
		u32 SZ1:1;
		u32 BE:1;
		u32 TRAS0:1;
		u32 TRAS1:1;
		u32 TRAS2:1;
		u32 TRWL0:1;
		u32 TRWL1:1;
		u32 TRWL2:1;
		//16
		u32 RCD0:1;
		u32 RCD1:1;
		u32 res_0:1;
		u32 TPC0:1;
		u32 TPC1:1;
		u32 TPC2:1;
		u32 res_1:1;
		u32 TCAS:1;
		//24
		u32 res_2:1;
		u32 res_3:1;
		u32 res_4:1;
		u32 TRC0:1;
		u32 TRC1:1;
		u32 TRC2:1;	
		u32 MRSET:1;	
		u32 RASD:1;		
	};

	u32 full;
};


extern MCR_type BSC_MCR;

//16 bits
union PCR_type
{
	struct
	{
		u32 A6TEH0:1;
		u32 A6TEH1:1;
		u32 A6TEH2:1;
		u32 A5TEH0:1;
		u32 A5TEH1:1;
		u32 A5TEH2:1;
		u32 A6TED0:1;
		u32 A6TED1:1;
		//8
		u32 A6TED2:1;
		u32 A5TED0:1;
		u32 A5TED1:1;
		u32 A5TED2:1;
		u32 A6PCW0:1;
		u32 A6PCW1:1;
		u32 A5PCW0:1;
		u32 A5PCW1:1;
		//16
	};
	u16 full;
};

extern PCR_type BSC_PCR;

//16 bits -> misstype on manual ? RTSCR vs RTCSR...
union RTCSR_type
{
	struct
	{
		u32 LMTS:1;
		u32 OVIE:1;
		u32 OVF:1;
		u32 CKS0:1;
		u32 CKS1:1;
		u32 CKS2:1;
		u32 CMIE:1;
		u32 CMF:1;
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

extern RTCSR_type BSC_RTCSR;

//16 bits
union RTCNT_type
{
	struct
	{
		u32 VALUE:8;
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

extern RTCNT_type BSC_RTCNT;

//16 bits
union RTCOR_type
{
	struct
	{
		u32 VALUE:8;
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


extern RTCOR_type BSC_RTCOR;

//16 bits
union RFCR_type
{
	struct
	{
		u32 VALUE:10;
		//10
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

extern RFCR_type BSC_RFCR;

//32 bits
union PCTRA_type
{
	struct
	{
		u32 PB0IO:1;
		u32 PB0PUP:1;
		u32 PB1IO:1;
		u32 PB1PUP:1;
		u32 PB2IO:1;
		u32 PB2PUP:1;
		u32 PB3IO:1;
		u32 PB3PUP:1;
		//8
		u32 PB4IO:1;
		u32 PB4PUP:1;
		u32 PB5IO:1;
		u32 PB5PUP:1;
		u32 PB6IO:1;
		u32 PB6PUP:1;
		u32 PB7IO:1;
		u32 PB7PUP:1;
		//16
		u32 PB8IO:1;
		u32 PB8PUP:1;
		u32 PB9IO:1;
		u32 PB9PUP:1;
		u32 PB10IO:1;
		u32 PB10PUP:1;
		u32 PB11IO:1;
		u32 PB11PUP:1;
		//24
		u32 PB12IO:1;
		u32 PB12PUP:1;
		u32 PB13IO:1;
		u32 PB13PUP:1;
		u32 PB14IO:1;
		u32 PB14PUP:1;
		u32 PB15IO:1;
		u32 PB15PUP:1;
	};

	u32 full;
};

extern PCTRA_type BSC_PCTRA;

//16 bits
union PDTRA_type
{
	struct
	{
		u32 PB0DT:1;
		u32 PB1DT:1;
		u32 PB2DT:1;
		u32 PB3DT:1;
		u32 PB4DT:1;
		u32 PB5DT:1;
		u32 PB6DT:1;
		u32 PB7DT:1;
		//8
		u32 PB8DT:1;
		u32 PB9DT:1;
		u32 PB10DT:1;
		u32 PB11DT:1;
		u32 PB12DT:1;
		u32 PB13DT:1;
		u32 PB14DT:1;
		u32 PB15DT:1;
		//16
	};
	u16 full;
};

extern PDTRA_type BSC_PDTRA;

//32 bits
union PCTRB_type
{
	struct
	{
		u32 PB16IO:1;
		u32 PB16PUP:1;
		u32 PB17IO:1;
		u32 PB17PUP:1;
		u32 PB18IO:1;
		u32 PB18PUP:1;
		u32 PB19IO:1;
		u32 PB19PUP:1;
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
		u32 res_8:1;
		u32 res_9:1;
		u32 res_10:1;
		u32 res_11:1;
		u32 res_12:1;
		u32 res_13:1;
		u32 res_14:1;
		u32 res_15:1;
		//24
		u32 res_16:1;
		u32 res_17:1;
		u32 res_18:1;
		u32 res_19:1;
		u32 res_20:1;
		u32 res_21:1;
		u32 res_22:1;
		u32 res_23:1;
	};

	u32 full;
};

extern PCTRB_type BSC_PCTRB;

//16 bits
union PDTRB_type
{
	struct
	{
		u32 PB16DT:1;
		u32 PB17DT:1;
		u32 PB18DT:1;
		u32 PB19DT:1;
		u32 res_0:1;
		u32 res_1:1;
		u32 res_2:1;
		u32 res_3:1;
		//8
		u32 res_4:1;
		u32 res_5:1;
		u32 res_6:1;
		u32 res_7:1;
		u32 res_8:1;
		u32 res_9:1;
		u32 res_10:1;
		u32 res_11:1;
		//16
	};
	u16 full;
};

extern PDTRB_type BSC_PDTRB;

//16 bits
union GPIOIC_type
{
	struct
	{
		u32 PTIREN0:1;
		u32 PTIREN1:1;
		u32 PTIREN2:1;
		u32 PTIREN3:1;
		u32 PTIREN4:1;
		u32 PTIREN5:1;
		u32 PTIREN6:1;
		u32 PTIREN7:1;
		//8
		u32 PTIREN8:1;
		u32 PTIREN9:1;
		u32 PTIREN10:1;
		u32 PTIREN11:1;
		u32 PTIREN12:1;
		u32 PTIREN13:1;
		u32 PTIREN14:1;
		u32 PTIREN15:1;
		//16
	};
	u16 full;
};

extern GPIOIC_type BSC_GPIOIC;