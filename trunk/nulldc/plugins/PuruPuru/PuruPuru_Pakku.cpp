#include "Devices.h"

#ifdef BUILD_DREAMCAST

char m_strName[64] = "Puru Puru Pack\0";
char m_License[64] = "Produced By or Under License From SEGA ENTERPRISES,LTD.\0";
char m_strNameEx[64] = "Version 1.000,1998/11/10,315-6211-AH\0";
//char* m_strNameEx = "Version 1.000,1998/11/10,315-6211-AH   ,Vibration Motor:1,Fm:4 E30Hz,Pow:7   ";

//////////////////////////////////////////////////////////////////////////
// Common to all vibration devices
union SourceSettings
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
	};

	SourceSettings() {U32 = 0;}
	SourceSettings(u32 _hex) {U32 = _hex;}
};

union Config
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

	Config() {U32 = 0;}
	Config(u32 _hex) {U32 = _hex;}
};

struct Status
{
	SourceSettings srcSettings;
	Config config;
		
	u8 AST; // 1:.25 seconds
	CRITICAL_SECTION section;
	HANDLE thread;

	u8 port;
};

Status m_state[4];

u32 FASTCALL PakkuDMA(void* device_instance, u32 Command, u32* buffer_in, u32 buffer_in_len, u32* buffer_out, u32& buffer_out_len)
{
	u8 port = ((maple_subdevice_instance*)device_instance)->port>>6;
	Status *m_status = &m_state[port];


	// Update the xpad the thread uses...
	//EnterCriticalSection(&m_status.section);
	//m_status.currentXPad = 
	//LeaveCriticalSection(&m_status.section);

	u8* buffer_out_b = (u8*)buffer_out;

	switch (Command)
	{
	case 1: // GetStatus		
		w32(1<<16);
		w32(0x00000101);
		w32(0x0);
		w32(0x0);
		w8(0xFF);
		w8(0x0);
		wStrings(m_strName, m_License);		
		w16(0x00C8);
		w16(0x0640);		
		return 5; // 5 = DeviceInfo

	case 2: // GetStatus ALL ?
		w32(1<<16);
		w32(0x00000101);
		w32(0x0);
		w32(0x0);
		w8(0xFF);
		w8(0x0);
		wStrings(m_strName, m_License);		
		w16(0x00C8);
		w16(0x0640);		
		wStringX(m_strNameEx, 40);		
		return 6; // 6 = DeviceInfoEx

	case 9: // GetCondition		
		w32(1<<16);
		w32(m_status->srcSettings.U32);				
		return 8; // 8 = DataTransfer

	case 10: // GetMediaInfo
		{
			if (!m_status->srcSettings.VN) // there are no vibration sources?!?!
				return 252; // 252 = Transmit again

			w32(1<<16);
			w32(m_status->srcSettings.U32);
			u8 source = (*(++buffer_in) & 0x000000FF);
		}
		return 8; // 8 = DataTransfer

	case 11: // BlockRead
		if (*(++buffer_in) == 0) // It's not looking for waveform data
		{
			// Read back the AutoStop settings
			w32(1<<16);
			w32(0);			// VN(1), Phase(1), Block No.(2)
			w16(0x0200);	// ASR for VN 1
			w8(m_status->AST);
			return 8; // 8 = DataTransfer
		}
		else
		{
			printf("BlockRead for waveform!\n");
			printf("REPORT THIS\n");
			return 253; // 253 = Unknown Command
		}		

	case 12: // BlockWrite
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
					//EnterCriticalSection(&m_status.section);
					m_status->AST = (*(buffer_in) & 0x00FF0000) >> 16;
					//LeaveCriticalSection(&m_status.section);					
				}
				else
				{
					printf("BlockWrite set numAST: %i ASR: 0x%04x\n", numAST, ASR);
					printf("REPORT THIS\n");
					return 252; // 252 = Transmit again
				}
			}
			else
			{
				u32 buffer = *(buffer_in);
				u32 buffer2 = *(++buffer_in);
				printf("BlockWrite set waveform: 0x%08x 0x%08x...\n", buffer, buffer2);
				printf("REPORT THIS\n");
				return 253; // 253 = Unknown Command
			}
		}
		return 7; // 7 = DeviceReply

	case 14: // Set Condition
		{
			int numSources = (buffer_in_len-4)/4;
			// numsources to be set greater than number of sources which can be concurrently selected?
			if (numSources > (u8)(0x00000101))
				return 252; // 252 = TransmitAgain

			//EnterCriticalSection(&m_status.section);
			m_status->config.U32 = *(++buffer_in);
			//LeaveCriticalSection(&m_status.section);

			// convergent and divergent can't be set at the same time
			if (m_status->config.INH && m_status->config.EXH)
				return 252; // 252 = TransmitAgain
			/*
			printf("   VN: %01x CNT: %01x INH: %01x Ppow: %01x EXH: %01x Mpow: %01x Freq: %02x Inc: %02x\n",
				m_status.config.VN,
				m_status.config.CNT,
				m_status.config.INH,
				m_status.config.Ppow,
				m_status.config.EXH,
				m_status.config.Mpow,
				m_status.config.FREQ,
				m_status.config.INC);
				*/
		}
		return 7; // 7 = DeviceReply

	default:
		printf("UNKNOWN MAPLE COMMAND %d (sent to maxi)\n", Command);
		return 253; // 253 = Command Unknown
	}
}

