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

	GetJoyState(port);

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

			if( joystate[port].button[CTL_A_BUTTON] )	kcode[port] ^= key_CONT_A;
			if( joystate[port].button[CTL_B_BUTTON] )	kcode[port] ^= key_CONT_B;
			if( joystate[port].button[CTL_X_BUTTON] )	kcode[port] ^= key_CONT_X;
			if( joystate[port].button[CTL_Y_BUTTON] )	kcode[port] ^= key_CONT_Y;			
			if( joystate[port].button[CTL_START]    )	kcode[port] ^= key_CONT_START;
												
			if( joystate[port].dpad[CTL_D_PAD_UP]    )	kcode[port] ^= key_CONT_DPAD_UP;
			if( joystate[port].dpad[CTL_D_PAD_DOWN]  )	kcode[port] ^= key_CONT_DPAD_DOWN;			
			if( joystate[port].dpad[CTL_D_PAD_LEFT]  )	kcode[port] ^= key_CONT_DPAD_LEFT;			
			if( joystate[port].dpad[CTL_D_PAD_RIGHT] )	kcode[port] ^= key_CONT_DPAD_RIGHT;		
			
			w16(kcode[port] | 0xF901); //0xF901 -> buttons that are allways up on a dc
			
			// Set triggers		
			w8(joystate[port].trigger[CTL_R_SHOULDER]);										
			w8(joystate[port].trigger[CTL_L_SHOULDER]);				
	
			// Set Analog sticks (Main)
			// Reset!

			int center = 0x80;
			
			// Set analog controllers
			// Set Deadzones perhaps out of function
			
			float deadzone = (float)joysticks[port].deadzone + 1.0f;		
			
			// The value returned by SDL_JoystickGetAxis is a signed integer (-32768 to 32767)
			// The value used for the gamecube controller is an unsigned char (0 to 255)
			
			float main_stick_x = (float)joystate[port].axis[CTL_MAIN_X];
			float main_stick_y = (float)joystate[port].axis[CTL_MAIN_Y];						
			
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

void GetKeyState(char* keys)
{
	 for (int k = 0; k < 256; k++)	 
		 keys[k] = (char)(GetAsyncKeyState(k) >> 8);	

	 // Workaround for Lenovo
	 if(keys[255]) keys[255] = 0;
}

int GetStateKey (int port, int type, wchar* input )
{
	char key[256];
	GetKeyState(key);			

	int num = _wtoi(&input[1]);		

	if(input[0] == L'K')
	{
		switch(type)
		{
			case AXIS:
				{					
					if(key[num]) return 32767;
					else		 return 0;													
				}
			case TRIGGER:
				{													
					if(key[num]) return 255;
					else		 return 0;				
				}				
			case DIGITAL: return key[num];				
		}
	}

	return 0;
}

