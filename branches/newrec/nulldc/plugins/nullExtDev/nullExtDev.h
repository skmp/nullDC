#pragma once
//bleh stupid windoze header
#include "nullDC\plugins\plugin_header.h"
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>

#include <string.h>

#define BUILD 0
#define MINOR 0
#define MAJOR 1

#define DCclock (200*1000*1000)


#define 	ReadMemArrRet(arr,addr,sz)				\
			{if (sz==1)								\
				return arr[addr];					\
			else if (sz==2)							\
				return *(u16*)&arr[addr];			\
			else if (sz==4)							\
				return *(u32*)&arr[addr];}	

#define WriteMemArrRet(arr,addr,data,sz)				\
			{if(sz==1)								\
				{arr[addr]=(u8)data;return;}				\
			else if (sz==2)							\
				{*(u16*)&arr[addr]=(u16)data;return;}		\
			else if (sz==4)							\
			{*(u32*)&arr[addr]=data;return;}}	

#define verify(x) if((x)==false){ printf("Verify Failed  : " #x "\n in %s : %d \n",__FILE__,__LINE__);__debugbreak();}
#define warn(x) if((x)==true){ printf("Warn Failed  : " #x "\n in %s : %d \n",__FILE__,__LINE__);}

#define naked   __declspec( naked )

#define PAGE_SIZE 4096
#define PAGE_MASL (PAGE_SIZE-1)

void SetUpdateCallback(void (*callback) (),u32 ms);
void ExpireUpdate(bool v=true);
extern emu_info emu;
extern ext_device_init_params params;
struct __settings
{
	u32 mode;
	u32 adapter;
} ;

extern __settings settings;

void LoadSettings();
void SaveSettings();