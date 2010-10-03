/*
	The rtc isn't working on dreamcast i'm told
*/
#include "types.h"
#include "dc/mem/sh4_internal_reg.h"
#include "rtc.h"




u8 RTC_R64CNT=0;
u8 RTC_RSECCNT;
u8 RTC_RMINCNT;
u8 RTC_RHRCNT;
u8 RTC_RWKCNT;
u8 RTC_RDAYCNT;
u8 RTC_RMONCNT;
u16 RTC_RYRCNT;
u8 RTC_RSECAR;
u8 RTC_RMINAR;
u8 RTC_RHRAR;
u8 RTC_RWKAR;
u8 RTC_RDAYAR;
u8 RTC_RMONAR;
u8 RTC_RCR1;
u8 RTC_RCR2;

//Init term res
void rtc_Init()
{
	// NAOMI reads from at least RTC_R64CNT

	//RTC R64CNT 0xFFC80000 0x1FC80000 8 Held Held Held Held Pclk
	RTC[(RTC_R64CNT_addr&0xFF)>>2].flags=REG_8BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	RTC[(RTC_R64CNT_addr&0xFF)>>2].readFunction=0;
	RTC[(RTC_R64CNT_addr&0xFF)>>2].writeFunction=0;
	RTC[(RTC_R64CNT_addr&0xFF)>>2].data8=&RTC_R64CNT;

	//RTC RSECCNT H'FFC8 0004 H'1FC8 0004 8 Held Held Held Held Pclk
	RTC[(RTC_RSECCNT_addr&0xFF)>>2].flags=REG_8BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	RTC[(RTC_RSECCNT_addr&0xFF)>>2].readFunction=0;
	RTC[(RTC_RSECCNT_addr&0xFF)>>2].writeFunction=0;
	RTC[(RTC_RSECCNT_addr&0xFF)>>2].data8=&RTC_RSECCNT;

	//RTC RMINCNT H'FFC8 0008 H'1FC8 0008 8 Held Held Held Held Pclk
	RTC[(RTC_RMINCNT_addr&0xFF)>>2].flags=REG_8BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	RTC[(RTC_RMINCNT_addr&0xFF)>>2].readFunction=0;
	RTC[(RTC_RMINCNT_addr&0xFF)>>2].writeFunction=0;
	RTC[(RTC_RMINCNT_addr&0xFF)>>2].data8=&RTC_RMINCNT;

	//RTC RHRCNT H'FFC8 000C H'1FC8 000C 8 Held Held Held Held Pclk
	RTC[(RTC_RHRCNT_addr&0xFF)>>2].flags=REG_8BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	RTC[(RTC_RHRCNT_addr&0xFF)>>2].readFunction=0;
	RTC[(RTC_RHRCNT_addr&0xFF)>>2].writeFunction=0;
	RTC[(RTC_RHRCNT_addr&0xFF)>>2].data8=&RTC_RHRCNT;

	//RTC RWKCNT H'FFC8 0010 H'1FC8 0010 8 Held Held Held Held Pclk
	RTC[(RTC_RWKCNT_addr&0xFF)>>2].flags=REG_8BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	RTC[(RTC_RWKCNT_addr&0xFF)>>2].readFunction=0;
	RTC[(RTC_RWKCNT_addr&0xFF)>>2].writeFunction=0;
	RTC[(RTC_RWKCNT_addr&0xFF)>>2].data8=&RTC_RWKCNT;

	//RTC RDAYCNT H'FFC8 0014 H'1FC8 0014 8 Held Held Held Held Pclk
	RTC[(RTC_RDAYCNT_addr&0xFF)>>2].flags=REG_8BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	RTC[(RTC_RDAYCNT_addr&0xFF)>>2].readFunction=0;
	RTC[(RTC_RDAYCNT_addr&0xFF)>>2].writeFunction=0;
	RTC[(RTC_RDAYCNT_addr&0xFF)>>2].data8=&RTC_RDAYCNT;

	//RTC RMONCNT H'FFC8 0018 H'1FC8 0018 8 Held Held Held Held Pclk
	RTC[(RTC_RMONCNT_addr&0xFF)>>2].flags=REG_8BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	RTC[(RTC_RMONCNT_addr&0xFF)>>2].readFunction=0;
	RTC[(RTC_RMONCNT_addr&0xFF)>>2].writeFunction=0;
	RTC[(RTC_RMONCNT_addr&0xFF)>>2].data8=&RTC_RMONCNT;

	//RTC RYRCNT H'FFC8 001C H'1FC8 001C 16 Held Held Held Held Pclk
	RTC[(RTC_RYRCNT_addr&0xFF)>>2].flags=REG_16BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	RTC[(RTC_RYRCNT_addr&0xFF)>>2].readFunction=0;
	RTC[(RTC_RYRCNT_addr&0xFF)>>2].writeFunction=0;
	RTC[(RTC_RYRCNT_addr&0xFF)>>2].data16=&RTC_RYRCNT;

	//RTC RSECAR H'FFC8 0020 H'1FC8 0020 8 Held *2 Held Held Held Pclk
	RTC[(RTC_RSECAR_addr&0xFF)>>2].flags=REG_8BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	RTC[(RTC_RSECAR_addr&0xFF)>>2].readFunction=0;
	RTC[(RTC_RSECAR_addr&0xFF)>>2].writeFunction=0;
	RTC[(RTC_RSECAR_addr&0xFF)>>2].data8=&RTC_RSECAR;

	//RTC RMINAR H'FFC8 0024 H'1FC8 0024 8 Held *2 Held Held Held Pclk
	RTC[(RTC_RMINAR_addr&0xFF)>>2].flags=REG_8BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	RTC[(RTC_RMINAR_addr&0xFF)>>2].readFunction=0;
	RTC[(RTC_RMINAR_addr&0xFF)>>2].writeFunction=0;
	RTC[(RTC_RMINAR_addr&0xFF)>>2].data8=&RTC_RMINAR;

	//RTC RHRAR H'FFC8 0028 H'1FC8 0028 8 Held *2 Held Held Held Pclk
	RTC[(RTC_RHRAR_addr&0xFF)>>2].flags=REG_8BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	RTC[(RTC_RHRAR_addr&0xFF)>>2].readFunction=0;
	RTC[(RTC_RHRAR_addr&0xFF)>>2].writeFunction=0;
	RTC[(RTC_RHRAR_addr&0xFF)>>2].data8=&RTC_RHRAR;

	//RTC RWKAR H'FFC8 002C H'1FC8 002C 8 Held *2 Held Held Held Pclk
	RTC[(RTC_RWKAR_addr&0xFF)>>2].flags=REG_8BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	RTC[(RTC_RWKAR_addr&0xFF)>>2].readFunction=0;
	RTC[(RTC_RWKAR_addr&0xFF)>>2].writeFunction=0;
	RTC[(RTC_RWKAR_addr&0xFF)>>2].data8=&RTC_RWKAR;

	//RTC RDAYAR H'FFC8 0030 H'1FC8 0030 8 Held *2 Held Held Held Pclk
	RTC[(RTC_R64CNT_addr&0xFF)>>2].flags=REG_8BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	RTC[(RTC_RDAYAR_addr&0xFF)>>2].readFunction=0;
	RTC[(RTC_RDAYAR_addr&0xFF)>>2].writeFunction=0;
	RTC[(RTC_RDAYAR_addr&0xFF)>>2].data8=&RTC_RDAYAR;

	//RTC RMONAR H'FFC8 0034 H'1FC8 0034 8 Held *2 Held Held Held Pclk
	RTC[(RTC_RMONAR_addr&0xFF)>>2].flags=REG_8BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	RTC[(RTC_RMONAR_addr&0xFF)>>2].readFunction=0;
	RTC[(RTC_RMONAR_addr&0xFF)>>2].writeFunction=0;
	RTC[(RTC_RMONAR_addr&0xFF)>>2].data8=&RTC_RMONAR;

	//RTC RCR1 H'FFC8 0038 H'1FC8 0038 8 H'00*2 H'00*2 Held Held Pclk
	RTC[(RTC_RCR1_addr&0xFF)>>2].flags=REG_8BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	RTC[(RTC_RCR1_addr&0xFF)>>2].readFunction=0;
	RTC[(RTC_RCR1_addr&0xFF)>>2].writeFunction=0;
	RTC[(RTC_RCR1_addr&0xFF)>>2].data8=&RTC_RCR1;

	//RTC RCR2 H'FFC8 003C H'1FC8 003C 8 H'09*2 H'00*2 Held Held Pclk
	RTC[(RTC_RCR2_addr&0xFF)>>2].flags=REG_8BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	RTC[(RTC_RCR2_addr&0xFF)>>2].readFunction=0;
	RTC[(RTC_RCR2_addr&0xFF)>>2].writeFunction=0;
	RTC[(RTC_RCR2_addr&0xFF)>>2].data8=&RTC_RCR2;

}

