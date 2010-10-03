#pragma once

#include "types.h"

union DMAC_CHCR_type
{
	struct
	{
		u32 DE:1;	//Channel Enable
		u32 TE:1;	//Transfer End
		u32 IE:1;	//Interrupt Enable
		u32 res0:1;

		u32 TS:3;	//Transmit Size
		//u32 TS1:1;
		//u32 TS2:1;
		u32 TM:1;	//Transmit Mode

		u32 RS:4;	//Resource Select
		//u32 RS1:1;
		//u32 RS2:1;
		//u32 RS3:1;

		u32 SM:2;	//SRC mode
		//u32 SM1:1;
		u32 DM:2;	//DST mode
		//u32 DM1:1;

		u32 AL:1;	//Acknowledge Level
		u32 AM:1;	//Acknowledge Mode
		u32 RL:1;	//In normal DMA mode, this bit is valid only in CHCR0 and CHCR1. In DDT mode, this bit is invalid.
		u32 DS:1;	//In normal DMA mode, this bit is valid only in CHCR0 and CHCR1. In DDT mode, it is valid in CHCR0–CHCR3.

		u32 res1:4;

		u32 DTC:1;
		u32 DSA:3;
		//u32 DSA1:1;
		//u32 DSA2:1;

		u32 STC:1;
		u32 SSA:3;
		//u32 SSA1:1;
		//u32 SSA2:1;
	};
	u32 full;
};

union DMAC_DMAOR_type
{
	struct
	{
		u32 DME:1;
		u32 NMIF:1;
		u32 AE:1;
		u32 res0:1;

		u32 COD:1;
		u32 res1:3;
		
		u32 PR0:1;
		u32 PR1:1;
		u32 res2:2;

		u32 res3:3;
		u32 DDT:1;

		u32 res4:16;
	};
	u32 full;
};

extern u32 DMAC_SAR[4];
extern u32 DMAC_DAR[4];
extern u32 DMAC_DMATCR[4];//only 24 bits valid
extern DMAC_CHCR_type DMAC_CHCR[4];

extern DMAC_DMAOR_type DMAC_DMAOR;

//
void dmac_ddt_ch0_ddt(u32 src,u32 dst,u32 count);
void dmac_ddt_ch2_direct(u32 dst,u32 count);
void DMAC_Ch2St();

//Init/Res/Term
void dmac_Init();
void dmac_Reset(bool Manual);
void dmac_Term();
void UpdateDMA();



#define DMAOR_MASK	0xFFFF8201