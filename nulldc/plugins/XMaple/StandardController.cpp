#include "FT0.h"
#include "StandardController.h"
#include "XInputBackend.h"
#include "XMaple.h" // for debug log crapro

namespace EmulatedDevices
{

StdController::StdController(maple_device_instance* instance)
	: FT0(instance)
{
	m_deviceID = ID_STDCONTROLLER;
	m_FuncDef[0] = 0xFE060F00;
	m_strName = "Dreamcast Controller";
	m_mAstandby = 0x01AE;
	m_mAmax = 0x01F4;
	m_strNameEx = "Version 1.000,1998/05/11,315-6125-AB   ,Analog Module: The 4th Edition. 05/08";

	m_buttonMask = 0xF901; // DPad2, C, D, Z
}

u32 StdController::DMA(u32 Command,
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
