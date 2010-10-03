#pragma once
#include "nullAICA.h"

//Get used size in the ring buffer
u32 asRingUsedCount();
//Get free size in the ring buffer
u32 asRingFreeCount();
//Read 'Count' samples from the ring buffer.Returns true if successful.
//If sz==0 then a whole buffer is read
bool asRingRead(u8* dst,u32 count=0);
bool asRingStretchRead(u8* dst,u32 sz=0);