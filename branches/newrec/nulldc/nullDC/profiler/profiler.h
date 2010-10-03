#pragma once
#include "types.h"

extern bool TBP_Enabled;
void init_Profiler(void* param);
void term_Profiler();

void start_Profiler();
void stop_Profiler();

u64 CycleDiff();

int percent(int tick, int total);
int effsceas(int tick, int cycleDif);

enum
{		
	GFX_TC = 0,
	AICA_TC,
	ARM_TC,
	GDROM_TC,
	MAPLE_TC,
	DYNA_TC,
	DYNA_LOOP_TC,
	MAIN_TC,
	REST_TC
};

enum
{
	TICKS = 0, 	
	PERCENT,	
	EFFSCEAS,	
};

struct prof_stats
{	
	int avg_count[9][3];
	int max_count[9][3];
	int avg_counter;
};

struct prof_info
{						
	int current_count[9];	
	int total_tc; //total tics		

	/*
	int gfx_tick_count;			//on gfx dll
	int aica_tick_count;		//on aica dll
	int arm_tick_count;			//on arm dll
	int gdrom_tick_count;		//on gdrom dll
	int maple_tick_count;		//on maple dll
	int dyna_tick_count;		//on dynarec mem
	int dyna_loop_tick_count;	//on dynarec loop exe
	int main_tick_count;		//on main exe
	int rest_tick_count;		//dunno where :p
	*/	

	u64 cd;

	void ToText(wchar* dest, prof_stats* stats)
	{
		cd = CycleDiff();

		stats->avg_counter++;

		for(int i=0; i<9; i++)
		{	
			if(current_count[i] > stats->max_count[i][TICKS])
			{
				stats->max_count[i][TICKS]    = current_count[i];			
				stats->max_count[i][PERCENT]  = percent(stats->max_count[i][TICKS],total_tc); // x/100.0f
				stats->max_count[i][EFFSCEAS] = effsceas(stats->max_count[i][TICKS],cd);      // x/1000.0f
			}											

			stats->avg_count[i][TICKS]    += (current_count[i] - stats->avg_count[i][TICKS]) / stats->avg_counter;	
			stats->avg_count[i][PERCENT]  += (percent(stats->avg_count[i][TICKS],total_tc) - stats->avg_count[i][PERCENT]) / stats->avg_counter;
			stats->avg_count[i][EFFSCEAS] += (effsceas(stats->avg_count[i][TICKS],cd) - stats->avg_count[i][EFFSCEAS]) / stats->avg_counter;
		}
	
		dest+=swprintf(dest,_T("\nGFX  cur %.3f, %5.1f%% | avg %.3f, %5.1f%% | max %.3f, %5.1f%%\n"),
			effsceas(current_count[GFX_TC],cd)/1000.0f, percent(current_count[GFX_TC],total_tc)/100.0f,		
			stats->avg_count[GFX_TC][EFFSCEAS]/1000.0f, stats->avg_count[GFX_TC][PERCENT]/100.0f,		
			stats->max_count[GFX_TC][EFFSCEAS]/1000.0f, stats->max_count[GFX_TC][PERCENT]/100.0f);
		dest+=swprintf(dest,_T("AICA cur %.3f, %5.1f%% | avg %.3f, %5.1f%% | max %.3f, %5.1f%%\n"),
			effsceas(current_count[AICA_TC],cd)/1000.0f, percent(current_count[AICA_TC],total_tc)/100.0f,		
			stats->avg_count[AICA_TC][EFFSCEAS]/1000.0f, stats->avg_count[AICA_TC][PERCENT]/100.0f,		
			stats->max_count[AICA_TC][EFFSCEAS]/1000.0f, stats->max_count[AICA_TC][PERCENT]/100.0f);
		dest+=swprintf(dest,_T("ARM  cur %.3f, %5.1f%% | avg %.3f, %5.1f%% | max %.3f, %5.1f%%\n"),
			effsceas(current_count[ARM_TC],cd)/1000.0f, percent(current_count[ARM_TC],total_tc)/100.0f,		
			stats->avg_count[ARM_TC][EFFSCEAS]/1000.0f, stats->avg_count[ARM_TC][PERCENT]/100.0f,		
			stats->max_count[ARM_TC][EFFSCEAS]/1000.0f, stats->max_count[ARM_TC][PERCENT]/100.0f);
		dest+=swprintf(dest,_T("GDR  cur %.3f, %5.1f%% | avg %.3f, %5.1f%% | max %.3f, %5.1f%%\n"),
			effsceas(current_count[GDROM_TC],cd)/1000.0f, percent(current_count[GDROM_TC],total_tc)/100.0f,		
			stats->avg_count[GDROM_TC][EFFSCEAS]/1000.0f, stats->avg_count[GDROM_TC][PERCENT]/100.0f,		
			stats->max_count[GDROM_TC][EFFSCEAS]/1000.0f, stats->max_count[GDROM_TC][PERCENT]/100.0f);		
		dest+=swprintf(dest,_T("MAIN cur %.3f, %5.1f%% | avg %.3f, %5.1f%% | max %.3f, %5.1f%%\n"),
			effsceas(current_count[MAIN_TC],cd)/1000.0f, percent(current_count[MAIN_TC],total_tc)/100.0f,		
			stats->avg_count[MAIN_TC][EFFSCEAS]/1000.0f, stats->avg_count[MAIN_TC][PERCENT]/100.0f,		
			stats->max_count[MAIN_TC][EFFSCEAS]/1000.0f, stats->max_count[MAIN_TC][PERCENT]/100.0f);
		dest+=swprintf(dest,_T("LOOP cur %.3f, %5.1f%% | avg %.3f, %5.1f%% | max %.3f, %5.1f%%\n"),
			effsceas(current_count[DYNA_LOOP_TC],cd)/1000.0f, percent(current_count[DYNA_LOOP_TC],total_tc)/100.0f,		
			stats->avg_count[DYNA_LOOP_TC][EFFSCEAS]/1000.0f, stats->avg_count[DYNA_LOOP_TC][PERCENT]/100.0f,		
			stats->max_count[DYNA_LOOP_TC][EFFSCEAS]/1000.0f, stats->max_count[DYNA_LOOP_TC][PERCENT]/100.0f);
		dest+=swprintf(dest,_T("DYNA cur %.3f, %5.1f%% | avg %.3f, %5.1f%% | max %.3f, %5.1f%%\n"),
			effsceas(current_count[DYNA_TC],cd)/1000.0f, percent(current_count[DYNA_TC],total_tc)/100.0f,		
			stats->avg_count[DYNA_TC][EFFSCEAS]/1000.0f, stats->avg_count[DYNA_TC][PERCENT]/100.0f,		
			stats->max_count[DYNA_TC][EFFSCEAS]/1000.0f, stats->max_count[DYNA_TC][PERCENT]/100.0f);
		dest+=swprintf(dest,_T("REST cur %.3f, %5.1f%% | avg %.3f, %5.1f%% | max %.3f, %5.1f%%"),
			effsceas(current_count[REST_TC],cd)/1000.0f, percent(current_count[REST_TC],total_tc)/100.0f,		
			stats->avg_count[REST_TC][EFFSCEAS]/1000.0f, stats->avg_count[REST_TC][PERCENT]/100.0f,		
			stats->max_count[REST_TC][EFFSCEAS]/1000.0f, stats->max_count[REST_TC][PERCENT]/100.0f);
		
	}

};

extern prof_info profile_info;