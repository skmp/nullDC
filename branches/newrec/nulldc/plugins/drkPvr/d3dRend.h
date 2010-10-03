#pragma once
#include "drkPvr.h"
#include "renderer_if.h"

#if REND_API == REND_D3D

	bool InitRenderer();
	void TermRenderer();
	void ResetRenderer(bool Manual);
	
	void HandleEvent(u32 evid,void* p);
	bool ThreadStart();
	void ThreadEnd();
	void VBlank();
	void StartRender();
	void EndRender();

	void ListCont();
	void ListInit();
	void SoftReset();

	void SetRenderRect(float* rect,bool  drc);
	void SetFBScale(float x,float y);
	void ListModes(void(* callback)(u32 w,u32 h,u32 rr));

	void VramLockedWrite(vram_block* bl);

#define rend_init         InitRenderer
#define rend_term         TermRenderer
#define rend_reset        ResetRenderer

#define rend_thread_start ThreadStart
#define rend_thread_end	  ThreadEnd
#define rend_vblank       VBlank
#define rend_start_render StartRender
#define rend_end_render   EndRender
#define rend_handle_event HandleEvent

#define rend_list_cont ListCont
#define rend_list_init ListInit
#define rend_list_srst SoftReset

#define rend_text_invl VramLockedWrite
#define rend_set_render_rect SetRenderRect
#define rend_set_fb_scale SetFBScale
#define rend_list_modes ListModes
#endif