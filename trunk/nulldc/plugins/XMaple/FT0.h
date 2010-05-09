// base class for controllers (function type 0)
// also serves as a factory
#pragma once

#include "MapleInterface.h"

namespace EmulatedDevices
{

#define FUNCTION_TYPE0		(1 << 24)

// CONT_* bits
#define CONT_BUTTON_C		(1 << 0)
#define CONT_BUTTON_B		(1 << 1)
#define CONT_BUTTON_A		(1 << 2)
#define CONT_BUTTON_START	(1 << 3)
#define CONT_DPAD_UP		(1 << 4)
#define CONT_DPAD_DOWN		(1 << 5)
#define CONT_DPAD_LEFT		(1 << 6)
#define CONT_DPAD_RIGHT		(1 << 7)
#define CONT_BUTTON_Z		(1 << 8)
#define CONT_BUTTON_Y		(1 << 9)
#define CONT_BUTTON_X		(1 << 10)
#define CONT_BUTTON_D		(1 << 11)
#define CONT_DPAD2_UP		(1 << 12)
#define CONT_DPAD2_DOWN		(1 << 13)
#define CONT_DPAD2_LEFT		(1 << 14)
#define CONT_DPAD2_RIGHT	(1 << 15)

class FT0 : public MapleInterface
{
public:
	FT0(maple_device_instance* instance);

	u32 DMA(u32 Command,
		u32* buffer_in, u32 buffer_in_len,
		u32* buffer_out, u32& buffer_out_len);

	//needed to get port/etc
	maple_device_instance* m_instance;

	struct SStatus
	{					//8 bytes
		u16 buttons;	// ORd CONT_BUTTON* bits
		u8 rtrig;		// right trigger
		u8 ltrig;		// left trigger
		u8 joyx;		// joystick X
		u8 joyy;		// joystick Y
		u8 joy2x;		// second joystick X
		u8 joy2y;		// second joystick Y
	};
	SStatus m_status;

	// buttons the device DOES NOT support
	u16 m_buttonMask;
};

extern FT0* CreateFT0(u32 DeviceID, maple_device_instance* instance);

} //namespace
