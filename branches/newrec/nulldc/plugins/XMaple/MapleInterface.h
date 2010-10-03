// The lowest of the low! woot
#pragma once

#include <string>
#include "..\..\nullDC\plugins\plugin_header.h"

namespace EmulatedDevices
{

enum
{
	// May be ORd
	REGION_USA	= 0x01,
	REGION_JAP	= 0x02,
	REGION_ASIA	= 0x04,
	REGION_EURO	= 0x08,
	REGION_ALL	= 0xFF, // Only use this, really...
};

enum
{
	DIRECTION_UP	= 0, // We only really use Up, anyways...
	DIRECTION_DOWN	= 1,
	DIRECTION_LEFT	= 2,
	DIRECTION_RIGHT	= 3,
};

// Maple commands and return codes
enum
{
	GetStatus		= 0x01,
	GetStatusAll,
	DeviceReset,
	DeviceKill,
	
	//return codes
	DeviceInfo,
	DeviceInfoEx,
	DeviceReply,
	DataTransfer,

	GetCondition,
	GetMediaInfo,
	BlockRead,
	BlockWrite,
	GetLastError,
	SetCondition,
	FT4Control,
	ARControl,


	//errors
	ARError		= 0xF9,
	LCDError,
	FileError,
	TransmitAgain,
	CommandUnknown,
	FuncTypeUnknown
};

class MapleInterface
{
protected:
	// Device ID
	u32 m_FuncType;
	u32 m_FuncDef[3];
	// Destination code
	u8 m_Region;
	// Connection direction
	u8 m_Direction;
	// Product name
	const char* m_strName;
	// License
	const char* m_License;
	// Standby current consumption
	u16 m_mAstandby;
	// Maximum current consumption
	u16 m_mAmax;
	// Free Device Status
	const char* m_strNameEx;

	// the device ID as determined by nullDC
	u32 m_deviceID;
	// the xpad this device is mapped to
	u8 m_xpad;

public:
	virtual u32 DMA(u32 Command,
		u32* buffer_in, u32 buffer_in_len,
		u32* buffer_out, u32& buffer_out_len) = 0;

	static u32 FASTCALL ClassDMA(void* device_instance,
		u32 Command,
		u32* buffer_in, u32 buffer_in_len,
		u32* buffer_out, u32& buffer_out_len)
	{
		//the class pointer is the device_instance, just cast it
		MapleInterface * pthis = (MapleInterface*)device_instance;
		return pthis->DMA(Command, buffer_in, buffer_in_len, buffer_out, buffer_out_len);
	}

	void SetXPad(u8 pad) {m_xpad = pad;}
	u8 GetXPad() {return m_xpad;}
	u32 GetID() {return m_deviceID;}
};

} //namespace