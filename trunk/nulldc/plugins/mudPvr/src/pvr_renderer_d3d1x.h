#ifndef _PVR_RENDERER_H_
#define _PVR_RENDERER_H_

#include "mudpvr.h"

bool rend_init();         
void rend_term();
void rend_reset(bool Manual);
bool rend_thread_start();
void rend_thread_end  ();
void rend_vblank      ();
void rend_start_render ();
void rend_end_render   ();
void rend_handle_event (u32 evid,void* p);

void rend_list_cont();
void rend_list_init();
void rend_list_softreset();

void rend_text_invl (vram_block* bl);
void rend_set_render_rect (float* rect,bool  drc);
void rend_set_fb_scale (float x,float y);
void rend_list_modes(void(* callback)(u32 w,u32 h,u32 rr));

#endif