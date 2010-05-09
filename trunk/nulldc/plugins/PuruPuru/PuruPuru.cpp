//////////////////////////////////////////////////////////////////////////////////////////
// Project description
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
// Name: nullDC 
// Description: A nullDC Compatible Input Plugin
//
// Author: Falcon4ever (nullDC@falcon4ever.com)
// Site: www.multigesture.net
// Copyright (C) 2007-2009 nullDC Project.
//


#include "PuruPuru.h"

//////////////////////////////////////////////////////////////////////////////////////////
// Variables
// ¯¯¯¯¯¯¯¯¯
HINSTANCE PuruPuru_hInst = NULL;
emu_info host;
u32 current_port = 0;
bool emulator_running  = FALSE;

CONTROLLER_STATE joystate[4];
CONTROLLER_MAPPING joysticks[4];
CONTROLLER_INFO	*joyinfo = 0;

//////////////////////////////////////////////////////////////////////////////////////////
// DllMain 
// ¯¯¯¯¯¯¯
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
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯

void EXPORT_CALL dcGetInterface(plugin_interface* info)
{
	// Set plugin info
	info->InterfaceVersion = PLUGIN_I_F_VERSION;
	info->common.InterfaceVersion = MAPLE_PLUGIN_I_F_VERSION;
	wcscpy(info->common.Name, L"PuruPuru input plugin v" _T(INPUT_VERSION) L" by Falcon4ever [" _T(__DATE__) L"]");
	
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
	wcscpy(info->maple.devices[0].Name, L"PuruPuru input plugin v" _T(INPUT_VERSION) L" by Falcon4ever [" _T(__DATE__) L"]");
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
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯

// Load plugin
// ¯¯¯¯¯¯¯¯¯¯¯
// Notes: Called when plugin is loaded by the emu, the param has some handy functions so i make a copy of it ;).
s32 FASTCALL Load(emu_info* emu)
{	
	wprintf(L"PuruPuru -> Load\n");

	memcpy(&host, emu, sizeof(host));
	
	return rv_ok;
}

// Unload plugin
// ¯¯¯¯¯¯¯¯¯¯¯¯¯
// Notes: Called when plugin is unloaded by emulator (only if Load was called before).
void FASTCALL Unload()
{
	wprintf(L"PuruPuru -> Unload\n");
}

//////////////////////////////////////////////////////////////////////////////////////////
// Maple Input Plugin Functions
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯

// CreateMain
// ¯¯¯¯¯¯¯¯¯¯
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

	if (id!=0)
		return rv_error;

	inst->data = inst;
	inst->dma = ControllerDMA;
	
	wchar temp[512];
	swprintf(temp, L"Config keys for Player %d", (inst->port >> 6) + 1);	
	u32 ckid = host.AddMenuItem(rootmenu, -1, temp, ConfigMenuCallback, 0);

	// Set the user defined pointer for the menu to the device instance, so we can tell for which port the menu was called ;)	
	MenuItem mi;
	mi.PUser = inst;
	host.SetMenuItem(ckid, &mi, MIM_PUser);
	
	return rv_ok;
}

// CreateSub
// ¯¯¯¯¯¯¯¯¯
// Notes: Called to create a sub device, uses same params as CreateMain
s32 FASTCALL CreateSub(maple_subdevice_instance* inst, u32 id, u32 flags, u32 rootmenu)
{	
	wprintf(L"PuruPuru -> CreateSub\n");

	// This plugin has no subdevices so return an error.
	return rv_error;
}

// Init PAD (start emulation)
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
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
		
	if(SDL_Init(SDL_INIT_JOYSTICK ) < 0)
	{		
		MessageBoxA(NULL, SDL_GetError(), "Could not initialize SDL!", MB_ICONERROR);		
		return rv_error;
	}

	LoadConfig();	// Load joystick mapping
	
	u32 port = ((maple_device_instance*)data)->port >> 6;

	if(joysticks[port].enabled)
		joystate[port].joy = SDL_JoystickOpen(joysticks[port].ID);

	return rv_ok;
}

