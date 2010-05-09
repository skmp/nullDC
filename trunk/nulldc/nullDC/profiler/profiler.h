#pragma once
#include "types.h"

extern bool TBP_Enabled;
void init_Profiler(void* param);
void term_Profiler();

void start_Profiler();
void stop_Profiler();

u64 CycleDiff();
struct prof_info
{
	int total_tick_count;		//total tics

	int gfx_tick_count;			//on gfx dll
	int aica_tick_count;		//on aica dll
	int arm_tick_count;		//on arm dll
	int gdrom_tick_count;		//on gdrom dll
	int maple_tick_count;		//on maple dll
	int dyna_tick_count;		//on dynarec mem
	int dyna_loop_tick_count;		//on dynarec loop exe
	int main_tick_count;		//on main exe
	int rest_tick_count;		//dunno where :p
	u64 cd;
	#define percent(x) (x##_tick_count*100.0)/(total_tick_count)
	#define effsceas(x)    (cd/(double)x##_tick_count/1000.0/1000.0/20.0) 
	void ToText(wchar* dest)
	{
		if (total_tick_count==0)
		{
			wcscpy(dest,_T("No profile info"));
			return;
		}
		cd=CycleDiff();
		dest+=swprintf(dest,_T("gfx %.3f,% 3.1f%% | "),effsceas(gfx),percent(gfx));
		dest+=swprintf(dest,_T("aica %.3f,% 3.1f%% | "),effsceas(aica),percent(aica));
		dest+=swprintf(dest,_T("arm %.3f,% 3.1f%% | "),effsceas(arm),percent(arm));
		if (percent(gdrom)!=0)
		{
			dest+=swprintf(dest,_T("gdrom %.3f,% 3.1f%% | "),effsceas(gdrom),percent(gdrom));
		}
		dest+=swprintf(dest,_T("main %.3f,% 3.1f%% | "),effsceas(main),percent(main));
		dest+=swprintf(dest,_T("dyna loop %.3f,% 3.1f%% | "),effsceas(dyna_loop),percent(dyna_loop));
		dest+=swprintf(dest,_T("dyna %.3f,% 3.1f%% | "),effsceas(dyna),percent(dyna));
		dest+=swprintf(dest,_T("rest %.3f,% 3.1f%%"),effsceas(rest),percent(rest));
		
	}
	#undef percent
};

extern prof_info profile_info;