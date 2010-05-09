#pragma once
#include "types.h"
#include "plugins/plugin_manager.h"

//NEVER touch this name, it was inspired by the great Z
struct eMDevInf
{
	MapleDeviceDefinition* mdd;
	bool Created;
	bool Inited;
};
extern eMDevInf MapleDevices_dd[4][6];

extern maple_device_instance MapleDevices[4];

void maple_Init();
void maple_Reset(bool Manual);
void maple_Term();

void maple_vblank();