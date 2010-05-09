/*
	Lovely timers, its amazing how many times this module was bugged
*/

#include "types.h"
#include "tmu.h"
#include "intc.h"
#include "dc/mem/sh4_internal_reg.h"


#define tmu_underflow	0x0100
#define tmu_UNIE		0x0020
u32 tmu_prescaler[3];
u32 tmu_prescaler_shift[3];
u32 tmu_prescaler_mask[3];
const u32 tmu_ch_bit[3]={1,2,4};

u32 tmu_regs_CNT[3];
u32 tmu_regs_COR[3];
u16 tmu_regs_CR[3];
u32 old_mode[3] = {0xFFFF,0xFFFF,0xFFFF};
u8 TMU_TOCR,TMU_TSTR;

const InterruptID tmu_intID[3]={sh4_TMU0_TUNI0,sh4_TMU1_TUNI1,sh4_TMU2_TUNI2};

//Accurate counts for the chanel ch
template<u32 ch>
void UpdateTMU_chan(u32 clc)
{
	//if chanel is on
	if ((TMU_TSTR & tmu_ch_bit[ch])!=0)
	{
		//count :D
		tmu_prescaler[ch]+=clc;
		u32 steps=tmu_prescaler[ch]>>tmu_prescaler_shift[ch];
		
		if (steps>tmu_regs_CNT[ch])
		{
			//remove the 'extra' steps to overflow
			steps-=tmu_regs_CNT[ch];
			//refill the counter
			tmu_regs_CNT[ch] = tmu_regs_COR[ch];
			//raise the interrupt
			tmu_regs_CR[ch] |= tmu_underflow;
			InterruptPend(tmu_intID[ch],1);
			
			//remove the full underflows (possible because we only check every 448 cycles)
			//this can be done with a div, but its very very very rare so this is probably faster
			//THIS can probably be replaced with a verify check on counter setup (havn't seen any game do this)
			while(steps>tmu_regs_COR[ch])
				steps-=tmu_regs_COR[ch];

			//steps now has the partial steps needed for update, guaranteeded it won't cause an overflow
		}
		//count down
		tmu_regs_CNT[ch]-=steps;
		//remove the full steps from the prescaler counter
		tmu_prescaler[ch]&=tmu_prescaler_mask[ch];
	}
}

void UpdateTMU(u32 Cycles)
{
	UpdateTMU_chan<0>(Cycles);
	UpdateTMU_chan<1>(Cycles);
	UpdateTMU_chan<2>(Cycles);
}

//Update internal counter registers
void UpdateTMUCounts(u32 reg)
{
	InterruptPend(tmu_intID[reg],tmu_regs_CR[reg] & tmu_underflow);
	InterruptMask(tmu_intID[reg],tmu_regs_CR[reg] & tmu_UNIE);

	if (old_mode[reg]==(tmu_regs_CR[reg] & 0x7))
		return;
	else
		old_mode[reg]=(tmu_regs_CR[reg] & 0x7);

	switch(tmu_regs_CR[reg] & 0x7)
	{
		case 0:	//4
			tmu_prescaler_shift[reg]=2;
			break;

		case 1:	//16
			tmu_prescaler_shift[reg]=4;
			break;

		case 2:	//64
			tmu_prescaler_shift[reg]=6;
			break;

		case 3:	//256
			tmu_prescaler_shift[reg]=8;
			break;

		case 4:	//1024
			tmu_prescaler_shift[reg]=10;
			break;

		case 5:	//reserved
			printf("TMU ch%d , TCR%d mode is reserved (5)",reg,reg);
			break;

		case 6:	//RTC
			printf("TMU ch%d , TCR%d mode is RTC (6) , can't be used on dreamcast",reg,reg);
			break;

		case 7:	//external
			printf("TMU ch%d , TCR%d mode is External (7) , can't be used on dreamcast",reg,reg);
			break;
	}
	tmu_prescaler_shift[reg]+=2;// because we count in Iö cycles (cpu core cycles) and the tmu is provided w/
					    // the Pö (perhipal clock)
	tmu_prescaler_mask[reg]=(1<<tmu_prescaler_shift[reg])-1;

	tmu_prescaler[reg]=0;
}

