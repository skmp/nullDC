#pragma once
#include "types.h"
#include <memory.h>
#include <vector>
#include <random>
#include "log/logging_interface.h"

#define PAGE_SIZE 4096
#define PAGE_MASK (PAGE_SIZE-1)

extern u32 Array_T_id_count;
wchar* GetNullDCSoruceFileName(wchar* full);
void GetPathFromFileName(wchar* full);
void GetFileNameFromPath(wchar* path,wchar* outp);
u32 fastrand();
//comonly used classes across the project
 
//Windoze code
//Threads
#define THREADCALL __stdcall

typedef  u32 THREADCALL ThreadEntryFP(void* param);
typedef void* THREADHANDLE;

class cThread
{
private:
	ThreadEntryFP* Entry;
	void* param;
public :
	THREADHANDLE hThread;
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


//Dll loader - unloader
typedef void* DLLHANDLE;
class cDllHandler
{
private :
	DLLHANDLE lib;
public:
	cDllHandler();
	~cDllHandler();
	bool Load(wchar* dll);
	bool IsLoaded();
	void Unload();
	void* GetProcAddress(char* name);
};

typedef void FileFoundCB(wchar* file,void* param);
void FindAllFiles(FileFoundCB* callback,wchar* dir,void* param);
void GetApplicationPath(wchar* path,u32 size);
wchar* GetEmuPath(const wchar* subpath);

class VArray2
{
public:

	u8* data;
	u32 size;
	//void Init(void* data,u32 sz);
	//void Term();
	void LockRegion(u32 offset,u32 size);
	void UnLockRegion(u32 offset,u32 size);

	VArray2() : m_last_lock_offs((u32)-1),m_last_lock_size((u32)-1),m_last_unlock_offs((u32)-1),m_last_unlock_size((u32)-1) {}
	void Zero()
	{
		UnLockRegion(0,size);
		memset(data,0,size);
	}

	INLINE u8& operator [](const u32 i)
    {
#ifdef MEM_BOUND_CHECK
        if (i>=size)
		{
			log("Error: VArray2 , index out of range (%d>%d)\n",i,size-1);
			MEM_DO_BREAK;
		}
#endif
		return data[i];
    }
private:
	u32 m_last_lock_offs,m_last_lock_size;
	u32 m_last_unlock_offs,m_last_unlock_size;
};

int ExeptionHandler(u32 dwCode, void* pExceptionPointers);
int msgboxf(wchar* text,unsigned int type,...);


#define MBX_OK                       0x00000000L
#define MBX_OKCANCEL                 0x00000001L
#define MBX_ABORTRETRYIGNORE         0x00000002L
#define MBX_YESNOCANCEL              0x00000003L
#define MBX_YESNO                    0x00000004L
#define MBX_RETRYCANCEL              0x00000005L


#define MBX_ICONHAND                 0x00000010L
#define MBX_ICONQUESTION             0x00000020L
#define MBX_ICONEXCLAMATION          0x00000030L
#define MBX_ICONASTERISK             0x00000040L


#define MBX_USERICON                 0x00000080L
#define MBX_ICONWARNING              MBX_ICONEXCLAMATION
#define MBX_ICONERROR                MBX_ICONHAND


#define MBX_ICONINFORMATION          MBX_ICONASTERISK
#define MBX_ICONSTOP                 MBX_ICONHAND

#define MBX_DEFBUTTON1               0x00000000L
#define MBX_DEFBUTTON2               0x00000100L
#define MBX_DEFBUTTON3               0x00000200L

#define MBX_DEFBUTTON4               0x00000300L


#define MBX_APPLMODAL                0x00000000L
#define MBX_SYSTEMMODAL              0x00001000L
#define MBX_TASKMODAL                0x00002000L

#define MBX_HELP                     0x00004000L // Help Button


#define MBX_NOFOCUS                  0x00008000L
#define MBX_SETFOREGROUND            0x00010000L
#define MBX_DEFAULT_DESKTOP_ONLY     0x00020000L

#define MBX_TOPMOST                  0x00040000L
#define MBX_RIGHT                    0x00080000L
#define MBX_RTLREADING               0x00100000L

#define MBX_RV_OK                1
#define MBX_RV_CANCEL            2
#define MBX_RV_ABORT             3
#define MBX_RV_RETRY             4
#define MBX_RV_IGNORE            5
#define MBX_RV_YES               6
#define MBX_RV_NO                7
