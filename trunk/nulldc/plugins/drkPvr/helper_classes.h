#pragma once

#include "../common/x86/x86_mem.hpp"
#include <vector>
using namespace std;

#define LIST_MAX_ALLOC_CHUNK ((1024*1024*4)/sizeof(T))	//max 4 mb alloc
#define LIST_FREE_STEP_CHUNK ((1024*128*8)/sizeof(T))		//128 kb steps for free :)

#define ChunkSize (512*1024)
u8* GetBuffer();
void FreeBuffer(u8* buffer);




template <class T,u32 MaxAllocChunk=LIST_MAX_ALLOC_CHUNK,u32 FreeStepChunk=LIST_FREE_STEP_CHUNK>
class List
{
public :
	T* data;
	u32 used;
	u32 size;

	u32 last_sizes[8];
	u32 last_size_index;
	u32 avg_sz;
	
	__declspec(noinline) void resize(u32 min_size=1)
	{
		//verify(size!=0x000004d7);
		u32 new_size=used+4+min_size;
		resize_2(new_size);
	}
	void resize_2(u32 new_size)
	{
		//verify(size!=0x000004d7);
		//printf("resize_2 , size = %d:%d\n",size,new_size);
		data=(T*)realloc(data,new_size*sizeof(T));
		size=new_size;
		//printf("resize_2 , size = %d\n",size);
	}
	void Init(u32 pre_alloc=0)
	{
		data=0;
		used=0;
		size=0;
		memset(last_sizes,0,sizeof(last_sizes));
		last_size_index=0;
		avg_sz=0;
		if (pre_alloc)
			resize(pre_alloc);
	}

	__forceinline T* LastPtr()
	{
		return &data[used-1];
	}
	__forceinline T* Append()
	{
		if (used==size)
			resize();
		return &data[used++];
	}
	__forceinline T* Append(u32 count)
	{
		if ((used+count)>=size)
			resize(count);
		T* rv=&data[used];
		used+=count;
		return rv;
	}
	void Clear()
	{
		u32 ls=last_sizes[last_size_index];
		last_sizes[last_size_index]=used;
		last_size_index=(last_size_index+1)&7;

		avg_sz-=ls;
		avg_sz+=used;

		u32 real_avg=avg_sz/8;
		if (used<real_avg)
		{
			//consider resizing ONLY if the used is less than the average
			//if diff is > FreeStepChunk
			u32 used_top=used+FreeStepChunk;

			if (used_top<real_avg)
			{
				resize(real_avg + FreeStepChunk/2);
			}
		}
		//printf("Clear , size = %d:%d\n",size,used);
		used=0;
	}
	void Free()
	{
		Clear();
		if (data)
			free(data);
		data=0;
	}
};
template <class T>
class List2
{
public :

	vector<u8*>* allocate_list_ptr;
	vector<u32>* allocate_list_sz;
	u8* ptr;
	static const u32 ItemsPerChunk=ChunkSize/sizeof(T);
private:
	u32 freesz;
	
	__declspec(noinline) void GetChunk(u32 ac)
	{
		if (ptr!=0)
		{
			allocate_list_sz->push_back((ItemsPerChunk-freesz)*sizeof(T));
			//u32 usedsz=ChunkSize-sizeof(T)*freesz;
			//alloc_info t={ptr-usedsz,usedsz};
			//allocate_list.push_back(t);
		}
		
		u8* nptr=GetBuffer();
		
		ptr=nptr;
		allocate_list_ptr->push_back(nptr);

		freesz=ItemsPerChunk;
	}
public :
	u32 used;
	T* Guarantee(u32 gcnt,u32 acnt)
	{
		verify(gcnt>=acnt);
		if(freesz<gcnt)
		{
			GetChunk(acnt);
		}

		return Append(acnt);
	}
	T* Append(u32 count)
	{
		used+=count;
		if(freesz<count)
		{
			GetChunk(count);
		}

		freesz-=count;
		T* rv=(T*)ptr;
		ptr+=count*sizeof(T);
		return rv;
	}
	T* Append()
	{
		used+=1;
		if(freesz==0)
		{
			GetChunk(1);	
		}
		freesz-=1;
		T* rv=(T*)ptr;
		ptr+=sizeof(T);
		return rv;
	}
	