//Write to status registers
void TMU_TCR0_write(u32 data)
{
	tmu_regs_CR[0]=(u16)data;
	UpdateTMUCounts(0);
}
void TMU_TCR1_write(u32 data)
{
	tmu_regs_CR[1]=(u16)data;
	UpdateTMUCounts(1);
}
void TMU_TCR2_write(u32 data)
{
	tmu_regs_CR[2]=(u16)data;
	UpdateTMUCounts(2);
}

//Chan 2 not used functions
u32 TMU_TCPR2_read()
{
	EMUERROR("Read from TMU_TCPR2  , this regiser should be not used on dreamcast according to docs");
	return 0;
}

void TMU_TCPR2_write(u32 data)
{
	EMUERROR2("Write to TMU_TCPR2  , this regiser should be not used on dreamcast according to docs , data=%d",data);
}

//Init/Res/Term
void tmu_Init()
{
	//TMU TOCR 0xFFD80000 0x1FD80000 8 0x00 0x00 Held Held Pclk
	TMU[(TMU_TOCR_addr&0xFF)>>2].flags=REG_8BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	TMU[(TMU_TOCR_addr&0xFF)>>2].readFunction=0;
	TMU[(TMU_TOCR_addr&0xFF)>>2].writeFunction=0;
	TMU[(TMU_TOCR_addr&0xFF)>>2].data8=&TMU_TOCR;

	//TMU TSTR 0xFFD80004 0x1FD80004 8 0x00 0x00 Held 0x00 Pclk
	TMU[(TMU_TSTR_addr&0xFF)>>2].flags=REG_8BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	TMU[(TMU_TSTR_addr&0xFF)>>2].readFunction=0;
	TMU[(TMU_TSTR_addr&0xFF)>>2].writeFunction=0;
	TMU[(TMU_TSTR_addr&0xFF)>>2].data8=&TMU_TSTR;

	//TMU TCOR0 0xFFD80008 0x1FD80008 32 0xFFFFFFFF 0xFFFFFFFF Held Held Pclk
	TMU[(TMU_TCOR0_addr&0xFF)>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	TMU[(TMU_TCOR0_addr&0xFF)>>2].readFunction=0;
	TMU[(TMU_TCOR0_addr&0xFF)>>2].writeFunction=0;
	TMU[(TMU_TCOR0_addr&0xFF)>>2].data32=&tmu_regs_COR[0];

	//TMU TCNT0 0xFFD8000C 0x1FD8000C 32 0xFFFFFFFF 0xFFFFFFFF Held Held Pclk
	TMU[(TMU_TCNT0_addr&0xFF)>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	TMU[(TMU_TCNT0_addr&0xFF)>>2].readFunction=0;
	TMU[(TMU_TCNT0_addr&0xFF)>>2].writeFunction=0;
	TMU[(TMU_TCNT0_addr&0xFF)>>2].data32=&tmu_regs_CNT[0];

	//TMU TCR0 0xFFD80010 0x1FD80010 16 0x0000 0x0000 Held Held Pclk
	TMU[(TMU_TCR0_addr&0xFF)>>2].flags=REG_16BIT_READWRITE | REG_READ_DATA;
	TMU[(TMU_TCR0_addr&0xFF)>>2].readFunction=0;
	TMU[(TMU_TCR0_addr&0xFF)>>2].writeFunction=TMU_TCR0_write;
	TMU[(TMU_TCR0_addr&0xFF)>>2].data16=&tmu_regs_CR[0];

	//TMU TCOR1 0xFFD80014 0x1FD80014 32 0xFFFFFFFF 0xFFFFFFFF Held Held Pclk
	TMU[(TMU_TCOR1_addr&0xFF)>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	TMU[(TMU_TCOR1_addr&0xFF)>>2].readFunction=0;
	TMU[(TMU_TCOR1_addr&0xFF)>>2].writeFunction=0;
	TMU[(TMU_TCOR1_addr&0xFF)>>2].data32=&tmu_regs_COR[1];

	//TMU TCNT1 0xFFD80018 0x1FD80018 32 0xFFFFFFFF 0xFFFFFFFF Held Held Pclk
	TMU[(TMU_TCNT1_addr&0xFF)>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	TMU[(TMU_TCNT1_addr&0xFF)>>2].readFunction=0;
	TMU[(TMU_TCNT1_addr&0xFF)>>2].writeFunction=0;
	TMU[(TMU_TCNT1_addr&0xFF)>>2].data32=&tmu_regs_CNT[1];

	//TMU TCR1 0xFFD8001C 0x1FD8001C 16 0x0000 0x0000 Held Held Pclk
	TMU[(TMU_TCR1_addr&0xFF)>>2].flags=REG_16BIT_READWRITE | REG_READ_DATA;
	TMU[(TMU_TCR1_addr&0xFF)>>2].readFunction=0;
	TMU[(TMU_TCR1_addr&0xFF)>>2].writeFunction=TMU_TCR1_write;
	TMU[(TMU_TCR1_addr&0xFF)>>2].data16=&tmu_regs_CR[1];

	//TMU TCOR2 0xFFD80020 0x1FD80020 32 0xFFFFFFFF 0xFFFFFFFF Held Held Pclk
	TMU[(TMU_TCOR2_addr&0xFF)>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	TMU[(TMU_TCOR2_addr&0xFF)>>2].readFunction=0;
	TMU[(TMU_TCOR2_addr&0xFF)>>2].writeFunction=0;
	TMU[(TMU_TCOR2_addr&0xFF)>>2].data32=&tmu_regs_COR[2];

	//TMU TCNT2 0xFFD80024 0x1FD80024 32 0xFFFFFFFF 0xFFFFFFFF Held Held Pclk
	TMU[(TMU_TCNT2_addr&0xFF)>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	TMU[(TMU_TCNT2_addr&0xFF)>>2].readFunction=0;
	TMU[(TMU_TCNT2_addr&0xFF)>>2].writeFunction=0;
	TMU[(TMU_TCNT2_addr&0xFF)>>2].data32=&tmu_regs_CNT[2];

	//TMU TCR2 0xFFD80028 0x1FD80028 16 0x0000 0x0000 Held Held Pclk
	TMU[(TMU_TCR2_addr&0xFF)>>2].flags=REG_16BIT_READWRITE | REG_READ_DATA;
	TMU[(TMU_TCR2_addr&0xFF)>>2].readFunction=0;
	TMU[(TMU_TCR2_addr&0xFF)>>2].writeFunction=TMU_TCR2_write;
	TMU[(TMU_TCR2_addr&0xFF)>>2].data16=&tmu_regs_CR[2];

	//TMU TCPR2 0xFFD8002C 0x1FD8002C 32 Held Held Held Held Pclk
	TMU[(TMU_TCPR2_addr&0xFF)>>2].flags=REG_32BIT_READWRITE;
	TMU[(TMU_TCPR2_addr&0xFF)>>2].readFunction=TMU_TCPR2_read;
	TMU[(TMU_TCPR2_addr&0xFF)>>2].writeFunction=TMU_TCPR2_write;
	TMU[(TMU_TCPR2_addr&0xFF)>>2].data32=0;
}

void tmu_Reset(bool Manual)
{
	TMU_TOCR=TMU_TSTR=0;
	tmu_regs_COR[0] = tmu_regs_COR[1] = tmu_regs_COR[2] = 0xffffffff;
	tmu_regs_CNT[0] = tmu_regs_CNT[1] = tmu_regs_CNT[2] = 0xffffffff;	
	tmu_regs_CR[0] = tmu_regs_CR[1] = tmu_regs_CR[2] = 0;

	UpdateTMUCounts(0);
	UpdateTMUCounts(1);
	UpdateTMUCounts(2);
}

void tmu_Term()
{
}