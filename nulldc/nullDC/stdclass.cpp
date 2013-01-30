#include "types.h"
#include <string.h>
#include <windows.h>
#include "dc/mem/_vmem.h"
#include "plugins/plugin_manager.h"


//comonly used classes across the project

static std::mt19937 random_dev;
static bool r_seeded = false;

u32 fastrand() {
	if (!r_seeded) {
		r_seeded = true;
		random_dev.seed((u32)__rdtsc());
	}
	return random_dev();
}

//Misc function to get relative source directory for log's
wchar temp[1000];
wchar* GetNullDCSoruceFileName(wchar* full)
{
	size_t len = wcslen(full);
	while(len>18)
	{
		if (full[len]=='\\')
		{
			memcpy(&temp[0],&full[len-14],15*sizeof(wchar));
			temp[15]=0;
			if (wcscmp(&temp[0],L"\\nulldc\\nulldc\\")==0)
			{
				wcscpy(temp,&full[len+1]);
				return temp;
			}
		}
		len--;
	}
	wcscpy(temp,full);
	return &temp[0];
}

wchar* GetPathFromFileNameTemp(wchar* full)
{
	size_t len = wcslen(full);
	while(len>2)
	{
		if (full[len]=='\\')
		{
			memcpy(&temp[0],&full[0],(len+1)*sizeof(wchar));
			temp[len+1]=0;
			return temp;	
		}
		len--;
	}
	wcscpy(temp,full);
	return &temp[0];
}

void GetPathFromFileName(wchar* path)
{
	wcscpy(path,GetPathFromFileNameTemp(path));
}

void GetFileNameFromPath(wchar* path,wchar* outp)
{
	
	size_t i=wcslen(path);
	
	while (i>0)
	{
		i--;
		if (path[i]=='\\')
		{
			wcscpy(outp,&path[i+1]);
			return;
		}
	}

	wcscpy(outp,path);
}

wchar AppPath[1024];
void GetApplicationPath(wchar* path,u32 size)
{
	if (AppPath[0]==0)
	{
		GetModuleFileName(NULL,&AppPath[0],1024);
		GetPathFromFileName(AppPath);
	}

	wcscpy(path,AppPath);
}

wchar* GetEmuPath(const wchar* subpath)
{
	wchar* temp=(wchar*)malloc(1024);
	GetApplicationPath(temp,1024);
	wcscat(temp,subpath);
	return temp;
}

//Windoze Code implementation of commong classes from here and after ..

//Thread class
cThread::cThread(ThreadEntryFP* function,void* prm)
{
	Entry=function;
	param=prm;
	hThread=CreateThread(NULL,NULL,(LPTHREAD_START_ROUTINE)function,prm,CREATE_SUSPENDED,NULL);
}
cThread::~cThread()
{
	//gota think of something !
}
	
void cThread::Start()
{
	ResumeThread(hThread);
}
void cThread::Suspend()
{
	SuspendThread(hThread);
}
void cThread::WaitToEnd(u32 msec)
{
	WaitForSingleObject(hThread,msec);
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
//End AutoResetEvent


//Dll loader/unloader
//.ctor
cDllHandler::cDllHandler()
{
	lib=0;
}

//.dtor
cDllHandler::~cDllHandler()
{
	if (lib)
	{
		#ifdef DEBUG_DLL
		EMUWARN("cDllHandler::~cDllHandler() -> dll still loaded , unloading it..");
		#endif
		Unload();
	}
}

bool cDllHandler::Load(wchar* dll)
{
	lib=LoadLibrary(dll);
	if (lib==0)
	{
	#ifdef DEBUG_DLL
		EMUERROR2("void cDllHandler::Load(char* dll) -> dll %s could not be loaded",dll);
	#endif
	}

	return IsLoaded();
}

bool cDllHandler::IsLoaded()
{
	return lib!=0;
}

void cDllHandler::Unload()
{
	if (lib==0)
	{
		#ifdef DEBUG_DLL
		EMUWARN("void cDllHandler::Unload() -> dll is not loaded");
		#endif
	}
	else
	{
		u32 rv =FreeLibrary((HMODULE)lib);
		if (!rv)
		{
			log("FreeLibrary -- failed %d\n",GetLastError());
		}
		lib=0;
	}
}

void* cDllHandler::GetProcAddress(char* name)
{
	if (lib==0)
	{
		EMUERROR("void* cDllHandler::GetProcAddress(char* name) -> dll is not loaded");
		return 0;
	}
	else
	{
		void* rv = ::GetProcAddress((HMODULE)lib,name);

		if (rv==0)
		{
			//EMUERROR3("void* cDllHandler::GetProcAddress(char* name) :  Export named %s is not found on lib %p",name,lib);
		}

		return rv;
	}
}

//File Enumeration
void FindAllFiles(FileFoundCB* callback,wchar* dir,void* param)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	wchar DirSpec[MAX_PATH + 1];  // directory specification
	DWORD dwError;

	wcsncpy (DirSpec, dir, wcslen(dir)+1);
	//strncat (DirSpec, "\\*", 3);

	hFind = FindFirstFile( DirSpec, &FindFileData);

	if (hFind == INVALID_HANDLE_VALUE) 
	{
		return;
	} 
	else 
	{

		if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)==0)
		{
			callback(FindFileData.cFileName,param);
		}
