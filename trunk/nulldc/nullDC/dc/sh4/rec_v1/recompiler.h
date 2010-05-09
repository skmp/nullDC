//Global header for v1 dynarec
#pragma once
#include <List>
#include "types.h"
//#include "dc\sh4\shil\shil.h"

//count fallbacks and average block size, average execution time , ect :)
//#define PROFILE_DYNAREC
//#define RET_CACHE_PROF
extern u32 ret_cache_hits;
extern u32 ret_cache_total;

void printprofile();

typedef void BasicBlockEP();

extern void* Dynarec_Mainloop_no_update;
extern void* Dynarec_Mainloop_do_update;
extern u32 rec_cycles;

#define GET_CURRENT_FPU_MODE() (fpscr.PR_SZ)

#define BLOCKLIST_MAX_CYCLES (448)    


/*
	//shit i gota put somewhere

*/