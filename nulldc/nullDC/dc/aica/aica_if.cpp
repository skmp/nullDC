/*
	aica interface
		Handles RTC, Display mode reg && arm reset reg !
	arm7 is handled on a separate arm plugin now
*/
#include "types.h"
#include "aica_if.h"
#include "dc/mem/sh4_mem.h"
#include "dc/mem/sb.h"
#include "plugins/plugin_manager.h"
#include "dc/asic/asic.h"

#include <time.h>

VArray2 aica_ram;
u32 VREG;//video reg =P
u32 ARMRST;//arm reset reg
u32 rtc_EN=0;
u32 GetRTC_now()
{
	
	time_t rawtime=0;
	tm  timeinfo;
	timeinfo.tm_year=1998-1900;
	timeinfo.tm_mon=11-1;
	timeinfo.tm_mday=27;
	timeinfo.tm_hour=0;
	timeinfo.tm_min=0;
	timeinfo.tm_sec=0;

	rawtime=mktime( &timeinfo );
	
	rawtime=time (0)-rawtime;//get delta of time since the known dc date
	
	time_t temp=time(0);
	timeinfo=*localtime(&temp);
	if (timeinfo.tm_isdst)
		rawtime+=24*3600;//add an hour if dst (maby rtc has a reg for that ? *watch* and add it if yes :)

	u32 RTC=0x5bfc8900 + (u32)rawtime;// add delta to known dc time
	return RTC;
}
extern s32 rtc_cycles;
u32 ReadMem_aica_rtc(u32 addr,u32 sz)
{
	//settings.dreamcast.RTC=GetRTC_now();
	switch( addr & 0xFF )
	{
	case 0:	
		return settings.dreamcast.RTC>>16;
	case 4:	
		return settings.dreamcast.RTC &0xFFFF;
	case 8:	
		return 0;
	}

	log("ReadMem_aica_rtc : invalid address\n");
	return 0;
}

void WriteMem_aica_rtc(u32 addr,u32 data,u32 sz)
{
	switch( addr & 0xFF )
	{
	case 0:	
		if (rtc_EN)
		{
			settings.dreamcast.RTC&=0xFFFF;
			settings.dreamcast.RTC|=(data&0xFFFF)<<16;
			rtc_EN=0;
			SaveSettings();
		}
		return;
	case 4:	
		if (rtc_EN)
		{
			settings.dreamcast.RTC&=0xFFFF0000;
			settings.dreamcast.RTC|= data&0xFFFF;
			rtc_cycles=200*1000*1000;	//clear the internal cycle counter ;)
		}
		return;
	case 8:	
		rtc_EN=data&1;
		return;
	}

	return;
}
u32 FASTCALL ReadMem_aica_reg(u32 addr,u32 sz)
{
	addr&=0x7FFF;
	if (sz==1)
	{
		if (addr==0x2C01)
		{
			return VREG;
		}
		else if (addr==0x2C00)
		{
			return ARMRST;
		}
		else
			return libAICA.ReadMem_aica_reg(addr,sz);
	}
	else
	{
		if (addr==0x2C00)
		{
			return (VREG<<8) | ARMRST;
		}
		else
			return libAICA.ReadMem_aica_reg(addr,sz);
	}
}

void ArmSetRST()
{
	ARMRST&=1;
	libARM.SetResetState(ARMRST);
}
void FASTCALL WriteMem_aica_reg(u32 addr,u32 data,u32 sz)
{
	addr&=0x7FFF;

	if (sz==1)
	{
		if (addr==0x2C01)
		{
			VREG=data;
			log("VREG = %02X\n",VREG);
		}
		else if (addr==0x2C00)
		{
			ARMRST=data;
			log("ARMRST = %02X\n",ARMRST);
			ArmSetRST();
		}
		else
		{
			libAICA.WriteMem_aica_reg(addr,data,sz);
		}
	}
	else
	{
		if (addr==0x2C00)
		{
			VREG=(data>>8)&0xFF;
			ARMRST=data&0xFF;
			log("VREG = %02X ARMRST %02X\n",VREG,ARMRST);
			ArmSetRST();
		}
		else
		{
			libAICA.WriteMem_aica_reg(addr,data,sz);
		}
	}
}
//Init/res/term
void aica_Init()
{
	//mmnnn ? gota fill it w/ something
}

void aica_Reset(bool Manual)
{
	if (!Manual)
	{
		aica_ram.Zero();
	}
}

void aica_Term()
{

}

s32 aica_pending_dma = 0;

void aica_periodical(u32 cycl)
{
	if (aica_pending_dma > 0)
	{
		verify(SB_ADST==1);

		cycl = (aica_pending_dma <= 0) ? 0 : cycl;
		aica_pending_dma-=cycl;

		if (aica_pending_dma <= 0)
		{
			//log("%u %d\n",cycl,(s32)aica_pending_dma);
			asic_RaiseInterrupt(holly_SPU_DMA);
			aica_pending_dma = 0;
			SB_ADST=0;
		}
	}
}

