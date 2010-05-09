#pragma once
#include "aica.h"

u32 FASTCALL aica_ReadMem_reg(u32 addr,u32 size);
void FASTCALL aica_WriteMem_reg(u32 addr,u32 data,u32 size);

void init_mem();
void term_mem();

extern u8 *aica_reg;
extern u8 *aica_ram;
#define aica_reg_16 ((u16*)aica_reg)

#define AICA_RAM_SIZE (ARAM_SIZE)
#define AICA_RAM_MASK (ARAM_MASK)