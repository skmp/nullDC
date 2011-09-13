#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE
#include "config.h"



//bleh stupid windoze header
#include "..\..\nullDC\plugins\plugin_header.h"
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>

#include <string.h>

int msgboxf(wchar* text,unsigned int type,...);

#define BUILD 0
#define MINOR 1
#define MAJOR 0
#define DCclock (200*1000*1000)


float GetSeconds();


#define dbgbreak __debugbreak()

#define fastcall __fastcall

#if DO_VERIFY==OP_ON
#define verify(x) if((x)==false){ static bool d_msg=true; if (d_msg) { d_msg = msgboxf(L"Verify Failed  : " _T(#x) L"\n in %s -> %s : %d \nWant to report this error again ?",MB_ICONERROR|MB_YESNO,_T(__FUNCTION__),_T(__FILE__),__LINE__)==IDYES?true:false;} if (d_msg){ dbgbreak;}}
#define verifyf(x) if((x)==false){ msgboxf(L"Verify Failed  : " _T(#x) L"\n in %s -> %s : %d \n",MB_ICONERROR,_T(__FUNCTION__),_T(__FILE__),__LINE__); dbgbreak;}
#define verifyc(x) {HRESULT hrr=(x); if(FAILED(hrr)){ msgboxf(L"Verify Failed  : " _T(#x) L"\n in %s -> %s : %d HR=0x%08X\n",MB_ICONERROR,_T(__FUNCTION__),_T(__FILE__),__LINE__,hrr); dbgbreak;} }
#else
#define verify(x)
#define verifyf(x) (x)
#define verifyc(x) (x)
#endif

#define die(reason) { printf("Fatal error : " #reason "\n in %s -> %s : %d \n",__FUNCTION__,__FILE__,__LINE__); dbgbreak;}
#define fverify verify

#define log0(xx) printf(xx " (from "__FUNCTION__ ")\n");
#define log1(xx,yy) printf(xx " (from "__FUNCTION__ ")\n",yy);
#define log2(xx,yy,zz) printf(xx " (from "__FUNCTION__ ")\n",yy,zz);
#define log3(xx,yy,gg) printf(xx " (from "__FUNCTION__ ")\n",yy,zz,gg);

#include "helper_classes.h"

extern bool render_end_pending;
extern u32 render_end_pending_cycles;

extern pvr_init_params params;
extern emu_info emu;
extern wchar emu_name[512];

void LoadSettings();
void SaveSettings();

#if REND_API == REND_D3D
	#define REND_NAME L"Direct3D HAL"
	#define GetRenderer GetDirect3DRenderer
#elif REND_API == REND_OGL
	#define REND_NAME L"OpenGL SM4+ HAL"
	#define GetRenderer GetOpenGLRenderer
#elif  REND_API == REND_SW
	#define REND_NAME L"Software SBR"
	#define GetRenderer GetSWRenderer
#elif REND_API == REND_D3D11
	#define REND_NAME L"Direct3D11 HAL"
	#define GetRenderer GetDirect3DRenderer
#elif  REND_API == REND_NONE
	#define REND_NAME L"No Rendering"
	#define GetRenderer GetNORenderer
#else
	#error invalid config (REND_API)
#endif

struct _settings_type
{
	struct
	{
		u32 ResolutionMode;
		u32 VSync;
	} Video;

	struct 
	{
		u32 MultiSampleCount;
		u32 MultiSampleQuality;
		u32 AspectRatioMode;
	} Enhancements;
	
	struct
	{
		u32 PaletteMode;
		u32 AlphaSortMode;
		u32 ModVolMode;
		u32 ZBufferMode;
		u32 TexCacheMode;
	} Emulation;

	struct
	{
		u32 ShowFPS;
		u32 ShowStats;
	} OSD;
};
enum ModVolMode
{
	MVM_NormalAndClip,
	MVM_Normal,
	MVM_Off,
	MVM_Volume,
};
extern _settings_type settings;
void UpdateRRect();