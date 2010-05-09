#pragma once
#include "types.h"

#define GDROM_OPCODE	((u16)0x30F9)	// GDROMCALL- 0011 0000 1111 1001

#define patch_syscall_system		32
#define patch_syscall_font			16
#define patch_syscall_flashrom		8
#define patch_syscall_GDrom_misc	4
#define patch_syscall_GDrom_HLE		2
#define patch_resets_Misc			1
#define patch_all					0xFFFFFFFF

void SetPatches(u32 Value,u32 Mask);

u32 LoadFileToSh4Mem(u32 offset,wchar*file);
u32 LoadBinfileToSh4Mem(u32 offset,wchar*file);


void LoadSyscallHooks();