// Shutdown PAD (stop emulation)
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
// Called when emuation is terminated
// data: the inst->data pointer as filled by the Create* functions
// id: device index on the dcGetInterface
// Additional notes: 
// Called only if Init() was called ;)
void FASTCALL Term(void* data, u32 id)
{
	u32 port = ((maple_device_instance*)data)->port >> 6;

	if(joysticks[port].enabled)
	{
		if(joystate[port].joy)
			SDL_JoystickClose(joystate[port].joy);
	}
	
	SDL_Quit();

	delete [] joyinfo;
	emulator_running = FALSE;

	//kill whatever you did on Init()
	wprintf(L"PuruPuru -> Term\n");
}

// Free memory (quit emulator)?
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
// Notes: Destroy the device
// data: the inst->data pointer as filled by the Create* functions
// id: device index on the dcGetInterface
void FASTCALL Destroy(void* data, u32 id)
{
	//Free any memory allocted (if any)
	wprintf(L"PuruPuru -> Destroy\n");
}

//////////////////////////////////////////////////////////////////////////////////////////
// General Plugin Functions
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯

// ControllerDMA
// ¯¯¯¯¯¯¯¯¯¯¯¯¯
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
			if (joystate[port].buttons[CTL_A_BUTTON])	kcode[port] ^= key_CONT_A;
			if (joystate[port].buttons[CTL_B_BUTTON])	kcode[port] ^= key_CONT_B;
			if (joystate[port].buttons[CTL_X_BUTTON])	kcode[port] ^= key_CONT_X;
			if (joystate[port].buttons[CTL_Y_BUTTON])	kcode[port] ^= key_CONT_Y;			
			if (joystate[port].buttons[CTL_START])		kcode[port] ^= key_CONT_START;

			if(joysticks[port].controllertype == CTL_TYPE_JOYSTICK)
			{
				if(joystate[port].dpad == SDL_HAT_LEFTUP	|| joystate[port].dpad == SDL_HAT_UP	|| joystate[port].dpad == SDL_HAT_RIGHTUP )		kcode[port] ^= key_CONT_DPAD_UP;
				if(joystate[port].dpad == SDL_HAT_LEFTUP	|| joystate[port].dpad == SDL_HAT_LEFT	|| joystate[port].dpad == SDL_HAT_LEFTDOWN )	kcode[port] ^= key_CONT_DPAD_LEFT;
				if(joystate[port].dpad == SDL_HAT_LEFTDOWN	|| joystate[port].dpad == SDL_HAT_DOWN	|| joystate[port].dpad == SDL_HAT_RIGHTDOWN )	kcode[port] ^= key_CONT_DPAD_DOWN;
				if(joystate[port].dpad == SDL_HAT_RIGHTUP	|| joystate[port].dpad == SDL_HAT_RIGHT	|| joystate[port].dpad == SDL_HAT_RIGHTDOWN )	kcode[port] ^= key_CONT_DPAD_RIGHT;
			}
			else
			{
				if(joystate[port].dpad2[CTL_D_PAD_UP])
					kcode[port] ^= key_CONT_DPAD_UP;
				if(joystate[port].dpad2[CTL_D_PAD_DOWN])
					kcode[port] ^= key_CONT_DPAD_DOWN;
				if(joystate[port].dpad2[CTL_D_PAD_LEFT])
					kcode[port] ^= key_CONT_DPAD_LEFT;
				if(joystate[port].dpad2[CTL_D_PAD_RIGHT])
					kcode[port] ^= key_CONT_DPAD_RIGHT;
			}
			w16(kcode[port] | 0xF901); //0xF901 -> buttons that are allways up on a dc
			
			// Set shoulder buttons (analog)
			int triggervalue = 255;
			if (joystate[port].halfpress)
				triggervalue = 100;

			// Set shoulder buttons (actually analog)
			if (joystate[port].buttons[CTL_L_SHOULDER])
			{
				w8(triggervalue);				
			}
			else
			{
				w8(0);	// not pressed?
			}

			if (joystate[port].buttons[CTL_R_SHOULDER])	
			{
				w8(triggervalue);				
			}
			else
			{
				w8(0);	// not pressed?
			}

			// Set Analog sticks (Main)
			// Reset!
			int base = 0x80;
			// Set analog controllers
			// Set Deadzones perhaps out of function
			int deadzone = (int)(((float)(128.00/100.00)) * (float)(joysticks[port].deadzone+1));
			int deadzone2 = (int)(((float)(128.00/100.00)) * (float)(joysticks[port].deadzone+1));

			// Adjust range
			// The value returned by SDL_JoystickGetAxis is a signed integer (-32768 to 32768)
			// The value used for the gamecube controller is an unsigned char (0 to 255)
			int main_stick_x = (joystate[port].axis[CTL_MAIN_X]>>8);
			int main_stick_y = (joystate[port].axis[CTL_MAIN_Y]>>8);

			// Quick fix
			if(main_stick_x > 127)
				main_stick_x = 127;
			if(main_stick_y > 127)
				main_stick_y = 127;		

			if(main_stick_x < -128)
				main_stick_x = -128;
			if(main_stick_y < -128)
				main_stick_y = -128;
			
			if ((main_stick_x < deadzone2)	|| (main_stick_x > deadzone))	
			{
				w8(base + main_stick_x);
			}
			else
				w8(base); // base

			if ((main_stick_y < deadzone2)	|| (main_stick_y > deadzone))
			{
				w8(base + main_stick_y);
			}
			else
				w8(base); // base
				
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

