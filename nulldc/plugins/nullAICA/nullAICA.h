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
struct setts
{
	u32 SoundRenderer;	//0 -> sdl , (1) -> DS
	u32 HW_mixing;		//(0) -> SW , 1 -> HW , 2 -> Auto
	u32 BufferSize;		//In samples ,*4 for bytes (1024)
	u32 LimitFPS;		//0 -> no , (1) -> limit
	u32 GlobalFocus;	//0 -> only hwnd , (1) -> Global
	u32 BufferCount;	//BufferCount+2 buffers used , max 60 , default 0
	u32 CDDAMute;
	u32 GlobalMute;
	u32 DSPEnabled;		//0 -> no, 1 -> yes
	u32 Volume;		    //0-100
};

extern setts settings;
extern aica_init_params aica_params;
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

#define PAGE_SIZE 4096
#define PAGE_MASL (PAGE_SIZE-1)

int cfgGetInt(char* key,int def);

#define THREADCALL __stdcall

typedef  u32 THREADCALL ThreadEntryFP(void* param);
typedef void* THREADHANDLE;

class cThread
{
public:
	ThreadEntryFP* Entry;
	void* param;
	THREADHANDLE hThread;
public :
	cThread(ThreadEntryFP* function,void* param);
	~cThread();
	//Simple thread functions
	void Start();
	void Suspend();
	void WaitToEnd(u32 msec);
};

//Wait Events
typedef void* EVENTHANDLE;
class cResetEvent
{
private:
	EVENTHANDLE hEvent;
public :
	cResetEvent(bool State,bool Auto);
	~cResetEvent();
	void Set();		//Set state to signaled
	void Reset();	//Set state to non signaled
	void Wait(u32 msec);//Wait for signal , then reset[if auto]
	void Wait();	//Wait for signal , then reset[if auto]
};

extern HINSTANCE hinst;