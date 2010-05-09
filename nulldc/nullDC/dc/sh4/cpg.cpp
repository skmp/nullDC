#include "types.h"
#include "dc/mem/sh4_internal_reg.h"
#include "cpg.h"


u16 CPG_FRQCR;
u8 CPG_STBCR;
u16 CPG_WTCNT;
u16 CPG_WTCSR;
u8 CPG_STBCR2;

//Init term res
void cpg_Init()
{
	//CPG FRQCR H'FFC0 0000 H'1FC0 0000 16 *2 Held Held Held Pclk
	CPG[(CPG_FRQCR_addr&0xFF)>>2].flags=REG_16BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	CPG[(CPG_FRQCR_addr&0xFF)>>2].readFunction=0;
	CPG[(CPG_FRQCR_addr&0xFF)>>2].writeFunction=0;
	CPG[(CPG_FRQCR_addr&0xFF)>>2].data16=&CPG_FRQCR;

	//CPG STBCR H'FFC0 0004 H'1FC0 0004 8 H'00 Held Held Held Pclk
	CPG[(CPG_STBCR_addr&0xFF)>>2].flags=REG_8BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	CPG[(CPG_STBCR_addr&0xFF)>>2].readFunction=0;
	CPG[(CPG_STBCR_addr&0xFF)>>2].writeFunction=0;
	CPG[(CPG_STBCR_addr&0xFF)>>2].data8=&CPG_STBCR;

	//CPG WTCNT H'FFC0 0008 H'1FC0 0008 8/16*3 H'00 Held Held Held Pclk
	CPG[(CPG_WTCNT_addr&0xFF)>>2].flags=REG_16BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	CPG[(CPG_WTCNT_addr&0xFF)>>2].readFunction=0;
	CPG[(CPG_WTCNT_addr&0xFF)>>2].writeFunction=0;
	CPG[(CPG_WTCNT_addr&0xFF)>>2].data16=&CPG_WTCNT;

	//CPG WTCSR H'FFC0 000C H'1FC0 000C 8/16*3 H'00 Held Held Held Pclk
	CPG[(CPG_WTCSR_addr&0xFF)>>2].flags=REG_16BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	CPG[(CPG_WTCSR_addr&0xFF)>>2].readFunction=0;
	CPG[(CPG_WTCSR_addr&0xFF)>>2].writeFunction=0;
	CPG[(CPG_WTCSR_addr&0xFF)>>2].data16=&CPG_WTCSR;

	//CPG STBCR2 H'FFC0 0010 H'1FC0 0010 8 H'00 Held Held Held Pclk
	CPG[(CPG_STBCR2_addr&0xFF)>>2].flags=REG_8BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	CPG[(CPG_STBCR2_addr&0xFF)>>2].readFunction=0;
	CPG[(CPG_STBCR2_addr&0xFF)>>2].writeFunction=0;
	CPG[(CPG_STBCR2_addr&0xFF)>>2].data8=&CPG_STBCR2;


}
void cpg_Reset(bool Manual)
{
	/*
	CPG FRQCR H'FFC0 0000 H'1FC0 0000 16 *2 Held Held Held Pclk
	CPG STBCR H'FFC0 0004 H'1FC0 0004 8 H'00 Held Held Held Pclk
	CPG WTCNT H'FFC0 0008 H'1FC0 0008 8/16*3 H'00 Held Held Held Pclk
	CPG WTCSR H'FFC0 000C H'1FC0 000C 8/16*3 H'00 Held Held Held Pclk
	CPG STBCR2 H'FFC0 0010 H'1FC0 0010 8 H'00 Held Held Held Pclk
	*/
	CPG_FRQCR = 0;
	CPG_STBCR = 0;
	CPG_WTCNT = 0;
	CPG_WTCSR = 0;
	CPG_STBCR2 = 0;
	
}
void cpg_Term()
{
}