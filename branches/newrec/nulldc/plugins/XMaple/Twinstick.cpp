#include "FT0.h"
#include "Twinstick.h"

#include "XMaple.h" // for debug log crapro

namespace EmulatedDevices
{

Twinstick::Twinstick(maple_device_instance* instance)
	: FT0(instance)
{
	m_deviceID = ID_TWINSTICK;
	m_FuncDef[0] = 0xFEFE0000;
	m_strName = "Twin Stick";
	m_mAstandby = 0x00DC;
	m_mAmax = 0x012C;
	m_strNameEx = "Version 1.000,1999/05/21,315-6211-AF   ,Ra,La,Da,Ua,Rb,Lb,Db,Ub,A,B,X,Y,D,S key";

	m_buttonMask = 0x0101; // C, Z
}

u32 Twinstick::DMA(u32 Command,
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
