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

#ifndef X86_EMIT_H
#define X86_EMIT_H

u32 x86_update_gba(u32 pc);

// Although these are defined as a function, don't call them as
// such (jump to it instead)
void x86_indirect_branch_arm(u32 address);
void x86_indirect_branch_thumb(u32 address);
void x86_indirect_branch_dual(u32 address);

void step_debug_x86(u32 pc);

typedef enum
{
  x86_reg_number_eax,
  x86_reg_number_ecx,
  x86_reg_number_edx,
  x86_reg_number_ebx,
  x86_reg_number_esp,
  x86_reg_number_ebp,
  x86_reg_number_esi,
  x86_reg_number_edi
} x86_reg_number;

#define x86_emit_byte(value)                                                  \
  *translation_ptr = value;                                                   \
  translation_ptr++                                                           \

#define x86_emit_dword(value)                                                 \
  *((u32 *)translation_ptr) = value;                                          \
  translation_ptr += 4                                                        \

typedef enum
{
  x86_mod_mem        = 0,
  x86_mod_mem_disp8  = 1,
  x86_mod_mem_disp32 = 2,
  x86_mod_reg        = 3
} x86_mod;

#define x86_emit_mod_rm(mod, rm, spare)                                       \
  x86_emit_byte((mod << 6) | (spare << 3) | rm)                               \

#define x86_emit_mem_op(dest, base, offset)                                   \
  if(offset == 0)                                                             \
  {                                                                           \
    x86_emit_mod_rm(x86_mod_mem, base, dest);                                 \
  }                                                                           \
  else                                                                        \
                                                                              \
  if(((s32)offset < 127) && ((s32)offset > -128))                             \
  {                                                                           \
    x86_emit_mod_rm(x86_mod_mem_disp8, base, dest);                           \
    x86_emit_byte((s8)offset);                                                \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    x86_emit_mod_rm(x86_mod_mem_disp32, base, dest);                          \
    x86_emit_dword(offset);                                                   \
  }                                                                           \

#define x86_emit_reg_op(dest, source)                                         \
  x86_emit_mod_rm(x86_mod_reg, source, dest)                                  \


typedef enum
{
  x86_opcode_mov_rm_reg                 = 0x89,
  x86_opcode_mov_reg_rm                 = 0x8B,
  x86_opcode_mov_reg_imm                = 0xB8,
  x86_opcode_mov_rm_imm                 = 0x00C7,
  x86_opcode_ror_reg_imm                = 0x01C1,
  x86_opcode_shl_reg_imm                = 0x04C1,
  x86_opcode_shr_reg_imm                = 0x05C1,
  x86_opcode_sar_reg_imm                = 0x07C1,
  x86_opcode_push_reg                   = 0x50,
  x86_opcode_push_rm                    = 0xFF,
  x86_opcode_push_imm                   = 0x0668,
  x86_opcode_call_offset                = 0xE8,
  x86_opcode_ret                        = 0xC3,
  x86_opcode_test_rm_imm                = 0x00F7,
  x86_opcode_test_reg_rm                = 0x85,
  x86_opcode_mul_eax_rm                 = 0x04F7,
  x86_opcode_imul_eax_rm                = 0x05F7,
  x86_opcode_idiv_eax_rm                = 0x07F7,
  x86_opcode_add_rm_imm                 = 0x0081,
  x86_opcode_and_rm_imm                 = 0x0481,
  x86_opcode_sub_rm_imm                 = 0x0581,
  x86_opcode_xor_rm_imm                 = 0x0681,
  x86_opcode_add_reg_rm                 = 0x03,
  x86_opcode_adc_reg_rm                 = 0x13,
  x86_opcode_or_reg_rm                  = 0x0B,
  x86_opcode_sub_reg_rm                 = 0x2B,
  x86_opcode_xor_reg_rm                 = 0x33,
  x86_opcode_cmp_reg_rm                 = 0x39,
  x86_opcode_cmp_rm_imm                 = 0x053B,
  x86_opcode_lea_reg_rm                 = 0x8D,
  x86_opcode_j                          = 0x80,
  x86_opcode_jmp                        = 0xE9,
  x86_opcode_jmp_reg                    = 0x04FF,
  x86_opcode_ext                        = 0x0F
} x86_opcodes;

typedef enum
{
  x86_condition_code_o                  = 0x00,
  x86_condition_code_no                 = 0x01,
  x86_condition_code_c                  = 0x02,
  x86_condition_code_nc                 = 0x03,
  x86_condition_code_z                  = 0x04,
  x86_condition_code_nz                 = 0x05,
  x86_condition_code_na                 = 0x06,
  x86_condition_code_a                  = 0x07,
  x86_condition_code_s                  = 0x08,
  x86_condition_code_ns                 = 0x09,
  x86_condition_code_p                  = 0x0A,
  x86_condition_code_np                 = 0x0B,
  x86_condition_code_l                  = 0x0C,
  x86_condition_code_nl                 = 0x0D,
  x86_condition_code_ng                 = 0x0E,
  x86_condition_code_g                  = 0x0F
} x86_condition_codes;

#define x86_relative_offset(source, offset, next)                             \
  ((u32)offset - ((u32)source + next))                                        \

