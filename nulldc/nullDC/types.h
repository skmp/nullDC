#pragma once

#define NO_MMU

//Basic types & stuff
#include "plugins\plugin_header.h"

//SHUT UP M$ COMPILER !@#!@$#
#ifdef _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES
#undef _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES
#endif

#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1

#ifdef _CRT_SECURE_NO_DEPRECATE
#undef _CRT_SECURE_NO_DEPRECATE
#endif

#define _CRT_SECURE_NO_DEPRECATE 

//Basic types :)
//#include "basic_types.h"
#include <vector>
#include <string>
using namespace std;

#define No_gdb_stub
//#define DEBUG_DLL

//Do not complain when i use enum::member
//#pragma warning( disable : 4482)

//unnamed struncts/unions
#pragma warning( disable : 4201)

//unused parameters
#pragma warning( disable : 4100)

//basic includes from runtime lib
#include <stdlib.h>
#include <stdio.h>
#include <wchar.h>
#include <tchar.h>

//used for asm-olny functions
#ifdef X86
#define naked   __declspec( naked )
#else
#define naked
#endif

//pageup = fast forward
#define DEV_TOOL_FAST_FW_KEY (VK_PRIOR)

#undef INCLUDE_DEV_TOOLS
#define NO_VERIFY
#undef SB_MAP_UNKNOWN_REGS

//On release we have no checks
#ifndef RELEASE
	#define MEM_ALLOC_CHECK
	#define MEM_BOUND_CHECK
	#define MEM_ERROR_BREAK
#endif

#ifdef MEM_ALLOC_CHECK

	extern  void * debug_malloc(size_t size);
	extern  void * debug_realloc(void* ptr,size_t size);
	extern  void debug_free(void* ptr);

	#define malloc debug_malloc
	#define realloc debug_realloc
	#define free debug_free

	//enable bound checks if on MEM_ALLOC_CHECK mode
	#ifndef MEM_BOUNDCHECK
	#define MEM_BOUNDCHECK
	#endif

#endif

#if DEBUG
//force
#define INLINE
//sugest
#define SINLINE
#else
//force
#define INLINE __forceinline
//sugest
#define SINLINE __inline
#endif
//no inline :)
#define NOINLINE __declspec(noinline)

#ifdef MEM_ERROR_BREAK
	#ifdef X86
		#define MEM_DO_BREAK {__debugbreak(); }
	#else
		#define MEM_DO_BREAK {log("**Mem Error Break**\n");getc(stdin);}
	#endif
#else
	#define MEM_DO_BREAK
#endif

#ifdef TRACE
	#ifdef DEBUG
		#ifdef X86
			#define TRACE_DO_BREAK {dbgbreak;}
		#else
			#define TRACE_DO_BREAK {log("**Trace Break**\n");getc(stdin);}
		#endif
	#else
		#define TRACE_DO_BREAK
	#endif
#else
	#define TRACE_DO_BREAK
#endif

//basic includes
#include "stdclass.h"

#define EMUERROR(x)(wprintf(_T("Error in %s:") _T(__FUNCTION__) _T(":%d  -> ") _T(x)  _T("\n"),GetNullDCSoruceFileName(_T(__FILE__)),__LINE__))
#define EMUERROR2(x,a)(wprintf(_T("Error in %s:") _T(__FUNCTION__) _T(":%d  -> ") _T(x)  _T("\n)"),GetNullDCSoruceFileName(_T(__FILE__)),__LINE__,a))
#define EMUERROR3(x,a,b)(wprintf(_T("Error in %s:") _T(__FUNCTION__) _T(":%d  -> ") _T(x)  _T("\n)"),GetNullDCSoruceFileName(_T(__FILE__)),__LINE__,a,b))
#define EMUERROR4(x,a,b,c)(wprintf(_T("Error in %s:") _T(__FUNCTION__) _T(":%d  -> ") _T(x)  _T("\n"),GetNullDCSoruceFileName(_T(__FILE__)),__LINE__,a,b,c))

