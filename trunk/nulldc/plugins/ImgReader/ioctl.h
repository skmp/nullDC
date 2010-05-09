#pragma once
#include "common.h"

void FASTCALL ioctl_DriveReadSector(u8 * buff,u32 StartSector,u32 SectorCount,u32 secsz);
void ioctl_DriveGetTocInfo(TocInfo* toc,DiskArea area);
u32 FASTCALL ioctl_DriveGetDiscType();
bool ioctl_init(wchar* file);
void ioctl_term();
void ioctl_GetSessionsInfo(SessionInfo* sessions);