#define x86_unequal_operands(op_a, op_b)                                      \
  (x86_reg_number_##op_a != x86_reg_number_##op_b)                            \

#define x86_emit_opcode_1b_reg(opcode, dest, source)                          \
{                                                                             \
  x86_emit_byte(x86_opcode_##opcode);                                         \
  x86_emit_reg_op(x86_reg_number_##dest, x86_reg_number_##source);            \
}                                                                             \

#define x86_emit_opcode_1b_mem(opcode, dest, base, offset)                    \
{                                                                             \
  x86_emit_byte(x86_opcode_##opcode);                                         \
  x86_emit_mem_op(x86_reg_number_##dest, x86_reg_number_##base, offset);      \
}                                                                             \

#define x86_emit_opcode_1b(opcode, reg)                                       \
  x86_emit_byte(x86_opcode_##opcode | x86_reg_number_##reg)                   \

#define x86_emit_opcode_1b_ext_reg(opcode, dest)                              \
  x86_emit_byte(x86_opcode_##opcode & 0xFF);                                  \
  x86_emit_reg_op(x86_opcode_##opcode >> 8, x86_reg_number_##dest)            \

#define x86_emit_opcode_1b_ext_mem(opcode, base, offset)                      \
  x86_emit_byte(x86_opcode_##opcode & 0xFF);                                  \
  x86_emit_mem_op(x86_opcode_##opcode >> 8, x86_reg_number_##base, offset)    \

#define x86_emit_mov_reg_mem(dest, base, offset)                              \
  x86_emit_opcode_1b_mem(mov_reg_rm, dest, base, offset)                      \

#define x86_emit_mov_mem_reg(source, base, offset)                            \
  x86_emit_opcode_1b_mem(mov_rm_reg, source, base, offset)                    \

#define x86_emit_mov_reg_reg(dest, source)                                    \
  if(x86_unequal_operands(dest, source))                                      \
  {                                                                           \
    x86_emit_opcode_1b_reg(mov_reg_rm, dest, source)                          \
  }                                                                           \

#define x86_emit_mov_reg_imm(dest, imm)                                       \
  x86_emit_opcode_1b(mov_reg_imm, dest);                                      \
  x86_emit_dword(imm)                                                         \

#define x86_emit_mov_mem_imm(imm, base, offset)                               \
  x86_emit_opcode_1b_ext_mem(mov_rm_imm, base, offset);                       \
  x86_emit_dword(imm)                                                         \

#define x86_emit_shl_reg_imm(dest, imm)                                       \
  x86_emit_opcode_1b_ext_reg(shl_reg_imm, dest);                              \
  x86_emit_byte(imm)                                                          \

#define x86_emit_shr_reg_imm(dest, imm)                                       \
  x86_emit_opcode_1b_ext_reg(shr_reg_imm, dest);                              \
  x86_emit_byte(imm)                                                          \

#define x86_emit_sar_reg_imm(dest, imm)                                       \
  x86_emit_opcode_1b_ext_reg(sar_reg_imm, dest);                              \
  x86_emit_byte(imm)                                                          \

#define x86_emit_ror_reg_imm(dest, imm)                                       \
  x86_emit_opcode_1b_ext_reg(ror_reg_imm, dest);                              \
  x86_emit_byte(imm)                                                          \

#define x86_emit_add_reg_reg(dest, source)                                    \
  x86_emit_opcode_1b_reg(add_reg_rm, dest, source)                            \

#define x86_emit_adc_reg_reg(dest, source)                                    \
  x86_emit_opcode_1b_reg(adc_reg_rm, dest, source)                            \

#define x86_emit_sub_reg_reg(dest, source)                                    \
  x86_emit_opcode_1b_reg(sub_reg_rm, dest, source)                            \

#define x86_emit_or_reg_reg(dest, source)                                     \
  x86_emit_opcode_1b_reg(or_reg_rm, dest, source)                             \

#define x86_emit_xor_reg_reg(dest, source)                                    \
  x86_emit_opcode_1b_reg(xor_reg_rm, dest, source)                            \

#define x86_emit_add_reg_imm(dest, imm)                                       \
  if(imm != 0)                                                                \
  {                                                                           \
    x86_emit_opcode_1b_ext_reg(add_rm_imm, dest);                             \
    x86_emit_dword(imm);                                                      \
  }                                                                           \

#define x86_emit_sub_reg_imm(dest, imm)                                       \
  if(imm != 0)                                                                \
  {                                                                           \
    x86_emit_opcode_1b_ext_reg(sub_rm_imm, dest);                             \
    x86_emit_dword(imm);                                                      \
  }                                                                           \

#define x86_emit_and_reg_imm(dest, imm)                                       \
  x86_emit_opcode_1b_ext_reg(and_rm_imm, dest);                               \
  x86_emit_dword(imm)                                                         \

#define x86_emit_xor_reg_imm(dest, imm)                                       \
  x86_emit_opcode_1b_ext_reg(xor_rm_imm, dest);                               \
  x86_emit_dword(imm)                                                         \

#define x86_emit_test_reg_imm(dest, imm)                                      \
  x86_emit_opcode_1b_ext_reg(test_rm_imm, dest);                              \
  x86_emit_dword(imm)                                                         \

#define x86_emit_cmp_reg_reg(dest, source)                                    \
  x86_emit_opcode_1b_reg(cmp_reg_rm, dest, source)                            \

#define x86_emit_test_reg_reg(dest, source)                                   \
  x86_emit_opcode_1b_reg(test_reg_rm, dest, source)                           \

#define x86_emit_cmp_reg_imm(dest, imm)                                       \
  x86_emit_opcode_1b_ext_reg(cmp_rm_imm, dest);                               \
  x86_emit_dword(imm)                                                         \

#define x86_emit_mul_eax_reg(source)                                          \
  x86_emit_opcode_1b_ext_reg(mul_eax_rm, source)                              \

#define x86_emit_imul_eax_reg(source)                                         \
  x86_emit_opcode_1b_ext_reg(imul_eax_rm, source)                             \

#define x86_emit_idiv_eax_reg(source)                                         \
  x86_emit_opcode_1b_ext_reg(idiv_eax_rm, source)                             \

#define x86_emit_push_mem(base, offset)                                       \
  x86_emit_opcode_1b_mem(push_rm, 0x06, base, offset)                         \

#define x86_emit_push_imm(imm)                                                \
  x86_emit_byte(x86_opcode_push_imm);                                         \
  x86_emit_dword(imm)                                                         \

#define x86_emit_call_offset(relative_offset)                                 \
  x86_emit_byte(x86_opcode_call_offset);                                      \
  x86_emit_dword(relative_offset)                                             \

#define x86_emit_ret()                                                        \
  x86_emit_byte(x86_opcode_ret)                                               \

#define x86_emit_lea_reg_mem(dest, base, offset)                              \
  x86_emit_opcode_1b_mem(lea_reg_rm, dest, base, offset)                      \

#define x86_emit_j_filler(condition_code, writeback_location)                 \
  x86_emit_byte(x86_opcode_ext);                                              \
  x86_emit_byte(x86_opcode_j | condition_code);                               \
  (writeback_location) = translation_ptr;                                     \
  translation_ptr += 4                                                        \

#define x86_emit_j_offset(condition_code, offset)                             \
  x86_emit_byte(x86_opcode_ext);                                              \
  x86_emit_byte(x86_opcode_j | condition_code);                               \
  x86_emit_dword(offset)                                                      \

#define x86_emit_jmp_filler(writeback_location)                               \
  x86_emit_byte(x86_opcode_jmp);                                              \
  (writeback_location) = translation_ptr;                                     \
  translation_ptr += 4                                                        \

#define x86_emit_jmp_offset(offset)                                           \
  x86_emit_byte(x86_opcode_jmp);                                              \
  x86_emit_dword(offset)                                                      \

#define x86_emit_jmp_reg(source)                                              \
  x86_emit_opcode_1b_ext_reg(jmp_reg, source)                                 \

#define reg_base    ebx
#define reg_cycles  edi
#define reg_a0      eax
#define reg_a1      edx
#define reg_a2      ecx
#define reg_rv      eax
#define reg_s0      esi

#define generate_load_reg(ireg, reg_index)                                    \
  x86_emit_mov_reg_mem(reg_##ireg, reg_base, reg_index * 4);                  \

#define generate_load_pc(ireg, new_pc)                                        \
  x86_emit_mov_reg_imm(reg_##ireg, new_pc)                                    \

#define generate_load_imm(ireg, imm)                                          \
  x86_emit_mov_reg_imm(reg_##ireg, imm)                                       \

#define generate_store_reg(ireg, reg_index)                                   \
  x86_emit_mov_mem_reg(reg_##ireg, reg_base, reg_index * 4)                   \

#define generate_shift_left(ireg, imm)                                        \
  x86_emit_shl_reg_imm(reg_##ireg, imm)                                       \

#define generate_shift_right(ireg, imm)                                       \
  x86_emit_shr_reg_imm(reg_##ireg, imm)                                       \

#define generate_shift_right_arithmetic(ireg, imm)                            \
  x86_emit_sar_reg_imm(reg_##ireg, imm)                                       \

#define generate_rotate_right(ireg, imm)                                      \
  x86_emit_ror_reg_imm(reg_##ireg, imm)                                       \

#define generate_add(ireg_dest, ireg_src)                                     \
  x86_emit_add_reg_reg(reg_##ireg_dest, reg_##ireg_src)                       \

#define generate_sub(ireg_dest, ireg_src)                                     \
  x86_emit_sub_reg_reg(reg_##ireg_dest, reg_##ireg_src)                       \

#define generate_or(ireg_dest, ireg_src)                                      \
  x86_emit_or_reg_reg(reg_##ireg_dest, reg_##ireg_src)                        \

#define generate_xor(ireg_dest, ireg_src)                                     \
  x86_emit_xor_reg_reg(reg_##ireg_dest, reg_##ireg_src)                       \

#define generate_add_imm(ireg, imm)                                           \
  x86_emit_add_reg_imm(reg_##ireg, imm)                                       \

#define generate_sub_imm(ireg, imm)                                           \
  x86_emit_sub_reg_imm(reg_##ireg, imm)                                       \

#define generate_xor_imm(ireg, imm)                                           \
  x86_emit_xor_reg_imm(reg_##ireg, imm)                                       \

#define generate_add_reg_reg_imm(ireg_dest, ireg_src, imm)                    \
  x86_emit_lea_reg_mem(reg_##ireg_dest, reg_##ireg_src, imm)                  \

#define generate_and_imm(ireg, imm)                                           \
  x86_emit_and_reg_imm(reg_##ireg, imm)                                       \

#define generate_mov(ireg_dest, ireg_src)                                     \
  x86_emit_mov_reg_reg(reg_##ireg_dest, reg_##ireg_src)                       \

#define generate_multiply(ireg)                                               \
  x86_emit_imul_eax_reg(reg_##ireg)                                           \

#define generate_multiply_s64(ireg)                                           \
  x86_emit_imul_eax_reg(reg_##ireg)                                           \

#define generate_multiply_u64(ireg)                                           \
  x86_emit_mul_eax_reg(reg_##ireg)                                            \

#define generate_multiply_s64_add(ireg_src, ireg_lo, ireg_hi)                 \
  x86_emit_imul_eax_reg(reg_##ireg_src);                                      \
  x86_emit_add_reg_reg(reg_a0, reg_##ireg_lo);                                \
  x86_emit_adc_reg_reg(reg_a1, reg_##ireg_hi)                                 \

#define generate_multiply_u64_add(ireg_src, ireg_lo, ireg_hi)                 \
  x86_emit_mul_eax_reg(reg_##ireg_src);                                       \
  x86_emit_add_reg_reg(reg_a0, reg_##ireg_lo);                                \
  x86_emit_adc_reg_reg(reg_a1, reg_##ireg_hi)                                 \


#define generate_function_call(function_location)                             \
  x86_emit_call_offset(x86_relative_offset(translation_ptr,                   \
   function_location, 4));                                                    \

#define generate_exit_block()                                                 \
  x86_emit_ret();                                                             \

#define generate_branch_filler_true(ireg_dest, ireg_src, writeback_location)  \
  x86_emit_test_reg_imm(reg_##ireg_dest, 1);                                  \
  x86_emit_j_filler(x86_condition_code_z, writeback_location)                 \

#define generate_branch_filler_false(ireg_dest, ireg_src, writeback_location) \
  x86_emit_test_reg_imm(reg_##ireg_dest, 1);                                  \
  x86_emit_j_filler(x86_condition_code_nz, writeback_location)                \

#define generate_branch_filler_equal(ireg_dest, ireg_src, writeback_location) \
  x86_emit_cmp_reg_reg(reg_##ireg_dest, reg_##ireg_src);                      \
  x86_emit_j_filler(x86_condition_code_nz, writeback_location)                \

#define generate_branch_filler_not_equal(ireg_dest, ireg_src,                 \
 writeback_location)                                                          \
  x86_emit_cmp_reg_reg(reg_##ireg_dest, reg_##ireg_src);                      \
  x86_emit_j_filler(x86_condition_code_z, writeback_location)                 \

#define generate_update_pc(new_pc)                                            \
  x86_emit_mov_reg_imm(eax, new_pc)                                           \

#define generate_cycle_update()                                               \
  x86_emit_sub_reg_imm(reg_cycles, cycle_count);                              \
  cycle_count = 0                                                             \

#define generate_branch_patch_conditional(dest, offset)                       \
  *((u32 *)(dest)) = x86_relative_offset(dest, offset, 4)                     \

#define generate_branch_patch_unconditional(dest, offset)                     \
  *((u32 *)(dest)) = x86_relative_offset(dest, offset, 4)                     \

#define generate_branch_no_cycle_update(writeback_location, new_pc)           \
  if(pc == idle_loop_target_pc)                                               \
  {                                                                           \
    x86_emit_mov_reg_imm(eax, new_pc);                                        \
    generate_function_call(x86_update_gba);                                   \
    x86_emit_jmp_filler(writeback_location);                                  \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    x86_emit_test_reg_reg(reg_cycles, reg_cycles);                            \
    x86_emit_j_offset(x86_condition_code_ns, 10);                             \
    x86_emit_mov_reg_imm(eax, new_pc);                                        \
    generate_function_call(x86_update_gba);                                   \
    x86_emit_jmp_filler(writeback_location);                                  \
  }                                                                           \

#define generate_branch_cycle_update(writeback_location, new_pc)              \
  generate_cycle_update();                                                    \
  generate_branch_no_cycle_update(writeback_location, new_pc)                 \

#define generate_conditional_branch(ireg_a, ireg_b, type, writeback_location) \
  generate_branch_filler_##type(ireg_a, ireg_b, writeback_location)           \

// a0 holds the destination

#define generate_indirect_branch_cycle_update(type)                           \
  /*generate_cycle_update();                                                  \
  x86_emit_j_offset(x86_condition_code_ns, 5);                                \
  generate_function_call(x86_update_gba);*/                                     \
  x86_emit_jmp_offset(x86_relative_offset(translation_ptr,                    \
   x86_indirect_branch_##type, 4))                                            \

#define generate_indirect_branch_no_cycle_update(type)                        \
  /*x86_emit_test_reg_reg(reg_cycles, reg_cycles);                              \
  x86_emit_j_offset(x86_condition_code_ns, 5);                                \
  generate_function_call(x86_update_gba);*/                                     \
  x86_emit_jmp_offset(x86_relative_offset(translation_ptr,                    \
   x86_indirect_branch_##type, 4))                                            \

#define generate_block_prologue()                                             \

#define generate_block_extra_vars_arm()                                       \
  void generate_indirect_branch_arm()                                         \
  {                                                                           \
    if(condition == 0x0E)                                                     \
    {                                                                         \
      generate_indirect_branch_cycle_update(arm);                             \
    }                                                                         \
    else                                                                      \
    {                                                                         \
      generate_indirect_branch_no_cycle_update(arm);                          \
    }                                                                         \
  }                                                                           \
                                                                              \
  void generate_indirect_branch_dual()                                        \
  {                                                                           \
    if(condition == 0x0E)                                                     \
    {                                                                         \
      generate_indirect_branch_cycle_update(dual);                            \
    }                                                                         \
    else                                                                      \
    {                                                                         \
      generate_indirect_branch_no_cycle_update(dual);                         \
    }                                                                         \
  }                                                                           \

#define generate_block_extra_vars_thumb()                                     \


#define translate_invalidate_dcache()                                         \

#define block_prologue_size 0

#define calculate_z_flag(dest)                                                \
  reg[REG_Z_FLAG] = (dest == 0)                                               \

#define calculate_n_flag(dest)                                                \
  reg[REG_N_FLAG] = ((signed)dest < 0)                                        \

#define calculate_c_flag_sub(dest, src_a, src_b)                              \
  reg[REG_C_FLAG] = ((unsigned)src_b <= (unsigned)src_a)                      \

#define calculate_v_flag_sub(dest, src_a, src_b)                              \
  reg[REG_V_FLAG] = ((signed)src_b > (signed)src_a) != ((signed)dest < 0)     \

#define calculate_c_flag_add(dest, src_a, src_b)                              \
  reg[REG_C_FLAG] = ((unsigned)dest < (unsigned)src_a)                        \

#define calculate_v_flag_add(dest, src_a, src_b)                              \
  reg[REG_V_FLAG] = ((signed)dest < (signed)src_a) != ((signed)src_b < 0)     \



#define get_shift_imm()                                                       \
  u32 shift = (opcode >> 7) & 0x1F                                            \

#define generate_shift_reg(ireg, name, flags_op)                              \
  generate_load_reg_pc(ireg, rm, 12);                                         \
  generate_load_reg(a1, ((opcode >> 8) & 0x0F));                              \
  generate_function_call(execute_##name##_##flags_op##_reg);                  \
  generate_mov(ireg, rv)                                                      \

u32 function_cc execute_lsl_no_flags_reg(u32 value, u32 shift)
{
  if(shift != 0)
  {
    if(shift > 31)
      value = 0;
    else
      value <<= shift;
  }
  return value;
}

u32 function_cc execute_lsr_no_flags_reg(u32 value, u32 shift)
{
  if(shift != 0)
  {
    if(shift > 31)
      value = 0;
    else
      value >>= shift;
  }
  return value;
}

u32 function_cc execute_asr_no_flags_reg(u32 value, u32 shift)
{
  if(shift != 0)
  {
    if(shift > 31)
      value = (s32)value >> 31;
    else
      value = (s32)value >> shift;
  }
  return value;
}

u32 function_cc execute_ror_no_flags_reg(u32 value, u32 shift)
{
  if(shift != 0)
  {
    ror(value, value, shift);
  }

  return value;
}


u32 function_cc execute_lsl_flags_reg(u32 value, u32 shift)
{
  if(shift != 0)
  {
    if(shift > 31)
    {
      if(shift == 32)
        reg[REG_C_FLAG] = value & 0x01;
      else
        reg[REG_C_FLAG] = 0;

      value = 0;
    }
    else
    {
      reg[REG_C_FLAG] = (value >> (32 - shift)) & 0x01;
      value <<= shift;
    }
  }
  return value;
}

u32 function_cc execute_lsr_flags_reg(u32 value, u32 shift)
{
  if(shift != 0)
  {
    if(shift > 31)
    {
      if(shift == 32)
        reg[REG_C_FLAG] = (value >> 31) & 0x01;
      else
        reg[REG_C_FLAG] = 0;

      value = 0;
    }
    else
    {
      reg[REG_C_FLAG] = (value >> (shift - 1)) & 0x01;
      value >>= shift;
    }
  }
  return value;
}

u32 function_cc execute_asr_flags_reg(u32 value, u32 shift)
{
  if(shift != 0)
  {
    if(shift > 31)
    {
      value = (s32)value >> 31;
      reg[REG_C_FLAG] = value & 0x01;
    }
    else
    {
      reg[REG_C_FLAG] = (value >> (shift - 1)) & 0x01;
      value = (s32)value >> shift;
    }
  }
  return value;
}

u32 function_cc execute_ror_flags_reg(u32 value, u32 shift)
{
  if(shift != 0)
  {
    reg[REG_C_FLAG] = (value >> (shift - 1)) & 0x01;
    ror(value, value, shift);
  }

  return value;
}

u32 function_cc execute_rrx_flags(u32 value)
{
  u32 c_flag = reg[REG_C_FLAG];
  reg[REG_C_FLAG] = value & 0x01;
  return (value >> 1) | (c_flag << 31);
}

u32 function_cc execute_rrx(u32 value)
{
  return (value >> 1) | (reg[REG_C_FLAG] << 31);
}

#define generate_shift_imm_lsl_no_flags(ireg)                                 \
  generate_load_reg_pc(ireg, rm, 8);                                          \
  if(shift != 0)                                                              \
  {                                                                           \
    generate_shift_left(ireg, shift);                                         \
  }                                                                           \

#define generate_shift_imm_lsr_no_flags(ireg)                                 \
  if(shift != 0)                                                              \
  {                                                                           \
    generate_load_reg_pc(ireg, rm, 8);                                        \
    generate_shift_right(ireg, shift);                                        \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    generate_load_imm(ireg, 0);                                               \
  }                                                                           \

#define generate_shift_imm_asr_no_flags(ireg)                                 \
  generate_load_reg_pc(ireg, rm, 8);                                          \
  if(shift != 0)                                                              \
  {                                                                           \
    generate_shift_right_arithmetic(ireg, shift);                             \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    generate_shift_right_arithmetic(ireg, 31);                                \
  }                                                                           \

#define generate_shift_imm_ror_no_flags(ireg)                                 \
  if(shift != 0)                                                              \
  {                                                                           \
    generate_load_reg_pc(ireg, rm, 8);                                        \
    generate_rotate_right(ireg, shift);                                       \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    generate_load_reg_pc(a0, rm, 8);                                          \
    generate_function_call(execute_rrx);                                      \
    generate_mov(ireg, rv);                                                   \
  }                                                                           \

#define generate_shift_imm_lsl_flags(ireg)                                    \
  generate_load_reg_pc(ireg, rm, 8);                                          \
  if(shift != 0)                                                              \
  {                                                                           \
    generate_mov(a1, ireg);                                                   \
    generate_shift_right(a1, (32 - shift));                                   \
    generate_and_imm(a1, 1);                                                  \
    generate_store_reg(a1, REG_C_FLAG);                                       \
    generate_shift_left(ireg, shift);                                         \
  }                                                                           \

#define generate_shift_imm_lsr_flags(ireg)                                    \
  if(shift != 0)                                                              \
  {                                                                           \
    generate_load_reg_pc(ireg, rm, 8);                                        \
    generate_mov(a1, ireg);                                                   \
    generate_shift_right(a1, shift - 1);                                      \
    generate_and_imm(a1, 1);                                                  \
    generate_store_reg(a1, REG_C_FLAG);                                       \
    generate_shift_right(ireg, shift);                                        \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    generate_load_reg_pc(a1, rm, 8);                                          \
    generate_shift_right(a1, 31);                                             \
    generate_store_reg(a1, REG_C_FLAG);                                       \
    generate_load_imm(ireg, 0);                                               \
  }                                                                           \

#define generate_shift_imm_asr_flags(ireg)                                    \
  if(shift != 0)                                                              \
  {                                                                           \
    generate_load_reg_pc(ireg, rm, 8);                                        \
    generate_mov(a1, ireg);                                                   \
    generate_shift_right_arithmetic(a1, shift - 1);                           \
    generate_and_imm(a1, 1);                                                  \
    generate_store_reg(a1, REG_C_FLAG);                                       \
    generate_shift_right_arithmetic(ireg, shift);                             \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    generate_load_reg_pc(a0, rm, 8);                                          \
    generate_shift_right_arithmetic(ireg, 31);                                \
    generate_mov(a1, ireg);                                                   \
    generate_and_imm(a1, 1);                                                  \
    generate_store_reg(a1, REG_C_FLAG);                                       \
  }                                                                           \

#define generate_shift_imm_ror_flags(ireg)                                    \
  generate_load_reg_pc(ireg, rm, 8);                                          \
  if(shift != 0)                                                              \
  {                                                                           \
    generate_mov(a1, ireg);                                                   \
    generate_shift_right_arithmetic(a1, shift - 1);                           \
    generate_and_imm(a1, 1);                                                  \
    generate_store_reg(a1, REG_C_FLAG);                                       \
    generate_rotate_right(ireg, shift);                                       \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    generate_function_call(execute_rrx_flags);                                \
    generate_mov(ireg, rv);                                                   \
  }                                                                           \

#define generate_shift_imm(ireg, name, flags_op)                              \
  get_shift_imm();                                                            \
  generate_shift_imm_##name##_##flags_op(ireg)                                \

#define generate_load_rm_sh(flags_op)                                         \
  switch((opcode >> 4) & 0x07)                                                \
  {                                                                           \
    /* LSL imm */                                                             \
    case 0x0:                                                                 \
    {                                                                         \
      generate_shift_imm(a0, lsl, flags_op);                                  \
      break;                                                                  \
    }                                                                         \
                                                                              \
    /* LSL reg */                                                             \
    case 0x1:                                                                 \
    {                                                                         \
      generate_shift_reg(a0, lsl, flags_op);                                  \
      break;                                                                  \
    }                                                                         \
                                                                              \
    /* LSR imm */                                                             \
    case 0x2:                                                                 \
    {                                                                         \
      generate_shift_imm(a0, lsr, flags_op);                                  \
      break;                                                                  \
    }                                                                         \
                                                                              \
    /* LSR reg */                                                             \
    case 0x3:                                                                 \
    {                                                                         \
      generate_shift_reg(a0, lsr, flags_op);                                  \
      break;                                                                  \
    }                                                                         \
                                                                              \
    /* ASR imm */                                                             \
    case 0x4:                                                                 \
    {                                                                         \
      generate_shift_imm(a0, asr, flags_op);                                  \
      break;                                                                  \
    }                                                                         \
                                                                              \
    /* ASR reg */                                                             \
    case 0x5:                                                                 \
    {                                                                         \
      generate_shift_reg(a0, asr, flags_op);                                  \
      break;                                                                  \
    }                                                                         \
                                                                              \
    /* ROR imm */                                                             \
    case 0x6:                                                                 \
    {                                                                         \
      generate_shift_imm(a0, ror, flags_op);                                  \
      break;                                                                  \
    }                                                                         \
                                                                              \
    /* ROR reg */                                                             \
    case 0x7:                                                                 \
    {                                                                         \
      generate_shift_reg(a0, ror, flags_op);                                  \
      break;                                                                  \
    }                                                                         \
  }                                                                           \

#define generate_load_offset_sh()                                             \
  switch((opcode >> 5) & 0x03)                                                \
  {                                                                           \
    /* LSL imm */                                                             \
    case 0x0:                                                                 \
    {                                                                         \
      generate_shift_imm(a1, lsl, no_flags);                                  \
      break;                                                                  \
    }                                                                         \
                                                                              \
    /* LSR imm */                                                             \
    case 0x1:                                                                 \
    {                                                                         \
      generate_shift_imm(a1, lsr, no_flags);                                  \
      break;                                                                  \
    }                                                                         \
                                                                              \
    /* ASR imm */                                                             \
    case 0x2:                                                                 \
    {                                                                         \
      generate_shift_imm(a1, asr, no_flags);                                  \
      break;                                                                  \
    }                                                                         \
                                                                              \
    /* ROR imm */                                                             \
    case 0x3:                                                                 \
    {                                                                         \
      generate_shift_imm(a1, ror, no_flags);                                  \
      break;                                                                  \
    }                                                                         \
  }                                                                           \

#define calculate_flags_add(dest, src_a, src_b)                               \
  calculate_z_flag(dest);                                                     \
  calculate_n_flag(dest);                                                     \
  calculate_c_flag_add(dest, src_a, src_b);                                   \
  calculate_v_flag_add(dest, src_a, src_b)                                    \

#define calculate_flags_sub(dest, src_a, src_b)                               \
  calculate_z_flag(dest);                                                     \
  calculate_n_flag(dest);                                                     \
  calculate_c_flag_sub(dest, src_a, src_b);                                   \
  calculate_v_flag_sub(dest, src_a, src_b)                                    \

#define calculate_flags_logic(dest)                                           \
  calculate_z_flag(dest);                                                     \
  calculate_n_flag(dest)                                                      \

#define extract_flags()                                                       \
  reg[REG_N_FLAG] = reg[REG_CPSR] >> 31;                                      \
  reg[REG_Z_FLAG] = (reg[REG_CPSR] >> 30) & 0x01;                             \
  reg[REG_C_FLAG] = (reg[REG_CPSR] >> 29) & 0x01;                             \
  reg[REG_V_FLAG] = (reg[REG_CPSR] >> 28) & 0x01;                             \

#define collapse_flags()                                                      \
  reg[REG_CPSR] = (reg[REG_N_FLAG] << 31) | (reg[REG_Z_FLAG] << 30) |         \
   (reg[REG_C_FLAG] << 29) | (reg[REG_V_FLAG] << 28) |                        \
   reg[REG_CPSR] & 0xFF                                                       \

// It should be okay to still generate result flags, spsr will overwrite them.
// This is pretty infrequent (returning from interrupt handlers, et al) so
// probably not worth optimizing for.

#define check_for_interrupts()                                                \
  if((io_registers[REG_IE] & io_registers[REG_IF]) &&                         \
   io_registers[REG_IME] && ((reg[REG_CPSR] & 0x80) == 0))                    \
  {                                                                           \
    reg_mode[MODE_IRQ][6] = reg[REG_PC] + 4;                                  \
    spsr[MODE_IRQ] = reg[REG_CPSR];                                           \
    reg[REG_CPSR] = 0xD2;                                                     \
    address = 0x00000018;                                                     \
    set_cpu_mode(MODE_IRQ);                                                   \
  }                                                                           \

#define generate_load_reg_pc(ireg, reg_index, pc_offset)                      \
  if(reg_index == 15)                                                         \
  {                                                                           \
    generate_load_pc(ireg, pc + pc_offset);                                   \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    generate_load_reg(ireg, reg_index);                                       \
  }                                                                           \

#define generate_store_reg_pc_no_flags(ireg, reg_index)                       \
  generate_store_reg(ireg, reg_index);                                        \
  if(reg_index == 15)                                                         \
  {                                                                           \
    generate_mov(a0, ireg);                                                   \
    generate_indirect_branch_arm();                                           \
  }                                                                           \

u32 function_cc execute_spsr_restore(u32 address)
{
  reg[REG_CPSR] = spsr[reg[CPU_MODE]];
  extract_flags();
  set_cpu_mode(cpu_modes[reg[REG_CPSR] & 0x1F]);
  check_for_interrupts();

  if(reg[REG_CPSR] & 0x20)
    address |= 0x01;

  return address;
}

#define generate_store_reg_pc_flags(ireg, reg_index)                          \
  generate_store_reg(ireg, reg_index);                                        \
  if(reg_index == 15)                                                         \
  {                                                                           \
    generate_mov(a0, ireg);                                                   \
    generate_function_call(execute_spsr_restore);                             \
    generate_mov(a0, rv);                                                     \
    generate_indirect_branch_dual();                                          \
  }                                                                           \

typedef enum
{
  CONDITION_TRUE,
  CONDITION_FALSE,
  CONDITION_EQUAL,
  CONDITION_NOT_EQUAL
} condition_check_type;


#define generate_condition_eq(ireg_a, ireg_b)                                 \
  generate_load_reg(ireg_a, REG_Z_FLAG);                                      \
  condition_check = CONDITION_TRUE                                            \

#define generate_condition_ne(ireg_a, ireg_b)                                 \
  generate_load_reg(ireg_a, REG_Z_FLAG);                                      \
  condition_check = CONDITION_FALSE                                           \

#define generate_condition_cs(ireg_a, ireg_b)                                 \
  generate_load_reg(ireg_a, REG_C_FLAG);                                      \
  condition_check = CONDITION_TRUE                                            \

#define generate_condition_cc(ireg_a, ireg_b)                                 \
  generate_load_reg(ireg_a, REG_C_FLAG);                                      \
  condition_check = CONDITION_FALSE                                           \

#define generate_condition_mi(ireg_a, ireg_b)                                 \
  generate_load_reg(ireg_a, REG_N_FLAG);                                      \
  condition_check = CONDITION_TRUE                                            \

#define generate_condition_pl(ireg_a, ireg_b)                                 \
  generate_load_reg(ireg_a, REG_N_FLAG);                                      \
  condition_check = CONDITION_FALSE                                           \

#define generate_condition_vs(ireg_a, ireg_b)                                 \
  generate_load_reg(ireg_a, REG_V_FLAG);                                      \
  condition_check = CONDITION_TRUE                                            \

#define generate_condition_vc(ireg_a, ireg_b)                                 \
  generate_load_reg(ireg_a, REG_V_FLAG);                                      \
  condition_check = CONDITION_FALSE                                           \

#define generate_condition_hi(ireg_a, ireg_b)                                 \
  generate_load_reg(ireg_a, REG_C_FLAG);                                      \
  generate_xor_imm(ireg_a, 1);                                                \
  generate_load_reg(ireg_b, REG_Z_FLAG);                                      \
  generate_or(ireg_a, ireg_b);                                                \
  condition_check = CONDITION_FALSE                                           \

#define generate_condition_ls(ireg_a, ireg_b)                                 \
  generate_load_reg(ireg_a, REG_C_FLAG);                                      \
  generate_xor_imm(ireg_a, 1);                                                \
  generate_load_reg(ireg_b, REG_Z_FLAG);                                      \
  generate_or(ireg_a, ireg_b);                                                \
  condition_check = CONDITION_TRUE                                            \

#define generate_condition_ge(ireg_a, ireg_b)                                 \
  generate_load_reg(ireg_a, REG_N_FLAG);                                      \
  generate_load_reg(ireg_b, REG_V_FLAG);                                      \
  condition_check = CONDITION_EQUAL                                           \

#define generate_condition_lt(ireg_a, ireg_b)                                 \
  generate_load_reg(ireg_a, REG_N_FLAG);                                      \
  generate_load_reg(ireg_b, REG_V_FLAG);                                      \
  condition_check = CONDITION_NOT_EQUAL                                       \

#define generate_condition_gt(ireg_a, ireg_b)                                 \
  generate_load_reg(ireg_a, REG_N_FLAG);                                      \
  generate_load_reg(ireg_b, REG_V_FLAG);                                      \
  generate_xor(ireg_b, ireg_a);                                               \
  generate_load_reg(a0, REG_Z_FLAG);                                          \
  generate_or(ireg_a, ireg_b);                                                \
  condition_check = CONDITION_FALSE                                           \

#define generate_condition_le(ireg_a, ireg_b)                                 \
  generate_load_reg(ireg_a, REG_N_FLAG);                                      \
  generate_load_reg(ireg_b, REG_V_FLAG);                                      \
  generate_xor(ireg_b, ireg_a);                                               \
  generate_load_reg(a0, REG_Z_FLAG);                                          \
  generate_or(ireg_a, ireg_b);                                                \
  condition_check = CONDITION_TRUE                                            \


#define generate_condition(ireg_a, ireg_b)                                    \
  switch(condition)                                                           \
  {                                                                           \
    case 0x0:                                                                 \
      generate_condition_eq(ireg_a, ireg_b);                                  \
      break;                                                                  \
                                                                              \
    case 0x1:                                                                 \
      generate_condition_ne(ireg_a, ireg_b);                                  \
      break;                                                                  \
                                                                              \
    case 0x2:                                                                 \
      generate_condition_cs(ireg_a, ireg_b);                                  \
      break;                                                                  \
                                                                              \
    case 0x3:                                                                 \
      generate_condition_cc(ireg_a, ireg_b);                                  \
      break;                                                                  \
                                                                              \
    case 0x4:                                                                 \
      generate_condition_mi(ireg_a, ireg_b);                                  \
      break;                                                                  \
                                                                              \
    case 0x5:                                                                 \
      generate_condition_pl(ireg_a, ireg_b);                                  \
      break;                                                                  \
                                                                              \
    case 0x6:                                                                 \
      generate_condition_vs(ireg_a, ireg_b);                                  \
      break;                                                                  \
                                                                              \
    case 0x7:                                                                 \
      generate_condition_vc(ireg_a, ireg_b);                                  \
      break;                                                                  \
                                                                              \
    case 0x8:                                                                 \
      generate_condition_hi(ireg_a, ireg_b);                                  \
      break;                                                                  \
                                                                              \
    case 0x9:                                                                 \
      generate_condition_ls(ireg_a, ireg_b);                                  \
      break;                                                                  \
                                                                              \
    case 0xA:                                                                 \
      generate_condition_ge(ireg_a, ireg_b);                                  \
      break;                                                                  \
                                                                              \
    case 0xB:                                                                 \
      generate_condition_lt(ireg_a, ireg_b);                                  \
      break;                                                                  \
                                                                              \
    case 0xC:                                                                 \
      generate_condition_gt(ireg_a, ireg_b);                                  \
      break;                                                                  \
                                                                              \
    case 0xD:                                                                 \
      generate_condition_le(ireg_a, ireg_b);                                  \
      break;                                                                  \
                                                                              \
    case 0xE:                                                                 \
      /* AL       */                                                          \
      break;                                                                  \
                                                                              \
    case 0xF:                                                                 \
      /* Reserved */                                                          \
      break;                                                                  \
  }                                                                           \

#define generate_conditional_branch_type(ireg_a, ireg_b)                      \
  switch(condition_check)                                                     \
  {                                                                           \
    case CONDITION_TRUE:                                                      \
      generate_conditional_branch(ireg_a, ireg_b, true, backpatch_address);   \
      break;                                                                  \
                                                                              \
    case CONDITION_FALSE:                                                     \
      generate_conditional_branch(ireg_a, ireg_b, false, backpatch_address);  \
      break;                                                                  \
                                                                              \
    case CONDITION_EQUAL:                                                     \
      generate_conditional_branch(ireg_a, ireg_b, equal, backpatch_address);  \
      break;                                                                  \
                                                                              \
    case CONDITION_NOT_EQUAL:                                                 \
      generate_conditional_branch(ireg_a, ireg_b, not_equal,                  \
       backpatch_address);                                                    \
      break;                                                                  \
  }                                                                           \

#define generate_branch()                                                     \
{                                                                             \
  generate_branch_cycle_update(                                               \
   block_exits[block_exit_position].branch_source,                            \
   block_exits[block_exit_position].branch_target);                           \
  block_exit_position++;                                                      \
}                                                                             \

#define rm_op_reg rm
#define rm_op_imm imm

#define arm_data_proc_reg_flags()                                             \
  arm_decode_data_proc_reg();                                                 \
  if(flag_status & 0x02)                                                      \
  {                                                                           \
    generate_load_rm_sh(flags)                                                \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    generate_load_rm_sh(no_flags);                                            \
  }                                                                           \

#define arm_data_proc_reg()                                                   \
  arm_decode_data_proc_reg();                                                 \
  generate_load_rm_sh(no_flags)                                               \

#define arm_data_proc_imm()                                                   \
  arm_decode_data_proc_imm();                                                 \
  generate_load_imm(a0, imm)                                                  \

#define arm_data_proc(name, type, flags_op)                                   \
{                                                                             \
  arm_data_proc_##type();                                                     \
  generate_load_reg_pc(a1, rn, 8);                                            \
  generate_function_call(execute_##name);                                     \
  generate_store_reg_pc_##flags_op(rv, rd);                                   \
}                                                                             \

#define arm_data_proc_test(name, type)                                        \
{                                                                             \
  arm_data_proc_##type();                                                     \
  generate_load_reg_pc(a1, rn, 8);                                            \
  generate_function_call(execute_##name);                                     \
}                                                                             \

#define arm_data_proc_unary(name, type, flags_op)                             \
{                                                                             \
  arm_data_proc_##type();                                                     \
  generate_function_call(execute_##name);                                     \
  generate_store_reg_pc_##flags_op(rv, rd);                                   \
}                                                                             \

#define arm_data_proc_mov(type)                                               \
{                                                                             \
  arm_data_proc_##type();                                                     \
  generate_store_reg_pc_no_flags(a0, rd);                                     \
}                                                                             \

u32 function_cc execute_mul_flags(u32 dest)
{
  calculate_z_flag(dest);
  calculate_n_flag(dest);
}

#define arm_multiply_flags_yes()                                              \
  generate_function_call(execute_mul_flags)                                   \

#define arm_multiply_flags_no(_dest)                                          \

#define arm_multiply_add_no()                                                 \

#define arm_multiply_add_yes()                                                \
  generate_load_reg(a1, rn);                                                  \
  generate_add(a0, a1)                                                        \

#define arm_multiply(add_op, flags)                                           \
{                                                                             \
  arm_decode_multiply();                                                      \
  generate_load_reg(a0, rm);                                                  \
  generate_load_reg(a1, rs);                                                  \
  generate_multiply(a1);                                                      \
  arm_multiply_add_##add_op();                                                \
  generate_store_reg(a0, rd);                                                 \
  arm_multiply_flags_##flags();                                               \
}                                                                             \

u32 function_cc execute_mul_long_flags(u32 dest_lo, u32 dest_hi)
{
  reg[REG_Z_FLAG] = (dest_lo == 0) & (dest_hi == 0);
  calculate_n_flag(dest_hi);
}

#define arm_multiply_long_flags_yes()                                         \
  generate_function_call(execute_mul_long_flags)                              \

#define arm_multiply_long_flags_no(_dest)                                     \

#define arm_multiply_long_add_yes(name)                                       \
  generate_load_reg(a2, rdlo);                                                \
  generate_load_reg(s0, rdhi);                                                \
  generate_multiply_##name(a1, a2, s0)                                        \

#define arm_multiply_long_add_no(name)                                        \
  generate_multiply_##name(a1)                                                \

#define arm_multiply_long(name, add_op, flags)                                \
{                                                                             \
  arm_decode_multiply_long();                                                 \
  generate_load_reg(a0, rm);                                                  \
  generate_load_reg(a1, rs);                                                  \
  arm_multiply_long_add_##add_op(name);                                       \
  generate_store_reg(a0, rdlo);                                               \
  generate_store_reg(a1, rdhi);                                               \
  arm_multiply_long_flags_##flags();                                          \
}                                                                             \

u32 function_cc execute_read_cpsr()
{
  collapse_flags();
  return reg[REG_CPSR];
}

u32 function_cc execute_read_spsr()
{
  collapse_flags();
  return spsr[reg[CPU_MODE]];
}

#define arm_psr_read(op_type, psr_reg)                                        \
  generate_function_call(execute_read_##psr_reg);                             \
  generate_store_reg(rv, rd)                                                  \

void function_cc execute_store_cpsr(u32 new_cpsr, u32 store_mask)
{
  reg[REG_CPSR] = (new_cpsr & store_mask) | (reg[REG_CPSR] & (~store_mask));
  extract_flags();
  if(store_mask & 0xFF)
  {
    set_cpu_mode(cpu_modes[reg[REG_CPSR] & 0x1F]);
    // TODO: check for interrupts, since this can change PC it has to be
    // cased in ASM
  }
}

void function_cc execute_store_spsr(u32 new_spsr, u32 store_mask)
{
  u32 _spsr = spsr[reg[CPU_MODE]];
  spsr[reg[CPU_MODE]] = (new_spsr & store_mask) | (_spsr & (~store_mask));
}

#define arm_psr_load_new_reg()                                                \
  generate_load_reg(a0, rm)                                                   \

#define arm_psr_load_new_imm()                                                \
  generate_load_imm(a0, imm)                                                  \

#define arm_psr_store(op_type, psr_reg)                                       \
  arm_psr_load_new_##op_type();                                               \
  generate_load_imm(a1, psr_masks[psr_field]);                                \
  generate_function_call(execute_store_##psr_reg)                             \

#define arm_psr(op_type, transfer_type, psr_reg)                              \
{                                                                             \
  arm_decode_psr_##op_type();                                                 \
  arm_psr_##transfer_type(op_type, psr_reg);                                  \
}                                                                             \

#define aligned_address_mask8  0xF0000000
#define aligned_address_mask16 0xF0000001
#define aligned_address_mask32 0xF0000003

#define read_memory(size, type, address, dest)                                \
{                                                                             \
  u8 *map;                                                                    \
  if(((address & aligned_address_mask##size) == 0) &&                         \
   (map = memory_map_read[address >> 15]))                                    \
  {                                                                           \
    dest = *((type *)((u8 *)map + (address & 0x7FFF)));                       \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    dest = (type)read_memory##size(address);                                  \
  }                                                                           \
}                                                                             \

#define read_memory_s16(address, dest)                                        \
{                                                                             \
  u8 *map;                                                                    \
  if(((address & aligned_address_mask16) == 0) &&                             \
   (map = memory_map_read[address >> 15]))                                    \
  {                                                                           \
    dest = *((s16 *)((u8 *)map + (address & 0x7FFF)));                        \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    dest = (s16)read_memory16_signed(address);                                \
  }                                                                           \
}                                                                             \

#define access_memory_generate_read_function(mem_size, mem_type)              \
u32 function_cc execute_load_##mem_type(u32 address)                          \
{                                                                             \
  u32 dest;                                                                   \
  read_memory(mem_size, mem_type, address, dest);                             \
  return dest;                                                                \
}                                                                             \

access_memory_generate_read_function(8, u8);
access_memory_generate_read_function(8, s8);
access_memory_generate_read_function(16, u16);
access_memory_generate_read_function(32, u32);

u32 function_cc execute_load_s16(u32 address)
{
  u32 dest;
  read_memory_s16(address, dest);
  return dest;
}

#define access_memory_generate_write_function(mem_size, mem_type)             \
void function_cc execute_store_##mem_type(u32 address, u32 source)            \
{                                                                             \
  u8 *map;                                                                    \
  if(((address & aligned_address_mask##mem_size) == 0) &&                     \
   (map = memory_map_write[address >> 15]))                                   \
  {                                                                           \
    *((mem_type *)((u8 *)map + (address & 0x7FFF))) = source;                 \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    write_memory##mem_size(address, source);                                  \
  }                                                                           \
}                                                                             \

#define arm_access_memory_load(mem_type)                                      \
  generate_function_call(execute_load_##mem_type);                            \
  generate_store_reg_pc_no_flags(rv, rd)                                      \

#define arm_access_memory_store(mem_type)                                     \
  generate_load_reg_pc(a1, rd, 12);                                           \
  generate_load_pc(a2, (pc + 4));                                             \
  generate_function_call(execute_store_##mem_type)                            \

#define no_op                                                                 \

#define arm_access_memory_writeback_yes(off_op)                               \
  reg[rn] = address off_op                                                    \

#define arm_access_memory_writeback_no(off_op)                                \

#define load_reg_op reg[rd]                                                   \

#define store_reg_op reg_op                                                   \

#define arm_access_memory_adjust_op_up      add
#define arm_access_memory_adjust_op_down    sub
#define arm_access_memory_reverse_op_up     sub
#define arm_access_memory_reverse_op_down   add

#define arm_access_memory_reg_pre(adjust_dir_op, reverse_dir_op)              \
  generate_load_reg_pc(a0, rn, 8);                                            \
  generate_##adjust_dir_op(a0, a1)                                            \

#define arm_access_memory_reg_pre_wb(adjust_dir_op, reverse_dir_op)           \
  arm_access_memory_reg_pre(adjust_dir_op, reverse_dir_op);                   \
  generate_store_reg(a0, rn)                                                  \

#define arm_access_memory_reg_post(adjust_dir_op, reverse_dir_op)             \
  generate_load_reg(a0, rn);                                                  \
  generate_##adjust_dir_op(a0, a1);                                           \
  generate_store_reg(a0, rn);                                                 \
  generate_##reverse_dir_op(a0, a1)                                           \

#define arm_access_memory_imm_pre(adjust_dir_op, reverse_dir_op)              \
  generate_load_reg_pc(a0, rn, 8);                                            \
  generate_##adjust_dir_op##_imm(a0, offset)                                  \

#define arm_access_memory_imm_pre_wb(adjust_dir_op, reverse_dir_op)           \
  arm_access_memory_imm_pre(adjust_dir_op, reverse_dir_op);                   \
  generate_store_reg(a0, rn)                                                  \

#define arm_access_memory_imm_post(adjust_dir_op, reverse_dir_op)             \
  generate_load_reg(a0, rn);                                                  \
  generate_##adjust_dir_op##_imm(a0, offset);                                 \
  generate_store_reg(a0, rn);                                                 \
  generate_##reverse_dir_op##_imm(a0, offset)                                 \


#define arm_data_trans_reg(adjust_op, adjust_dir_op, reverse_dir_op)          \
  arm_decode_data_trans_reg();                                                \
  generate_load_offset_sh();                                                  \
  arm_access_memory_reg_##adjust_op(adjust_dir_op, reverse_dir_op)            \

#define arm_data_trans_imm(adjust_op, adjust_dir_op, reverse_dir_op)          \
  arm_decode_data_trans_imm();                                                \
  arm_access_memory_imm_##adjust_op(adjust_dir_op, reverse_dir_op)            \

#define arm_data_trans_half_reg(adjust_op, adjust_dir_op, reverse_dir_op)     \
  arm_decode_half_trans_r();                                                  \
  generate_load_reg(a1, rm);                                                  \
  arm_access_memory_reg_##adjust_op(adjust_dir_op, reverse_dir_op)            \

#define arm_data_trans_half_imm(adjust_op, adjust_dir_op, reverse_dir_op)     \
  arm_decode_half_trans_of();                                                 \
  arm_access_memory_imm_##adjust_op(adjust_dir_op, reverse_dir_op)            \

#define arm_access_memory(access_type, direction, adjust_op, mem_type,        \
 offset_type)                                                                 \
{                                                                             \
  arm_data_trans_##offset_type(adjust_op,                                     \
   arm_access_memory_adjust_op_##direction,                                   \
   arm_access_memory_reverse_op_##direction);                                 \
                                                                              \
  arm_access_memory_##access_type(mem_type);                                  \
}                                                                             \

#define word_bit_count(word)                                                  \
  (bit_count[word >> 8] + bit_count[word & 0xFF])                             \

#define arm_block_address_preadjust_up_full()                                 \
  generate_add_imm(s0, (word_bit_count(reg_list) * 4))                        \

#define arm_block_address_preadjust_up()                                      \
  generate_add_imm(s0, 4)                                                     \

#define arm_block_address_preadjust_down_full()                               \
  generate_sub_imm(s0, (word_bit_count(reg_list) * 4))                        \

#define arm_block_address_preadjust_down()                                    \
  generate_sub_imm(s0, ((word_bit_count(reg_list) * 4) - 4))                  \

#define arm_block_address_preadjust_no()                                      \

#define arm_block_address_postadjust_no()                                     \

#define arm_block_address_postadjust_up()                                     \
  generate_add_imm(a0, (word_bit_count(reg_list) * 4))                        \

#define arm_block_address_postadjust_down()                                   \
  generate_sub_imm(a0, (word_bit_count(reg_list) * 4))                        \

#define sprint_no(access_type, pre_op, post_op, wb)                           \

#define sprint_yes(access_type, pre_op, post_op, wb)                          \
  /* printf("sbit on %s %s %s %s\n", #access_type, #pre_op, #post_op, #wb)*/   \

#define arm_block_writeback_yes(access_type)                                  \
  generate_store_reg(a0, rn)                                                  \

#define arm_block_writeback_no(access_type)                                   \

u32 function_cc execute_aligned_load32(u32 address)
{
  u8 *map;
  if(!(address & 0xF0000000) && (map = memory_map_read[address >> 15]))
    return address32(map, address & 0x7FFF);
  else
    return read_memory32(address);
}

void function_cc execute_aligned_store32(u32 address, u32 source)
{
  u8 *map;

  if(!(address & 0xF0000000) && (map = memory_map_write[address >> 15]))
    address32(map, address & 0x7FFF) = source;
  else
    write_memory32(address, source);
}

#define arm_block_memory_load()                                               \
  generate_function_call(execute_aligned_load32);                             \
  generate_store_reg(rv, i)                                                   \

#define arm_block_memory_store()                                              \
  generate_load_reg_pc(a1, i, 8);                                             \
  generate_function_call(execute_aligned_store32)                             \

#define arm_block_memory_final_load()                                         \
  arm_block_memory_load()                                                     \

#define arm_block_memory_final_store()                                        \
  generate_load_reg_pc(a1, i, 12);                                            \
  generate_load_pc(a2, (pc + 4));                                             \
  generate_function_call(execute_store_u32)                                   \

#define arm_block_memory_adjust_pc_store()                                    \

#define arm_block_memory_adjust_pc_load()                                     \
  if(reg_list & 0x8000)                                                       \
  {                                                                           \
    generate_mov(a0, rv);                                                     \
    generate_indirect_branch_arm();                                           \
  }                                                                           \

#define arm_block_memory(access_type, pre_op, post_op, wb, s_bit)             \
{                                                                             \
  arm_decode_block_trans();                                                   \
  u32 i;                                                                      \
  u32 offset = 0;                                                             \
                                                                              \
  generate_load_reg(s0, rn);                                                  \
  generate_and_imm(s0, ~0x03);                                                \
  arm_block_address_preadjust_##pre_op();                                     \
  generate_mov(a0, s0);                                                       \
  arm_block_address_postadjust_##post_op();                                   \
  arm_block_writeback_##wb(access_type);                                      \
                                                                              \
  sprint_##s_bit(access_type, pre_op, post_op, wb);                           \
                                                                              \
  for(i = 0; i < 16; i++)                                                     \
  {                                                                           \
    if((reg_list >> i) & 0x01)                                                \
    {                                                                         \
      generate_add_reg_reg_imm(a0, s0, offset)                                \
      if(reg_list & ~((2 << i) - 1))                                          \
      {                                                                       \
        arm_block_memory_##access_type();                                     \
        offset += 4;                                                          \
      }                                                                       \
      else                                                                    \
      {                                                                       \
        arm_block_memory_final_##access_type();                               \
      }                                                                       \
    }                                                                         \
  }                                                                           \
                                                                              \
  arm_block_memory_adjust_pc_##access_type();                                 \
}                                                                             \

#define arm_swap(type)                                                        \
{                                                                             \
  arm_decode_swap();                                                          \
  generate_load_reg(a0, rn);                                                  \
  generate_function_call(execute_load_##type);                                \
  generate_mov(s0, rv);                                                       \
  generate_load_reg(a0, rn);                                                  \
  generate_load_reg(a1, rm);                                                  \
  generate_function_call(execute_store_##type);                               \
  generate_store_reg(s0, rd);                                                 \
}                                                                             \

#define thumb_rn_op_reg(_rn)                                                  \
  generate_load_reg(a0, _rn)                                                  \

#define thumb_rn_op_imm(_imm)                                                 \
  generate_load_imm(a0, _imm)                                                 \

// Types: add_sub, add_sub_imm, alu_op, imm
// Affects N/Z/C/V flags

#define thumb_data_proc(type, name, rn_type, _rd, _rs, _rn)                   \
{                                                                             \
  thumb_decode_##type();                                                      \
  thumb_rn_op_##rn_type(_rn);                                                 \
  generate_load_reg(a1, _rs);                                                 \
  generate_function_call(execute_##name);                                     \
  generate_store_reg(rv, _rd);                                                \
}                                                                             \

#define thumb_data_proc_test(type, name, rn_type, _rs, _rn)                   \
{                                                                             \
  thumb_decode_##type();                                                      \
  thumb_rn_op_##rn_type(_rn);                                                 \
  generate_load_reg(a1, _rs);                                                 \
  generate_function_call(execute_##name);                                     \
}                                                                             \

#define thumb_data_proc_unary(type, name, rn_type, _rd, _rn)                  \
{                                                                             \
  thumb_decode_##type();                                                      \
  thumb_rn_op_##rn_type(_rn);                                                 \
  generate_function_call(execute_##name);                                     \
  generate_store_reg(rv, _rd);                                                \
}                                                                             \

#define thumb_data_proc_mov(type, rn_type, _rd, _rn)                          \
{                                                                             \
  thumb_decode_##type();                                                      \
  thumb_rn_op_##rn_type(_rn);                                                 \
  generate_store_reg(a0, _rd);                                                \
}                                                                             \

#define generate_store_reg_pc_thumb(ireg)                                     \
  generate_store_reg(ireg, rd);                                               \
  if(rd == 15)                                                                \
  {                                                                           \
    generate_indirect_branch_cycle_update(thumb);                             \
  }                                                                           \

#define thumb_data_proc_hi(name)                                              \
{                                                                             \
  thumb_decode_hireg_op();                                                    \
  generate_load_reg_pc(a0, rs, 4);                                            \
  generate_load_reg_pc(a1, rd, 4);                                            \
  generate_function_call(execute_##name);                                     \
  generate_store_reg_pc_thumb(rv);                                            \
}                                                                             \

#define thumb_data_proc_test_hi(name)                                         \
{                                                                             \
  thumb_decode_hireg_op();                                                    \
  generate_load_reg_pc(a0, rs, 4);                                            \
  generate_load_reg_pc(a1, rd, 4);                                            \
  generate_function_call(execute_##name);                                     \
}                                                                             \

#define thumb_data_proc_unary_hi(name)                                        \
{                                                                             \
  thumb_decode_hireg_op();                                                    \
  generate_load_reg_pc(a0, rn, 4);                                            \
  generate_function_call(execute_##name);                                     \
  generate_store_reg_pc_thumb(rv);                                            \
}                                                                             \

#define thumb_data_proc_mov_hi()                                              \
{                                                                             \
  thumb_decode_hireg_op();                                                    \
  generate_load_reg_pc(a0, rs, 4);                                            \
  generate_store_reg_pc_thumb(a0);                                            \
}                                                                             \

#define thumb_load_pc(_rd)                                                    \
{                                                                             \
  thumb_decode_imm();                                                         \
  generate_load_pc(a0, (((pc & ~2) + 4) + (imm * 4)));                        \
  generate_store_reg(a0, _rd);                                                \
}                                                                             \

#define thumb_load_sp(_rd)                                                    \
{                                                                             \
  thumb_decode_imm();                                                         \
  generate_load_reg(a0, 13);                                                  \
  generate_add_imm(a0, (imm * 4));                                            \
  generate_store_reg(a0, _rd);                                                \
}                                                                             \

#define thumb_adjust_sp(value)                                                \
{                                                                             \
  thumb_decode_add_sp();                                                      \
  generate_load_reg(a0, 13);                                                  \
  generate_add_imm(a0, (value));                                              \
  generate_store_reg(a0, 13);                                                 \
}                                                                             \

// Decode types: shift, alu_op
// Operation types: lsl, lsr, asr, ror
// Affects N/Z/C flags

u32 function_cc execute_lsl_reg_op(u32 value, u32 shift)
{
  if(shift != 0)
  {
    if(shift > 31)
    {
      if(shift == 32)
        reg[REG_C_FLAG] = value & 0x01;
      else
        reg[REG_C_FLAG] = 0;

      value = 0;
    }
    else
    {
      reg[REG_C_FLAG] = (value >> (32 - shift)) & 0x01;
      value <<= shift;
    }
  }

  calculate_flags_logic(value);
  return value;
}

u32 function_cc execute_lsr_reg_op(u32 value, u32 shift)
{
  if(shift != 0)
  {
    if(shift > 31)
    {
      if(shift == 32)
        reg[REG_C_FLAG] = (value >> 31) & 0x01;
      else
        reg[REG_C_FLAG] = 0;

      value = 0;
    }
    else
    {
      reg[REG_C_FLAG] = (value >> (shift - 1)) & 0x01;
      value >>= shift;
    }
  }

  calculate_flags_logic(value);
  return value;
}

u32 function_cc execute_asr_reg_op(u32 value, u32 shift)
{
  if(shift != 0)
  {
    if(shift > 31)
    {
      value = (s32)value >> 31;
      reg[REG_C_FLAG] = value & 0x01;
    }
    else
    {
      reg[REG_C_FLAG] = (value >> (shift - 1)) & 0x01;
      value = (s32)value >> shift;
    }
  }

  calculate_flags_logic(value);
  return value;
}

u32 function_cc execute_ror_reg_op(u32 value, u32 shift)
{
  if(shift != 0)
  {
    reg[REG_C_FLAG] = (value >> (shift - 1)) & 0x01;
    ror(value, value, shift);
  }

  calculate_flags_logic(value);
  return value;
}

u32 function_cc execute_lsl_imm_op(u32 value, u32 shift)
{
  if(shift != 0)
  {
    reg[REG_C_FLAG] = (value >> (32 - shift)) & 0x01;
    value <<= shift;
  }

  calculate_flags_logic(value);
  return value;
}

u32 function_cc execute_lsr_imm_op(u32 value, u32 shift)
{
  if(shift != 0)
  {
    reg[REG_C_FLAG] = (value >> (shift - 1)) & 0x01;
    value >>= shift;
  }
  else
  {
    reg[REG_C_FLAG] = value >> 31;
    value = 0;
  }

  calculate_flags_logic(value);
  return value;
}

u32 function_cc execute_asr_imm_op(u32 value, u32 shift)
{
  if(shift != 0)
  {
    reg[REG_C_FLAG] = (value >> (shift - 1)) & 0x01;
    value = (s32)value >> shift;
  }
  else
  {
    value = (s32)value >> 31;
    reg[REG_C_FLAG] = value & 0x01;
  }

  calculate_flags_logic(value);
  return value;
}

u32 function_cc execute_ror_imm_op(u32 value, u32 shift)
{
  if(shift != 0)
  {
    reg[REG_C_FLAG] = (value >> (shift - 1)) & 0x01;
    ror(value, value, shift);
  }
  else
  {
    u32 c_flag = reg[REG_C_FLAG];
    reg[REG_C_FLAG] = value & 0x01;
    value = (value >> 1) | (c_flag << 31);
  }

  calculate_flags_logic(value);
  return value;
}

#define generate_shift_load_operands_reg()                                    \
  generate_load_reg(a0, rd);                                                  \
  generate_load_reg(a1, rs)                                                   \

#define generate_shift_load_operands_imm()                                    \
  generate_load_reg(a0, rs);                                                  \
  generate_load_imm(a1, imm)                                                  \

#define thumb_shift(decode_type, op_type, value_type)                         \
{                                                                             \
  thumb_decode_##decode_type();                                               \
  generate_shift_load_operands_##value_type();                                \
  generate_function_call(execute_##op_type##_##value_type##_op);              \
  generate_store_reg(rv, rd);                                                 \
}                                                                             \

// Operation types: imm, mem_reg, mem_imm

#define thumb_access_memory_load(mem_type, reg_rd)                            \
  generate_function_call(execute_load_##mem_type);                            \
  generate_store_reg(rv, reg_rd)                                              \

#define thumb_access_memory_store(mem_type, reg_rd)                           \
  generate_load_reg(a1, reg_rd);                                              \
  generate_load_pc(a2, (pc + 2));                                             \
  generate_function_call(execute_store_##mem_type)                            \

#define thumb_access_memory_generate_address_pc_relative(offset, reg_rb,      \
 reg_ro)                                                                      \
  generate_load_pc(a0, (offset))                                              \

#define thumb_access_memory_generate_address_reg_imm(offset, reg_rb, reg_ro)  \
  generate_load_reg(a0, reg_rb);                                              \
  generate_add_imm(a0, (offset))                                              \

#define thumb_access_memory_generate_address_reg_reg(offset, reg_rb, reg_ro)  \
  generate_load_reg(a0, reg_rb);                                              \
  generate_load_reg(a1, reg_ro);                                              \
  generate_add(a0, a1)                                                        \

#define thumb_access_memory(access_type, op_type, reg_rd, reg_rb, reg_ro,     \
 address_type, offset, mem_type)                                              \
{                                                                             \
  thumb_decode_##op_type();                                                   \
  thumb_access_memory_generate_address_##address_type(offset, reg_rb,         \
   reg_ro);                                                                   \
  thumb_access_memory_##access_type(mem_type, reg_rd);                        \
}                                                                             \

#define thumb_block_address_preadjust_up()                                    \
  generate_add_imm(s0, (bit_count[reg_list] * 4))                             \

#define thumb_block_address_preadjust_down()                                  \
  generate_sub_imm(s0, (bit_count[reg_list] * 4))                             \

#define thumb_block_address_preadjust_push_lr()                               \
  generate_sub_imm(s0, ((bit_count[reg_list] + 1) * 4))                       \

#define thumb_block_address_preadjust_no()                                    \

#define thumb_block_address_postadjust_no(base_reg)                           \
  generate_store_reg(s0, base_reg)                                            \

#define thumb_block_address_postadjust_up(base_reg)                           \
  generate_add_reg_reg_imm(a0, s0, (bit_count[reg_list] * 4));                \
  generate_store_reg(a0, base_reg)                                            \

#define thumb_block_address_postadjust_down(base_reg)                         \
  generate_mov(a0, s0);                                                       \
  generate_sub_imm(a0, (bit_count[reg_list] * 4));                            \
  generate_store_reg(a0, base_reg)                                            \

#define thumb_block_address_postadjust_pop_pc(base_reg)                       \
  generate_add_reg_reg_imm(a0, s0, ((bit_count[reg_list] + 1) * 4));          \
  generate_store_reg(a0, base_reg)                                            \

#define thumb_block_address_postadjust_push_lr(base_reg)                      \
  generate_store_reg(s0, base_reg)                                            \

#define thumb_block_memory_extra_no()                                         \

#define thumb_block_memory_extra_up()                                         \

#define thumb_block_memory_extra_down()                                       \

#define thumb_block_memory_extra_pop_pc()                                     \
  generate_add_reg_reg_imm(a0, s0, (bit_count[reg_list] * 4));                \
  generate_function_call(execute_aligned_load32);                             \
  generate_store_reg(rv, REG_PC);                                             \
  generate_mov(a0, rv);                                                       \
  generate_indirect_branch_cycle_update(thumb)                                \

#define thumb_block_memory_extra_push_lr(base_reg)                            \
  generate_add_reg_reg_imm(a0, s0, (bit_count[reg_list] * 4));                \
  generate_load_reg(a1, REG_LR);                                              \
  generate_function_call(execute_aligned_store32)                             \

#define thumb_block_memory_load()                                             \
  generate_function_call(execute_aligned_load32);                             \
  generate_store_reg(rv, i)                                                   \

#define thumb_block_memory_store()                                            \
  generate_load_reg(a1, i);                                                   \
  generate_function_call(execute_aligned_store32)                             \

#define thumb_block_memory_final_load()                                       \
  thumb_block_memory_load()                                                   \

#define thumb_block_memory_final_store()                                      \
  generate_load_reg(a1, i);                                                   \
  generate_load_pc(a2, (pc + 2));                                             \
  generate_function_call(execute_store_u32)                                   \

#define thumb_block_memory_final_no(access_type)                              \
  thumb_block_memory_final_##access_type()                                    \

#define thumb_block_memory_final_up(access_type)                              \
  thumb_block_memory_final_##access_type()                                    \

#define thumb_block_memory_final_down(access_type)                            \
  thumb_block_memory_final_##access_type()                                    \

#define thumb_block_memory_final_push_lr(access_type)                         \
  thumb_block_memory_##access_type()                                          \

#define thumb_block_memory_final_pop_pc(access_type)                          \
  thumb_block_memory_##access_type()                                          \

#define thumb_block_memory(access_type, pre_op, post_op, base_reg)            \
{                                                                             \
  thumb_decode_rlist();                                                       \
  u32 i;                                                                      \
  u32 offset = 0;                                                             \
                                                                              \
  generate_load_reg(s0, base_reg);                                            \
  generate_and_imm(s0, ~0x03);                                                \
  thumb_block_address_preadjust_##pre_op();                                   \
  thumb_block_address_postadjust_##post_op(base_reg);                         \
                                                                              \
  for(i = 0; i < 8; i++)                                                      \
  {                                                                           \
    if((reg_list >> i) & 0x01)                                                \
    {                                                                         \
      generate_add_reg_reg_imm(a0, s0, offset)                                \
      if(reg_list & ~((2 << i) - 1))                                          \
      {                                                                       \
        thumb_block_memory_##access_type();                                   \
        offset += 4;                                                          \
      }                                                                       \
      else                                                                    \
      {                                                                       \
        thumb_block_memory_final_##post_op(access_type);                      \
      }                                                                       \
    }                                                                         \
  }                                                                           \
                                                                              \
  thumb_block_memory_extra_##post_op();                                       \
}                                                                             \


#define thumb_conditional_branch(condition)                                   \
{                                                                             \
  condition_check_type condition_check;                                       \
  generate_cycle_update();                                                    \
  generate_condition_##condition(a0, a1);                                     \
  generate_conditional_branch_type(a0, a1);                                   \
  generate_branch_no_cycle_update(                                            \
   block_exits[block_exit_position].branch_source,                            \
   block_exits[block_exit_position].branch_target);                           \
  generate_branch_patch_conditional(backpatch_address, translation_ptr);      \
  block_exit_position++;                                                      \
}                                                                             \

#define flags_vars(src_a, src_b)                                              \
  u32 dest;                                                                   \
  const u32 _sa = src_a;                                                      \
  const u32 _sb = src_b                                                       \

#define data_proc_generate_logic_function(name, expr)                         \
u32 function_cc execute_##name(u32 rm, u32 rn)                                \
{                                                                             \
  return expr;                                                                \
}                                                                             \
                                                                              \
u32 function_cc execute_##name##s(u32 rm, u32 rn)                             \
{                                                                             \
  u32 dest = expr;                                                            \
  calculate_z_flag(dest);                                                     \
  calculate_n_flag(dest);                                                     \
  return expr;                                                                \
}                                                                             \

#define data_proc_generate_logic_unary_function(name, expr)                   \
u32 function_cc execute_##name(u32 rm)                                        \
{                                                                             \
  return expr;                                                                \
}                                                                             \
                                                                              \
u32 function_cc execute_##name##s(u32 rm)                                     \
{                                                                             \
  u32 dest = expr;                                                            \
  calculate_z_flag(dest);                                                     \
  calculate_n_flag(dest);                                                     \
  return expr;                                                                \
}                                                                             \


#define data_proc_generate_sub_function(name, src_a, src_b)                   \
u32 function_cc execute_##name(u32 rm, u32 rn)                                \
{                                                                             \
  return (src_a) - (src_b);                                                   \
}                                                                             \
                                                                              \
u32 function_cc execute_##name##s(u32 rm, u32 rn)                             \
{                                                                             \
  flags_vars(src_a, src_b);                                                   \
  dest = _sa - _sb;                                                           \
  calculate_flags_sub(dest, _sa, _sb);                                        \
  return dest;                                                                \
}                                                                             \

#define data_proc_generate_add_function(name, src_a, src_b)                   \
u32 function_cc execute_##name(u32 rm, u32 rn)                                \
{                                                                             \
  return (src_a) + (src_b);                                                   \
}                                                                             \
                                                                              \
u32 function_cc execute_##name##s(u32 rm, u32 rn)                             \
{                                                                             \
  flags_vars(src_a, src_b);                                                   \
  dest = _sa + _sb;                                                           \
  calculate_flags_add(dest, _sa, _sb);                                        \
  return dest;                                                                \
}                                                                             \

#define data_proc_generate_sub_test_function(name, src_a, src_b)              \
void function_cc execute_##name(u32 rm, u32 rn)                               \
{                                                                             \
  flags_vars(src_a, src_b);                                                   \
  dest = _sa - _sb;                                                           \
  calculate_flags_sub(dest, _sa, _sb);                                        \
}                                                                             \

#define data_proc_generate_add_test_function(name, src_a, src_b)              \
void function_cc execute_##name(u32 rm, u32 rn)                               \
{                                                                             \
  flags_vars(src_a, src_b);                                                   \
  dest = _sa + _sb;                                                           \
  calculate_flags_add(dest, _sa, _sb);                                        \
}                                                                             \

#define data_proc_generate_logic_test_function(name, expr)                    \
void function_cc execute_##name(u32 rm, u32 rn)                               \
{                                                                             \
  u32 dest = expr;                                                            \
  calculate_z_flag(dest);                                                     \
  calculate_n_flag(dest);                                                     \
}                                                                             \

u32 function_cc execute_neg(u32 rm)                                           \
{                                                                             \
  u32 dest = 0 - rm;                                                          \
  calculate_flags_sub(dest, 0, rm);                                           \
  return dest;                                                                \
}                                                                             \

// Execute functions

data_proc_generate_logic_function(and, rn & rm);
data_proc_generate_logic_function(eor, rn ^ rm);
data_proc_generate_logic_function(orr, rn | rm);
data_proc_generate_logic_function(bic, rn & (~rm));
data_proc_generate_logic_function(mul, rn * rm);
data_proc_generate_logic_unary_function(mov, rm);
data_proc_generate_logic_unary_function(mvn, ~rm);

data_proc_generate_sub_function(sub, rn, rm);
data_proc_generate_sub_function(rsb, rm, rn);
data_proc_generate_sub_function(sbc, rn - rm, (reg[REG_C_FLAG] ^ 1));
data_proc_generate_sub_function(rsc, rm + reg[REG_C_FLAG] - 1, rn);
data_proc_generate_add_function(add, rn, rm);
data_proc_generate_add_function(adc, rn, rm + reg[REG_C_FLAG]);

data_proc_generate_logic_test_function(tst, rn & rm);
data_proc_generate_logic_test_function(teq, rn ^ rm);
data_proc_generate_sub_test_function(cmp, rn, rm);
data_proc_generate_add_test_function(cmn, rn, rm);

u32 function_cc execute_swi(u32 pc)
{
  reg_mode[MODE_SUPERVISOR][6] = pc;
  collapse_flags();
  spsr[MODE_SUPERVISOR] = reg[REG_CPSR];
  reg[REG_CPSR] = (reg[REG_CPSR] & ~0x3F) | 0x13;
  set_cpu_mode(MODE_SUPERVISOR);
}

#define arm_conditional_block_header()                                        \
  generate_condition(a0, a1);                                                 \
  generate_conditional_branch_type(a0, a1);                                   \

#define arm_b()                                                               \
  generate_branch()                                                           \

#define arm_bl()                                                              \
  generate_update_pc((pc + 4));                                               \
  generate_store_reg(a0, REG_LR);                                             \
  generate_branch()                                                           \

#define arm_bx()                                                              \
  arm_decode_branchx();                                                       \
  generate_load_reg(a0, rn);                                                  \
  generate_indirect_branch_dual();                                            \

#define arm_swi()                                                             \
  generate_swi_hle_handler((opcode >> 16) & 0xFF);                            \
  generate_update_pc((pc + 4));                                               \
  generate_function_call(execute_swi);                                        \
  generate_branch()                                                           \

#define thumb_b()                                                             \
  generate_branch_cycle_update(                                               \
   block_exits[block_exit_position].branch_source,                            \
   block_exits[block_exit_position].branch_target);                           \
  block_exit_position++                                                       \

#define thumb_bl()                                                            \
  generate_update_pc(((pc + 2) | 0x01));                                      \
  generate_store_reg(a0, REG_LR);                                             \
  generate_branch_cycle_update(                                               \
   block_exits[block_exit_position].branch_source,                            \
   block_exits[block_exit_position].branch_target);                           \
  block_exit_position++                                                       \

#define thumb_blh()                                                           \
{                                                                             \
  thumb_decode_branch();                                                      \
  generate_update_pc(((pc + 2) | 0x01));                                      \
  generate_load_reg(a1, REG_LR);                                              \
  generate_store_reg(a0, REG_LR);                                             \
  generate_mov(a0, a1);                                                       \
  generate_add_imm(a0, (offset * 2));                                         \
  generate_indirect_branch_cycle_update(thumb);                               \
}                                                                             \

#define thumb_bx()                                                            \
{                                                                             \
  thumb_decode_hireg_op();                                                    \
  generate_load_reg_pc(a0, rs, 4);                                            \
  generate_indirect_branch_cycle_update(dual);                                \
}                                                                             \

#define thumb_swi()                                                           \
  generate_swi_hle_handler(opcode & 0xFF);                                    \
  generate_update_pc((pc + 2));                                               \
  generate_function_call(execute_swi);                                        \
  generate_branch_cycle_update(                                               \
   block_exits[block_exit_position].branch_source,                            \
   block_exits[block_exit_position].branch_target);                           \
  block_exit_position++                                                       \

u8 swi_hle_handle[256] =
{
  0x0,    // SWI 0:  SoftReset
  0x0,    // SWI 1:  RegisterRAMReset
  0x0,    // SWI 2:  Halt
  0x0,    // SWI 3:  Stop/Sleep
  0x0,    // SWI 4:  IntrWait
  0x0,    // SWI 5:  VBlankIntrWait
  0x1,    // SWI 6:  Div
  0x0,    // SWI 7:  DivArm
  0x0,    // SWI 8:  Sqrt
  0x0,    // SWI 9:  ArcTan
  0x0,    // SWI A:  ArcTan2
  0x0,    // SWI B:  CpuSet
  0x0,    // SWI C:  CpuFastSet
  0x0,    // SWI D:  GetBIOSCheckSum
  0x0,    // SWI E:  BgAffineSet
  0x0,    // SWI F:  ObjAffineSet
  0x0,    // SWI 10: BitUnpack
  0x0,    // SWI 11: LZ77UnCompWram
  0x0,    // SWI 12: LZ77UnCompVram
  0x0,    // SWI 13: HuffUnComp
  0x0,    // SWI 14: RLUnCompWram
  0x0,    // SWI 15: RLUnCompVram
  0x0,    // SWI 16: Diff8bitUnFilterWram
  0x0,    // SWI 17: Diff8bitUnFilterVram
  0x0,    // SWI 18: Diff16bitUnFilter
  0x0,    // SWI 19: SoundBias
  0x0,    // SWI 1A: SoundDriverInit
  0x0,    // SWI 1B: SoundDriverMode
  0x0,    // SWI 1C: SoundDriverMain
  0x0,    // SWI 1D: SoundDriverVSync
  0x0,    // SWI 1E: SoundChannelClear
  0x0,    // SWI 1F: MidiKey2Freq
  0x0,    // SWI 20: SoundWhatever0
  0x0,    // SWI 21: SoundWhatever1
  0x0,    // SWI 22: SoundWhatever2
  0x0,    // SWI 23: SoundWhatever3
  0x0,    // SWI 24: SoundWhatever4
  0x0,    // SWI 25: MultiBoot
  0x0,    // SWI 26: HardReset
  0x0,    // SWI 27: CustomHalt
  0x0,    // SWI 28: SoundDriverVSyncOff
  0x0,    // SWI 29: SoundDriverVSyncOn
  0x0     // SWI 2A: SoundGetJumpList
};

void swi_hle_div()
{
  s32 result = (s32)reg[0] / (s32)reg[1];
  reg[0] = result;
  reg[1] = (s32)reg[0] % (s32)reg[1];
  reg[3] = (result ^ (result >> 31)) - (result >> 31);
}

#define generate_swi_hle_handler(_swi_number)                                 \
{                                                                             \
  u32 swi_number = _swi_number;                                               \
  if(swi_hle_handle[swi_number])                                              \
  {                                                                           \
    /* Div */                                                                 \
    if(swi_number == 0x06)                                                    \
    {                                                                         \
      generate_function_call(swi_hle_div);                                    \
    }                                                                         \
    break;                                                                    \
  }                                                                           \
}                                                                             \

#define generate_translation_gate(type)                                       \
  generate_update_pc(pc);                                                     \
  generate_indirect_branch_no_cycle_update(type)                              \

#define generate_step_debug()                                                 \
  generate_load_imm(a0, pc);                                                  \
  generate_function_call(step_debug_x86)                                      \

#endif