void GetJoyState(int controller)
{
	SDL_JoystickUpdate();

	joystate[controller].axis[CTL_MAIN_X] = SDL_JoystickGetAxis(joystate[controller].joy, joysticks[controller].axis[CTL_MAIN_X]);
	joystate[controller].axis[CTL_MAIN_Y] = SDL_JoystickGetAxis(joystate[controller].joy, joysticks[controller].axis[CTL_MAIN_Y]);

	joystate[controller].buttons[CTL_L_SHOULDER] = SDL_JoystickGetButton(joystate[controller].joy, joysticks[controller].buttons[CTL_L_SHOULDER]);
	joystate[controller].buttons[CTL_R_SHOULDER] = SDL_JoystickGetButton(joystate[controller].joy, joysticks[controller].buttons[CTL_R_SHOULDER]);
	joystate[controller].buttons[CTL_A_BUTTON] = SDL_JoystickGetButton(joystate[controller].joy, joysticks[controller].buttons[CTL_A_BUTTON]);
	joystate[controller].buttons[CTL_B_BUTTON] = SDL_JoystickGetButton(joystate[controller].joy, joysticks[controller].buttons[CTL_B_BUTTON]);
	joystate[controller].buttons[CTL_X_BUTTON] = SDL_JoystickGetButton(joystate[controller].joy, joysticks[controller].buttons[CTL_X_BUTTON]);
	joystate[controller].buttons[CTL_Y_BUTTON] = SDL_JoystickGetButton(joystate[controller].joy, joysticks[controller].buttons[CTL_Y_BUTTON]);	
	joystate[controller].buttons[CTL_START] = SDL_JoystickGetButton(joystate[controller].joy, joysticks[controller].buttons[CTL_START]);

	joystate[controller].halfpress = SDL_JoystickGetButton(joystate[controller].joy, joysticks[controller].halfpress);

	if(joysticks[controller].controllertype == CTL_TYPE_JOYSTICK)
		joystate[controller].dpad = SDL_JoystickGetHat(joystate[controller].joy, joysticks[controller].dpad);
	else
	{
		joystate[controller].dpad2[CTL_D_PAD_UP] = SDL_JoystickGetButton(joystate[controller].joy, joysticks[controller].dpad2[CTL_D_PAD_UP]);
		joystate[controller].dpad2[CTL_D_PAD_DOWN] = SDL_JoystickGetButton(joystate[controller].joy, joysticks[controller].dpad2[CTL_D_PAD_DOWN]);
		joystate[controller].dpad2[CTL_D_PAD_LEFT] = SDL_JoystickGetButton(joystate[controller].joy, joysticks[controller].dpad2[CTL_D_PAD_LEFT]);
		joystate[controller].dpad2[CTL_D_PAD_RIGHT] = SDL_JoystickGetButton(joystate[controller].joy, joysticks[controller].dpad2[CTL_D_PAD_RIGHT]);
	}
}

