#pragma once

#include "types.h"

/*
**	cfg* prototypes, if you pass NULL to a cfgSave* it will wipe out the section
**	} if you pass it to lpKey it will wipe out that particular entry
**	} if you add write to something it will create it if its not present
**	} ** Strings passed to LoadStr should be MAX_PATH in size ! **
*/

extern wchar cfgPath[512];
bool cfgOpen();
s32  EXPORT_CALL cfgLoadInt(const wchar * lpSection, const wchar * lpKey,s32 Default);
void EXPORT_CALL cfgSaveInt(const wchar * lpSection, const wchar * lpKey, s32 Int);
void EXPORT_CALL cfgLoadStr(const wchar * lpSection, const wchar * lpKey, wchar * lpReturn,const wchar* lpDefault);
void EXPORT_CALL cfgSaveStr(const wchar * lpSection, const wchar * lpKey, const wchar * lpString);
s32 EXPORT_CALL cfgExists(const wchar * Section, const wchar * Key);
void cfgSetVitual(const wchar * lpSection, const wchar * lpKey, const wchar * lpString);

