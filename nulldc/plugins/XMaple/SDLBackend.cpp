/*
#define _WIN32_WINNT 0x500
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <string.h>
#include <commctrl.h>
*/
#include <SDL.h>
#include <SDL_haptic.h>
#pragma comment(lib, "SDL.lib")

#include "SDLBackend.h"
#include "XMaple.h"
#include "FT8.h"

extern xmaple_settings settings;

namespace SDL
{		

SDL_Joystick *joy;
SDL_Haptic   *rumble;

bool Init()
{
	if(SDL_Init(SDL_INIT_JOYSTICK) < 0)
	{
		printf("Xmaple => SDL Init Error! %s\n\n", SDL_GetError());
		return false;
	}
	else if(SDL_Init(SDL_INIT_HAPTIC) < 0)
	{
		printf("Xmaple => SDL Haptic Error! %s\n\n", SDL_GetError());	
		return false;
	}	

	joy = SDL_JoystickOpen(0);

	if((joy != NULL) && (SDL_JoystickIsHaptic(joy) == 1))	
	{
		rumble = SDL_HapticOpenFromJoystick(joy);	

		if(rumble == NULL)
		{
			printf("Xmaple => SDL: Could not create Haptic device.\n");
			return false;
		}
	}
	else
	{
		printf("Xmaple => SDL: Joystick or Rumble not supported.\n");
		return false;
	}
	
	return true;
}

bool Read(int XPadPlayer, u32 deviceType, EmulatedDevices::FT0::SStatus* status)
{	
	printf("Xmaple => SDL Read.\n");
	return true;	
}

bool IsConnected(int XPadPlayer)
{
	printf("Xmaple => SDL Is Connected.\n");	
	return true;
}

void StopRumble(int XPadPlayer)
{
	printf("Xmaple => SDL Stop Rumble.\n");
	
	if(rumble != NULL)
		SDL_HapticClose(rumble);
	if(joy != NULL)
		SDL_JoystickClose(joy);
	
	SDL_Quit();
}

void VibrationThread(void* _status)
{
	EmulatedDevices::FT8::SStatus* status = (EmulatedDevices::FT8::SStatus*)_status;	
	EmulatedDevices::FT8::UVibConfig* config = &status->config;	

	if(!Init()) 
	{
		StopRumble(0);
		return;
	}

	printf("Xmaple => Using SDL Rumble.\n");
	
	SDL_HapticEffect FX;
	memset(&FX, 0, sizeof(SDL_HapticEffect));

	int timeLength = settings.PuruPuru.Length;
	
	int directionOld = 0;
	int directionNew = 0; 	
	int direction;

	FX.type = SDL_HAPTIC_SINE;
	FX.periodic.direction.type = SDL_HAPTIC_POLAR;	
	FX.periodic.attack_length = 0;
	FX.periodic.attack_level = 4095;
	FX.periodic.length = 0;
	FX.periodic.period = 200;
	FX.periodic.magnitude = 0;
	FX.periodic.fade_length = 0;
	FX.periodic.fade_level = 4095;	
	
	// Create effect
	int rumbleFX = SDL_HapticNewEffect(rumble, &FX);

	while (true)
	{			
		// intensity is 0 when there's no direction == autostop.
		direction = config->Mpow - config->Ppow;					
		
		if ( direction )		
		{				

			FX.periodic.length = timeLength;
			FX.periodic.period = (800*config->Freq)/status->srcSettings.FM1;									
			
			FX.periodic.magnitude  = (4095 + abs(direction) * 4096 * settings.PuruPuru.Intensity) / 100;			
			FX.periodic.magnitude &= 0x7FFF;
			
			directionOld = directionNew;
			directionNew = direction;					
								
			// Impact when motor changes direction.
			if ( directionNew * directionOld < 0 )
			{																				
				SDL_HapticUpdateEffect(rumble, rumbleFX, &FX);
				SDL_HapticRunEffect(rumble, rumbleFX, 1);

				Sleep(timeLength);
			}			

			// Convergence/Divergence
			if ( config->INC ) 
			{												
				int longTime = timeLength * config->INC;
				
				FX.periodic.length = longTime;				
				
				if(config->EXH)				
					FX.periodic.attack_length = longTime;				
				if(config->INH)				
					FX.periodic.fade_length = longTime;

				SDL_HapticUpdateEffect(rumble, rumbleFX, &FX);
				SDL_HapticRunEffect(rumble, rumbleFX, 1);				

				FX.periodic.attack_length = 0;
				FX.periodic.fade_length = 0;	

				Sleep(longTime);
			} 
			else // End of Convergence/Divergence	
			{
				SDL_HapticUpdateEffect(rumble, rumbleFX, &FX);
				SDL_HapticRunEffect(rumble, rumbleFX, 1);				

				Sleep(timeLength);
			}			

			// Stop once it's done.
			config->Ppow = config->Mpow = 0;					
		}	
		else Sleep(10);
		
	} //while(true)

} // Vibration Thread

} //namespace



