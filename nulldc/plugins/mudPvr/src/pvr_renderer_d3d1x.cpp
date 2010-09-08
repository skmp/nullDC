#include "pvr_renderer_d3d1x.h"
#include "pvr_impl.h"
#include "ta.h"
#include "regs.h"
#include "TexCache.h"

using namespace TASplitter;

bool rend_init()
{
	return true;

}
void rend_term()
{

}
void rend_reset(bool Manual)
{

}
bool rend_thread_start()
{

	return true;

}
void rend_thread_end  ()
{

}
void rend_vblank      ()
{

}
void rend_start_render ()
{

}
void rend_end_render   ()
{

}
void rend_handle_event (u32 evid,void* p)
{
	//gui resized
}

void rend_list_cont()
{
	//tile list continue

}
void rend_list_init()
{
	//tile list init

}
void rend_list_softreset()
{
	//tile list softreset

}

void rend_text_invl (vram_block* bl){

	params.vram_unlock(bl);
}
void rend_set_render_rect (float* rect,bool  drc){}
void rend_set_fb_scale (float x,float y){}
void rend_list_modes(void(* callback)(u32 w,u32 h,u32 rr)){}