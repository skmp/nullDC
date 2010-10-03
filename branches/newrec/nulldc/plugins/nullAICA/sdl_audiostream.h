#pragma once
#include "nullAICA.h"

#ifdef _INCLUDE_SDL
void sdl_InitAudio();
void sdl_TermAudio();
#else
#define sdl_InitAudio ds_InitAudio
#define sdl_TermAudio ds_TermAudio
#endif
