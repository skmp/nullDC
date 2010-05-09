#pragma once
#include "types.h"

//regs
u32 pvr_sb_readreg_Pvr(u32 addr,u32 sz);
void pvr_sb_writereg_Pvr(u32 addr,u32 data,u32 sz);

//Init/Term , global
void pvr_sb_Init();
void pvr_sb_Term();
//Reset -> Reset - Initialise
void pvr_sb_Reset(bool Manual);