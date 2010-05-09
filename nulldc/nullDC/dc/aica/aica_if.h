#pragma once
#include "types.h"

extern u32 VREG;
extern VArray2 aica_ram;
u32 ReadMem_aica_rtc(u32 addr,u32 sz);
void WriteMem_aica_rtc(u32 addr,u32 data,u32 sz);
u32 FASTCALL ReadMem_aica_reg(u32 addr,u32 sz);
void FASTCALL WriteMem_aica_reg(u32 addr,u32 data,u32 sz);

void aica_Init();
void aica_Reset(bool Manual);
void aica_Term();

#define UpdateAica(clc) libAICA.Update(clc)
#define UpdateArm(clc) libARM.Update(clc)

void aica_sb_Init();
void aica_sb_Reset(bool Manual);
void aica_sb_Term();