// ConfigMenuCallback
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
// Notes: MUST be EXPORT_CALL, because it is a callback for a menu ;)
// Called when the config menu is selected
// id = menu id (can be used to set the style with host.SetMenuItemStyle & co)
// w = window handle (HWND) that owns the menu
// p = user specified data
void EXPORT_CALL ConfigMenuCallback(u32 id, void* w, void* p)
{
	wprintf(L"PuruPuru -> ConfigMenuCallback\n");

	current_port = ((maple_device_instance*)p)->port >> 6;

	if(SDL_Init(SDL_INIT_JOYSTICK ) < 0)
	{
		MessageBoxA(NULL, SDL_GetError(), "Could not initialize SDL!", MB_ICONERROR);
		//wprintf(L"PuruPuru -> Could not initialize SDL! (%s)\n", SDL_GetError());
		return;
	}

	LoadConfig();	// load settings
	if(OpenConfig(PuruPuru_hInst, (HWND)w))
	{
		SaveConfig();
	}
	LoadConfig();	// reload settings	
}

// Save settings to file
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
void SaveConfig()
{
	wchar SectionName[32];
	for (int port=0;port<4;port++)
	{		
		wsprintf(SectionName, L"PuruPuru_Pad_%i", port+1);

		host.ConfigSaveInt(SectionName, L"l_shoulder", joysticks[port].buttons[CTL_L_SHOULDER]);		
		host.ConfigSaveInt(SectionName, L"l_shoulder", joysticks[port].buttons[CTL_L_SHOULDER]);
		host.ConfigSaveInt(SectionName, L"r_shoulder", joysticks[port].buttons[CTL_R_SHOULDER]);
		host.ConfigSaveInt(SectionName, L"a_button", joysticks[port].buttons[CTL_A_BUTTON]);
		host.ConfigSaveInt(SectionName, L"b_button", joysticks[port].buttons[CTL_B_BUTTON]);
		host.ConfigSaveInt(SectionName, L"x_button", joysticks[port].buttons[CTL_X_BUTTON]);
		host.ConfigSaveInt(SectionName, L"y_button", joysticks[port].buttons[CTL_Y_BUTTON]);
		host.ConfigSaveInt(SectionName, L"start_button", joysticks[port].buttons[CTL_START]);
		host.ConfigSaveInt(SectionName, L"dpad", joysticks[port].dpad);	
		host.ConfigSaveInt(SectionName, L"dpad_up", joysticks[port].dpad2[CTL_D_PAD_UP]);
		host.ConfigSaveInt(SectionName, L"dpad_down", joysticks[port].dpad2[CTL_D_PAD_DOWN]);
		host.ConfigSaveInt(SectionName, L"dpad_left", joysticks[port].dpad2[CTL_D_PAD_LEFT]);
		host.ConfigSaveInt(SectionName, L"dpad_right", joysticks[port].dpad2[CTL_D_PAD_RIGHT]);
		host.ConfigSaveInt(SectionName, L"main_x", joysticks[port].axis[CTL_MAIN_X]);
		host.ConfigSaveInt(SectionName, L"main_y", joysticks[port].axis[CTL_MAIN_Y]);
		host.ConfigSaveInt(SectionName, L"enabled", joysticks[port].enabled);
		host.ConfigSaveInt(SectionName, L"deadzone", joysticks[port].deadzone);
		host.ConfigSaveInt(SectionName, L"halfpress", joysticks[port].halfpress);
		host.ConfigSaveInt(SectionName, L"joy_id", joysticks[port].ID);
		host.ConfigSaveInt(SectionName, L"controllertype", joysticks[port].controllertype);
		host.ConfigSaveInt(SectionName, L"eventnum", joysticks[port].eventnum);
	}
}

