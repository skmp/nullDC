// base class for vibration (function type 8)
// also serves as a factory
#pragma once

#include <windows.h>
#include "MapleInterface.h"

namespace EmulatedDevices
{

#define FUNCTION_TYPE8		(1 << 16)

class FT8 : public MapleInterface
{
public:
	FT8(maple_subdevice_instance* instance);

	// pure virtual from MapleInterface, actual devices use it
	virtual u32 DMA(u32 Command,
		u32* buffer_in, u32 buffer_in_len,
		u32* buffer_out, u32& buffer_out_len) = 0;

	virtual void StartVibThread() = 0;
	virtual void StopVibThread() = 0;	

	//needed to get port/etc
	// LOOK! it's a subdevice!!
	maple_subdevice_instance* m_instance;

	// Thread to dispatch XInput settings
	void* m_thread;

	//////////////////////////////////////////////////////////////////////////
	// Common to all vibration devices
	union UVibSourceSettings
	{
		u32 U32;
		// byte swapped
		struct
		{
			u8 Vset0;
			u8 Vset1;
			u8 FM0;
			u8 FM1;
		};
		
		struct
		{
			unsigned VD		:2; // direction the source vibrates along 0: none 1: X 10: Y 11: Z
			unsigned VP		:2; // source position 0: front 1: back 10: left 11: right
			unsigned VN		:4; // number of sources (1-F) 0 is illegal

			unsigned VA		:4; // 0: max/min freq is specified 1: fixed freq 1111: not specified
			unsigned OWF	:1; // arbitrary vibration waveform can be selected
			unsigned PD		:1; // + and - directions are settable. (all + if not settable)
			unsigned CV		:1; // continuous vibration flag
			unsigned PF		:1; // 0: fixed intensity 1: variable to 8 levels

			unsigned Fm0	:8; // min or fixed freq or 0x00

			unsigned Fm1	:8; // max or 0x00
			//F=(Fm+1)/2  [Hz]	
		};
		UVibSourceSettings() {U32 = 0;}
		UVibSourceSettings(u32 _hex) {U32 = _hex;}
	};

	union UVibConfig
	{
		u32 U32;
		// byte swapped
		struct
		{
			u8 CTRL;
			u8 POW;
			u8 FREQ;
			u8 INC;
		};
		
		struct
		{
			unsigned CNT	:1; // continuous
			unsigned Res	:3; // reserved
			unsigned VN		:4; // source number (1-F) 0 is illegal

			unsigned Mpow	:3; // backward (- direction) intensity 0:0 111:-7
			unsigned EXH	:1; // divergent
			unsigned Ppow	:3; // forward (+ direction) intensity 0:0 111:+7
			unsigned INH	:1; // convergent

			unsigned Freq	:8; // frequency

			unsigned Inc	:8; // inclination
		};

		UVibConfig() {U32 = 0;}
		UVibConfig(u32 _hex) {U32 = _hex;}
	};

	struct SStatus
	{
		UVibSourceSettings srcSettings;
		UVibConfig config;
		
		u8 AST;			// 1:.25 seconds

		// Threading crap
		//u32 tickCount;
		//u32 tickAutoStop;
		//u32 tickIncPeriod;
		// To lock modifications to m_status
		CRITICAL_SECTION section;

		u8 currentXPad;
		// bool useFreq;
	};

	SStatus m_status;
};

extern FT8* CreateFT8(u32 DeviceID, maple_subdevice_instance* instance);

} //namespace