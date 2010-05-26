// Maxi (puru puru pack)
#pragma once

namespace EmulatedDevices
{

class Maxi : public FT8
{
public:
	Maxi(maple_subdevice_instance* instance);

	u32 DMA(u32 Command,
		u32* buffer_in, u32 buffer_in_len,
		u32* buffer_out, u32& buffer_out_len);

	void StartVibThread();
	void StopVibThread();

	// void UseFreqScaling(bool enable);
};

} //namespace
