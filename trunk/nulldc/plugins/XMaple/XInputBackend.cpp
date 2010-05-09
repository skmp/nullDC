#define _WIN32_WINNT 0x500
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <string.h>
#include <commctrl.h>

#include "XInput.h"
#include "XInputBackend.h"
#include "XMaple.h"
#include "FT8.h"

namespace XInput
{

void ScaleStickValues(unsigned char* outx, unsigned char* outy, short inx, short iny)
{
	const float kDeadZone = 0.1f;

	float x = ((float)inx + 0.5f) / 32767.5f;
	float y = ((float)iny + 0.5f) / 32767.5f;

	if ((x == 0.0f) && (y == 0.0f)) // to be safe
	{
		*outx = 0;
		*outy = 0;
		return;
	}

	float magnitude = sqrtf(x * x + y * y);
	float nx = x / magnitude;
	float ny = y / magnitude;

	if (magnitude < kDeadZone)
		magnitude = kDeadZone;

	magnitude  = (magnitude - kDeadZone) / (1.0f - kDeadZone);
	magnitude *= magnitude; // another power may be more appropriate
	nx *= magnitude;
	ny *= magnitude;
	int ix = (int)(nx * 100);
	int iy = (int)(ny * 100);
	*outx = 0x80 + ix;
	*outy = -(0x80 + iy);
}

bool Read(int XPadPlayer, u32 deviceType, EmulatedDevices::FT0::SStatus* status)
{
	const int base = 0x80;
	XINPUT_STATE xstate;
	DWORD xresult = XInputGetState(XPadPlayer, &xstate);

	// Let's .. yes, let's use XINPUT!
	if (xresult == ERROR_SUCCESS)
	{
		const XINPUT_GAMEPAD& xpad = xstate.Gamepad;

		switch (deviceType)
		{
		case ID_STDCONTROLLER:
			ScaleStickValues(&status->joyx, &status->joyy, xpad.sThumbLX, xpad.sThumbLY);
			ScaleStickValues(&status->joy2x, &status->joy2y, xpad.sThumbRX, xpad.sThumbRY);

			status->ltrig = xpad.bLeftTrigger;
			status->rtrig = xpad.bRightTrigger;

			if (xpad.wButtons & XINPUT_GAMEPAD_A)			{status->buttons ^= CONT_BUTTON_A;}
			if (xpad.wButtons & XINPUT_GAMEPAD_B)			{status->buttons ^= CONT_BUTTON_B;}
			if (xpad.wButtons & XINPUT_GAMEPAD_X)			{status->buttons ^= CONT_BUTTON_X;}
			if (xpad.wButtons & XINPUT_GAMEPAD_Y)			{status->buttons ^= CONT_BUTTON_Y;}
			if (xpad.wButtons & XINPUT_GAMEPAD_START)		{status->buttons ^= CONT_BUTTON_START;}
			if (xpad.wButtons & XINPUT_GAMEPAD_DPAD_UP)		{status->buttons ^= CONT_DPAD_UP;}
			if (xpad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN)	{status->buttons ^= CONT_DPAD_DOWN;}
			if (xpad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT)	{status->buttons ^= CONT_DPAD_LEFT;}
			if (xpad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)	{status->buttons ^= CONT_DPAD_RIGHT;}
			break;

		case ID_TWINSTICK:
			{
				ScaleStickValues(&status->joyx, &status->joyy, xpad.sThumbLX, xpad.sThumbLY);
				ScaleStickValues(&status->joy2x, &status->joy2y, xpad.sThumbRX, xpad.sThumbRY);

				int x = status->joyx;
				int y = status->joyy;
				int x2 = status->joy2x;
				int y2 = status->joy2y;
				if (x != 0)
				{
					if (x > 168)
						status->buttons ^= CONT_DPAD_RIGHT;
					if (x < 40)
						status->buttons ^= CONT_DPAD_LEFT;
				}
				if (y != 0)
				{
					if (y > 168)
						status->buttons ^= CONT_DPAD_DOWN;
					if (y < 40)
						status->buttons ^= CONT_DPAD_UP;
				}
				if (x2 != 0)
				{
					if (x2 > 168)
						status->buttons ^= CONT_DPAD2_RIGHT;
					if (x2 < 40)
						status->buttons ^= CONT_DPAD2_LEFT;
				}
				if (y2 != 0)
				{
					if (y2 > 168)
						status->buttons ^= CONT_DPAD2_DOWN;
					if (y2 < 40)
						status->buttons ^= CONT_DPAD2_UP;
				}

				if (xpad.bLeftTrigger > 0x10)					{status->buttons ^= CONT_BUTTON_X;}
				if (xpad.bRightTrigger > 0x10)					{status->buttons ^= CONT_BUTTON_A;}
				if (xpad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB)	{status->buttons ^= CONT_BUTTON_Y;}
				if (xpad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB)	{status->buttons ^= CONT_BUTTON_B;}
				if (xpad.wButtons & XINPUT_GAMEPAD_BACK)		{status->buttons ^= CONT_BUTTON_D;}
				if (xpad.wButtons & XINPUT_GAMEPAD_START)		{status->buttons ^= CONT_BUTTON_START;}
			}
			break;

		case ID_ARCADESTICK:
			{
				ScaleStickValues(&status->joyx, &status->joyy, xpad.sThumbLX, xpad.sThumbLY);

				int x = status->joyx;
				int y = status->joyy;
				if (x != 0)
				{
					if (x > 168)
						status->buttons ^= CONT_DPAD_RIGHT;
					if (x < 40)
						status->buttons ^= CONT_DPAD_LEFT;
				}
				if (y != 0)
				{
					if (y > 168)
						status->buttons ^= CONT_DPAD_DOWN;
					if (y < 40)
						status->buttons ^= CONT_DPAD_UP;
				}

				if (xpad.wButtons & XINPUT_GAMEPAD_A)			{status->buttons ^= CONT_BUTTON_A;}
				if (xpad.wButtons & XINPUT_GAMEPAD_B)			{status->buttons ^= CONT_BUTTON_B;}
				if (xpad.wButtons & XINPUT_GAMEPAD_X)			{status->buttons ^= CONT_BUTTON_X;}
				if (xpad.wButtons & XINPUT_GAMEPAD_Y)			{status->buttons ^= CONT_BUTTON_Y;}
				if (xpad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER){status->buttons ^= CONT_BUTTON_C;}
				if (xpad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER){status->buttons ^= CONT_BUTTON_Z;}
				if (xpad.wButtons & XINPUT_GAMEPAD_START)		{status->buttons ^= CONT_BUTTON_START;}
			}
			break;
		}
		return true;
	}
	else
	{
		if (xresult == ERROR_DEVICE_NOT_CONNECTED)
			printf("x360 pad %i not connected\n", XPadPlayer);
		return false;
	}
}

bool IsConnected(int XPadPlayer)
{
	XINPUT_STATE xstate;
	DWORD xresult = XInputGetState(XPadPlayer, &xstate);
	return (xresult == ERROR_SUCCESS);
}

void StopRumble(int XPadPlayer)
{
	XInputSetState(XPadPlayer, 0);
}

void VibrationThread(void* _status)
{
	DEBUG_LOG("   VIBRATION THREAD STARTED\n");

	EmulatedDevices::FT8::SStatus* status = (EmulatedDevices::FT8::SStatus*)_status;
	u32* tickCount = &status->tickCount;
	u32* tickAutoStop = &status->tickAutoStop;
	u32* tickIncPeriod = &status->tickIncPeriod;

	// ...aaaand here is the mess ><
	while (true)
	{
	// Left = low freq, right = high freq
	XINPUT_VIBRATION vib;
	vib.wLeftMotorSpeed = 0;
	vib.wRightMotorSpeed = 0;

	WORD intensity = 0;
	u16 intensityReal = 0;

	EmulatedDevices::FT8::UVibConfig* config = &status->config;

	Sleep(1);

	EnterCriticalSection(&status->section);
	*tickCount = GetTickCount();
	*tickAutoStop = *tickCount + (status->AST * 250);
	LeaveCriticalSection(&status->section);

	u8 periodsPast = 0;
	u32 startTime = *tickCount;
	u32 timeLength = *tickAutoStop - startTime;
	if (config->INC)
	{
		EnterCriticalSection(&status->section);
		*tickIncPeriod = timeLength / config->INC;
		LeaveCriticalSection(&status->section);
	}

	if (config->CNT && *tickCount < *tickAutoStop)
	{
		if (config->Ppow)
			intensity = intensityReal = config->Ppow * (65535 / 7);
		else if (config->Mpow)
			intensity = intensityReal = config->Mpow * (65535 / 7);

		while (config->CNT && (intensity || periodsPast)) // The continuous mode loop
		{
			Sleep(1); // Force kernel to release a timeslice
			EnterCriticalSection(&status->section);
			*tickCount = GetTickCount();
			LeaveCriticalSection(&status->section);
			if (*tickCount > *tickAutoStop)
			{
				config->CNT = 0; // break loop AND don't re-enter if
				intensity = 0;
			}

			if (config->INC && (config->INH || config->EXH) && *tickCount >= ((*tickIncPeriod * periodsPast) + startTime))
			{
				// it's time to handle con/divergence
				if (config->INH)
				{
					// diminishing
					if (periodsPast == config->INC - 1)
						intensity = 0;
					else
						intensity -= intensityReal / config->INC;
				}
				else
				{
					// rising
					if (periodsPast == 0)
						intensity = 0;
					else
						intensity += intensityReal / config->INC;
				}
				periodsPast++;

				if (periodsPast == config->INC)
					config->INC = 0;
				DEBUG_LOG("    %s intensity %i pPast %i\n", config->INH?"Convergence":"Divergence", intensity,periodsPast);
			}

			if (status->useFreq)
			{
				u8 fm = status->srcSettings.FM1 - status->srcSettings.FM0;
				if (config->Freq > (u8)((fm / 4)*3))	// top 1/4
					vib.wRightMotorSpeed = intensity;
				else if (config->Freq < (u8)(fm / 3))// low 1/3
					vib.wLeftMotorSpeed = intensity;
				else							// mid
				{
					vib.wRightMotorSpeed = intensity;
					vib.wLeftMotorSpeed = intensity;
				}
			}
			else
			{
				vib.wRightMotorSpeed = intensity;
				vib.wLeftMotorSpeed = intensity;
			}
			// Actually set that crap! D:
			XInputSetState(status->currentXPad, &vib);
		}
	}
	else if (config->Mpow || config->Ppow)
	{
		if (config->Ppow)
		{
			intensity = config->Ppow * (65535 / 7);
			config->Ppow = 0;
		}
		else
		{
			intensity = config->Mpow * (65535 / 7);
			config->Mpow = 0;
		}

		if (status->useFreq)
		{
			u8 fm = status->srcSettings.FM1 - status->srcSettings.FM0;
			if (config->Freq > (u8)((fm / 4)*3))	// top 1/4
				vib.wRightMotorSpeed = intensity;
			else if (config->Freq < (u8)(fm / 3))// low 1/3
				vib.wLeftMotorSpeed = intensity;
			else							// mid
			{
				vib.wRightMotorSpeed = intensity;
				vib.wLeftMotorSpeed = intensity;
			}
		}
		else
		{
			vib.wRightMotorSpeed = intensity;
			vib.wLeftMotorSpeed = intensity;
		}
		XInputSetState(status->currentXPad, &vib);
		//Sleep(800); // x360pad spin up time (the motor is slow...) :(
	}
	else
	{
		vib.wLeftMotorSpeed = 0;
		vib.wRightMotorSpeed = 0;
		XInputSetState(status->currentXPad, &vib);
	}
	} //while(true)
}

} //namespace
