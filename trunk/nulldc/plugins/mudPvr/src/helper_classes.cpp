#include "mudpvr.h"
#include "helper_classes.h"

u8* buffer_pool[(64*1024*1024)/ChunkSize];
u32 buffer_pool_count;

u8* GetBuffer()
{
	if (buffer_pool_count!=0)
	{
		u8* rv=buffer_pool[--buffer_pool_count];
		return rv;
	}
	else
	{
		u8* ptr=(u8*)VirtualAlloc(0,ChunkSize/*+8192*/,MEM_RESERVE | MEM_COMMIT,PAGE_READWRITE);
		return ptr /*+4096*/;
	}
}

void FreeBuffer(u8* buffer)
{
	if (buffer_pool_count!=1024)
	{
		buffer_pool[buffer_pool_count++]=buffer;
	}
	else
	{
		VirtualFree(buffer/*-4096*/,0,MEM_RELEASE);
	}
}


//Windoze Code implementation of commong classes from here and after ..

//Thread class
cThread::cThread(ThreadEntryFP* function,void* prm)
{
	ended=true;
	Entry=function;
	param=prm;
	//hThread=CreateThread(NULL,NULL,(LPTHREAD_START_ROUTINE)function,prm,CREATE_SUSPENDED,NULL);
}
cThread::~cThread()
{
	//gota think of something !
}

void cThread::Start()
{
	if (ended)
		hThread=CreateThread(NULL,NULL,(LPTHREAD_START_ROUTINE)Entry,param,CREATE_SUSPENDED,NULL);
	ended=false;
	ResumeThread(hThread);
}
void cThread::Suspend()
{
	if (ended)
		hThread=CreateThread(NULL,NULL,(LPTHREAD_START_ROUTINE)Entry,param,CREATE_SUSPENDED,NULL);
	ended=false;
	SuspendThread(hThread);
}
void cThread::WaitToEnd(u32 msec)
{
	WaitForSingleObject(hThread,msec);
	ended=true;
}
//End thread class

//cResetEvent Calss
cResetEvent::cResetEvent(bool State,bool Auto)
{
	hEvent = CreateEvent( 
		NULL,             // default security attributes
		Auto?FALSE:TRUE,  // auto-reset event?
		State?TRUE:FALSE, // initial state is State
		NULL			  // unnamed object
		);
}
cResetEvent::~cResetEvent()
{
	//Destroy the event object ?
	CloseHandle(hEvent);
}
void cResetEvent::Set()//Signal
{
	SetEvent(hEvent);
}
void cResetEvent::Reset()//reset
{
	ResetEvent(hEvent);
}
void cResetEvent::Wait(u32 msec)//Wait for signal , then reset
{
	WaitForSingleObject(hEvent,msec);
}
void cResetEvent::Wait()//Wait for signal , then reset
{
	WaitForSingleObject(hEvent,(u32)-1);
}