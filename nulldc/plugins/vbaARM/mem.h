#pragma once
#include "aica.h"

template<int sz,typename T>
T fastcall ReadMemArm(u32 addr);
template<int sz,typename T>
void fastcall WriteMemArm(u32 addr,T data);

#define arm_ReadMem8 ReadMemArm<1,u8>
#define arm_ReadMem16 ReadMemArm<2,u16>
#define arm_ReadMem32 ReadMemArm<4,u32>

#define arm_WriteMem8 WriteMemArm<1,u8>
#define arm_WriteMem16 WriteMemArm<2,u16>
#define arm_WriteMem32 WriteMemArm<4,u32>

u32 FASTCALL sh4_ReadMem_reg(u32 addr,u32 size);
void FASTCALL sh4_WriteMem_reg(u32 addr,u32 data,u32 size);

void init_mem();
void term_mem();

extern u8 *aica_ram;
#define aica_reg_16 ((u16*)aica_reg)

#define AICA_RAM_SIZE (ARAM_SIZE)
#define AICA_RAM_MASK (ARAM_MASK)

#define AICA_MEMMAP_RAM_SIZE (8*1024*1024)				//this is the max for the map, the actual ram size is AICA_RAM_SIZE
#define AICA_MEMMAP_RAM_MASK (AICA_MEMMAP_RAM_SIZE-1)