extern CONTROLLER_MAPPING joysticks[4];
extern CONTROLLER_STATE   joystate[4];

void XInput_VibrationThread(void *_status)
{							
	Status *status = (Status *)_status;
	Config *config = &status->config;

	printf("PuruPuru Pakku => [%d] Using XInput Rumble.\n", status->port);

	int joy = joysticks[status->port].ID;

	XINPUT_VIBRATION vib;
	
	int timeLength = joysticks[status->port].pakku_length;	
	int setIntensity = joysticks[status->port].pakku_intensity;

	int intensity, intensityH;
	int intensityX, intensityHX;		
	int fm, step, stepH;	
	
	int directionOld = 0;
	int directionNew = 0; 	
	int direction;

	int cdVergence;

	bool UseRealFreq = true;

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

			intensityX = (intensity * setIntensity) / 100;
			intensityHX = (intensityX * setIntensity) / 100;

			if (intensityX > 65535) intensityX = 65535;
			if (intensityHX > 65535) intensityHX = 65535;
			
			directionOld = directionNew;
			directionNew = direction;					
								
			// Impact when motor changes direction.
			if ( directionNew * directionOld < 0 )
			{																
				vib.wRightMotorSpeed = intensityHX;
				vib.wLeftMotorSpeed  = intensityX;
			
				XInputSetState(joy, &vib);
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
					
					intensityX = (intensity * setIntensity) / 100;
					intensityHX = (intensityH * setIntensity) / 100;

					if (intensityX > 65535) intensityX = 65535;
					if (intensityHX > 65535) intensityHX = 65535;					
					
					// Real Frequency
					if (UseRealFreq)
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
	
					XInputSetState(joy, &vib);

					intensity  = intensity  + step  * cdVergence;										
					intensityH = intensityH + stepH * cdVergence;

					Sleep(timeLength);					
				} 				

			} 
			else // End of Convergence/Divergence	
			{
				// Intensity setting
				intensityX = (intensity * setIntensity) / 100;
				intensityHX = (intensityH * setIntensity) / 100;

				if (intensityX > 65535) intensityX = 65535;
				if (intensityHX > 65535) intensityHX = 65535;				
				
				// Real Frequency
				if (UseRealFreq)
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
	
				XInputSetState(joy, &vib);
				Sleep(timeLength);
			}			

			// Stop once it's done.
			config->Ppow = config->Mpow = 0;
			
			vib.wRightMotorSpeed = 0;
			vib.wLeftMotorSpeed = 0;

			XInputSetState(joy, &vib);			
		}	
		else Sleep(10);

	} //while(true)

} // XInput Vibration Thread

void SDL_VibrationThread(void* _status)
{
	Status *status = (Status *)_status;
	Config *config = &status->config;

	printf("PuruPuru Pakku => [%d] Using SDL Rumble.\n", status->port);

	SDL_Haptic *rumble = joystate[status->port].rumble;
	
	SDL_HapticEffect FX;
	memset(&FX, 0, sizeof(SDL_HapticEffect));

	int timeLength = joysticks[status->port].pakku_length;	
	int setIntensity = joysticks[status->port].pakku_intensity;
	
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
			
			FX.periodic.magnitude  = (4095 + abs(direction) * 4096 * setIntensity) / 100;			
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

} // SDL Vibration Thread

void Start_Vibration(int port)
{
	m_state[port].port = port;
	
	m_state[port].AST			  = 0x13;	// AST of 5 secs
	m_state[port].srcSettings.VN = 1;
	m_state[port].srcSettings.VP = 0;
	m_state[port].srcSettings.VD = 0;
	m_state[port].srcSettings.PF = 1;
	m_state[port].srcSettings.CV = 1;
	m_state[port].srcSettings.PD = 1;
	m_state[port].srcSettings.OWF= 0;
	m_state[port].srcSettings.VA = 0;
	m_state[port].srcSettings.Fm0= 0x07;
	m_state[port].srcSettings.Fm1= 0x3B;
		
	if(joysticks[port].canRumble)
	{	
		if(joysticks[port].controllertype == CTL_TYPE_JOYSTICK_XINPUT)		
			m_state[port].thread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)XInput_VibrationThread, &m_state[port], 0, NULL);
		else 
			m_state[port].thread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)SDL_VibrationThread, &m_state[port], 0, NULL);

		InitializeCriticalSection(&m_state[port].section);
	}
}

void Stop_Vibration(int port)
{
	if(m_state[port].thread != NULL)
	{
		
		if(joysticks[port].controllertype == CTL_TYPE_JOYSTICK_XINPUT)		
		{
			XInputSetState(joysticks[port].ID, 0);
		}
		else 
		{
			if(joystate[port].rumble != NULL)
			SDL_HapticClose(joystate[port].rumble);
		}
		
		CloseHandle(m_state[port].thread);
		TerminateThread(m_state[port].thread, 0);
	}
}

#endif