// Load settings from file
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
void LoadConfig()
{
	wchar SectionName[32];
	for (int port=0;port<4;port++)
	{		
		wsprintf(SectionName, L"PuruPuru_Pad_%i", port+1);
					
		joysticks[port].buttons[CTL_L_SHOULDER] = host.ConfigLoadInt(SectionName, L"l_shoulder", 4);
		joysticks[port].buttons[CTL_R_SHOULDER] = host.ConfigLoadInt(SectionName, L"r_shoulder", 5);
		joysticks[port].buttons[CTL_A_BUTTON] = host.ConfigLoadInt(SectionName, L"a_button",  0);
		joysticks[port].buttons[CTL_B_BUTTON] = host.ConfigLoadInt(SectionName, L"b_button", 1);
		joysticks[port].buttons[CTL_X_BUTTON] = host.ConfigLoadInt(SectionName, L"x_button", 3);
		joysticks[port].buttons[CTL_Y_BUTTON] = host.ConfigLoadInt(SectionName, L"y_button", 2);
		joysticks[port].buttons[CTL_START] = host.ConfigLoadInt(SectionName, L"start_button", 9);
		joysticks[port].dpad = host.ConfigLoadInt(SectionName, L"dpad", 0);	
		joysticks[port].dpad2[CTL_D_PAD_UP] = host.ConfigLoadInt(SectionName, L"dpad_up", 0);
		joysticks[port].dpad2[CTL_D_PAD_DOWN] = host.ConfigLoadInt(SectionName, L"dpad_down", 0);
		joysticks[port].dpad2[CTL_D_PAD_LEFT] = host.ConfigLoadInt(SectionName, L"dpad_left", 0);
		joysticks[port].dpad2[CTL_D_PAD_RIGHT] = host.ConfigLoadInt(SectionName, L"dpad_right", 0);
		joysticks[port].axis[CTL_MAIN_X] = host.ConfigLoadInt(SectionName, L"main_x", 0);
		joysticks[port].axis[CTL_MAIN_Y] = host.ConfigLoadInt(SectionName, L"main_y", 1);
		joysticks[port].enabled = host.ConfigLoadInt(SectionName, L"enabled", 1);
		joysticks[port].deadzone = host.ConfigLoadInt(SectionName, L"deadzone", 9);
		joysticks[port].halfpress = host.ConfigLoadInt(SectionName, L"halfpress", 6);
		joysticks[port].ID = host.ConfigLoadInt(SectionName, L"joy_id", 0);
		joysticks[port].controllertype = host.ConfigLoadInt(SectionName, L"controllertype", 0);
		joysticks[port].eventnum = host.ConfigLoadInt(SectionName, L"eventnum", 0);
	}
}

// Search attached devices
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
int Search_Devices()
{
	int numjoy = SDL_NumJoysticks();

	if(numjoy == 0)
	{				
		MessageBoxA(NULL, "No Joystick detected!", NULL,  MB_ICONWARNING);		
		return 0;
	}

	if(joyinfo)
	{
		delete [] joyinfo;
		joyinfo = new CONTROLLER_INFO [numjoy];
	}
	else
	{
		joyinfo = new CONTROLLER_INFO [numjoy];
	}
	
	for(int i = 0; i < numjoy; i++ )
	{
		joyinfo[i].joy			= SDL_JoystickOpen(i);
		joyinfo[i].ID			= i;
		joyinfo[i].NumAxes		= SDL_JoystickNumAxes(joyinfo[i].joy);
		joyinfo[i].NumButtons	= SDL_JoystickNumButtons(joyinfo[i].joy);
		joyinfo[i].NumBalls		= SDL_JoystickNumBalls(joyinfo[i].joy);
		joyinfo[i].NumHats		= SDL_JoystickNumHats(joyinfo[i].joy);
		joyinfo[i].Name			= SDL_JoystickName(i);
  			
		// Close if opened
		if(SDL_JoystickOpened(i))
			SDL_JoystickClose(joyinfo[i].joy);
	}

	return numjoy;
}