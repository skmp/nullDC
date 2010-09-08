#include "pvr_impl.h"
#include "ta.h"
#include "regs.h"
#include "TexCache.h"

u8 regs[RegSize];
//------------------------------------
//SPG
//------------------------------------
u32 in_vblank=0;
u32 clc_pvr_scanline;
u32 pvr_numscanlines=512;
u32 prv_cur_scanline=-1;
u32 vblk_cnt=0;
u32 last_fps=0;
//54 mhz pixel clock :)
#define PIXEL_CLOCK (54*1000*1000/2)
u32 Line_Cycles=0;
u32 Frame_Cycles=0;
void CalculateSync()
{
	//clc_pvr_scanline=0;

	u32 pixel_clock;
	float scale_x=1,scale_y=1;

	if (FB_R_CTRL.vclk_div)
	{
		//VGA :)
		pixel_clock=PIXEL_CLOCK;
	}
	else
	{
		//It is half for NTSC/PAL
		pixel_clock=PIXEL_CLOCK/2;
	}
	//We need to caclulate the pixel clock
	u32 sync_cycles=(SPG_LOAD.hcount+1)*(SPG_LOAD.vcount+1);
	pvr_numscanlines=SPG_LOAD.vcount+1;
	Line_Cycles=(u32)((u64)DCclock*(u64)(SPG_LOAD.hcount+1)/(u64)pixel_clock);
	if (SPG_CONTROL.interlace)
	{
		//this is a temp hack
		Line_Cycles/=2;
		u32 interl_mode=VO_CONTROL.field_mode;
		//if (interl_mode==2)//3 will be funny =P
		//	scale_y=0.5f;//single interlace
		//else
		scale_y=1;
	}
	else
	{
		if (FB_R_CTRL.vclk_div)
		{
			scale_y=1.0f;//non interlaced vga mode has full resolution :)
		}
		else
			scale_y=0.5f;//non interlaced modes have half resolution
	}

	rend_set_fb_scale(scale_x,scale_y);

	//Frame_Cycles=(u64)DCclock*(u64)sync_cycles/(u64)pixel_clock;

	Frame_Cycles=pvr_numscanlines*Line_Cycles;
}

bool render_end_pending=false;
u32 render_end_pending_cycles;
//called from sh4 context , should update pvr/ta state and evereything else
void FASTCALL spgUpdatePvr(u32 cycles)
{
	if (Line_Cycles==0)
		return;
	clc_pvr_scanline += cycles;

	if (clc_pvr_scanline >  Line_Cycles)//60 ~herz = 200 mhz / 60=3333333.333 cycles per screen refresh
	{
		//ok .. here , after much effort , we did one line
		//now , we must chekc for raster beam interupts and vblank
		prv_cur_scanline=(prv_cur_scanline+1)%pvr_numscanlines;
		clc_pvr_scanline -= Line_Cycles;
		//Check for scanline interrupts -- realy need to test the scanline values

		if (SPG_VBLANK_INT.vblank_in_interrupt_line_number == prv_cur_scanline)
			params.RaiseInterrupt(holly_SCANINT1);

		if (SPG_VBLANK_INT.vblank_out_interrupt_line_number == prv_cur_scanline)
			params.RaiseInterrupt(holly_SCANINT2);

		if (SPG_VBLANK.vstart == prv_cur_scanline)
			in_vblank=1;

		if (SPG_VBLANK.vbend == prv_cur_scanline)
			in_vblank=0;

		if (SPG_CONTROL.interlace)
			SPG_STATUS.fieldnum=~SPG_STATUS.fieldnum;
		else
			SPG_STATUS.fieldnum=0;

		SPG_STATUS.vsync=in_vblank;
		SPG_STATUS.scanline=prv_cur_scanline;

		//Vblank start -- realy need to test the scanline values
		if (prv_cur_scanline==0)
		{
			//Vblank counter
			vblk_cnt++;
			params.RaiseInterrupt(holly_HBLank);// -> This turned out to be HBlank btw , needs to be emulater ;(
			//TODO : rend_if_VBlank();
			rend_vblank();//notify for vblank :)
		}
	}

	if (render_end_pending)
	{
		if (render_end_pending_cycles<cycles)
		{
			render_end_pending=false;
			params.RaiseInterrupt(holly_RENDER_DONE);
			params.RaiseInterrupt(holly_RENDER_DONE_isp);
			params.RaiseInterrupt(holly_RENDER_DONE_vd);
			rend_end_render();
		}
		render_end_pending_cycles-=cycles;
	}
}


bool spg_Init()
{
	return true;
}

void spg_Term()
{
}

void spg_Reset(bool Manual)
{
	CalculateSync();
}


//-------------------------------
//Registers
////////////////////////////////
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
				rend_list_softreset();
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