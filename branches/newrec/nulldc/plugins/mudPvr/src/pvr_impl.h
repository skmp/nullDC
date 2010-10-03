#ifndef _PVR_IMP_H_
#define _PVR_IMP_H_

#include "mudpvr.h"
#include "pvr_renderer_d3d1x.h"

bool spg_Init();
void spg_Term();
void spg_Reset(bool Manual);
void _fastcall spgUpdatePvr(u32 cycles);
bool spg_Init();
void spg_Term();
void spg_Reset(bool Manual);
void CalculateSync();
u32 FASTCALL ReadPvrRegister(u32 addr,u32 size);
void FASTCALL WritePvrRegister(u32 paddr,u32 data,u32 size);
bool Regs_Init();
void Regs_Term();
void Regs_Reset(bool Manual);

#endif