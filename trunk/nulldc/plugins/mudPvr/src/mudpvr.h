
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE

//bleh stupid windoze header
#include "..\..\..\nullDC\plugins\plugin_header.h"
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

extern bool render_end_pending;
extern u32 render_end_pending_cycles;
extern pvr_init_params params;

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