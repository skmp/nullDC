#include "regs.h"
#include "renderer_if.h"
#include "ta.h"
#include "spg.h"


//seems to work allright .. ;P
//many things need to be emulated , especialy to support lle emulation but for now that's not needed
u8 regs[RegSize];

u32 FASTCALL ReadPvrRegister(u32 addr,u32 size)
{
	if (size!=4)
	{
		//error
		return 0;
	}

	return PvrReg(addr,u32);
}

void PrintfInfo();
void FASTCALL WritePvrRegister(u32 paddr,u32 data,u32 size)
{
	if (size!=4)
	{
		//error
		return;
	}
	u32 addr=paddr&RegMask;

	if (addr==ID_addr)
		return;//read olny
	if (addr==REVISION_addr)
		return;//read olny

	if (addr==STARTRENDER_addr)
	{
		//start render
		rend_start_render();
		//PrintfInfo();
		//TODO : fix that mess -- now uses hacksync ;) later will be async too :P:P
		
		//RaiseInterrupt(InterruptID::holly_RENDER_DONE);
		//RaiseInterrupt(InterruptID::holly_RENDER_DONE_isp);
		//RaiseInterrupt(InterruptID::holly_RENDER_DONE_vd);
		render_end_pending=true;
		return;
	}

	if (addr==TA_LIST_INIT_addr)
	{
		if (data>>31)
		{
			rend_list_init();
			data=0;
		}
	}

	if (addr==SOFTRESET_addr)
	{
		if (data!=0)
		{
			if (data&1)
				rend_list_srst();
			data=0;
		}
	}

	if (addr==TA_LIST_CONT_addr)
	{
		//a write of anything works ?
		rend_list_cont();
	}
	
	if (addr == FB_R_CTRL_addr || 
		addr == SPG_CONTROL_addr || 
		addr == SPG_LOAD_addr)
	{
		PvrReg(addr,u32)=data;
		CalculateSync();
		return;
	}

	if (addr>=PALETTE_RAM_START_addr)
	{
		if (PvrReg(addr,u32)!=data)
		{
			u32 pal=addr&1023;

			pal_needs_update=true;
			_pal_rev_256[pal>>8]++;
			_pal_rev_16[pal>>4]++;
		}
	}
	PvrReg(addr,u32)=data;
}

bool Regs_Init()
{
	return true;
}

void Regs_Term()
{
}

void Regs_Reset(bool Manual)
{
	ID					= 0x17FD11DB;
	REVISION			= 0x00000011;
	SOFTRESET			= 0x00000007;
	SPG_HBLANK_INT.full	= 0x031D0000;
	SPG_VBLANK_INT.full	= 0x01500104;
	FPU_PARAM_CFG		= 0x0007DF77;
	HALF_OFFSET			= 0x00000007;
	ISP_FEED_CFG		= 0x00402000;
	SDRAM_REFRESH		= 0x00000020;
	SDRAM_ARB_CFG		= 0x0000001F;
	SDRAM_CFG			= 0x15F28997;
	SPG_HBLANK.full		= 0x007E0345;
	SPG_LOAD.full		= 0x01060359;
	SPG_VBLANK.full		= 0x01500104;
	SPG_WIDTH.full		= 0x07F1933F;
	VO_CONTROL.full		= 0x00000108;
	VO_STARTX.full		= 0x0000009D;
	VO_STARTY.full		= 0x00000015;
	SCALER_CTL.full		= 0x00000400;
	FB_BURSTCTRL		= 0x00090639;
	PT_ALPHA_REF		= 0x000000FF;
}