u32 rv;
		while ( (rv=FindNextFile(hFind, &FindFileData)) != 0) 
		{ 
			if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)==0)
			{
				callback(FindFileData.cFileName,param);
			}
		}
		dwError = GetLastError();
		FindClose(hFind);
		if (dwError != ERROR_NO_MORE_FILES) 
		{
			return ;
		}
	}
	return ;
}


void VArray2::LockRegion(u32 offset,u32 size) {
#if 1
	if ((offset == m_last_lock_offs) && (size == m_last_lock_size)) {
		//printf("Skip Lock %u %u\n",offset,size);
		return;
	}
#endif
	DWORD old;
	VirtualProtect(((u8*)data)+offset , size, PAGE_READONLY,&old);
	m_last_lock_offs = offset;
	m_last_lock_size = size;
	m_last_unlock_offs = (u32)-1;
	m_last_unlock_size = (u32)-1;
}
void VArray2::UnLockRegion(u32 offset,u32 size) {
#if 1
	if ((offset == m_last_unlock_offs) && (size == m_last_unlock_size)) {
		//printf("Skip UNLock %u %u\n",offset,size);
		return;
	}
#endif
	DWORD old;
	VirtualProtect(((u8*)data)+offset , size, PAGE_READWRITE,&old);
	m_last_unlock_offs = offset;
	m_last_unlock_size = size;
	m_last_lock_offs = (u32)-1;
	m_last_lock_size = (u32)-1;
}

#include "dc\sh4\rec_v1\compiledblock.h"
#include "dc\sh4\rec_v1\blockmanager.h"

bool VramLockedWrite(u8* address);
bool RamLockedWrite(u8* address,u32* sp);
extern u8* sh4_mem_marks;
extern u8* DynarecCache;
extern u32 DynarecCacheSize;
int ExeptionHandler(u32 dwCode, void* pExceptionPointers)
{
	EXCEPTION_POINTERS* ep=(EXCEPTION_POINTERS*)pExceptionPointers;
	
	EXCEPTION_RECORD* pExceptionRecord=ep->ExceptionRecord;

	if (dwCode != EXCEPTION_ACCESS_VIOLATION)
		return EXCEPTION_CONTINUE_SEARCH;
	
	u8* address=(u8*)pExceptionRecord->ExceptionInformation[1];

	if (VramLockedWrite(address))
		return EXCEPTION_CONTINUE_EXECUTION;
	else if (RamLockedWrite(address,(u32*)ep->ContextRecord->Esp))
		return EXCEPTION_CONTINUE_EXECUTION;
	else if (((u32)(address-sh4_reserved_mem))<(512*1024*1024) || ((u32)(address-sh4_mem_marks))<(64*2*PAGE_SIZE))
	{
		//k
		//
		//cmp ecx,mask1
		//jae full_lookup
		//and ecx,mask2
		//the write 
		u32 pos=ep->ContextRecord->Eip; //<- the write
		CompiledBlockInfo* cbi=bm_ReverseLookup((void*)pos);

		if (!cbi)
		{
			log("**DYNAREC_BUG: bm_ReverseLookup failed to resolve %08X, will blindly patch due to %08X**\n",pos,address);
			log("**PLEASE REPORT THIS IF NOT ON THE ISSUE TRACKER ALREADY --raz**\n");
			return EXCEPTION_CONTINUE_SEARCH;
		}

#ifdef DEBUG
		else
			log("Except in block %X | %X\n",cbi->Code,cbi);
#endif

		//cbb->Rewrite
		u32* ptr_jae_offset=(u32*)(pos-4-6);
		u8* offset_2=(u8*)(*ptr_jae_offset + pos -6-2);
		u8* ptr_cmp=(u8*)(pos-6-6-6);
		*ptr_cmp=0xE9;
		*(u32*) (ptr_cmp+1)=*ptr_jae_offset+7 + offset_2[0];
		ep->ContextRecord->Eip=(pos-6-6-6- offset_2[1]);
		//log("Patched %08X,%08X<-%08X %d %d\n",ep->ContextRecord->Eip,ep->ContextRecord->Ecx,offset_2[0],offset_2[1]);
		//		ep->ContextRecord->Ecx=ep->ContextRecord->Eax;
		return EXCEPTION_CONTINUE_EXECUTION;
	}
	else
	{
		log("[GPF]Unhandled access to : 0x%X\n",address);
	}

	return EXCEPTION_CONTINUE_SEARCH;
}

int msgboxf(wchar* text,unsigned int type,...)
{
	va_list args;

	wchar temp[2048];
	va_start(args, type);
	vswprintf(temp, text, args);
	va_end(args);

	if (libgui.MsgBox!=0)
	{
		return libgui.MsgBox(temp,type);
	}
	else
		return MessageBox(NULL,temp,VER_SHORTNAME,type | MB_TASKMODAL);
}