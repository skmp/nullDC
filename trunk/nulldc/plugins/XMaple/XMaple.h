#pragma once

#include "..\..\nullDC\plugins\plugin_header.h"
#include <memory.h>
#include <math.h>

#include "MapleInterface.h"
#include "XInputBackend.h"


//////////////////////////////////////////////////////////////////////////
// should probably be moved to MapleInterface or smth
// Handy write funcs for DMA....needs rethinking (requires buffer_out_b in each DMA)
#define w32(data) *(u32*)buffer_out_b=(data);buffer_out_b+=4;buffer_out_len+=4
#define w16(data) *(u16*)buffer_out_b=(data);buffer_out_b+=2;buffer_out_len+=2
#define w8(data) *(u8*)buffer_out_b=(data);buffer_out_b+=1;buffer_out_len+=1
// write str to buffer, pad with 0x20 (space) if str < len
#define wString(str, len) for (u32 i = 0; i < len; ++i){if (str[i] != 0){w8((u8)str[i]);}else{while (i < len){w8(0x20);++i;}}}

//////////////////////////////////////////////////////////////////////////
// Crapros
#ifdef _DEBUG
// ya let's spam console in debug build
#define DEBUG_LOG(...) printf(__VA_ARGS__);
// break into debugger
#define dbgbreak {while(1) __noop;}
// print some info and crash
#define verify(x) if((x)==false){	\
	printf("Verify Failed  : " #x "\n in %s -> %s : %d \n",__FUNCTION__,__FILE__,__LINE__);	\
	dbgbreak;}
#else
// just build, dammit
#define DEBUG_LOG(...)
#define dbgbreak
#define verify(x)
#endif

//#pragma pack(1)

extern emu_info host;

//////////////////////////////////////////////////////////////////////////
// For recognizing which device the emu requests
enum
{
	ID_STDCONTROLLER,
	ID_TWINSTICK,
	ID_ARCADESTICK,
	ID_PURUPURUPACK,
	ID_MIC,
	ID_DREAMEYEMIC,
};

static const wchar_t* deviceNames[] =
{
	L"Controller",
	L"Twinstick",
	L"Arcade Stick",
	L"Puru-Puru Pak",
	L"Mic",
	L"Dreameye Mic",
};

struct xmaple_settings
{
	struct 
	{
		int Deadzone;		
	} Controller;
	
	struct 
	{
		bool UseRealFreq;
		int Length;
		int Intensity;
	} PuruPuru;	
};

void loadConfig();
void saveConfig();