#define EMUWARN(x)(wprintf( _T("Warning in %s:") _T(__FUNCTION__) _T(":%d  -> ") _T(x)  _T("\n"),GetNullDCSoruceFileName(_T(__FILE__)),_T(__LINE__)))
#define EMUWARN2(x,a)(wprintf( _T("Warning in %s:") _T(__FUNCTION__) _T(":%d  -> ") _T(x) _T("\n"),GetNullDCSoruceFileName(_T(__FILE__)),_T(__LINE__),a))
#define EMUWARN3(x,a,b)(wprintf( _T("Warning in %s:")  _T(__FUNCTION__) _T(":%d  -> ") _T(x) _T("\n"),GetNullDCSoruceFileName(_T(__FILE__)),_T(__LINE__),a,b))
#define EMUWARN4(x,a,b,c)(wprintf( _T("Warning in %s:") _T(__FUNCTION__) _T(":%d  -> ") _T(x) _T("\n"),GetNullDCSoruceFileName(_T(__FILE__)),_T(__LINE__),a,b,c))


#ifndef NO_MMU
#define _X_x_X_MMU_VER_STR L"/mmu"
#else
#define _X_x_X_MMU_VER_STR L""
#endif


#if DC_PLATFORM==DC_PLATFORM_NORMAL
	#define VER_EMUNAME		L"nullDC"
#elif DC_PLATFORM==DC_PLATFORM_DEV_UNIT
	#define VER_EMUNAME		L"nullDC-DevKit-SET5.21"
#elif DC_PLATFORM==DC_PLATFORM_NAOMI
	#define VER_EMUNAME		L"nullDC-Naomi"
#elif DC_PLATFORM==DC_PLATFORM_ATOMISWAVE
	#define VER_EMUNAME		L"nullDC-AtomisWave"
#else
	#error unknown target platform
#endif


#define VER_FULLNAME	VER_EMUNAME L" v1.0.4" _X_x_X_MMU_VER_STR L" (built " _T(__DATE__) L"@" _T(__TIME__) L")"
#define VER_SHORTNAME	VER_EMUNAME L" 1.0.4" _X_x_X_MMU_VER_STR

#define dbgbreak __debugbreak(); 

#define fastcall __fastcall
#define FASTCALL __fastcall

#ifndef NO_VERIFY
#define verify(x) if(!(x)){ msgboxf(_T("Verify Failed  : ") _T(#x) _T("\n in %s -> %s : %d \n"),MBX_ICONERROR,_T(__FUNCTION__),_T(__FILE__),__LINE__); dbgbreak;}
#else
	#define verify(x) (x);
#endif

#define die(reason) { msgboxf(_T("Fatal error : %s\n in %s -> %s : %d \n"),MBX_ICONERROR,_T(reason),_T(__FUNCTION__),_T(__FILE__),__LINE__); dbgbreak;}
#define fverify verify

//will be removed sometime soon
//This shit needs to be moved to proper headers
typedef u32  RegReadFP();
typedef void RegWriteFP(u32 data);
typedef u32  RegChangeFP();

enum RegStructFlags
{
	//Basic :
	REG_8BIT_READWRITE=1,	//used when doing direct reads from data for checks [not  on pvr , size is allways 32bits]
	REG_16BIT_READWRITE=2,	//used when doing direct reads from data for checks [not  on pvr , size is allways 32bits]
	REG_32BIT_READWRITE=4,	//used when doing direct reads from data for checks [not  on pvr , size is allways 32bits]
	REG_READ_DATA=8,		//we can read the data from the data member
	REG_WRITE_DATA=16,		//we can write the data to the data member
	REG_READ_PREDICT=32,	//we can call the predict function and know when the register will change
	//Extended :
	REG_WRITE_NOCHANGE=64,	//we can read and write to this register , but the write won't change the value readed 
	REG_CONST=128,			//register contains constant value
	REG_NOT_IMPL=256		//Register is not implemented/unknown
};

struct RegisterStruct
{
	union{
	u32* data32;					//stores data of reg variable [if used] 32b
	u16* data16;					//stores data of reg variable [if used] 16b
	u8* data8  ;					//stores data of reg variable [if used]	8b
	};
	RegReadFP* readFunction;	//stored pointer to reg read function
	RegWriteFP* writeFunction;	//stored pointer to reg write function
	u32 flags;					//flags for read/write
	#ifdef SB_MAP_UNKNOWN_REGS
	u16 unk;
	#endif
};


struct __settings
{
	struct
	{
		bool Enable;
		bool CPpass;
		bool UnderclockFpu;
		bool Safe;	//only use the 'safe' subset of the dynarec
	} dynarec;
	
	struct
	{
		u32 cable;
		u32 RTC;
		u32 region;
		u32 broadcast;
	} dreamcast;
	struct 
	{
		bool AutoStart;
		bool NoConsole;
	} emulator;
};
extern __settings settings;

void LoadSettings();
void SaveSettings();
u32 GetRTC_now();
extern u32 patchRB;