/*
				--nullDC Runtime profiler--
	The runtime profiler is a mini-sampling profiler.
	A background thread collects samples of EIP , and it determines
	in witch dll the EIP points to.After we have some samples (MAX_TICK_COUNT)
	we print the usage statistics on the console and reset the counters
*/
#include "profiler.h"
#include "dc\sh4\rec_v1\blockmanager.h"
#include "plugins/plugin_manager.h"
#include <windows.h>

#define MAX_TICK_COUNT 800

prof_info profile_info;

cThread* prof_thread;

u32 THREADCALL ProfileThead(void* param);
extern s32 rtc_cycles;

struct Module
{
	u32 base;
	u32 end;
	u32 len;
	
	bool Inside(u32 val) { return val>=base && val<=end; }
	void FromAddress(void* ptr)
	{
		wchar filename[512];
		wchar filename2[512];
		static void* ptr_old=0;
		if (ptr_old==ptr)
			return;
		ptr_old=ptr;

		MEMORY_BASIC_INFORMATION mbi;
		VirtualQuery(ptr,&mbi,sizeof(mbi));
		base=(u32)mbi.AllocationBase;
		GetModuleFileName((HMODULE)mbi.AllocationBase,filename,512);
		len=(u8*)mbi.BaseAddress-(u8*)mbi.AllocationBase+mbi.RegionSize;

		for(;;)
		{
			VirtualQuery(((u8*)base)+len,&mbi,sizeof(mbi));
			if (!(mbi.Type&MEM_IMAGE))
				break;
			
			if (!GetModuleFileName((HMODULE)mbi.AllocationBase,filename2,512))
				break;

			if (wcscmp(filename,filename2)!=0)
				break;
			len+=mbi.RegionSize;
		}
		

		end=base+len-1;
	}
	void FromValues(void* b,u32 s)
	{
		base= (u32)b;
		len=s;
		end=base+len-1;
	}
};
Module main_mod,aica_mod,arm_mod,pvr_mod,gdrom_mod,dyna_mod;

u64 oldcycles;
u64 CycleDiff()
{
	u64 oo=oldcycles;
	oldcycles=200*1000*1000-rtc_cycles+(u64)settings.dreamcast.RTC*200*1000*1000;
	return oldcycles-oo;
}

bool RunProfiler;
bool TBP_Enabled;
bool Reset_Stats;

void init_ProfilerModules()
{
	main_mod.FromAddress(init_ProfilerModules);
	aica_mod.FromAddress(libAICA.Load);
	arm_mod.FromAddress(libARM.Load);
	pvr_mod.FromAddress(libPvr.Load);
	gdrom_mod.FromAddress(libGDR.Load);
	dyna_mod.FromValues(DynarecCache,DynarecCacheSize);
}
void init_Profiler(void* param)
{
	//Clear profile info
	memset(&profile_info,0,sizeof(prof_info));	

	RunProfiler=true;

	prof_thread = new cThread(ProfileThead,param);
	SetThreadPriority(prof_thread->hThread,THREAD_PRIORITY_TIME_CRITICAL);
	//prof_thread->Start();
}
void start_Profiler()
{
	init_ProfilerModules();

	TBP_Enabled=true;
	
	CycleDiff();

	if (prof_thread) prof_thread->Start();
}
void stop_Profiler()
{
	TBP_Enabled = false;
	Reset_Stats = true;

	if (prof_thread) prof_thread->Suspend();
}
void term_Profiler()
{
	RunProfiler=false;
	prof_thread->Start();//make sure it is started
	prof_thread->WaitToEnd(-1);
	delete prof_thread;
	//Clear profile info
	memset(&profile_info,0,sizeof(prof_info));	
}
extern void* Dynarec_Mainloop_no_update;
extern void* Dynarec_Mainloop_do_update;
extern void* Dynarec_Mainloop_end;

void AnalyseTick(u32 pc,prof_info* to)
{
	if (aica_mod.Inside(pc))
	{				
		to->current_count[AICA_TC]++;
	}
	else if (arm_mod.Inside(pc))
	{		
		to->current_count[ARM_TC]++;
	}
	else if (pvr_mod.Inside(pc))
	{
		to->current_count[GFX_TC]++;
	}
	else if (gdrom_mod.Inside(pc))
	{
		to->current_count[GDROM_TC]++;
	}
	else if (main_mod.Inside(pc))
	{
		if (pc >= (u32)Dynarec_Mainloop_no_update && pc <= (u32)Dynarec_Mainloop_end)
			to->current_count[DYNA_LOOP_TC]++;
		else
			to->current_count[MAIN_TC]++;
	}
	else if (dyna_mod.Inside(pc))
	{
		//dyna_profiler_tick((void*)pc);
		to->current_count[DYNA_TC]++;
	}
	else
		to->current_count[REST_TC]++;
}
extern u32 no_interrupts,yes_interrupts;
 u32 THREADCALL ProfileThead(void* param)
 {
	 prof_info info;		 
	 prof_stats stats;

	 memset(&info,0,sizeof(prof_info));
	 memset(&stats,0,sizeof(prof_stats));

	 CONTEXT cntx;

	 while(RunProfiler)
	 {
		 // Reset max/avg stats
		 if(Reset_Stats)
		 {
			memset(&stats,0,sizeof(prof_stats));
			Reset_Stats = false;
		 }
		
		 //get emulation thread's pc
		 memset(&cntx,0,sizeof(cntx));
		 cntx.ContextFlags= CONTEXT_FULL;
		 
		 {
			BOOL test = GetThreadContext((HANDLE)param,&cntx);
			verify(test);
		 }

		 //count ticks
		 info.total_tc++;
		 AnalyseTick(cntx.Eip,&info);

		 //Update Stats if needed
		 if (info.total_tc>MAX_TICK_COUNT)
		 {			 			 
			 wchar temp[1024];			 

			 memcpy(&profile_info,&info,sizeof(prof_info));			 			 	 
			 memset(&info,0,sizeof(prof_info));			 			 

			 profile_info.ToText(temp, &stats);
			 wprintf(_T("%s \n"),temp);
			 
			 if ( yes_interrupts+no_interrupts)
			 {
				 double cd=profile_info.cd/(200*1000*1000.0);
				 log("Interrupts : %.0f yes, %.0f no, %.2f ratio\n",yes_interrupts/cd,no_interrupts/cd,100*yes_interrupts/(float)(yes_interrupts+no_interrupts)/cd);
				 yes_interrupts=no_interrupts=0;
			 }
			 init_ProfilerModules();
		 }

		 //Sleep , so we dont realy use the cpu much
		 Sleep(1);
	 }

	 //CloseHandle((HANDLE)param);
	 return 0;
 }

int percent(int tick, int total) 
{
	if (total == 0) return 0.0f;
	else return (tick*10000)/total; // (tick*100.0f)/(float)total;
}

int effsceas(int tick, int cycleDif)
{
	if (tick == 0) return 0.0f;
	else return (int)(cycleDif/(double)tick/1000.0/20.0); // (float)(cycleDif/(double)tick/1000.0/1000.0/20.0);
}