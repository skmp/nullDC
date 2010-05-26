#pragma once
#include "FT0.h"

namespace XInput
{

// Returns false if there's an xinput error - we should hot-unplug
bool Read(int XPadPlayer, u32 deviceType, EmulatedDevices::FT0::SStatus* status);

// Returns true if the pad is ready to go
bool IsConnected(int XPadPlayer);

void StopRumble(int XPadPlayer);

void VibrationThread(void* _status);

} //namespace