	void Copy(void* Dst,u32 sz)
	{
		u8* dst=(u8*)Dst;
		for (u32 i=0;i<allocate_list_ptr->size();i++)
		{
			//The sse function (sse_memcpy_div22) is too fast for the current timing and vsync isn't reliable.
			//Ie if we hit perfectly aligned block the copy is 40-45% faster at least 
			memcpy(dst,allocate_list_ptr[0][i],allocate_list_sz[0][i]);
			dst+=allocate_list_sz[0][i];
			verify(sz>=allocate_list_sz[0][i]);
			sz-=allocate_list_sz[0][i];
		}
		verify(sz==0);
	}
	void ClearCounters()
	{
		used=0;
		ptr=0;
		freesz=0;
	}
	void Init()
	{
		allocate_list_ptr = new vector<u8*>();
		allocate_list_sz = new vector<u32>();
		ClearCounters();
	}
	void Finalise()
	{
		if (ptr!=0)
		{
			allocate_list_sz->push_back( (ItemsPerChunk-freesz)*sizeof(T));
		}
		ptr=0;
		freesz=0;
	}

	void Clear()
	{
		for (u32 i=0;i<allocate_list_ptr->size();i++)
		{
			FreeBuffer(allocate_list_ptr[0][i]);
		}
		allocate_list_ptr->clear();
		allocate_list_sz->clear();

		ClearCounters();
		used=0;
	}
	void Free()
	{
		for (u32 i=0;i<allocate_list_ptr->size();i++)
		{
			FreeBuffer(allocate_list_ptr[0][i]);
		}
		allocate_list_ptr->clear();
		allocate_list_sz->clear();

		ClearCounters();
		used=0;
	}
};
template <class T>
class List3
{
public :

	vector<u8*>* allocate_list_ptr;
	u8* ptr;
	static const u32 ItemsPerChunk=ChunkSize/sizeof(T);
private:
	__declspec(noinline) u8* NextChunk()
	{		
		u8* nptr=GetBuffer()-FreeItems*sizeof(T);
		FreeItems+=ItemsPerChunk;
		ptr=nptr;
		allocate_list_ptr->push_back(nptr);
		return nptr;
	}
	__declspec(noinline) void PrevChunk()
	{		
		FreeBuffer(allocate_list_ptr->back());
		allocate_list_ptr->pop_back();
		FreeItems-=ItemsPerChunk+1;
		ptr=allocate_list_ptr->back()+ItemsPerChunk-FreeItems+1;
	}
public :
	s32 FreeItems;//biased by 1
	void Waste(u32 count)
	{
		FreeItems-=count;
		if (FreeItems>0)
		{
			return;
		}
		NextChunk();
	}
	void UnWaste(u32 count)
	{
		FreeItems+=count;
		if (FreeItems>(ItemsPerChunk+1))
		{
			PrevChunk();
		}
	}
	T* Append()
	{
		if (--FreeItems==0)
			return (T*)NextChunk();
		else
			return (T*)(ptr+=sizeof(T));
	}

	
	void ClearCounters()
	{
		ptr=0;
		FreeItems=1;
	}
	void Init()
	{
		allocate_list_ptr = new vector<u8*>();
		ClearCounters();
	}

	void Clear()
	{
		for (u32 i=0;i<allocate_list_ptr->size();i++)
		{
			FreeBuffer(allocate_list_ptr[0][i]);
		}
		allocate_list_ptr->clear();

		ClearCounters();
	}
	void Free()
	{
		Clear();
	}
	T* Ptr()
	{
		return (T*)ptr;
	}
	u32 GetUsedCount()
	{
		return ItemsPerChunk*(allocate_list_ptr->size()+1)-(FreeItems-1)/sizeof(T);
	}
};
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
	THREADHANDLE hThread;
public :
	bool ended;
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