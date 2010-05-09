#pragma once
#include "common.h"

void FASTCALL cdi_DriveReadSector(u8 * buff,u32 StartSector,u32 SectorCount,u32 secsz);
void cdi_DriveGetTocInfo(TocInfo* toc,DiskArea area);
u32 FASTCALL cdi_DriveGetDiscType();
bool cdi_init(wchar* file);
void cdi_term();
void cdi_GetSessionsInfo(SessionInfo* sessions);