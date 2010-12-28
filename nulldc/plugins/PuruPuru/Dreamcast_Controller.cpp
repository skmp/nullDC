#include "Devices.h"
#include <math.h>

#ifdef BUILD_DREAMCAST

char Joy_strName[64] = "Dreamcast Controller\0";
char Joy_strBrand[64] = "Produced By or Under License From SEGA ENTERPRISES,LTD.\0";

u16 GetAnalog1(u32 port)
{		
	u8 center = 0x80;
	
	float deadzone = (float)(joysticks[port].deadzone + 1) * 327.68f;		
			
	// The value returned by SDL_JoystickGetAxis is a signed integer (-32768 to 32767)
	// The value used for the gamecube controller is an unsigned char (0 to 255)
			
	float main_stick_x = (float)joystate[port].control[CTL_MAIN_X];
	float main_stick_y = (float)joystate[port].control[CTL_MAIN_Y];						
			
	float radius = sqrt(main_stick_x*main_stick_x + main_stick_y*main_stick_y);	
			
	if (radius < deadzone)
	{ 
		radius = 0;
	}
	else
	{				
		main_stick_x /= radius;
		main_stick_y /= radius;
				
		radius  = (radius - deadzone) * 32767.0f/(32767.0f - deadzone);				
		radius /= 256.0f;
				
		if (radius > 128) radius = 128;				
	}					

	u8 outx = (short)(main_stick_x * radius) & 0xFF;
	u8 outy = (short)(main_stick_y * radius) & 0xFF;

	u16 outvalue = (center + outy)<<8;		
	    outvalue|= (center + outx);	

	return outvalue;
}


u32 FASTCALL ControllerDMA(void* device_instance, u32 Command,u32* buffer_in, u32 buffer_in_len, u32* buffer_out, u32& buffer_out_len)
{
	u8*buffer_out_b=(u8*)buffer_out;
	u32 port=((maple_device_instance*)device_instance)->port>>6;

	// Enabled check ain't necessary. Only the plugin's active ports are checked. :)
	// Update input information.
	GetJoyStatus(port);

	switch (Command)
	{
		case 1:		
		{												
			w32(1<<24);      //Controller function
			w32(0xFE060F00); //values a real dc controller returns
			w32(0);
			w32(0);	

			w8(0xFF); //1	area code ( 0xFF = all regions)			
			w8(0);    //1	connector direction , i think 0 means 'on top'

			wStrings(Joy_strName, Joy_strBrand); //30 and 60 chars respectively, pad with ' '						
									
			w16(0x01AE); // Standby current consumption			
			w16(0x01F4); // Maximum current consumption

			return 5; // 5 = Device Info.
		}		

		case 9:
		{			
			w32(1 << 24);	//its a reply about controller ;p
						
			// Set buttons
			u16 kcode = 0xFFFF;

			if( joystate[port].control[CTL_A_BUTTON] )	kcode ^= key_CONT_A;
			if( joystate[port].control[CTL_B_BUTTON] )	kcode ^= key_CONT_B;
			if( joystate[port].control[CTL_X_BUTTON] )	kcode ^= key_CONT_X;
			if( joystate[port].control[CTL_Y_BUTTON] )	kcode ^= key_CONT_Y;			
			if( joystate[port].control[CTL_START]    )	kcode ^= key_CONT_START;
												
			if( joystate[port].control[CTL_D_PAD_UP]    )	kcode ^= key_CONT_DPAD_UP;
			if( joystate[port].control[CTL_D_PAD_DOWN]  )	kcode ^= key_CONT_DPAD_DOWN;			
			if( joystate[port].control[CTL_D_PAD_LEFT]  )	kcode ^= key_CONT_DPAD_LEFT;			
			if( joystate[port].control[CTL_D_PAD_RIGHT] )	kcode ^= key_CONT_DPAD_RIGHT;								

			w16(kcode | 0xF901); //0xF901 -> buttons that are allways up on a dc
			
			// Set triggers		
			w8(joystate[port].control[CTL_R_SHOULDER]);										
			w8(joystate[port].control[CTL_L_SHOULDER]);				
	
			// Set Analog sticks (Main)
			w16(GetAnalog1(port));

			//x/y2 are missing on DC		
			w8(0x80); 			
			w8(0x80); 

			return 8;
		}		

		default:
			printf("PuruPuru -> UNKNOWN MAPLE COMMAND %d\n", Command);
			return 7;
	}
}
#endif