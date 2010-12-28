#include "FT8.h"
#include "Maxi.h"

#include "XMaple.h" // for debug log crapro

namespace EmulatedDevices
{

Maxi::Maxi(maple_subdevice_instance* instance)
	: FT8(instance)
{
	m_deviceID = ID_PURUPURUPACK;
	m_FuncDef[0] = 0x00000101;
	m_strName = "Puru Puru Pack";
	m_mAstandby = 0x00C8;
	m_mAmax = 0x0640;
	m_strNameEx = "Version 1.000,1998/11/10,315-6211-AH   ,Vibration Motor:1,Fm:4 E30Hz,Pow:7   ";


	// init with maxi srcSettings
	m_status.AST			= 0x13;	// AST of 5 secs
	m_status.srcSettings.VN = 1;
	m_status.srcSettings.VP = 0;
	m_status.srcSettings.VD = 0;
	m_status.srcSettings.PF = 1;
	m_status.srcSettings.CV = 1;
	m_status.srcSettings.PD = 1;
	m_status.srcSettings.OWF= 0;
	m_status.srcSettings.VA = 0;
	m_status.srcSettings.Fm0= 0x07;
	m_status.srcSettings.Fm1= 0x3B;
	m_status.currentXPad = m_xpad;
}

void Maxi::StartVibThread()
{
	DEBUG_LOG("   VIBRATION THREAD STARTING...\n");
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)XInput::VibrationThread, &m_status, 0, NULL);
	InitializeCriticalSection(&m_status.section);	
	
}

void Maxi::StopVibThread()
{				
	// Kill thread or whatever
	XInput::StopRumble(m_xpad);
	
	// It crashes when using Xmaple as main device too and closing the GUI.
	// Doesn't crash when using PuruPuru as main device, nor when closing through console.

	// TODO: Find out why. =S

	CloseHandle(m_thread);
	TerminateThread(m_thread, 0);	
	DEBUG_LOG("   VIBRATION THREAD STOPPED\n");
}

u32 Maxi::DMA(u32 Command,
			  u32* buffer_in, u32 buffer_in_len,
			  u32* buffer_out, u32& buffer_out_len)
{
	// Update the xpad the thread uses...
	EnterCriticalSection(&m_status.section);
	m_status.currentXPad = m_xpad;
	LeaveCriticalSection(&m_status.section);

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
		// This command should return ALL vibration sources' settings. we only have one at the moment...
		w32(m_FuncType);
		w32(m_status.srcSettings.U32);
		DEBUG_LOG("GetCondition\n");
		//w32(purupuru_cond.srcSettings[VN].U32);
		return DataTransfer;

	case GetMediaInfo:
		{
			if (!m_status.srcSettings.VN) // there are no vibration sources?!?!
				return TransmitAgain;
			w32(m_FuncType);
			w32(m_status.srcSettings.U32);
			u8 source = (*(++buffer_in) & 0x000000FF);
			DEBUG_LOG("GetMediaInfo for source 0x%02x\n", source);
		}
		return DataTransfer;

	case BlockRead:
		if (*(++buffer_in) == 0) // It's not looking for waveform data
		{
			// Read back the AutoStop settings
			w32(m_FuncType);
			w32(0);			// VN(1), Phase(1), Block No.(2)
			w16(0x0200);	// ASR for VN 1
			w8(m_status.AST);
		}
		else
		{
			printf("BlockRead for waveform!\n");
			printf("REPORT THIS\n");
			return CommandUnknown;
		}
		return DataTransfer;

	case BlockWrite:
		{
			// GAH this can also be used to set auto stop time so we have to support it
			if (*(++buffer_in) == 0) // It's not trying to send waveform data
			{
				int numAST = (buffer_in_len-6)/4; // Doesn't include VN(1), Phase(1), Block Number(2), or ASR(2)
				// example ASR and AST (as the game sends): 0x000c0200
				buffer_in++;
				u16 ASR = (u16)*(buffer_in) & 0x0000FFFF; // don't swap ASR because I am lazy
				if (numAST == 1 && ASR == 0x0200) // yeah, we'll handle it
				{
					EnterCriticalSection(&m_status.section);
					m_status.AST = (*(buffer_in) & 0x00FF0000) >> 16;
					LeaveCriticalSection(&m_status.section);

					DEBUG_LOG("BlockWrite set AutoStop: %f seconds\n", m_status.AST * .25);
				}
				else
				{
					printf("BlockWrite set numAST: %i ASR: 0x%04x\n", numAST, ASR);
					printf("REPORT THIS\n");
					return TransmitAgain;
				}
			}
			else
			{
				u32 buffer = *(buffer_in);
				u32 buffer2 = *(++buffer_in);
				printf("BlockWrite set waveform: 0x%08x 0x%08x...\n", buffer, buffer2);
				printf("REPORT THIS\n");
				return CommandUnknown;
			}
		}
		return DeviceReply;

	case SetCondition:
		{
			int numSources = (buffer_in_len-4)/4;
			// numsources to be set greater than number of sources which can be concurrently selected?
			if (numSources > (u8)(m_FuncDef[0]>>8))
				return TransmitAgain;

			EnterCriticalSection(&m_status.section);
			m_status.config.U32 = *(++buffer_in);
			LeaveCriticalSection(&m_status.section);

			// convergent and divergent can't be set at the same time
			if (m_status.config.INH && m_status.config.EXH)
				return TransmitAgain;

			DEBUG_LOG("   VN: %01x CNT: %01x INH: %01x Ppow: %01x EXH: %01x Mpow: %01x Freq: %02x Inc: %02x\n",
				m_status.config.VN,
				m_status.config.CNT,
				m_status.config.INH,
				m_status.config.Ppow,
				m_status.config.EXH,
				m_status.config.Mpow,
				m_status.config.FREQ,
				m_status.config.INC);
		}
		return DeviceReply;

	default:
		printf("UNKNOWN MAPLE COMMAND %d (sent to maxi)\n", Command);
		return CommandUnknown;
	}
}

} //namespace
