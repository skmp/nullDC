/*
	PVR-SB handling
	DMA hacks are here
*/

#include "types.h"
#include "dc/asic/asic.h"
#include "dc/sh4/intc.h"
#include "dc/mem/sb.h"
#include "dc/sh4/dmac.h"
#include "dc/mem/sh4_mem.h"
#include "pvr_sb_regs.h"
#include "plugins/plugin_manager.h"

u32 pvr_sb_readreg_Pvr(u32 addr,u32 sz);
void pvr_sb_writereg_Pvr(u32 addr,u32 data,u32 sz);

void RegWrite_SB_C2DST(u32 data)
{
	if(1&data)
	{
		SB_C2DST=1;
		DMAC_Ch2St();
	}
}
//PVR-DMA
void do_pvr_dma()
{
	//u32 chcr	= DMAC_CHCR[0].full,
	u32 dmaor	= DMAC_DMAOR.full;
	//dmatcr	= DMAC_DMATCR[0];

	u32	src		= SB_PDSTAR,
		dst		= SB_PDSTAP,
		len		= SB_PDLEN ;

	if(0x8201 != (dmaor &DMAOR_MASK)) {
		log("\n!\tDMAC: DMAOR has invalid settings (%X) !\n", dmaor);
		return;
	}
	if( len & 0x1F ) {
		log("\n!\tDMAC: SB_C2DLEN has invalid size (%X) !\n", len);
		return;
	}

	if (SB_PDDIR)
	{
		//PVR -> System
		for (u32 i=0;i<len;i+=4)
		{
			u32 temp=ReadMem32_nommu(dst+i);
			WriteMem32_nommu(src+i,temp);
		}
	}
	else
	{
		//System -> PVR
		//TODO : FIX THAT , to warp around on dmas :)
		WriteMemBlock_nommu_ptr(dst,(u32*)GetMemPtr(src,len),len);
	}

	DMAC_SAR[0] = (src + len);
	DMAC_CHCR[0].full &= 0xFFFFFFFE;
	DMAC_DMATCR[0] = 0x00000000;

	SB_PDST = 0x00000000;

	//TODO : *CHECKME* is that ok here ? the docs don't say here it's used [PVR-DMA , bit 11]
	asic_RaiseInterrupt(holly_PVR_DMA);
}
void RegWrite_SB_PDST(u32 data)
{
	if(1&data)
	{
		SB_PDST=1;
		do_pvr_dma();
	}
}
u32 calculate_start_link_addr()
{
	u8* base=&mem_b[SB_SDSTAW & RAM_MASK];
	u32 rv;
	if (SB_SDWLT==0)
	{
		//16b width
		rv=((u16*)base)[SB_SDDIV];
	}
	else
	{
		//32b width
		rv=((u32*)base)[SB_SDDIV];
	}
	SB_SDDIV++; //next index

	return rv;
}
void pvr_do_sort_dma()
{

	SB_SDDIV=0;//index is 0 now :)
	u32 link_addr=calculate_start_link_addr();
	u32 link_base_addr = SB_SDBAAW;

	while (link_addr!=1)
	{
		if (SB_SDLAS==1)
			link_addr*=32;

		u32 ea=(link_base_addr+link_addr) & RAM_MASK;
		u32* ea_ptr=(u32*)&mem_b[ea];

		link_addr=ea_ptr[0x1C>>2];//Next link
		//transfer global param
		libPvr.TaDMA(ea_ptr,ea_ptr[0x18>>2]);
		if (link_addr==2)
		{
			link_addr=calculate_start_link_addr();
		}
	}

	//end of dma :)
	SB_SDST=0;
	asic_RaiseInterrupt(holly_PVR_SortDMA);
}
//auto sort dma :|
void RegWrite_SB_SDST(u32 data)
{
	if(1&data)
	{
		pvr_do_sort_dma();
	}
}


//Init/Term , global
void pvr_sb_Init()
{
	//0x005F7C18	SB_PDST	RW	PVR-DMA start
	sb_regs[((SB_PDST_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA;
	sb_regs[((SB_PDST_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_PDST_addr-SB_BASE))>>2].writeFunction=RegWrite_SB_PDST;
	sb_regs[((SB_PDST_addr-SB_BASE))>>2].data32=&SB_PDST;

	//0x005F6808	SB_C2DST	RW	ch2-DMA start 
	sb_regs[((SB_C2DST_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA;
	sb_regs[((SB_C2DST_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_C2DST_addr-SB_BASE))>>2].writeFunction=RegWrite_SB_C2DST;
	sb_regs[((SB_C2DST_addr-SB_BASE))>>2].data32=&SB_C2DST;


	//0x005F6820	SB_SDST	RW	Sort-DMA start
	sb_regs[((SB_SDST_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA;
	sb_regs[((SB_SDST_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_SDST_addr-SB_BASE))>>2].writeFunction=RegWrite_SB_SDST;
	sb_regs[((SB_SDST_addr-SB_BASE))>>2].data32=&SB_SDST;
}
void pvr_sb_Term()
{
}
//Reset -> Reset - Initialise
void pvr_sb_Reset(bool Manual)
{
}