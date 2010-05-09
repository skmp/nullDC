#pragma once

#include "types.h"

//Init/Res/Term
void cpg_Init();
void cpg_Reset(bool Manual);
void cpg_Term();

extern u16 CPG_FRQCR;
extern u8 CPG_STBCR;
extern u16 CPG_WTCNT;
extern u16 CPG_WTCSR;
extern u8 CPG_STBCR2;