void rtc_Reset(bool Manual)
{
	/*
	RTC R64CNT H'FFC8 0000 H'1FC8 0000 8 Held Held Held Held Pclk
	RTC RSECCNT H'FFC8 0004 H'1FC8 0004 8 Held Held Held Held Pclk
	RTC RMINCNT H'FFC8 0008 H'1FC8 0008 8 Held Held Held Held Pclk
	RTC RHRCNT H'FFC8 000C H'1FC8 000C 8 Held Held Held Held Pclk
	RTC RWKCNT H'FFC8 0010 H'1FC8 0010 8 Held Held Held Held Pclk
	RTC RDAYCNT H'FFC8 0014 H'1FC8 0014 8 Held Held Held Held Pclk
	RTC RMONCNT H'FFC8 0018 H'1FC8 0018 8 Held Held Held Held Pclk
	RTC RYRCNT H'FFC8 001C H'1FC8 001C 16 Held Held Held Held Pclk
	RTC RSECAR H'FFC8 0020 H'1FC8 0020 8 Held *2 Held Held Held Pclk
	RTC RMINAR H'FFC8 0024 H'1FC8 0024 8 Held *2 Held Held Held Pclk
	RTC RHRAR H'FFC8 0028 H'1FC8 0028 8 Held *2 Held Held Held Pclk
	RTC RWKAR H'FFC8 002C H'1FC8 002C 8 Held *2 Held Held Held Pclk
	RTC RDAYAR H'FFC8 0030 H'1FC8 0030 8 Held *2 Held Held Held Pclk
	RTC RMONAR H'FFC8 0034 H'1FC8 0034 8 Held *2 Held Held Held Pclk
	RTC RCR1 H'FFC8 0038 H'1FC8 0038 8 H'00*2 H'00*2 Held Held Pclk
	RTC RCR2 H'FFC8 003C H'1FC8 003C 8 H'09*2 H'00*2 Held Held Pclk
	*/
	RTC_RCR1=0x00;
	RTC_RCR2=0x09;
}
void rtc_Term()
{
}