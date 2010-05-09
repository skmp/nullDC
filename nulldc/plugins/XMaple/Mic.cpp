#include "FT4.h"
#include "Mic.h"

#include "XMaple.h" // for debug log crapro

namespace EmulatedDevices
{

Mic::Mic(maple_subdevice_instance* instance)
	: FT4(instance)
{
	m_deviceID = ID_MIC;
	m_FuncDef[0] = 0x0F000000;
	m_strName = "SoundInputPeripheral (S.I.P.)";
	m_mAstandby = 0x012C;
	m_mAmax = 0x012C;
	m_strNameEx = "Version 1.000,1998/05/22,315-6182      ,"
		"S4(SegaSoundSamplingSystem)/SamplingRate11.0257or8.085kHz"
		"/Bit14or8bit/NTSC only/Consumer Deveropment & Mamufacturing DIV.CSPD2-3";
}

u32 Mic::DMA(u32 Command,
	u32* buffer_in, u32 buffer_in_len,
	u32* buffer_out, u32& buffer_out_len)
{
	u8* buffer_out_b = (u8*)buffer_out;

	// Buffer overflow
	if (buffer_in_len > 8)
		return CommandUnknown;

	switch (Command)
	{
	case GetStatus:
		//functionality info
		w32(m_FuncType);
		w32(m_FuncDef[0]);
		w32(m_FuncDef[1]);
		w32(m_FuncDef[2]);
		w8(m_Region);
		w8(m_Direction);
		wString(m_strName, 30);
		wString(m_License, 60);
		w16(m_mAstandby);
		w16(m_mAmax);
		DEBUG_LOG("GetStatus\n");
		return DeviceInfo;

	case GetStatusAll:
		w32(m_FuncType);
		w32(m_FuncDef[0]);
		w32(m_FuncDef[1]);
		w32(m_FuncDef[2]);
		w8(m_Region);
		w8(m_Direction);
		wString(m_strName, 30);
		wString(m_License, 60);
		w16(m_mAstandby);
		w16(m_mAmax);
		wString(m_strNameEx, 172); // TODO verify
		DEBUG_LOG("GetStatusAll\n");
		return DeviceInfoEx;

	case GetCondition:
		return DataTransfer;

	default:
		printf("UNKNOWN MAPLE COMMAND %d (sent to %s)\n", Command, m_strName);
		return CommandUnknown;
	}
}

} //namespace
