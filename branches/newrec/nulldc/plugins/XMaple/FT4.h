// base class for audio input (function type 4)
// also serves as a factory
#pragma once

#include "MapleInterface.h"

namespace EmulatedDevices
{

#define FUNCTION_TYPE4		0x10000000

class FT4 : public MapleInterface
{
public:
	FT4(maple_subdevice_instance* instance);

	virtual u32 DMA(u32 Command,
		u32* buffer_in, u32 buffer_in_len,
		u32* buffer_out, u32& buffer_out_len) = 0;

	//needed to get port/etc
	maple_subdevice_instance* m_instance;
};

extern FT4* CreateFT4(u32 DeviceID, maple_subdevice_instance* instance);

} //namespace
