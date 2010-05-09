#include "FT4.h"
#include "XMaple.h"
#include "Mic.h"
#include "DreameyeMic.h"

namespace EmulatedDevices
{

FT4::FT4(maple_subdevice_instance* instance)
	: m_instance(instance)
{
	m_FuncType = FUNCTION_TYPE4;

	m_FuncDef[1] = 0;
	m_FuncDef[2] = 0;
	m_Region = REGION_USA|REGION_JAP|REGION_ASIA|REGION_EURO;
	m_Direction = DIRECTION_DOWN;
	m_License = "Produced By or Under License From SEGA ENTERPRISES,LTD.";

	m_xpad = instance->port >> 6;
}

//////////////////////////////////////////////////////////////////////////
// Make them bitches!
FT4* CreateFT4(u32 DeviceID, maple_subdevice_instance* instance)
{
	switch (DeviceID)
	{
	case ID_MIC:
		return new Mic(instance);
		break;

	case ID_DREAMEYEMIC:
		return new DreameyeMic(instance);
		break;

	default:
		return NULL;
		break;
	}
}

} //namespace
