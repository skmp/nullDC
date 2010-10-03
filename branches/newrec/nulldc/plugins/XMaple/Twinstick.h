// Twinstick (svo)
#pragma once

namespace EmulatedDevices
{

class Twinstick : public FT0
{
public:
	Twinstick(maple_device_instance* instance);

	u32 DMA(u32 Command,
		u32* buffer_in, u32 buffer_in_len,
		u32* buffer_out, u32& buffer_out_len);
};

} //namespace
