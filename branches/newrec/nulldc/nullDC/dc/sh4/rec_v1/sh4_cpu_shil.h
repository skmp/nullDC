#pragma once
#include "types.h"
#include "dc/sh4/shil/shil.h"
#include "BasicBlock.h"

void shil_DynarecInit();
bool __fastcall Scanner_FindSOM(u32 opcode,u32 pc,u32* SOM);
extern shil_stream* ilst;  