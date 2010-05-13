/* gameplaySP
 *
 * Copyright (C) 2006 Exophase <exophase@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef CPU_H
#define CPU_H

// System mode and user mode are represented as the same here

typedef enum
{
  MODE_USER,
  MODE_IRQ,
  MODE_FIQ,
  MODE_SUPERVISOR,
  MODE_ABORT,
  MODE_UNDEFINED,
  MODE_INVALID
} cpu_mode_type;

typedef enum
{
  CPU_ALERT_NONE,
  CPU_ALERT_HALT,
  CPU_ALERT_SMC,
  CPU_ALERT_IRQ
} cpu_alert_type;

typedef enum
{
  CPU_ACTIVE,
  CPU_HALT,
  CPU_STOP
} cpu_halt_type;

typedef enum
{
  IRQ_NONE = 0x0000,
  IRQ_VBLANK = 0x0001,
  IRQ_HBLANK = 0x0002,
  IRQ_VCOUNT = 0x0004,
  IRQ_TIMER0 = 0x0008,
  IRQ_TIMER1 = 0x0010,
  IRQ_TIMER2 = 0x0020,
  IRQ_TIMER3 = 0x0040,
  IRQ_SERIAL = 0x0080,
  IRQ_DMA0 = 0x0100,
  IRQ_DMA1 = 0x0200,
  IRQ_DMA2 = 0x0400,
  IRQ_DMA3 = 0x0800,
  IRQ_KEYPAD = 0x1000,
  IRQ_GAMEPAK = 0x2000,
} irq_type;

typedef enum
{
  REG_SP            = 13,
  REG_LR            = 14,
  REG_PC            = 15,
  REG_N_FLAG        = 16,
  REG_Z_FLAG        = 17,
  REG_C_FLAG        = 18,
  REG_V_FLAG        = 19,
  REG_CPSR          = 20,
  REG_SAVE          = 21,
  REG_SAVE2         = 22,
  REG_SAVE3         = 23,
  CPU_MODE          = 29,
  CPU_HALT_STATE    = 30,
  CHANGED_PC_STATUS = 31
} ext_reg_numbers;

typedef enum
{
  STEP,
  PC_BREAKPOINT,
  VCOUNT_BREAKPOINT,
  Z_BREAKPOINT,
  COUNTDOWN_BREAKPOINT,
  COUNTDOWN_BREAKPOINT_B,
  COUNTDOWN_BREAKPOINT_C,
  STEP_RUN,
  RUN
} debug_state;

typedef enum
{
  TRANSLATION_REGION_RAM,
  TRANSLATION_REGION_ROM,
  TRANSLATION_REGION_BIOS
} translation_region_type;

extern debug_state current_debug_state;
extern u32 instruction_count;
extern u32 last_instruction;

u32 function_cc step_debug(u32 pc, u32 cycles);
u32 execute_arm(u32 cycles);
void raise_interrupt(irq_type irq_raised);

u32 function_cc execute_load_u8(u32 address);
u32 function_cc execute_load_u16(u32 address);
u32 function_cc execute_load_u32(u32 address);
u32 function_cc execute_load_s8(u32 address);
u32 function_cc execute_load_s16(u32 address);
void function_cc execute_store_u8(u32 address, u32 source);
void function_cc execute_store_u16(u32 address, u32 source);
void function_cc execute_store_u32(u32 address, u32 source);
u32 function_cc execute_arm_translate(u32 cycles);
void init_translater();
void cpu_write_mem_savestate(file_tag_type savestate_file);
void cpu_read_savestate(file_tag_type savestate_file);

u8 function_cc *block_lookup_address_arm(u32 pc);
u8 function_cc *block_lookup_address_thumb(u32 pc);
s32 translate_block_arm(u32 pc, translation_region_type translation_region,
 u32 smc_enable);
s32 translate_block_thumb(u32 pc, translation_region_type translation_region,
 u32 smc_enable);

#ifdef GP2X_BUILD
#define ROM_TRANSLATION_CACHE_SIZE (1024 * 512 * 4 * 5)
#define RAM_TRANSLATION_CACHE_SIZE (1024 * 384 * 2)
#define BIOS_TRANSLATION_CACHE_SIZE (1024 * 128 * 2)
#define TRANSLATION_CACHE_LIMIT_THRESHOLD (1024 * 32)

#else

#define ROM_TRANSLATION_CACHE_SIZE (1024 * 512 * 4)
#define RAM_TRANSLATION_CACHE_SIZE (1024 * 384)
#define BIOS_TRANSLATION_CACHE_SIZE (1024 * 128)
#define TRANSLATION_CACHE_LIMIT_THRESHOLD (1024)

#endif

extern u8 rom_translation_cache[ROM_TRANSLATION_CACHE_SIZE];
extern u8 ram_translation_cache[RAM_TRANSLATION_CACHE_SIZE];
extern u8 bios_translation_cache[BIOS_TRANSLATION_CACHE_SIZE];
extern u8 *rom_translation_ptr;
extern u8 *ram_translation_ptr;
extern u8 *bios_translation_ptr;

#define MAX_TRANSLATION_GATES 8

extern u32 idle_loop_target_pc;
extern u32 force_pc_update_target;
extern u32 iwram_stack_optimize;
extern u32 allow_smc_ram_u8;
extern u32 allow_smc_ram_u16;
extern u32 allow_smc_ram_u32;
extern u32 direct_map_vram;
extern u32 translation_gate_targets;
extern u32 translation_gate_target_pc[MAX_TRANSLATION_GATES];

extern u32 in_interrupt;

#define ROM_BRANCH_HASH_SIZE (1024 * 64)

/* EDIT: Shouldn't this be extern ?! */
extern u32 *rom_branch_hash[ROM_BRANCH_HASH_SIZE];

void flush_translation_cache_rom();
void flush_translation_cache_ram();
void flush_translation_cache_bios();
void dump_translation_cache();

extern u32 reg_mode[7][7];
extern u32 spsr[6];

extern u32 cpu_modes[32];
extern const u32 psr_masks[16];

extern u32 breakpoint_value;

extern u32 memory_region_access_read_u8[16];
extern u32 memory_region_access_read_s8[16];
extern u32 memory_region_access_read_u16[16];
extern u32 memory_region_access_read_s16[16];
extern u32 memory_region_access_read_u32[16];
extern u32 memory_region_access_write_u8[16];
extern u32 memory_region_access_write_u16[16];
extern u32 memory_region_access_write_u32[16];
extern u32 memory_reads_u8;
extern u32 memory_reads_s8;
extern u32 memory_reads_u16;
extern u32 memory_reads_s16;
extern u32 memory_reads_u32;
extern u32 memory_writes_u8;
extern u32 memory_writes_u16;
extern u32 memory_writes_u32;

void init_cpu();
void move_reg();

#endif
