#pragma once

#include "types.h"

//Init/Res/Term
void ubc_Init();
void ubc_Reset(bool Manual);
void ubc_Term();


//UBC BARA 0xFF200000 0x1F200000 32 Undefined Held Held Held Iclk
extern u32 UBC_BARA;
//UBC BAMRA 0xFF200004 0x1F200004 8 Undefined Held Held Held Iclk
extern u8 UBC_BAMRA;
//UBC BBRA 0xFF200008 0x1F200008 16 0x0000 Held Held Held Iclk
extern u16 UBC_BBRA;
//UBC BARB 0xFF20000C 0x1F20000C 32 Undefined Held Held Held Iclk
extern u32 UBC_BARB;
//UBC BAMRB 0xFF200010 0x1F200010 8 Undefined Held Held Held Iclk
extern u8 UBC_BAMRB;
//UBC BBRB 0xFF200014 0x1F200014 16 0x0000 Held Held Held Iclk
extern u16 UBC_BBRB;
//UBC BDRB 0xFF200018 0x1F200018 32 Undefined Held Held Held Iclk
extern u32 UBC_BDRB;
//UBC BDMRB 0xFF20001C 0x1F20001C 32 Undefined Held Held Held Iclk
extern u32 UBC_BDMRB;
//UBC BRCR 0xFF200020 0x1F200020 16 0x0000 Held Held Held Iclk
extern u16 UBC_BRCR;