int GetStateXInput (int port, int type, wchar* input )
{	
	XInputGetState( joysticks[port].ID, &xoyinfo[port].state );
	port = joysticks[port].ID;	

	char key[256];
	if(joysticks[port].keys) GetKeyState(key);		
		
	int num = _wtoi(&input[1]);

	switch(type)
	{
		case AXIS:
			{
				if(input[0] == L'L' && input[1] == L'X' && input[2] == '+')
				{
					SHORT axis = xoyinfo[port].state.Gamepad.sThumbLX;

					if(axis > 0) return  axis;
					else		 return	 0;
				}
				else if(input[0] == L'L' && input[1] == L'X' && input[2] == '-')
				{
					SHORT axis = xoyinfo[port].state.Gamepad.sThumbLX;

					if(axis < 0) return  -axis;
					else		 return   0;
				}
				else if(input[0] == L'L' && input[1] == L'Y' && input[2] == '+')
				{
					SHORT axis = xoyinfo[port].state.Gamepad.sThumbLY;

					if(axis > 0) return  axis;
					else		 return	 0;
				}
				else if(input[0] == L'L' && input[1] == L'Y' && input[2] == '-')
				{
					SHORT axis = xoyinfo[port].state.Gamepad.sThumbLY;

					if(axis < 0) return  -axis;
					else		 return	 0;
				}
				else if(input[0] == L'R' && input[1] == L'X' && input[2] == '+')
				{
					SHORT axis = xoyinfo[port].state.Gamepad.sThumbRX;

					if(axis > 0) return  axis;
					else		 return	 0;
				}
				else if(input[0] == L'R' && input[1] == L'X' && input[2] == '-')
				{
					SHORT axis = xoyinfo[port].state.Gamepad.sThumbRX;

					if(axis < 0) return  -axis;
					else		 return	 0;
				}
				else if(input[0] == L'R' && input[1] == L'Y' && input[2] == '+')
				{
					SHORT axis = xoyinfo[port].state.Gamepad.sThumbRY;

					if(axis > 0) return  axis;
					else		 return	 0;
				}
				else if(input[0] == L'R' && input[1] == L'Y' && input[2] == '-')
				{
					SHORT axis = xoyinfo[port].state.Gamepad.sThumbRY;

					if(axis < 0) return  -axis;
					else		 return	 0;
				}
				else if(input[0] == L'L' && input[1] == L'T')
				{
					return xoyinfo[port].state.Gamepad.bLeftTrigger * 128;
				}
				else if(input[0] == L'R' && input[1] == L'T')
				{
					return xoyinfo[port].state.Gamepad.bRightTrigger * 128;
				}
				else if(input[0] == L'U' && input[1] == L'P')
				{
					if( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP)
						return 32767;
					else
						return 0;
				}
				else if(input[0] == L'D' && input[1] == L'O' && input[2] == 'W')
				{
					if( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN)
						return 32767;
					else
						return 0;
				}
				else if(input[0] == L'L' && input[1] == L'E' && input[2] == 'F')
				{
					if( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT)
						return 32767;
					else
						return 0;
				}
				else if(input[0] == L'R' && input[1] == L'I' && input[2] == 'G')
				{
					if( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)
						return 32767;
					else
						return 0;
				}
				else if(input[0] == L'S' && input[1] == L'T' && input[2] == 'A')
				{
					if( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_START)
						return 32767;
					else
						return 0;
				}
				else if(input[0] == L'B' && input[1] == L'A' && input[2] == 'C')
				{
					if( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK)
						return 32767;
					else
						return 0;
				}
				else if(input[0] == L'L' && input[1] == L'S')
				{
					if( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB)
						return 32767;
					else
						return 0;
				}
				else if(input[0] == L'R' && input[1] == L'S')
				{
					if( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB)
						return 32767;
					else
						return 0;
				}
				else if(input[0] == L'L' && input[1] == L'B')
				{
					if( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER)
						return 32767;
					else
						return 0;
				}
				else if(input[0] == L'R' && input[1] == L'B')
				{
					if( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER)
						return 32767;
					else
						return 0;
				}
				else if(input[0] == L'A' && input[1] == '.')
				{
					if( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_A)
						return 32767;
					else
						return 0;
				}
				else if(input[0] == L'B' && input[1] == '.')
				{
					if( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_B)
						return 32767;
					else
						return 0;
				}
				else if(input[0] == L'X' && input[1] == '.')
				{
					if( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_X)
						return 32767;
					else
						return 0;
				}
				else if(input[0] == L'Y' && input[1] == '.')
				{
					if( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_Y)
						return 32767;
					else
						return 0;
				}
				else if(input[0] == L'K')
				{
					if(key[num]) return 32767;
					else		 return 0;
				}
						
			}
		case TRIGGER:
			{
				if(input[0] == L'L' && input[1] == L'X' && input[2] == '+')
				{
					SHORT axis = xoyinfo[port].state.Gamepad.sThumbLX/128;

					if(axis > 255) axis = 255;

					if(axis > 0) return  axis;
					else		 return	 0;
				}
				else if(input[0] == L'L' && input[1] == L'X' && input[2] == '-')
				{
					SHORT axis = xoyinfo[port].state.Gamepad.sThumbLX/128;

					if(axis < -255) axis = -255;

					if(axis < 0) return  -axis;
					else		 return   0;
				}
				else if(input[0] == L'L' && input[1] == L'Y' && input[2] == '+')
				{
					SHORT axis = xoyinfo[port].state.Gamepad.sThumbLY/128;

					if(axis > 255) axis = 255;

					if(axis > 0) return  axis;
					else		 return	 0;
				}
				else if(input[0] == L'L' && input[1] == L'Y' && input[2] == '-')
				{
					SHORT axis = xoyinfo[port].state.Gamepad.sThumbLY/128;

					if(axis < -255) axis = -255;

					if(axis < 0) return  -axis;
					else		 return   0;
				}
				else if(input[0] == L'R' && input[1] == L'X' && input[2] == '+')
				{
					SHORT axis = xoyinfo[port].state.Gamepad.sThumbRX/128;

					if(axis > 255) axis = 255;

					if(axis > 0) return  axis;
					else		 return	 0;
				}
				else if(input[0] == L'R' && input[1] == L'X' && input[2] == '-')
				{
					SHORT axis = xoyinfo[port].state.Gamepad.sThumbRX/128;

					if(axis < -255) axis = -255;

					if(axis < 0) return  -axis;
					else		 return   0;
				}
				else if(input[0] == L'R' && input[1] == L'Y' && input[2] == '+')
				{
					SHORT axis = xoyinfo[port].state.Gamepad.sThumbRY/128;

					if(axis > 255) axis = 255;

					if(axis > 0) return  axis;
					else		 return	 0;
				}
				else if(input[0] == L'R' && input[1] == L'Y' && input[2] == '-')
				{
					SHORT axis = xoyinfo[port].state.Gamepad.sThumbRY/128;

					if(axis < -255) axis = -255;

					if(axis < 0) return  -axis;
					else		 return   0;
				}
				else if(input[0] == L'L' && input[1] == L'T')
				{
					return xoyinfo[port].state.Gamepad.bLeftTrigger;
				}
				else if(input[0] == L'R' && input[1] == L'T')
				{
					return xoyinfo[port].state.Gamepad.bRightTrigger;
				}
				else if(input[0] == L'U' && input[1] == L'P')
				{
					if( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP)
						return 255;
					else
						return 0;
				}
				else if(input[0] == L'D' && input[1] == L'O' && input[2] == 'W')
				{
					if( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN)
						return 255;
					else
						return 0;
				}
				else if(input[0] == L'L' && input[1] == L'E' && input[2] == 'F')
				{
					if( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT)
						return 255;
					else
						return 0;
				}
				else if(input[0] == L'R' && input[1] == L'I' && input[2] == 'G')
				{
					if( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)
						return 255;
					else
						return 0;
				}
				else if(input[0] == L'S' && input[1] == L'T' && input[2] == 'A')
				{
					if( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_START)
						return 255;
					else
						return 0;
				}
				else if(input[0] == L'B' && input[1] == L'A' && input[2] == 'C')
				{
					if( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK)
						return 255;
					else
						return 0;
				}
				else if(input[0] == L'L' && input[1] == L'S')
				{
					if( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB)
						return 255;
					else
						return 0;
				}
				else if(input[0] == L'R' && input[1] == L'S')
				{
					if( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB)
						return 255;
					else
						return 0;
				}
				else if(input[0] == L'L' && input[1] == L'B')
				{
					if( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER)
						return 255;
					else
						return 0;
				}
				else if(input[0] == L'R' && input[1] == L'B')
				{
					if( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER)
						return 255;
					else
						return 0;
				}
				else if(input[0] == L'A' && input[1] == '.')
				{
					if( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_A)
						return 255;
					else
						return 0;
				}
				else if(input[0] == L'B' && input[1] == '.')
				{
					if( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_B)
						return 255;
					else
						return 0;
				}
				else if(input[0] == L'X' && input[1] == '.')
				{
					if( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_X)
						return 255;
					else
						return 0;
				}
				else if(input[0] == L'Y' && input[1] == '.')
				{
					if( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_Y)
						return 255;
					else
						return 0;
				}
				else if(input[0] == L'K')
					{
						if(key[num]) return 255;
						else		 return 0;
					}
						
			}				
		case DIGITAL:	
			{				
				if(input[0] == L'L' && input[1] == L'X' && input[2] == '+')
				{
					SHORT axis = xoyinfo[port].state.Gamepad.sThumbLX;

					if(axis > 22000) return  1;
					else			 return	 0;
				}
				else if(input[0] == L'L' && input[1] == L'X' && input[2] == '-')
				{
					SHORT axis = xoyinfo[port].state.Gamepad.sThumbLX;

					if(axis < -22000) return  1;
					else		 	 return	 0;
				}
				else if(input[0] == L'L' && input[1] == L'Y' && input[2] == '+')
				{
					SHORT axis = xoyinfo[port].state.Gamepad.sThumbLY;

					if(axis > 22000) return  1;
					else			 return	 0;
				}
				else if(input[0] == L'L' && input[1] == L'Y' && input[2] == '-')
				{
					SHORT axis = xoyinfo[port].state.Gamepad.sThumbLY;

					if(axis < -22000) return  1;
					else			 return	 0;
				}
				else if(input[0] == L'R' && input[1] == L'X' && input[2] == '+')
				{
					SHORT axis = xoyinfo[port].state.Gamepad.sThumbRX;

					if(axis > 22000) return  1;
					else			 return	 0;
				}
				else if(input[0] == L'R' && input[1] == L'X' && input[2] == '-')
				{
					SHORT axis = xoyinfo[port].state.Gamepad.sThumbRX;

					if(axis < -22000) return  1;
					else			 return	 0;
				}
				else if(input[0] == L'R' && input[1] == L'Y' && input[2] == '+')
				{
					SHORT axis = xoyinfo[port].state.Gamepad.sThumbRY;

					if(axis > 22000) return  1;
					else			 return	 0;
				}
				else if(input[0] == L'R' && input[1] == L'Y' && input[2] == '-')
				{
					SHORT axis = xoyinfo[port].state.Gamepad.sThumbRY;

					if(axis < -22000) return  1;
					else			 return	 0;
				}
				else if(input[0] == L'L' && input[1] == L'T')
				{
					if(xoyinfo[port].state.Gamepad.bLeftTrigger > 100) 
						return 1;
					else
						return 0;										
				}
				else if(input[0] == L'R' && input[1] == L'T')
				{
					if(xoyinfo[port].state.Gamepad.bRightTrigger > 100)
						return 1;
					else
						return 0;
				}
				else if(input[0] == L'U' && input[1] == L'P')  
					return( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP);			
				else if(input[0] == L'D' && input[1] == L'O' && input[2] == 'W')
					return( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN);				
				else if(input[0] == L'L' && input[1] == L'E' && input[2] == 'F')
					return( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT);				
				else if(input[0] == L'R' && input[1] == L'I' && input[2] == 'G')
					return( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);				
				else if(input[0] == L'S' && input[1] == L'T' && input[2] == 'A')
					return( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_START);								
				else if(input[0] == L'B' && input[1] == L'A' && input[2] == 'C')
					return( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK);

				else if(input[0] == L'L' && input[1] == L'S')
					return( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB);				
				else if(input[0] == L'R' && input[1] == L'S')   
					return( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB);
				else if(input[0] == L'L' && input[1] == L'B')   
					return( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
				else if(input[0] == L'R' && input[1] == L'B')
					return( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);				

				else if(input[0] == L'A' && input[1] == '.') return( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_A);				
				else if(input[0] == L'B' && input[1] == '.') return( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_B);				
				else if(input[0] == L'X' && input[1] == '.') return( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_X);				
				else if(input[0] == L'Y' && input[1] == '.') return( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_Y);				
				else if(input[0] == L'K') return key[num];		
				
						
			}
	}

	return 0;
}


// 22000 AXIS to BUTTON threshold.

int GetStateSDL (int port, int type, wchar* input )
{							
	port = joysticks[port].ID;

	char key[256];
	if(joysticks[port].keys) GetKeyState(key);

	int num = _wtoi(&input[1]); // wtoi Works a lot better than I thought.
	bool plus = false;

	if(input[0] == 'A' && input[sizeof(input)/2] == '+') plus = true;	

	int currentHat = SDL_JoystickGetHat(joystate[port].joy, 0);

	switch(type)
	{
		case AXIS:
			{
				switch(input[0])
				{
				case L'A': 
					{
						int axis = SDL_JoystickGetAxis(joystate[port].joy, num);
						
						if(plus)
						{
							if(axis > 0)  return  axis;						
							else		  return 0;
						}
						else
						{
							if(axis < 0)  return -axis;						
							else		  return 0;
						}
					}
				case L'B':
					{
						if( SDL_JoystickGetButton(joystate[port].joy, num) )
							return 32767;
						else
							return 0;
					}
				case L'H':
					{
						if(num & currentHat) return 32767;
						else				 return 0;	
					}
				case L'K':
					{
						if(key[num]) return 32767;
						else		 return 0;
					}
				}				
			}
		case TRIGGER:
			{
				switch(input[0])
				{
				case L'A': 
					{
					
						int axis = SDL_JoystickGetAxis(joystate[port].joy, num)/128;

						if(plus)
						{
							if(axis > 255) axis = 255;							
							
							if(axis > 0)  return  axis;						
							else				  return 0;
						}
						else
						{
							if(axis < -255) axis = -255;
							
							if(axis < 0)  return -axis;						
							else				  return 0;
						}
						
						
					
					}
				case L'B': 
					{
						if( SDL_JoystickGetButton(joystate[port].joy, num) )
							return 255;
						else
							return 0;
					}
				case L'H': 
					{
						if(num & currentHat) return 255;
						else				 return 0;	
					}
				case L'K':
					{
						if(key[num]) return 255;
						else		 return 0;
					}
				}
			}				
		case DIGITAL:	
			{
				switch(input[0])
				{
				case L'A': 
					{						
						int axis = SDL_JoystickGetAxis(joystate[port].joy, num);
						
						if(plus)
						{
							if(axis > 22000) return 1;
							else		     return 0;
						}
						else						
						{
							if(axis <-22000) return 1;
							else             return 0;
						}
					}
				case L'B': return SDL_JoystickGetButton(joystate[port].joy, num);
				case L'H': 
					{	
						if(num & currentHat) return 1;
						else				 return 0;						
					}
				case L'K': return key[num];
				}
			}
	}

	return 0;
}

void GetJoyState(int controller)
{	
	if(joysticks[controller].controllertype == CTL_TYPE_JOYSTICK_SDL)
	{	
		SDL_JoystickUpdate();

		joystate[controller].axis[CTL_MAIN_X] = 0;
		joystate[controller].axis[CTL_MAIN_Y] = 0;

		joystate[controller].axis[CTL_MAIN_X] -= GetStateSDL( controller, AXIS, joysticks[controller].control[MAP_A_XL] );
		joystate[controller].axis[CTL_MAIN_X] += GetStateSDL( controller, AXIS, joysticks[controller].control[MAP_A_XR] );				
		
		joystate[controller].axis[CTL_MAIN_Y] -= GetStateSDL( controller, AXIS, joysticks[controller].control[MAP_A_YU] );		
		joystate[controller].axis[CTL_MAIN_Y] += GetStateSDL( controller, AXIS, joysticks[controller].control[MAP_A_YD] );		

		joystate[controller].halfpress = GetStateSDL( controller, DIGITAL, joysticks[controller].control[MAP_HALF] );

		joystate[controller].trigger[CTL_L_SHOULDER] = GetStateSDL( controller, TRIGGER, joysticks[controller].control[MAP_LT] );
		joystate[controller].trigger[CTL_R_SHOULDER] = GetStateSDL( controller, TRIGGER, joysticks[controller].control[MAP_RT] );
		
		if ( joystate[controller].halfpress ) 
		{
			joystate[controller].trigger[CTL_L_SHOULDER] /= 2;
			joystate[controller].trigger[CTL_R_SHOULDER] /= 2;

			joystate[controller].axis[CTL_MAIN_X] /= 2;
			joystate[controller].axis[CTL_MAIN_Y] /= 2;
		}				
				
		joystate[controller].button[CTL_A_BUTTON]   = GetStateSDL( controller, DIGITAL,  joysticks[controller].control[MAP_A] );
		joystate[controller].button[CTL_B_BUTTON]   = GetStateSDL( controller, DIGITAL,  joysticks[controller].control[MAP_B] );
		joystate[controller].button[CTL_X_BUTTON]   = GetStateSDL( controller, DIGITAL,  joysticks[controller].control[MAP_X] );
		joystate[controller].button[CTL_Y_BUTTON]   = GetStateSDL( controller, DIGITAL,  joysticks[controller].control[MAP_Y] );
		joystate[controller].button[CTL_START]	    = GetStateSDL( controller, DIGITAL,  joysticks[controller].control[MAP_START] );				

		joystate[controller].dpad[CTL_D_PAD_UP]		= GetStateSDL( controller, DIGITAL,  joysticks[controller].control[MAP_D_UP] );
		joystate[controller].dpad[CTL_D_PAD_DOWN]	= GetStateSDL( controller, DIGITAL,  joysticks[controller].control[MAP_D_DOWN] );	
		joystate[controller].dpad[CTL_D_PAD_LEFT]	= GetStateSDL( controller, DIGITAL,  joysticks[controller].control[MAP_D_LEFT] );
		joystate[controller].dpad[CTL_D_PAD_RIGHT]	= GetStateSDL( controller, DIGITAL,  joysticks[controller].control[MAP_D_RIGHT] );
	}
	else if(joysticks[controller].controllertype ==  CTL_TYPE_JOYSTICK_XINPUT)
	{
		XInputGetState( controller, &xoyinfo[controller].state );

		joystate[controller].axis[CTL_MAIN_X] = 0;
		joystate[controller].axis[CTL_MAIN_Y] = 0;

		joystate[controller].axis[CTL_MAIN_X] -= GetStateXInput( controller, AXIS, joysticks[controller].control[MAP_A_XL] );
		joystate[controller].axis[CTL_MAIN_X] += GetStateXInput( controller, AXIS, joysticks[controller].control[MAP_A_XR] );				
		
		joystate[controller].axis[CTL_MAIN_Y] -= GetStateXInput( controller, AXIS, joysticks[controller].control[MAP_A_YU] );		
		joystate[controller].axis[CTL_MAIN_Y] += GetStateXInput( controller, AXIS, joysticks[controller].control[MAP_A_YD] );		

		joystate[controller].halfpress = GetStateXInput( controller, DIGITAL, joysticks[controller].control[MAP_HALF] );

		joystate[controller].trigger[CTL_L_SHOULDER] = GetStateXInput( controller, TRIGGER, joysticks[controller].control[MAP_LT] );
		joystate[controller].trigger[CTL_R_SHOULDER] = GetStateXInput( controller, TRIGGER, joysticks[controller].control[MAP_RT] );
		
		if ( joystate[controller].halfpress ) 
		{
			joystate[controller].trigger[CTL_L_SHOULDER] /= 2;
			joystate[controller].trigger[CTL_R_SHOULDER] /= 2;

			joystate[controller].axis[CTL_MAIN_X] /= 2;
			joystate[controller].axis[CTL_MAIN_Y] /= 2;
		}				
				
		joystate[controller].button[CTL_A_BUTTON]   = GetStateXInput( controller, DIGITAL,  joysticks[controller].control[MAP_A] );
		joystate[controller].button[CTL_B_BUTTON]   = GetStateXInput( controller, DIGITAL,  joysticks[controller].control[MAP_B] );
		joystate[controller].button[CTL_X_BUTTON]   = GetStateXInput( controller, DIGITAL,  joysticks[controller].control[MAP_X] );
		joystate[controller].button[CTL_Y_BUTTON]   = GetStateXInput( controller, DIGITAL,  joysticks[controller].control[MAP_Y] );
		joystate[controller].button[CTL_START]	    = GetStateXInput( controller, DIGITAL,  joysticks[controller].control[MAP_START] );				

		joystate[controller].dpad[CTL_D_PAD_UP]		= GetStateXInput( controller, DIGITAL,  joysticks[controller].control[MAP_D_UP] );
		joystate[controller].dpad[CTL_D_PAD_DOWN]	= GetStateXInput( controller, DIGITAL,  joysticks[controller].control[MAP_D_DOWN] );	
		joystate[controller].dpad[CTL_D_PAD_LEFT]	= GetStateXInput( controller, DIGITAL,  joysticks[controller].control[MAP_D_LEFT] );
		joystate[controller].dpad[CTL_D_PAD_RIGHT]	= GetStateXInput( controller, DIGITAL,  joysticks[controller].control[MAP_D_RIGHT] );
	}
	else if(joysticks[controller].controllertype == CTL_TYPE_KEYBOARD)
	{			
		joystate[controller].axis[CTL_MAIN_X] = 0;
		joystate[controller].axis[CTL_MAIN_Y] = 0;

		joystate[controller].axis[CTL_MAIN_X] -= GetStateKey( controller, AXIS, joysticks[controller].control[MAP_A_XL] );
		joystate[controller].axis[CTL_MAIN_X] += GetStateKey( controller, AXIS, joysticks[controller].control[MAP_A_XR] );				
		
		joystate[controller].axis[CTL_MAIN_Y] -= GetStateKey( controller, AXIS, joysticks[controller].control[MAP_A_YU] );		
		joystate[controller].axis[CTL_MAIN_Y] += GetStateKey( controller, AXIS, joysticks[controller].control[MAP_A_YD] );		

		joystate[controller].halfpress = GetStateKey( controller, DIGITAL, joysticks[controller].control[MAP_HALF] );

		joystate[controller].trigger[CTL_L_SHOULDER] = GetStateKey( controller, TRIGGER, joysticks[controller].control[MAP_LT] );
		joystate[controller].trigger[CTL_R_SHOULDER] = GetStateKey( controller, TRIGGER, joysticks[controller].control[MAP_RT] );
		
		if ( joystate[controller].halfpress ) 
		{
			joystate[controller].trigger[CTL_L_SHOULDER] /= 2;
			joystate[controller].trigger[CTL_R_SHOULDER] /= 2;

			joystate[controller].axis[CTL_MAIN_X] /= 2;
			joystate[controller].axis[CTL_MAIN_Y] /= 2;
		}				
				
		joystate[controller].button[CTL_A_BUTTON]   = GetStateKey( controller, DIGITAL,  joysticks[controller].control[MAP_A] );
		joystate[controller].button[CTL_B_BUTTON]   = GetStateKey( controller, DIGITAL,  joysticks[controller].control[MAP_B] );
		joystate[controller].button[CTL_X_BUTTON]   = GetStateKey( controller, DIGITAL,  joysticks[controller].control[MAP_X] );
		joystate[controller].button[CTL_Y_BUTTON]   = GetStateKey( controller, DIGITAL,  joysticks[controller].control[MAP_Y] );
		joystate[controller].button[CTL_START]	    = GetStateKey( controller, DIGITAL,  joysticks[controller].control[MAP_START] );				

		joystate[controller].dpad[CTL_D_PAD_UP]		= GetStateKey( controller, DIGITAL,  joysticks[controller].control[MAP_D_UP] );
		joystate[controller].dpad[CTL_D_PAD_DOWN]	= GetStateKey( controller, DIGITAL,  joysticks[controller].control[MAP_D_DOWN] );	
		joystate[controller].dpad[CTL_D_PAD_LEFT]	= GetStateKey( controller, DIGITAL,  joysticks[controller].control[MAP_D_LEFT] );
		joystate[controller].dpad[CTL_D_PAD_RIGHT]	= GetStateKey( controller, DIGITAL,  joysticks[controller].control[MAP_D_RIGHT] );		
	}
	
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
						
		host.ConfigSaveStr(SectionName, L"l_shoulder",		joysticks[port].control[MAP_LT]);
		host.ConfigSaveStr(SectionName, L"r_shoulder",		joysticks[port].control[MAP_RT]);
		host.ConfigSaveStr(SectionName, L"a_button",		joysticks[port].control[MAP_A]);
		host.ConfigSaveStr(SectionName, L"b_button",		joysticks[port].control[MAP_B]);
		host.ConfigSaveStr(SectionName, L"x_button",		joysticks[port].control[MAP_X]);
		host.ConfigSaveStr(SectionName, L"y_button",		joysticks[port].control[MAP_Y]);
		host.ConfigSaveStr(SectionName, L"start_button",	joysticks[port].control[MAP_START]);		
		host.ConfigSaveStr(SectionName, L"dpad_up",			joysticks[port].control[MAP_D_UP]);
		host.ConfigSaveStr(SectionName, L"dpad_down",		joysticks[port].control[MAP_D_DOWN]);
		host.ConfigSaveStr(SectionName, L"dpad_left",		joysticks[port].control[MAP_D_LEFT]);
		host.ConfigSaveStr(SectionName, L"dpad_right",		joysticks[port].control[MAP_D_RIGHT]);
		host.ConfigSaveStr(SectionName, L"main_x_left",		joysticks[port].control[MAP_A_XL]);
		host.ConfigSaveStr(SectionName, L"main_x_right",	joysticks[port].control[MAP_A_XR]);
		host.ConfigSaveStr(SectionName, L"main_y_up",		joysticks[port].control[MAP_A_YU]);				
		host.ConfigSaveStr(SectionName, L"main_y_down",		joysticks[port].control[MAP_A_YD]);
		host.ConfigSaveStr(SectionName, L"halfpress",		joysticks[port].control[MAP_HALF]);

		host.ConfigSaveInt(SectionName, L"deadzone",		joysticks[port].deadzone);
		host.ConfigSaveInt(SectionName, L"keyboard",		joysticks[port].keys);		
		host.ConfigSaveInt(SectionName, L"joy_id",			joysticks[port].ID);
		host.ConfigSaveInt(SectionName, L"controllertype",	joysticks[port].controllertype);
		host.ConfigSaveInt(SectionName, L"eventnum",		joysticks[port].eventnum);
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

					
		host.ConfigLoadStr(SectionName, L"l_shoulder",		joysticks[port].control[MAP_LT],	  L"-1");
		host.ConfigLoadStr(SectionName, L"r_shoulder",		joysticks[port].control[MAP_RT],	  L"-1");
		host.ConfigLoadStr(SectionName, L"a_button",		joysticks[port].control[MAP_A],		  L"-1");
		host.ConfigLoadStr(SectionName, L"b_button",		joysticks[port].control[MAP_B],		  L"-1");
		host.ConfigLoadStr(SectionName, L"x_button",		joysticks[port].control[MAP_X],		  L"-1");
		host.ConfigLoadStr(SectionName, L"y_button",		joysticks[port].control[MAP_Y],		  L"-1");
		host.ConfigLoadStr(SectionName, L"start_button",	joysticks[port].control[MAP_START],   L"-1");		
		host.ConfigLoadStr(SectionName, L"dpad_up",			joysticks[port].control[MAP_D_UP],	  L"-1");
		host.ConfigLoadStr(SectionName, L"dpad_down",		joysticks[port].control[MAP_D_DOWN],  L"-1");
		host.ConfigLoadStr(SectionName, L"dpad_left",		joysticks[port].control[MAP_D_LEFT],  L"-1");
		host.ConfigLoadStr(SectionName, L"dpad_right",		joysticks[port].control[MAP_D_RIGHT], L"-1");
		host.ConfigLoadStr(SectionName, L"main_x_left",		joysticks[port].control[MAP_A_XL],	  L"-1");
		host.ConfigLoadStr(SectionName, L"main_x_right",	joysticks[port].control[MAP_A_XR],	  L"-1");
		host.ConfigLoadStr(SectionName, L"main_y_up",		joysticks[port].control[MAP_A_YU],	  L"-1");				
		host.ConfigLoadStr(SectionName, L"main_y_down",		joysticks[port].control[MAP_A_YD],	  L"-1");				
		host.ConfigLoadStr(SectionName, L"halfpress",		joysticks[port].control[MAP_HALF],	  L"-1");
		

		joysticks[port].deadzone		= host.ConfigLoadInt(SectionName, L"deadzone",		24);
		joysticks[port].keys			= host.ConfigLoadInt(SectionName, L"keyboard",		 0);
		joysticks[port].ID				= host.ConfigLoadInt(SectionName, L"joy_id",		 0);
		joysticks[port].controllertype	= host.ConfigLoadInt(SectionName, L"controllertype", 0);
		joysticks[port].eventnum		= host.ConfigLoadInt(SectionName, L"eventnum",		 0);
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
