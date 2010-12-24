#include "Devices.h"
#include <math.h>

#ifdef BUILD_DREAMCAST

char Joy_strName[64] = "Dreamcast Controller\0";
char Joy_strBrand_2[64] = "Produced By or Under License From SEGA ENTERPRISES,LTD.\0";

u32 FASTCALL ControllerDMA(void* device_instance, u32 Command,u32* buffer_in, u32 buffer_in_len, u32* buffer_out, u32& buffer_out_len)
{
	u8*buffer_out_b=(u8*)buffer_out;
	u32 port=((maple_device_instance*)device_instance)->port>>6;
	
	if(!joysticks[port].enabled)
		return 7;

	GetJoyStatus(port);

	switch (Command)
	{
		/*typedef struct {
			DWORD		func;//4
			DWORD		function_data[3];//3*4
			u8			area_code;//1
			u8			connector_direction;//1
			char		product_name[30];//30*1
			char		product_license[60];//60*1
			WORD		standby_power;//2
			WORD		max_power;//2
		} maple_devinfo_t;*/
		case 1:		
		{
			//function
			//4
			w32(1 << 24);	//Controller function

			//function info
			//3*4	
			w32(0xfe060f00); //values a real dc controller returns
			w32(0);
			w32(0);

			//1	area code ( 0xFF = all regions)
			w8(0xFF);

			//1	connector direction , i think 0 means 'on top'
			w8(0);

			//30 chars, pad with ' '
			for (u32 i = 0; i < 30; i++)
			{
				if (Joy_strName[i]!=0)	
				{
					w8((u8)Joy_strName[i]);				
				}
				else				
					w8(0x20);				
			}

			//60 chars, pad with ' '
			for (u32 i = 0; i < 60; i++)
			{
				if (Joy_strBrand_2[i]!=0)				
				{
					w8((u8)Joy_strBrand_2[i]);				
				}
				else				
					w8(0x20);				
			}

			//2
			w16(0x01AE); 

			//2
			w16(0x01F4); 
			return 5;
		}
		break;

		/* controller condition structure 
		typedef struct {//8 bytes
		WORD buttons;			///* buttons bitfield	/2
		u8 rtrig;			///* right trigger			/1
		u8 ltrig;			///* left trigger 			/1
		u8 joyx;			////* joystick X 			/1
		u8 joyy;			///* joystick Y				/1
		u8 joy2x;			///* second joystick X 		/1
		u8 joy2y;			///* second joystick Y 		/1
		} cont_cond_t;*/
		case 9:
		{
			//function
			//4
			w32(1 << 24);	//its a reply about controller ;p
			
			//Controller condition info
			// Set button
			u16 kcode[4]={0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};

			if( joystate[port].control[CTL_A_BUTTON] )	kcode[port] ^= key_CONT_A;
			if( joystate[port].control[CTL_B_BUTTON] )	kcode[port] ^= key_CONT_B;
			if( joystate[port].control[CTL_X_BUTTON] )	kcode[port] ^= key_CONT_X;
			if( joystate[port].control[CTL_Y_BUTTON] )	kcode[port] ^= key_CONT_Y;			
			if( joystate[port].control[CTL_START]    )	kcode[port] ^= key_CONT_START;
												
			if( joystate[port].control[CTL_D_PAD_UP]    )	kcode[port] ^= key_CONT_DPAD_UP;
			if( joystate[port].control[CTL_D_PAD_DOWN]  )	kcode[port] ^= key_CONT_DPAD_DOWN;			
			if( joystate[port].control[CTL_D_PAD_LEFT]  )	kcode[port] ^= key_CONT_DPAD_LEFT;			
			if( joystate[port].control[CTL_D_PAD_RIGHT] )	kcode[port] ^= key_CONT_DPAD_RIGHT;		
			
			w16(kcode[port] | 0xF901); //0xF901 -> buttons that are allways up on a dc
			
			// Set triggers		
			w8(joystate[port].control[CTL_R_SHOULDER]);										
			w8(joystate[port].control[CTL_L_SHOULDER]);				
	
			// Set Analog sticks (Main)
			// Reset!

			int center = 0x80;
			
			// Set analog controllers
			// Set Deadzones perhaps out of function
			
			float deadzone = (float)joysticks[port].deadzone + 1.0f;		
			
			// The value returned by SDL_JoystickGetAxis is a signed integer (-32768 to 32767)
			// The value used for the gamecube controller is an unsigned char (0 to 255)
			
			float main_stick_x = (float)joystate[port].control[CTL_MAIN_X];
			float main_stick_y = (float)joystate[port].control[CTL_MAIN_Y];						
			
			float radius = sqrt(main_stick_x*main_stick_x + main_stick_y*main_stick_y);

			deadzone = deadzone * 327.68f;
			
			if (radius < deadzone)
			{ 
				radius = 0;
			}
			else
			{				
				main_stick_x = main_stick_x / radius;
				main_stick_y = main_stick_y / radius;
				
				radius = (radius - deadzone) * 32767.0f/(32767.0f - deadzone);
				
				radius = radius / 256.0f;
				
				if (radius > 128) radius = 128;				
			}					

			main_stick_x = main_stick_x * radius;
			main_stick_y = main_stick_y * radius;
			
			w8(center + (int)main_stick_x);
			w8(center + (int)main_stick_y);
				
			//x/y2 are missing on DC
			//1
			w8(0x80); 
			//1
			w8(0x80); 

			return 8;
		}
		break; 

		default:
			printf("PuruPuru -> UNKNOWN MAPLE COMMAND %d\n", Command);
			return 7;
	}
}
#endif