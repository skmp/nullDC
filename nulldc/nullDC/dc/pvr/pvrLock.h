#pragma once

#include "types.h"

//16 mb for naomi.. so be carefull not to hardcode it anywhere


#define Is_64_Bit(addr) ((addr &0x1000000)==0)
 
//vram_block, vramLockCBFP on plugin headers


u32 vramlock_ConvAddrtoOffset64(u32 Address);
u32 vramlock_ConvOffset32toOffset64(u32 offset32);

void FASTCALL vramlock_Unlock_block(vram_block* block);
vram_block* FASTCALL vramlock_Lock_32(u32 start_offset32,u32 end_offset32,void* userdata);
vram_block* FASTCALL vramlock_Lock_64(u32 start_offset64,u32 end_offset64,void* userdata);

void vram_LockedWrite(u32 offset64);