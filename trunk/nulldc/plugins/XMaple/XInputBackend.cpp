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

extern xmaple_settings settings;

namespace XInput
{		

static void ScaleStickValues(unsigned char* outx, unsigned char* outy, short inx, short iny)
{
	const float kDeadZone = (float)settings.Controller.Deadzone * 327.68f;
	const int	center 	  = 0x80;

	float magnitude = sqrtf( (float)inx*inx + (float)iny*iny );		
	
	float x = inx / magnitude;
	float y = iny / magnitude;

	if (magnitude < kDeadZone)
		magnitude = 0;
	else
	{
		magnitude  = (magnitude - kDeadZone) * 32767.0f / (32767.0f - kDeadZone);
		magnitude /= 256.0f; // Reducing to 0-128 values
		
		if ( magnitude > 128.0f ) magnitude = 128; // Just for safety
	}
	
	x *= magnitude;
	y *= magnitude;
	
	*outx = (unsigned char)(center + x);
	*outy = (unsigned char)(center - y);
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

	XINPUT_VIBRATION vib;

	EmulatedDevices::FT8::UVibConfig* config = &status->config;	

	int timeLength = settings.PuruPuru.Length;	
	int intensity, intensityH;
	int intensityX, intensityHX;		
	int fm, step, stepH;	
	
	int directionOld = 0;
	int directionNew = 0; 	
	int direction;

	int cdVergence;

	while (true)
	{			
		// intensity is 0 when there's no direction == autostop.
		direction = config->Mpow - config->Ppow;					
		
		if ( direction )		
		{											
			// Get middle frequency for usage with Real Freq.
			fm = (status->srcSettings.FM1 + status->srcSettings.FM0)/2; 

			intensity = abs(direction) * 9362;
			intensityH = abs(direction) * 8192 + 8190;		

			intensityX = (intensity * settings.PuruPuru.Intensity) / 100;
			intensityHX = (intensityX * settings.PuruPuru.Intensity) / 100;

			if (intensityX > 65535) intensityX = 65535;
			if (intensityHX > 65535) intensityHX = 65535;
			
			directionOld = directionNew;
			directionNew = direction;					
								
			// Impact when motor changes direction.
			if ( directionNew * directionOld < 0 )
			{																
				vib.wRightMotorSpeed = intensityHX;
				vib.wLeftMotorSpeed  = intensityX;
			
				XInputSetState(status->currentXPad, &vib);
				Sleep(timeLength);
			}			

			// Convergence/Divergence
			if ( config->INC ) 
			{								
				cdVergence = config->EXH - config->INH;
				
				switch(cdVergence)
				{
					case -1:
						{
							step = intensity / config->INC;
							stepH = (intensityH - 8190) / config->INC;
						} break;
					case  0: step = stepH = 0; break;
					case  1:
						{
							step = (65534 - intensity) / config->INC;
							stepH = (65534 - intensityH) / config->INC;
						} break;
				}

				for(int i=0; i<=config->INC; i++)
				{
					// Intensity setting...
					
					intensityX = (intensity * settings.PuruPuru.Intensity) / 100;
					intensityHX = (intensityH * settings.PuruPuru.Intensity) / 100;

					if (intensityX > 65535) intensityX = 65535;
					if (intensityHX > 65535) intensityHX = 65535;					
					
					// Real Frequency
					if (settings.PuruPuru.UseRealFreq)
					{										
						if ( config->Freq > (u8)((fm*3)/2))	// Top 1/4
						{
							// Level up high freq motor.
							vib.wRightMotorSpeed = intensityHX;
							vib.wLeftMotorSpeed  = 0;
						}
						else if ( config->Freq < (u8)((fm*2)/3))	// Low 1/3
						{
							vib.wRightMotorSpeed = 0;	
							vib.wLeftMotorSpeed  = intensityX;	
						}
						else
						{							
							vib.wRightMotorSpeed = intensityHX;
							vib.wLeftMotorSpeed  = intensityX;
						}
					}
					else
					{
						vib.wRightMotorSpeed = intensityHX;
						vib.wLeftMotorSpeed = intensityX;
					}
	
					XInputSetState(status->currentXPad, &vib);

					intensity  = intensity  + step  * cdVergence;										
					intensityH = intensityH + stepH * cdVergence;

					Sleep(timeLength);					
				} 				

			} 
			else // End of Convergence/Divergence	
			{
				// Intensity setting
				intensityX = (intensity * settings.PuruPuru.Intensity) / 100;
				intensityHX = (intensityH * settings.PuruPuru.Intensity) / 100;

				if (intensityX > 65535) intensityX = 65535;
				if (intensityHX > 65535) intensityHX = 65535;				
				
				// Real Frequency
				if (settings.PuruPuru.UseRealFreq)
				{								
					if ( config->Freq > (u8)((fm*3)/2))	// Top 1/4
					{					
						vib.wRightMotorSpeed = intensityHX;
						vib.wLeftMotorSpeed  = 0;
					}
					else if ( config->Freq < (u8)((fm*2)/3))	// Low 1/3
					{
						vib.wRightMotorSpeed = 0;	
						vib.wLeftMotorSpeed  = intensityX;	
					}
					else
					{																				
						vib.wRightMotorSpeed = intensityHX;
						vib.wLeftMotorSpeed  = intensityX;
					}
				}
				else
				{
					vib.wRightMotorSpeed = intensityHX;
					vib.wLeftMotorSpeed = intensityX;
				}
	
				XInputSetState(status->currentXPad, &vib);
				Sleep(timeLength);
			}			

			// Stop once it's done.
			config->Ppow = config->Mpow = 0;
			
			vib.wRightMotorSpeed = 0;
			vib.wLeftMotorSpeed = 0;

			XInputSetState(status->currentXPad, &vib);			
		}	
		else Sleep(10);

	} //while(true)

} // Vibration Thread

} //namespace



