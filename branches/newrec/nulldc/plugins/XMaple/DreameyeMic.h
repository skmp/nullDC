// Dreameye Mic
#pragma once

namespace EmulatedDevices
{

class DreameyeMic : public FT4
{
public:
	DreameyeMic(maple_subdevice_instance* instance);

	u32 DMA(u32 Command,
		u32* buffer_in, u32 buffer_in_len,
		u32* buffer_out, u32& buffer_out_len);
};

} //namespace

