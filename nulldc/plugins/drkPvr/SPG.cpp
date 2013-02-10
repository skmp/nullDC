#include "spg.h"
#include "renderer_if.h"
#include "regs.h"

//SPG emulation; Scanline/Raster beam registers & interrupts
//Time to emulate that stuff correctly ;)

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
//called from sh4 context , should update pvr/ta state and everything else
void FASTCALL spgUpdatePvr(u32 cycles)
{
	if (Line_Cycles==0)
		return;
	clc_pvr_scanline += cycles;

	if (clc_pvr_scanline >  Line_Cycles)//60 ~herz = 200 mhz / 60=3333333.333 cycles per screen refresh
	{
		//ok .. here , after much effort , we did one line
		//now , we must check for raster beam interrupts and vblank
		prv_cur_scanline=(prv_cur_scanline+1)%pvr_numscanlines;
		clc_pvr_scanline -= Line_Cycles;
		//Check for scanline interrupts -- really need to test the scanline values
		
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

		//Vblank start -- really need to test the scanline values
		if (prv_cur_scanline==0)
		{
			//Vblank counter
			vblk_cnt++;
			params.RaiseInterrupt(holly_HBLank);// -> This turned out to be HBlank btw , needs to be emulated ;(
			//TODO : rend_if_VBlank();
			rend_vblank();//notify for vblank :)
			UpdateRRect();
			if ((timeGetTime()-last_fps)>1000)
			{
				double spd_fps=(double)(FrameCount)/(double)((double)(timeGetTime()-(double)last_fps)/1000);
				double spd_vbs=(double)(vblk_cnt)/(double)((double)(timeGetTime()-(double)last_fps)/1000);
				double spd_cpu=spd_vbs*Frame_Cycles;
				spd_cpu/=1000000;	//mrhz kthx
				double fullvbs=(spd_vbs/spd_cpu)*200;
				double mv=VertexCount;
				char mv_c=' ';

				if (mv>750)
				{
					mv/=1000;	//KV
					mv_c='K';
				}
				if (mv>750)
				{
					mv/=1000;	//
					mv_c='M';
				}
				VertexCount=0;
				last_fps=timeGetTime();
				FrameCount=0;
				vblk_cnt=0;

				wchar fpsStr[256];
				wchar* mode=0;
				wchar* res=0;

				res=SPG_CONTROL.interlace?L"480i":L"240p";

				if (SPG_CONTROL.NTSC==0 && SPG_CONTROL.PAL==1)
					mode=L"PAL";
				else if (SPG_CONTROL.NTSC==1 && SPG_CONTROL.PAL==0)
					mode=L"NTSC";
				else
				{
					res=SPG_CONTROL.interlace?L"480i":L"480p";
					mode=L"VGA";
				}

				swprintf(fpsStr,256,L"%s/%c - %4.2f%% - VPS: %4.2f(%s%s%4.2f) RPS: %4.2f Vert: %4.2f%c Sh4: %4.2f mhz", 
					emu_name,'n',spd_cpu*100/200,spd_vbs,
					mode,res,fullvbs,
					spd_fps,mv,mv_c, spd_cpu);

				
				if (GetWindowLong((HWND)emu.GetRenderTarget(),GWL_STYLE)&WS_BORDER)
				{
					SetWindowText((HWND)emu.GetRenderTarget(), fpsStr);
				}
			}
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