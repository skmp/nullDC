#pragma once
//bleh stupid windoze header
#include "nullDC\plugins\plugin_header.h"
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>

#include <string.h>

#define BUILD 0
#define MINOR 1
#define MAJOR 0

#define DCclock (200*1000*1000)

//called when plugin is used by emu (you should do first time init here)
void dcInit(void* param,PluginType type);

//called when plugin is unloaded by emu , olny if dcInit is called (eg , not called to enumerate plugins)
void dcTerm(PluginType type);

//It's suposed to reset anything 
void dcReset(bool Manual,PluginType type);

//called when entering sh4 thread , from the new thread context (for any thread speciacific init)
void dcThreadInit(PluginType type);

//called when exiting from sh4 thread , from the new thread context (for any thread speciacific de init) :P
void dcThreadTerm(PluginType type);

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
#define WriteMemArr(arr,addr,data,sz)				\
			{if(sz==1)								\
				{arr[addr]=(u8)data;}				\
			else if (sz==2)							\
				{*(u16*)&arr[addr]=(u16)data;}		\
			else if (sz==4)							\
			{*(u32*)&arr[addr]=data;}}	

extern arm_init_params arm_params;
extern emu_info eminf;
#define naked   __declspec( naked )

void LoadSettings();
void SaveSettings();

#define dbgbreak  { __debugbreak();/* printf("Press ANY key to continue\n");getchar();*/}

#define fastcall __fastcall
#define verify(x) if((x)==false){ printf("Verify Failed  : " #x "\n in %s -> %s : %d \n",__FUNCTION__,__FILE__,__LINE__); dbgbreak;}
#define verifyc(x) if(FAILED(x)){ printf("Verify Failed  : " #x "\n in %s -> %s : %d \n",__FUNCTION__,__FILE__,__LINE__); dbgbreak;}
#define die(reason) { printf("Fatal error : %s\n in %s -> %s : %d \n",reason,__FUNCTION__,__FILE__,__LINE__); dbgbreak;}
#define fverify verify

int cfgGetInt(char* key,int def);

extern HINSTANCE hinst;