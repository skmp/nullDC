#pragma once
#include "drkPvr.h"
#include "renderer_if.h"

#if REND_API == REND_OGL
namespace OpenGLRenderer
{

	bool InitRenderer();
	void TermRenderer();
	void ResetRenderer(bool Manual);
	
	bool ThreadStart();
	void ThreadEnd();
	void VBlank();
	void StartRender();
	void EndRender();

	void ListCont();
	void ListInit();
	void SoftReset();


	void VramLockedWrite(vram_block* bl);
	void SetFpsText(wchar* text);
};
#define rend_init         OpenGLRenderer::InitRenderer
#define rend_term         OpenGLRenderer::TermRenderer
#define rend_reset        OpenGLRenderer::ResetRenderer

#define rend_thread_start OpenGLRenderer::ThreadStart
#define rend_thread_end	  OpenGLRenderer::ThreadEnd
#define rend_vblank       OpenGLRenderer::VBlank
#define rend_start_render OpenGLRenderer::StartRender
#define rend_end_render   OpenGLRenderer::EndRender

#define rend_list_cont OpenGLRenderer::ListCont
#define rend_list_init OpenGLRenderer::ListInit
#define rend_list_srst OpenGLRenderer::SoftReset

#define rend_text_invl OpenGLRenderer::VramLockedWrite

#define rend_set_fps_text OpenGLRenderer::SetFpsText
#define rend_set_render_rect()
#define rend_set_fb_scale()
#define rend_list_modes()
#define rend_handle_event()
#endif