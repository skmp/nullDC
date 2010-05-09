#include "FT0.h"
#include "XMaple.h"
#include "StandardController.h"
#include "Twinstick.h"
#include "ArcadeStick.h"

namespace EmulatedDevices
{

FT0::FT0(maple_device_instance* instance)
	: m_instance(instance)
{
	m_FuncType = FUNCTION_TYPE0;

	m_FuncDef[1] = 0;
	m_FuncDef[2] = 0;
	m_Region = REGION_ALL;
	m_Direction = DIRECTION_UP;
	m_License = "Produced By or Under License From SEGA ENTERPRISES,LTD.";

	m_xpad = instance->port >> 6;
}

u32 FT0::DMA(u32 Command,
			 u32* buffer_in, u32 buffer_in_len,
			 u32* buffer_out, u32& buffer_out_len)
{
	u8* buffer_out_b = (u8*)buffer_out;

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
		// Same as GetStatus with the extra string
		// Dunno if it's ever used...
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
		wString(m_strNameEx, 40);
		DEBUG_LOG("GetStatusAll\n");
		return DeviceInfoEx;

	case GetCondition:
		// Set base values
		m_status.buttons	= 0xFFFF; //0 = ON, 1 = OFF on the dc
		m_status.rtrig		= 0x80;
		m_status.ltrig		= 0x80;
		m_status.joyx		= 0x80;
		m_status.joyy		= 0x80;
		m_status.joy2x		= 0x80;
		m_status.joy2y		= 0x80;

		if (!XInput::Read(m_xpad, m_deviceID, &m_status))
		{
			static int numOut = 0;
			if (numOut == 0)
			{
				printf("XInput_Read for port %i - failed. hot-unplugging :)\n"
					"Sadly this is currently unsupported (by nullDC), hopefully a device can unplug itself soon ;)\n");
				numOut += 1;
			}
			//				((maple_device_instance*)device_instance)->connected = false; // TODO find the real way to do this
		}
// 		else
// 			printf("XInput_Read for port %i - %04x%02x%02x:%03u,%03u:%03u,%03u\n",
// 			port,
// 			m_status.buttons,
// 			m_status.rtrig, m_status.ltrig,
// 			m_status.joyx, m_status.joyy,
// 			m_status.joy2x, m_status.joy2y);

		w32(m_FuncType);

		// Controller condition info
		w16(m_status.buttons | m_buttonMask);
		// triggers
		w8(m_status.rtrig);
		w8(m_status.ltrig);
		// Main joystick
		w8(m_status.joyx);
		w8(m_status.joyy);
		// Second joystick
		w8(m_status.joy2x);
		w8(m_status.joy2y);

		return DataTransfer;

	default:
		// The derived class' DMA will take care of it
		return 0;
	}

	// The derived class' DMA will take care of the rest of it
	return 0;
}

//////////////////////////////////////////////////////////////////////////
// Make them bitches!
FT0* CreateFT0(u32 DeviceID, maple_device_instance* instance)
{
	switch (DeviceID)
	{
	case ID_STDCONTROLLER:
		return new StdController(instance);
		break;

	case ID_TWINSTICK:
		return new Twinstick(instance);
		break;

	case ID_ARCADESTICK:
		return new ArcadeStick(instance);
		break;

	default:
		return NULL;
		break;
	}
}

} //namespace
