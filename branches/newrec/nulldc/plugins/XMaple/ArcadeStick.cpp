#include "FT0.h"
#include "ArcadeStick.h"

#include "XMaple.h" // for debug log crapro

namespace EmulatedDevices
{

ArcadeStick::ArcadeStick(maple_device_instance* instance)
	: FT0(instance)
{
	m_deviceID = ID_ARCADESTICK;
	m_FuncDef[0] = 0xFF070000;
	m_strName = "Arcade Stick";
	m_mAstandby = 0x012C;
	m_mAmax = 0x0190;
	m_strNameEx = "Version 1.000,1998/05/25,315-6125- AC";

	m_buttonMask = 0xF800; // DPad2, D
}

u32 ArcadeStick::DMA(u32 Command,
					 u32* buffer_in, u32 buffer_in_len,
					 u32* buffer_out, u32& buffer_out_len)
{
	u8* buffer_out_b = (u8*)buffer_out;

	u32 ret;
	if ((ret = FT0::DMA(Command, buffer_in, buffer_in_len, buffer_out, buffer_out_len)) == 0)
	{
		switch (Command)
		{
		case 0: // bogus
		default:
			printf("UNKNOWN MAPLE COMMAND %d (sent to %s)\n", Command, m_strName);
			return CommandUnknown;
		}
	}
	else
		return ret;
}

} //namespace
