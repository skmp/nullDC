//////////////////////////////////////////////////////////////////////////////////////////
// Project description
// -------------------
// Name: nullDC 
// Description: A nullDC Compatible Input Plugin
//
// Author: Falcon4ever (nullDC@falcon4ever.com)
// Site: www.multigesture.net
// Copyright (C) 2007-2009 nullDC Project.
//


#include "PuruPuru.h"
#include <math.h>

//////////////////////////////////////////////////////////////////////////////////////////
// Variables
// ---------
HINSTANCE PuruPuru_hInst = NULL;
emu_info host;
u32 current_port = 0;
bool emulator_running  = FALSE;

bool canSDL		= false;
bool canXInput  = false;

CONTROLLER_STATE joystate[4];
CONTROLLER_MAPPING joysticks[4];
CONTROLLER_INFO_SDL		*joyinfo = 0;
CONTROLLER_INFO_XINPUT	 xoyinfo[4];

//////////////////////////////////////////////////////////////////////////////////////////
// DllMain 
// -------
BOOL APIENTRY DllMain(	HINSTANCE hinstDLL,	// DLL module handle
						DWORD dwReason,		// reason called
						LPVOID lpvReserved)	// reserved
{
	InitCommonControls();
	PuruPuru_hInst = hinstDLL;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
// dcGetInterface
// --------------

void EXPORT_CALL dcGetInterface(plugin_interface* info)
{
	// Set plugin info
	info->InterfaceVersion = PLUGIN_I_F_VERSION;
	info->common.InterfaceVersion = MAPLE_PLUGIN_I_F_VERSION;
	wcscpy_s(info->common.Name, L"PuruPuru input plugin v" _T(INPUT_VERSION) L" by Falcon4ever [" _T(__DATE__) L"]");
	
	// Assign callback functions
	info->common.Load = Load;
	info->common.Unload = Unload;
	info->common.Type = Plugin_Maple;
	info->maple.CreateMain = CreateMain;
	info->maple.CreateSub = CreateSub;
	info->maple.Init = Init;
	info->maple.Term = Term;
	info->maple.Destroy = Destroy;

	// 1 maple device
	wcscpy_s(info->maple.devices[0].Name, L"PuruPuru input plugin v" _T(INPUT_VERSION) L" by Falcon4ever [" _T(__DATE__) L"]");
	// Main device (Like controller, lightgun and such)
	info->maple.devices[0].Type = MDT_Main;
	// Can have 2 subdevices (a dc controller has 2 subdevice ports for vmus / etc)
	// Can be 'hot plugged' (after emulation was started)
	info->maple.devices[0].Flags = MDTF_Sub0 | MDTF_Sub1 | MDTF_Hotplug;

	//EOL marker
	info->maple.devices[1].Type = MDT_EndOfList;	// wtf is this raz?
}

//////////////////////////////////////////////////////////////////////////////////////////
// Common Input Plugin Functions
// -----------------------------

// Load plugin
// -----------
// Notes: Called when plugin is loaded by the emu, the param has some handy functions so i make a copy of it ;).
s32 FASTCALL Load(emu_info* emu)
{	

	wprintf(L"PuruPuru -> Load\n");	

	memcpy(&host, emu, sizeof(host));

	if(SDL_Init(SDL_INIT_JOYSTICK ) < 0)
	{		
		MessageBoxA(NULL, SDL_GetError(), "Could not initialize SDL!", MB_ICONERROR);		
		return rv_error;
	}
	else
	{
		int joys = Search_Devices();
		if ( !joys ) wprintf(L"PuruPuru: No SDL Joystick Found!");


	}

	// All devices disabled unless connected later.
	joysticks[0].enabled = 0;
	joysticks[1].enabled = 0;
	joysticks[2].enabled = 0;
	joysticks[3].enabled = 0;
	
	return rv_ok;
}

// Unload plugin
// -------------
// Notes: Called when plugin is unloaded by emulator (only if Load was called before).
void FASTCALL Unload()
{
	wprintf(L"PuruPuru -> Unload\n");

	SDL_Quit();

	delete [] joyinfo;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Maple Input Plugin Functions
// ----------------------------

// CreateMain
// ----------
// Notes: Called to create a main device, like joystick, lightgun and such.
// inst: instance info
//	inst->port is the port (in the native maple format) of the device
//	inst->dma must be filled 
//	inst->data can be filled (its what you get as data parameter on all the maple callbacks ;p)
//	rest of it can be ignored (unless you want to do some ugly shit ;p)
// id: maple device id
//	Its the index of the device on the Maple device list that the dcGetInterface gives to the emn
// flags: creation flags, ingore it for now (MDCF_HOTPLUG if the plugin is loaded after the emu startup)
// rootmenu: Root menu for the device
s32 FASTCALL CreateMain(maple_device_instance* inst, u32 id, u32 flags, u32 rootmenu)
{
	wprintf(L"PuruPuru -> CreateMain\n");

	// Enable pad on port.
	joysticks[(inst->port >> 6)].enabled = 1;

	if (id!=0)
		return rv_error;

	inst->data = inst;
	inst->dma = ControllerDMA;
	
	wchar temp[512];
	swprintf(temp, sizeof(temp), L"Config keys for Player %d", (inst->port >> 6) + 1);	
	u32 ckid = host.AddMenuItem(rootmenu, -1, temp, ConfigMenuCallback, 0);

	// Set the user defined pointer for the menu to the device instance, so we can tell for which port the menu was called ;)	
	MenuItem mi;
	mi.PUser = inst;
	host.SetMenuItem(ckid, &mi, MIM_PUser);
	
	return rv_ok;
}

// CreateSub
// ---------
// Notes: Called to create a sub device, uses same params as CreateMain
s32 FASTCALL CreateSub(maple_subdevice_instance* inst, u32 id, u32 flags, u32 rootmenu)
{	
	wprintf(L"PuruPuru -> CreateSub\n");

	// This plugin has no subdevices so return an error.
	return rv_error;
}

// Init PAD (start emulation)
// --------------------------
// Notes: Called when emulation is started.
// data: the inst->data pointer as filled by the Create* functions
// id: device index on the dcGetInterface
// params: nothing useful (just a placeholder)
// Additional notes:
// Hot plugged devices can miss this call if the emulation is started before the device is hotplugged. This is a bug on the emu %) ;p
s32 FASTCALL Init(void* data, u32 id, maple_init_params* params)
{
	// Init input lib, this can also be done in Create*;
	wprintf(L"PuruPuru -> Init\n");
	emulator_running = TRUE;

	LoadConfig();	// Load joystick mapping
	
	u32 port = ((maple_device_instance*)data)->port >> 6;	

	if(joysticks[port].enabled)
		joystate[port].joy = SDL_JoystickOpen(joysticks[port].ID);

	return rv_ok;
}

// Shutdown PAD (stop emulation)
// -----------------------------
// Called when emuation is terminated
// data: the inst->data pointer as filled by the Create* functions
// id: device index on the dcGetInterface
// Additional notes: 
// Called only if Init() was called ;)
void FASTCALL Term(void* data, u32 id)
{
	wprintf(L"PuruPuru -> Term\n");

	emulator_running = FALSE;

	//kill whatever you did on Init()	
}

// Free memory (quit emulator)?
// ----------------------------
// Notes: Destroy the device
// data: the inst->data pointer as filled by the Create* functions
// id: device index on the dcGetInterface
void FASTCALL Destroy(void* data, u32 id)
{
	//Free any memory allocted (if any)
	wprintf(L"PuruPuru -> Destroy\n");

	// Disable on Hot-Unplug
	u32 port = ((maple_device_instance*)data)->port >> 6;
	
	joysticks[port].enabled = 0;
	
	if(joystate[port].joy != NULL) SDL_JoystickClose(joystate[port].joy);	
}

//////////////////////////////////////////////////////////////////////////////////////////
// General Plugin Functions
// ------------------------

// ControllerDMA
// -------------
// http://mc.pp.se/dc/controller.html

char Joy_strName[64] = "Dreamcast Controller\0";
char Joy_strBrand_2[64] = "Produced By or Under License From SEGA ENTERPRISES,LTD.\0";

#define w32(data) *(u32*)buffer_out_b=(data);buffer_out_b+=4;buffer_out_len+=4
#define w16(data) *(u16*)buffer_out_b=(data);buffer_out_b+=2;buffer_out_len+=2
#define w8(data) *(u8*)buffer_out_b=(data);buffer_out_b+=1;buffer_out_len+=1

#define key_CONT_C  (1 << 0)
#define key_CONT_B  (1 << 1)
#define key_CONT_A  (1 << 2)
#define key_CONT_START  (1 << 3)
#define key_CONT_DPAD_UP  (1 << 4)
#define key_CONT_DPAD_DOWN  (1 << 5)
#define key_CONT_DPAD_LEFT  (1 << 6)
#define key_CONT_DPAD_RIGHT  (1 << 7)
#define key_CONT_Z  (1 << 8)
#define key_CONT_Y  (1 << 9)
#define key_CONT_X  (1 << 10)
#define key_CONT_D  (1 << 11)
#define key_CONT_DPAD2_UP  (1 << 12)
#define key_CONT_DPAD2_DOWN  (1 << 13)
#define key_CONT_DPAD2_LEFT  (1 << 14)
#define key_CONT_DPAD2_RIGHT  (1 << 15)

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

void GetKeyStatus(int port, char* keys)
{	
	for(int i=0; i<16; i++)
	{
		if((joysticks[port].control[i]>>24) & inKEY)
		{
			int num = joysticks[port].control[i] & 0xFF;
			keys[num] = (char)(GetAsyncKeyState(num)>>12);
		}
	}		
}

int GetStatusKey (int port, int type, int input)
{
	char key[256] = {0};
	GetKeyStatus(port, key);			

	int num = input & 0xFF;

	if((input>>24) & inKEY)
	{
		switch(type)
		{
			case AXIS:	  return key[num]? 32767:0;				
			case TRIGGER: return key[num]? 255:0;
			case DIGITAL: return key[num];				
		}
	}

	return 0;
}

int GetStateXInput (int port, int type, int input)
{			
	char key[256] = {0};
	if(joysticks[port].keys) GetKeyStatus(port, key);		
		
	XInputGetState( joysticks[port].ID, &xoyinfo[port].state );
	port = joysticks[port].ID;	

	int num = input & 0xFF;
	int mode = input >> 24;

	switch(type)
	{
	case AXIS:
		{
			switch(mode)
			{
			case inAXIS_0:
				{
					switch(num)
					{
					case X360_LX:
						{
							SHORT axis = xoyinfo[port].state.Gamepad.sThumbLX;

							if(axis < 0) return  -axis;
							else		 return   0;
						}
					case X360_LY:
						{
							SHORT axis = xoyinfo[port].state.Gamepad.sThumbLY;

							if(axis < 0) return  -axis;
							else		 return	 0;
						}
					case X360_RX:
						{
							SHORT axis = xoyinfo[port].state.Gamepad.sThumbRX;

							if(axis < 0) return  -axis;
							else		 return	 0;
						}
					case X360_RY:
						{
							SHORT axis = xoyinfo[port].state.Gamepad.sThumbRY;

							if(axis < 0) return  -axis;
							else		 return	 0;
						}
					}
				}
			case inAXIS_1:
				{
					switch(num)
					{
					case X360_LX:
						{
							SHORT axis = xoyinfo[port].state.Gamepad.sThumbLX;

							if(axis > 0) return  axis;
							else		 return   0;
						}
					case X360_LY:
						{
							SHORT axis = xoyinfo[port].state.Gamepad.sThumbLY;

							if(axis > 0) return  axis;
							else		 return	 0;
						}
					case X360_RX:
						{
							SHORT axis = xoyinfo[port].state.Gamepad.sThumbRX;

							if(axis > 0) return  axis;
							else		 return	 0;
						}
					case X360_RY:
						{
							SHORT axis = xoyinfo[port].state.Gamepad.sThumbRY;

							if(axis > 0) return  axis;
							else		 return	 0;
						}
					}
				}
			case inTRIGGER:
				{
					if(num == X360_LT)
						return xoyinfo[port].state.Gamepad.bLeftTrigger * 128;
					else
						return xoyinfo[port].state.Gamepad.bRightTrigger * 128;
				}
			case inHAT:
				{
					switch(num)
					{
					case X360_UP:
						{							
							return (xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP)?32767:0;
						}
					case X360_DOWN:
						{							
							return (xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN)?32767:0;
						}
					case X360_LEFT:
						{							
							return (xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT)?32767:0;
						}
					case X360_RIGHT:
						{							
							return (xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)?32767:0;
						}
					}
				}
			case inBUTTON:
				{
					switch(num)
					{	
					case X360_START:
						{
							return (xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_START)?32767:0;
						}
					case X360_BACK:
						{
							return (xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK)?32767:0;
						}
					case X360_LS:
						{
							return (xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB)?32767:0;
						}
					case X360_RS:
						{
							return (xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB)?32767:0;
						}
					case X360_LB:
						{
							return (xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER)?32767:0;
						}
					case X360_RB:
						{
							return (xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER)?32767:0;
						}
					case X360_A:
						{
							return (xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_A)?32767:0;
						}
					case X360_B:
						{
							return (xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_B)?32767:0;
						}
					case X360_X:
						{
							return (xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_X)?32767:0;
						}
					case X360_Y:
						{
							return (xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_Y)?32767:0;
						}
					}
				}
			case inKEY:
				{					
					return (key[num])?32767:0;
				}
			}				
		}
	case TRIGGER:
		{
			switch(mode)
			{
			case inAXIS_0:
				{
					switch(num)
					{
					case X360_LX:
						{
							SHORT axis = xoyinfo[port].state.Gamepad.sThumbLX/128;							

							if(axis < 0)
							{
								if(axis < -255) axis = -255;
								return  -axis;
							}
							else return   0;
						}
					case X360_LY:
						{
							SHORT axis = xoyinfo[port].state.Gamepad.sThumbLY/128;

							if(axis < 0)
							{
								if(axis < -255) axis = -255;
								return  -axis;
							}
							else return   0;
						}
					case X360_RX:
						{
							SHORT axis = xoyinfo[port].state.Gamepad.sThumbRX/128;

							if(axis < 0)
							{
								if(axis < -255) axis = -255;
								return  -axis;
							}
							else return   0;
						}
					case X360_RY:
						{
							SHORT axis = xoyinfo[port].state.Gamepad.sThumbRY/128;

							if(axis < 0)
							{
								if(axis < -255) axis = -255;
								return  -axis;
							}
							else return   0;
						}
					}
				}
			case inAXIS_1:
				{
					switch(num)
					{
					case X360_LX:
						{
							SHORT axis = xoyinfo[port].state.Gamepad.sThumbLX/128;							

							if(axis > 0)
							{
								if(axis > 255) axis = 255;
								return  axis;
							}
							else return	 0;
						}
					case X360_LY:
						{
							SHORT axis = xoyinfo[port].state.Gamepad.sThumbLY/128;

							if(axis > 0)
							{
								if(axis > 255) axis = 255;
								return  axis;
							}
							else return	 0;
						}
					case X360_RX:
						{
							SHORT axis = xoyinfo[port].state.Gamepad.sThumbRX/128;

							if(axis > 0)
							{
								if(axis > 255) axis = 255;
								return  axis;
							}
							else return	 0;
						}
					case X360_RY:
						{
							SHORT axis = xoyinfo[port].state.Gamepad.sThumbRY/128;

							if(axis > 0)
							{
								if(axis > 255) axis = 255;
								return  axis;
							}
							else return	 0;
						}
					}
				}
			case inTRIGGER:
				{
					if(num == X360_LT)
						return xoyinfo[port].state.Gamepad.bLeftTrigger;
					else if(num == X360_RT)
						return xoyinfo[port].state.Gamepad.bRightTrigger;
				}
			case inHAT:
				{
					switch(num)
					{
					case X360_UP:
						{
							return (xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP)?255:0;
						}
					case X360_DOWN:
						{	
							return (xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN)?255:0;
						}
					case X360_LEFT:
						{							
							return (xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT)?255:0;
						}
					case X360_RIGHT:
						{							
							return (xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)?255:0;
						}
					}
				}
			case inBUTTON:
				{
					switch(num)
					{	
					case X360_START:
						{
							return (xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_START)?255:0;
						}
					case X360_BACK:
						{
							return (xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK)?255:0;
						}
					case X360_LS:
						{
							return (xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB)?255:0;
						}
					case X360_RS:
						{
							return (xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB)?255:0;
						}
					case X360_LB:
						{
							return (xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER)?255:0;
						}
					case X360_RB:
						{
							return (xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER)?255:0;
						}
					case X360_A:
						{
							return (xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_A)?255:0;
						}
					case X360_B:
						{
							return (xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_B)?255:0;
						}
					case X360_X:
						{
							return (xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_X)?255:0;
						}
					case X360_Y:
						{
							return (xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_Y)?255:0;
						}
					}
				}
			case inKEY:
				{
					return (key[num])?255:0;					
				}
			}												
		}				
	case DIGITAL:	
		{				
			switch(mode)
			{
			case inAXIS_0:
				{
					switch(num)
					{
					case X360_LX:
						{
							SHORT axis = xoyinfo[port].state.Gamepad.sThumbLX;
							return (axis < -22000)?1:0;							
						}
					case X360_LY:
						{
							SHORT axis = xoyinfo[port].state.Gamepad.sThumbLY;
							return (axis < -22000)?1:0;
						}
					case X360_RX:
						{
							SHORT axis = xoyinfo[port].state.Gamepad.sThumbRX;
							return (axis < -22000)?1:0;
						}
					case X360_RY:
						{
							SHORT axis = xoyinfo[port].state.Gamepad.sThumbRY;
							return (axis < -22000)?1:0;
						}
					}
				}
			case inAXIS_1:
				{
					switch(num)
					{
					case X360_LX:
						{
							SHORT axis = xoyinfo[port].state.Gamepad.sThumbLX;
							return (axis > 22000)?1:0;							
						}
					case X360_LY:
						{
							SHORT axis = xoyinfo[port].state.Gamepad.sThumbLY;
							return (axis > 22000)?1:0;
						}
					case X360_RX:
						{
							SHORT axis = xoyinfo[port].state.Gamepad.sThumbRX;
							return (axis > 22000)?1:0;
						}
					case X360_RY:
						{
							SHORT axis = xoyinfo[port].state.Gamepad.sThumbRY;
							return (axis > 22000)?1:0;
						}
					}
				}
			case inTRIGGER:
				{
					if(num == X360_LT) 
						return (xoyinfo[port].state.Gamepad.bLeftTrigger > 100)?1:0;
					else if(num == X360_RT) 
						return (xoyinfo[port].state.Gamepad.bLeftTrigger > 100)?1:0;
				}
			case inHAT:
				{
					switch(num)
					{
					case X360_UP:
						{
							return( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP);
						}
					case X360_DOWN:
						{	
							return( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
						}
					case X360_LEFT:
						{
							return( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
						}
					case X360_RIGHT:
						{
							return( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
						}
					}
				}
			case inBUTTON:
				{
					switch(num)
					{	
					case X360_START:
						{						
							return( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_START);
						}
					case X360_BACK:
						{						
							return( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK);
						}
					case X360_LS:
						{						
							return( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB);
						}
					case X360_RS:
						{						
							return( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB);
						}
					case X360_LB:
						{						
							return( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
						}
					case X360_RB:
						{						
							return( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
						}
					case X360_A:
						{						
							return( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_A);
						}
					case X360_B:
						{						
							return( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_B);
						}
					case X360_X:
						{						
							return( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_X);
						}
					case X360_Y:
						{							
							return( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_Y);
						}
					}
				}
			case inKEY:
				{
					return key[num];					
				}
			}					
		}
	}
	
	return 0;
}


// 22000 AXIS to BUTTON threshold.

int GetStateSDL (int port, int type, int input)
{							
	char key[256] = {0};
	if(joysticks[port].keys) GetKeyStatus(port, key);
	
	port = joysticks[port].ID;

	int num = input & 0xFF;
	int mode = input >> 24;
	int currentHat = SDL_JoystickGetHat(joystate[port].joy, 0);	

	switch(type)
	{
	case AXIS:
		{
			switch(mode)
			{
			case inAXIS_0: 
				{
					int axis = SDL_JoystickGetAxis(joystate[port].joy, num);

					if(axis < 0)  return -axis;						
					else		  return 0;					
				}
			case inAXIS_1: 
				{
					int axis = SDL_JoystickGetAxis(joystate[port].joy, num);
											
					if(axis > 0)  return  axis;						
					else		  return 0;										
				}
			case inBUTTON:
				{
					if( SDL_JoystickGetButton(joystate[port].joy, num) )
						return 32767;
					else
						return 0;
				}
			case inHAT:
				{
					if(num & currentHat) return 32767;
					else				 return 0;	
				}
			case inKEY:
				{
					if(key[num]) return 32767;
					else		 return 0;
				}
			}				
		}
	case TRIGGER:
		{
			switch(mode)
			{
			case inAXIS_0: 
				{
					
					int axis = SDL_JoystickGetAxis(joystate[port].joy, num)/128;										
							
					if(axis < 0)
					{
						if(axis < -255) axis = -255;						
						
						return -axis;
					}
					else return 0;
					
				}
			case inAXIS_1: 
				{
					
					int axis = SDL_JoystickGetAxis(joystate[port].joy, num)/128;										
							
					if(axis > 0)
					{
						if(axis > 255) axis = 255;

						return  axis;
					}
					else return 0;
					
				}
			case inBUTTON: 
				{
					if( SDL_JoystickGetButton(joystate[port].joy, num) )
						return 255;
					else
						return 0;
				}
			case inHAT: 
				{
					if(num & currentHat) return 255;
					else				 return 0;	
				}
			case inKEY:
				{
					if(key[num]) return 255;
					else		 return 0;
				}
			}
		}				
	case DIGITAL:	
		{
			switch(mode)
			{
			case inAXIS_0: 
				{						
					int axis = SDL_JoystickGetAxis(joystate[port].joy, num);
											
					if(axis <-22000) return 1;
					else             return 0;					
				}
			case inAXIS_1: 
				{						
					int axis = SDL_JoystickGetAxis(joystate[port].joy, num);
					
					if(axis > 22000) return 1;
					else		     return 0;										
				}
			case inBUTTON:
				{
					return SDL_JoystickGetButton(joystate[port].joy, num);
				}
			case inHAT: 
				{	
					if(num & currentHat) return 1;
					else				 return 0;						
				}
			case inKEY: 
				{
					return key[num];
				}
			}
		}
	}

	return 0;
}

int GetInputStatus(int port, int type, int input)
{		
	switch(joysticks[port].controllertype)
	{
	case CTL_TYPE_JOYSTICK_SDL: 
		{
			SDL_JoystickUpdate(); 
			return GetStateSDL(port, type, input);
		}

	case CTL_TYPE_JOYSTICK_XINPUT:
		{
			XInputGetState(port, &xoyinfo[port].state); 
			return GetStateXInput(port, type, input);
		}
	case CTL_TYPE_KEYBOARD:
		{
			return GetStatusKey(port, type, input);
		}
	}

	return 0;
}


void GetJoyStatus(int controller)
{	
		joystate[controller].control[CTL_MAIN_X] = 0;
		joystate[controller].control[CTL_MAIN_Y] = 0;

		joystate[controller].control[CTL_MAIN_X] -= GetInputStatus(controller, AXIS, joysticks[controller].control[MAP_A_XL]);
		joystate[controller].control[CTL_MAIN_X] += GetInputStatus(controller, AXIS, joysticks[controller].control[MAP_A_XR]);				
		
		joystate[controller].control[CTL_MAIN_Y] -= GetInputStatus(controller, AXIS, joysticks[controller].control[MAP_A_YU]);		
		joystate[controller].control[CTL_MAIN_Y] += GetInputStatus(controller, AXIS, joysticks[controller].control[MAP_A_YD]);		

		joystate[controller].halfpress = GetInputStatus(controller, DIGITAL, joysticks[controller].control[MAP_HALF] );

		joystate[controller].control[CTL_L_SHOULDER] = GetInputStatus(controller, TRIGGER, joysticks[controller].control[MAP_LT]);
		joystate[controller].control[CTL_R_SHOULDER] = GetInputStatus(controller, TRIGGER, joysticks[controller].control[MAP_RT]);
		
		if ( joystate[controller].halfpress ) 
		{
			joystate[controller].control[CTL_L_SHOULDER] /= 2;
			joystate[controller].control[CTL_R_SHOULDER] /= 2;

			joystate[controller].control[CTL_MAIN_X] /= 2;
			joystate[controller].control[CTL_MAIN_Y] /= 2;
		}				
				
		joystate[controller].control[CTL_A_BUTTON] = GetInputStatus(controller, DIGITAL,  joysticks[controller].control[MAP_A]);
		joystate[controller].control[CTL_B_BUTTON] = GetInputStatus(controller, DIGITAL,  joysticks[controller].control[MAP_B]);
		joystate[controller].control[CTL_X_BUTTON] = GetInputStatus(controller, DIGITAL,  joysticks[controller].control[MAP_X]);
		joystate[controller].control[CTL_Y_BUTTON] = GetInputStatus(controller, DIGITAL,  joysticks[controller].control[MAP_Y]);
		joystate[controller].control[CTL_START]	   = GetInputStatus(controller, DIGITAL,  joysticks[controller].control[MAP_START]);				

		joystate[controller].control[CTL_D_PAD_UP]	  = GetInputStatus(controller, DIGITAL,  joysticks[controller].control[MAP_D_UP]);
		joystate[controller].control[CTL_D_PAD_DOWN]  = GetInputStatus(controller, DIGITAL,  joysticks[controller].control[MAP_D_DOWN]);	
		joystate[controller].control[CTL_D_PAD_LEFT]  = GetInputStatus(controller, DIGITAL,  joysticks[controller].control[MAP_D_LEFT]);
		joystate[controller].control[CTL_D_PAD_RIGHT] = GetInputStatus(controller, DIGITAL,  joysticks[controller].control[MAP_D_RIGHT]);	
}

// ConfigMenuCallback
// ------------------
// Notes: MUST be EXPORT_CALL, because it is a callback for a menu ;)
// Called when the config menu is selected
// id = menu id (can be used to set the style with host.SetMenuItemStyle & co)
// w = window handle (HWND) that owns the menu
// p = user specified data
void EXPORT_CALL ConfigMenuCallback(u32 id, void* w, void* p)
{
	wprintf(L"PuruPuru -> ConfigMenuCallback\n");

	current_port = ((maple_device_instance*)p)->port >> 6;

	LoadConfig();	// load settings	

	if( DialogBox(PuruPuru_hInst, MAKEINTRESOURCE(IDD_CONFIG), (HWND)w, OpenConfig) )
	{
		SaveConfig();
	}

	LoadConfig();	// reload settings	
}

// Save settings to file
// ---------------------
void SaveConfig()
{
	wchar SectionName[32];
	for (int port=0;port<4;port++)
	{		
		wsprintf(SectionName, L"PuruPuru_Pad_%i", port+1);		
						
		host.ConfigSaveStr(SectionName, L"l_shoulder",		joysticks[port].names[MAP_LT]);
		host.ConfigSaveStr(SectionName, L"r_shoulder",		joysticks[port].names[MAP_RT]);
		host.ConfigSaveStr(SectionName, L"a_button",		joysticks[port].names[MAP_A]);
		host.ConfigSaveStr(SectionName, L"b_button",		joysticks[port].names[MAP_B]);
		host.ConfigSaveStr(SectionName, L"x_button",		joysticks[port].names[MAP_X]);
		host.ConfigSaveStr(SectionName, L"y_button",		joysticks[port].names[MAP_Y]);
		host.ConfigSaveStr(SectionName, L"start_button",	joysticks[port].names[MAP_START]);		
		host.ConfigSaveStr(SectionName, L"dpad_up",			joysticks[port].names[MAP_D_UP]);
		host.ConfigSaveStr(SectionName, L"dpad_down",		joysticks[port].names[MAP_D_DOWN]);
		host.ConfigSaveStr(SectionName, L"dpad_left",		joysticks[port].names[MAP_D_LEFT]);
		host.ConfigSaveStr(SectionName, L"dpad_right",		joysticks[port].names[MAP_D_RIGHT]);
		host.ConfigSaveStr(SectionName, L"main_x_left",		joysticks[port].names[MAP_A_XL]);
		host.ConfigSaveStr(SectionName, L"main_x_right",	joysticks[port].names[MAP_A_XR]);
		host.ConfigSaveStr(SectionName, L"main_y_up",		joysticks[port].names[MAP_A_YU]);				
		host.ConfigSaveStr(SectionName, L"main_y_down",		joysticks[port].names[MAP_A_YD]);
		host.ConfigSaveStr(SectionName, L"halfpress",		joysticks[port].names[MAP_HALF]);

		host.ConfigSaveInt(SectionName, L"deadzone",		joysticks[port].deadzone);
		host.ConfigSaveInt(SectionName, L"keyboard",		joysticks[port].keys);		
		host.ConfigSaveInt(SectionName, L"joy_id",			joysticks[port].ID);
		host.ConfigSaveInt(SectionName, L"controllertype",	joysticks[port].controllertype);
		host.ConfigSaveInt(SectionName, L"eventnum",		joysticks[port].eventnum);
	}
}

void Names2Control(int port)
{
	for(int i=0; i<16; i++)
	{
		wchar* temp = joysticks[port].names[i];
		int num = _wtoi(&temp[1]);

		if(joysticks[port].controllertype == CTL_TYPE_JOYSTICK_SDL)
		{
			bool plus = false;
			if(temp[0] == L'A' && temp[sizeof(temp)/2] == L'+') plus = true;

			switch(temp[0])
			{
			case L'A': 
				{						
					if(plus) joysticks[port].control[i] = (inAXIS_1<<24) + num;
					else	 joysticks[port].control[i] = (inAXIS_0<<24) + num;
				}break;

			case L'B':
				{
					joysticks[port].control[i] = (inBUTTON<<24) + num;
				}break;

			case L'H':
				{
					joysticks[port].control[i] = (inHAT<<24) + num;
				}break;
			}
		}
		else if(joysticks[port].controllertype == CTL_TYPE_JOYSTICK_XINPUT)
		{
			if(temp[0] == L'L' && temp[1] == L'X' && temp[2] == '+')
			{
				joysticks[port].control[i] = (inAXIS_1<<24) + X360_LX;
			}
			else if(temp[0] == L'L' && temp[1] == L'X' && temp[2] == '-')
			{
				joysticks[port].control[i] = (inAXIS_0<<24) + X360_LX;
			}
			else if(temp[0] == L'L' && temp[1] == L'Y' && temp[2] == '+')
			{
				joysticks[port].control[i] = (inAXIS_1<<24) + X360_LY;
			}
			else if(temp[0] == L'L' && temp[1] == L'Y' && temp[2] == '-')
			{
				joysticks[port].control[i] = (inAXIS_0<<24) + X360_LY;
			}
			else if(temp[0] == L'R' && temp[1] == L'X' && temp[2] == '+')
			{
				joysticks[port].control[i] = (inAXIS_1<<24) + X360_RX;
			}
			else if(temp[0] == L'R' && temp[1] == L'X' && temp[2] == '-')
			{
				joysticks[port].control[i] = (inAXIS_0<<24) + X360_RX;
			}
			else if(temp[0] == L'R' && temp[1] == L'Y' && temp[2] == '+')
			{
				joysticks[port].control[i] = (inAXIS_1<<24) + X360_RY;
			}
			else if(temp[0] == L'R' && temp[1] == L'Y' && temp[2] == '-')
			{
				joysticks[port].control[i] = (inAXIS_0<<24) + X360_RY;
			}
			else if(temp[0] == L'L' && temp[1] == L'T')
			{
				joysticks[port].control[i] = (inTRIGGER<<24) + X360_LT;
			}
			else if(temp[0] == L'R' && temp[1] == L'T')
			{
				joysticks[port].control[i] = (inTRIGGER<<24) + X360_RT;
			}
			else if(temp[0] == L'U' && temp[1] == L'P')
			{
				joysticks[port].control[i] = (inHAT<<24) + X360_UP;
			}
			else if(temp[0] == L'D' && temp[1] == L'O' && temp[2] == 'W')
			{
				joysticks[port].control[i] = (inHAT<<24) + X360_DOWN;
			}
			else if(temp[0] == L'L' && temp[1] == L'E' && temp[2] == 'F')
			{
				joysticks[port].control[i] = (inHAT<<24) + X360_LEFT;
			}
			else if(temp[0] == L'R' && temp[1] == L'I' && temp[2] == 'G')
			{
				joysticks[port].control[i] = (inHAT<<24) + X360_RIGHT;
			}
			else if(temp[0] == L'S' && temp[1] == L'T' && temp[2] == 'A')
			{
				joysticks[port].control[i] = (inBUTTON<<24) + X360_START;
			}
			else if(temp[0] == L'B' && temp[1] == L'A' && temp[2] == 'C')
			{
				joysticks[port].control[i] = (inBUTTON<<24) + X360_BACK;
			}
			else if(temp[0] == L'L' && temp[1] == L'S')
			{
				joysticks[port].control[i] = (inBUTTON<<24) + X360_LS;
			}
			else if(temp[0] == L'R' && temp[1] == L'S')
			{
				joysticks[port].control[i] = (inBUTTON<<24) + X360_RS;
			}
			else if(temp[0] == L'L' && temp[1] == L'B')
			{
				joysticks[port].control[i] = (inBUTTON<<24) + X360_LB;
			}
			else if(temp[0] == L'R' && temp[1] == L'B')
			{
				joysticks[port].control[i] = (inBUTTON<<24) + X360_RB;
			}
			else if(temp[0] == L'A' && temp[1] == '.')
			{
				joysticks[port].control[i] = (inBUTTON<<24) + X360_A;
			}
			else if(temp[0] == L'B' && temp[1] == '.')
			{
				joysticks[port].control[i] = (inBUTTON<<24) + X360_B;
			}
			else if(temp[0] == L'X' && temp[1] == '.')
			{
				joysticks[port].control[i] = (inBUTTON<<24) + X360_X;
			}
			else if(temp[0] == L'Y' && temp[1] == '.')
			{
				joysticks[port].control[i] = (inBUTTON<<24) + X360_Y;
			}			
		}

		if(temp[0]==L'K')
			joysticks[port].control[i] = (inKEY<<24) + num;
	}
}


// Load settings from file
// -----------------------
void LoadConfig()
{
	wchar SectionName[32];
	for (int port=0;port<4;port++)
	{		
		wsprintf(SectionName, L"PuruPuru_Pad_%i", port+1);
					
		host.ConfigLoadStr(SectionName, L"l_shoulder",		joysticks[port].names[MAP_LT], L"-1");
		host.ConfigLoadStr(SectionName, L"r_shoulder",		joysticks[port].names[MAP_RT], L"-1");
		host.ConfigLoadStr(SectionName, L"a_button",		joysticks[port].names[MAP_A], L"-1");
		host.ConfigLoadStr(SectionName, L"b_button",		joysticks[port].names[MAP_B], L"-1");
		host.ConfigLoadStr(SectionName, L"x_button",		joysticks[port].names[MAP_X], L"-1");
		host.ConfigLoadStr(SectionName, L"y_button",		joysticks[port].names[MAP_Y], L"-1");
		host.ConfigLoadStr(SectionName, L"start_button",	joysticks[port].names[MAP_START], L"-1");		
		host.ConfigLoadStr(SectionName, L"dpad_up",			joysticks[port].names[MAP_D_UP], L"-1");
		host.ConfigLoadStr(SectionName, L"dpad_down",		joysticks[port].names[MAP_D_DOWN], L"-1");
		host.ConfigLoadStr(SectionName, L"dpad_left",		joysticks[port].names[MAP_D_LEFT], L"-1");
		host.ConfigLoadStr(SectionName, L"dpad_right",		joysticks[port].names[MAP_D_RIGHT], L"-1");
		host.ConfigLoadStr(SectionName, L"main_x_left",		joysticks[port].names[MAP_A_XL], L"-1");
		host.ConfigLoadStr(SectionName, L"main_x_right",	joysticks[port].names[MAP_A_XR], L"-1");
		host.ConfigLoadStr(SectionName, L"main_y_up",		joysticks[port].names[MAP_A_YU], L"-1");				
		host.ConfigLoadStr(SectionName, L"main_y_down",		joysticks[port].names[MAP_A_YD], L"-1");				
		host.ConfigLoadStr(SectionName, L"halfpress",		joysticks[port].names[MAP_HALF], L"-1");
		
		joysticks[port].deadzone		= host.ConfigLoadInt(SectionName, L"deadzone",		24);
		joysticks[port].keys			= host.ConfigLoadInt(SectionName, L"keyboard",		 0);
		joysticks[port].ID				= host.ConfigLoadInt(SectionName, L"joy_id",		 0);
		joysticks[port].controllertype	= host.ConfigLoadInt(SectionName, L"controllertype", 0);
		joysticks[port].eventnum		= host.ConfigLoadInt(SectionName, L"eventnum",		 0);

		Names2Control(port);
	}
}

// Search attached devices
// -----------------------
	

void AnsiToWide(wchar* dest, const char *src)
{
	size_t len = strlen(src) + 1;
	mbstowcs_s(0, dest, len, src, len);
}

int Search_Devices()
{	
	int numjoy = SDL_NumJoysticks();

	wprintf(L"PuruPuru: %i joystics detected.\n", numjoy);

	if(numjoy > 0) 
	{
		canSDL = true;

		if(joyinfo)
		{
			delete [] joyinfo;
			joyinfo = new CONTROLLER_INFO_SDL [numjoy];
		}
		else
		{
			joyinfo = new CONTROLLER_INFO_SDL [numjoy];
		}
	
		for(int i = 0; i < numjoy; i++ )
		{
			joyinfo[i].joy			= SDL_JoystickOpen(i);
			joyinfo[i].ID			= i;
			joyinfo[i].NumAxes		= SDL_JoystickNumAxes(joyinfo[i].joy);
			joyinfo[i].NumButtons	= SDL_JoystickNumButtons(joyinfo[i].joy);
			joyinfo[i].NumBalls		= SDL_JoystickNumBalls(joyinfo[i].joy);
			joyinfo[i].NumHats		= SDL_JoystickNumHats(joyinfo[i].joy);	

			AnsiToWide( joyinfo[i].Name, SDL_JoystickName(i) );
		
			// Close if opened
			if(SDL_JoystickOpened(i))
				SDL_JoystickClose(joyinfo[i].joy);
		}
	}

	ZeroMemory( xoyinfo, sizeof( CONTROLLER_INFO_XINPUT ) * 4 );

	DWORD status;
    canXInput = false;

	for( int i = 0; i < 4; i++ )
    {        
        status = XInputGetState( i, &xoyinfo[i].state );

        if( status == ERROR_SUCCESS )
		{            
			xoyinfo[i].connected = true;
			canXInput = true;
		}			
        else
            xoyinfo[i].connected = false;
    }  	    	

	return numjoy;
}