void Write_SB_ADST(u32 data)
{
	//0x005F7800	SB_ADSTAG	RW	AICA:G2-DMA G2 start address 
	//0x005F7804	SB_ADSTAR	RW	AICA:G2-DMA system memory start address 
	//0x005F7808	SB_ADLEN	RW	AICA:G2-DMA length 
	//0x005F780C	SB_ADDIR	RW	AICA:G2-DMA direction 
	//0x005F7810	SB_ADTSEL	RW	AICA:G2-DMA trigger select 
	//0x005F7814	SB_ADEN	RW	AICA:G2-DMA enable 
	//0x005F7818	SB_ADST	RW	AICA:G2-DMA start 
	//0x005F781C	SB_ADSUSP	RW	AICA:G2-DMA suspend 
	
	u32 total_bytes=0;

	if (data&1)
	{
		if (SB_ADEN&1)
		{
			u32 src=SB_ADSTAR;
			u32 dst=SB_ADSTAG;
			u32 len=SB_ADLEN & 0x7FFFFFFF;

			if ((SB_ADDIR&1)==1)
			{
				//swap direction
				u32 t=src;
				src=dst;
				dst=t;
				log("**AICA DMA : SB_ADDIR==1: Not sure this works, please report if broken/missing sound or crash\n**");
			}


			for (u32 i=0;i<len;i+=4)
			{
				u32 data=ReadMem32_nommu(src+i);
				WriteMem32_nommu(dst+i,data);
			}

			if (SB_ADLEN & 0x80000000)
				SB_ADEN=1;//
			else
				SB_ADEN=0;//

			SB_ADSTAR+=len;
			SB_ADSTAG+=len;
			total_bytes+=len;
			SB_ADLEN = 0x00000000;

			aica_pending_dma=((total_bytes*200000000)/65536)+1;
			SB_ADST=1;
		}
	}
}

void Write_SB_E1ST(u32 data)
{
	//0x005F7800	SB_ADSTAG	RW	AICA:G2-DMA G2 start address 
	//0x005F7804	SB_ADSTAR	RW	AICA:G2-DMA system memory start address 
	//0x005F7808	SB_ADLEN	RW	AICA:G2-DMA length 
	//0x005F780C	SB_ADDIR	RW	AICA:G2-DMA direction 
	//0x005F7810	SB_ADTSEL	RW	AICA:G2-DMA trigger select 
	//0x005F7814	SB_ADEN	RW	AICA:G2-DMA enable 
	//0x005F7818	SB_ADST	RW	AICA:G2-DMA start 
	//0x005F781C	SB_ADSUSP	RW	AICA:G2-DMA suspend 
	
	if (data&1)
	{
		if (SB_E1EN&1)
		{
			u32 src=SB_E1STAR;
			u32 dst=SB_E1STAG;
			u32 len=SB_E1LEN & 0x7FFFFFFF;

			if (SB_E1DIR==1)
			{
				u32 t=src;
				src=dst;
				dst=t;
				log("G2-EXT1 DMA : SB_E1DIR==1 DMA Read to 0x%X from 0x%X %d bytes\n",dst,src,len);
			}
			else
				log("G2-EXT1 DMA : SB_E1DIR==0:DMA Write to 0x%X from 0x%X %d bytes\n",dst,src,len);

			for (u32 i=0;i<len;i+=4)
			{
				u32 data=ReadMem32_nommu(src+i);
				WriteMem32_nommu(dst+i,data);
			}

			if (SB_E1LEN & 0x80000000)
				SB_E1EN=1;//
			else
				SB_E1EN=0;//

			SB_E1STAR+=len;
			SB_E1STAG+=len;
			SB_E1ST = 0x00000000;//dma done
			SB_E1LEN = 0x00000000;

			
			asic_RaiseInterrupt(holly_EXT_DMA1);
		}
	}
}

void aica_sb_Init()
{
	//NRM
	//6
	sb_regs[((SB_ADST_addr-SB_BASE)>>2)].flags=REG_32BIT_READWRITE | REG_READ_DATA;
	sb_regs[((SB_ADST_addr-SB_BASE)>>2)].writeFunction=Write_SB_ADST;

	//I realy need to implement G2 dma (and rest dmas actualy) properly
	//THIS IS NOT AICA, its G2-EXT (BBA)
	sb_regs[((SB_E1ST_addr-SB_BASE)>>2)].flags=REG_32BIT_READWRITE | REG_READ_DATA;
	sb_regs[((SB_E1ST_addr-SB_BASE)>>2)].writeFunction=Write_SB_E1ST;
}

void aica_sb_Reset(bool Manual)
{
}

void aica_sb_Term()
{
}
