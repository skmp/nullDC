#include "FT8.h"
#include "XMaple.h"
#include "Maxi.h"

namespace EmulatedDevices
{

FT8::FT8(maple_subdevice_instance* instance)
	: m_instance(instance)
{
	m_FuncType = FUNCTION_TYPE8;

	m_FuncDef[1] = 0;
	m_FuncDef[2] = 0;
	m_Region = REGION_ALL;
	m_Direction = DIRECTION_UP;
	m_License = "Produced By or Under License From SEGA ENTERPRISES,LTD.";

	m_xpad = instance->port >> 6;
}

//////////////////////////////////////////////////////////////////////////
// Make them bitches!
FT8* CreateFT8(u32 DeviceID, maple_subdevice_instance* instance)
{
	switch (DeviceID)
	{
	case ID_PURUPURUPACK:
		return new Maxi(instance);
		break;

	default:
		return NULL;
		break;
	}
}

} //namespace
