//dc.h
//header for dc.cpp
#pragma once
#include "types.h"
#include "mem/sh4_mem.h"
#include "mem/memutil.h"
#include "sh4/sh4_if.h"

bool Init_DC();
//this is to be called from emulation code
bool SoftReset_DC();
//this is to be called from external thread
bool Reset_DC(bool Manual);
void Term_DC();
void Start_DC();
void Stop_DC();
bool IsDCInited();
void SwitchCPU_DC();

extern void* hEmuThread;