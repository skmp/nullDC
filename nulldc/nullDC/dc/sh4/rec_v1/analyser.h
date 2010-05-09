#pragma once
#include "recompiler.h"
#include "BasicBlock.h"
#include "ops.h"
#include "dc\sh4\sh4_opcode_list.h"


void ScanCode(u32 pc,CodeRegion* to);
void AnalyseCode(BasicBlock* to);

//Init/Reset/Term
void InitAnalyser();
void ResetAnalyser();
void TermAnalyser();