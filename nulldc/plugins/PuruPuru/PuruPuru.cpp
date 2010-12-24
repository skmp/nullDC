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

//////////////////////////////////////////////////////////////////////////////////////////
// Variables
// ---------
HINSTANCE PuruPuru_hInst = NULL;
emu_info host;
u32 current_port = 0;
bool emulator_running  = FALSE;

bool canSDL		= false;
bool canXInput  = false;

CONTROLLER_STATE		joystate[4];
CONTROLLER_MAPPING		joysticks[4];
CONTROLLER_INFO_SDL	   *joyinfo = 0;
CONTROLLER_INFO_XINPUT	xoyinfo[4];

RAWINPUTDEVICE Rid[1]; // RAW keyboard

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

typedef INT_PTR CALLBACK dlgp( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
INT_PTR CALLBACK sch( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
dlgp* oldptr=0;

bool keyboard_map[256];

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
	
#ifdef BUILD_DREAMCAST
	wcscpy_s(info->maple.devices[0].Name, L"PuruPuru Dreamcast Controller v" _T(INPUT_VERSION) L" by Falcon4ever [" _T(__DATE__) L"]");
	// Main device (Like controller, lightgun and such)
	info->maple.devices[0].Type = MDT_Main;
	// Can have 2 subdevices (a dc controller has 2 subdevice ports for vmus / etc)
	// Can be 'hot plugged' (after emulation was started)
	info->maple.devices[0].Flags = MDTF_Sub0 | MDTF_Sub1 | MDTF_Hotplug;
#elif defined BUILD_NAOMI
	wcscpy_s(info->maple.devices[0].Name, L"PuruPuru NAOMI JAMMA Controller v" _T(INPUT_VERSION) L" by Falcon4ever [" _T(__DATE__) L"]");
	info->maple.devices[0].Type = MDT_Main;
	info->maple.devices[0].Flags = 0;
#endif
	
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
	memset(keyboard_map, 0, sizeof(keyboard_map));

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

	if (oldptr==0)
		oldptr = (dlgp*)SetWindowLongPtr((HWND)host.GetRenderTarget(),GWLP_WNDPROC,(LONG_PTR)sch);	

	// All devices disabled unless connected later.
	joysticks[0].enabled = 0;
	joysticks[1].enabled = 0;
	joysticks[2].enabled = 0;
	joysticks[3].enabled = 0;

	// Register RAW keyboard		
	Rid[0].usUsagePage = 0x01;
	Rid[0].usUsage = 0x06;
	Rid[0].dwFlags = 0;
	Rid[0].hwndTarget = NULL;

	RegisterRawInputDevices(Rid, 1, sizeof(Rid[0]));

	return rv_ok;
}

// Unload plugin
// -------------
// Notes: Called when plugin is unloaded by emulator (only if Load was called before).
void FASTCALL Unload()
{
	wprintf(L"PuruPuru -> Unload\n");

	if (oldptr!=0)
	{
		SetWindowLongPtrW((HWND)host.GetRenderTarget(),GWLP_WNDPROC,(LONG_PTR)oldptr);
		oldptr=0;
	}

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

#ifdef BUILD_NAOMI
	inst->dma = ControllerDMA_NAOMI;
#else
	inst->dma = ControllerDMA;
#endif
		
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


INT_PTR CALLBACK sch( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{			
	switch(uMsg)
	{
	case WM_INPUT:
		{						
			UINT dwSize = 40;
			static BYTE lpb[40];		
			    
			GetRawInputData((HRAWINPUT)lParam, RID_INPUT, 
							lpb, &dwSize, sizeof(RAWINPUTHEADER));
    
			RAWINPUT* raw = (RAWINPUT*)lpb;			
    
			if(raw->header.dwType == RIM_TYPEKEYBOARD)			
			{
				u16 key = raw->data.keyboard.VKey;
					
				switch(raw->data.keyboard.Message)
				{
				case WM_KEYDOWN:	
					keyboard_map[key] = 1;
					break;	
				case WM_KEYUP:
					keyboard_map[key] = 0;
					break;
				case WM_SYSKEYDOWN:
					keyboard_map[key] = 1;
					break;
				case WM_SYSKEYUP:		
					keyboard_map[key] = 0;
					break;	
				}										
			} 			  
		}
		break; // Case WM_INPUT
	}
	
	return oldptr(hWnd,uMsg,wParam,lParam);
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

#ifdef BUILD_NAOMI
	if( DialogBox(PuruPuru_hInst, MAKEINTRESOURCE(IDD_CONFIG_NAOMI), (HWND)w, OpenConfig) )
	{
		SaveConfig();
	}
#else	
	if( DialogBox(PuruPuru_hInst, MAKEINTRESOURCE(IDD_CONFIG_DC), (HWND)w, OpenConfig) )
	{
		SaveConfig();
	}
#endif

	LoadConfig();	// reload settings	
}

// Save settings to file
// ---------------------
void SaveConfig()
{
	wchar SectionName[32];

#ifdef BUILD_NAOMI
	for (int port=0;port<2;port++)
	{		
		wsprintf(SectionName, L"PuruPuru_NAOMI_%i", port+1);

		host.ConfigSaveStr(SectionName, L"d_up",		joysticks[port].names[MAPN_D_UP]);
		host.ConfigSaveStr(SectionName, L"d_down",		joysticks[port].names[MAPN_D_DOWN]);
		host.ConfigSaveStr(SectionName, L"d_left",		joysticks[port].names[MAPN_D_LEFT]);
		host.ConfigSaveStr(SectionName, L"d_right",		joysticks[port].names[MAPN_D_RIGHT]);
		host.ConfigSaveStr(SectionName, L"button1",		joysticks[port].names[MAPN_BUTTON1]);
		host.ConfigSaveStr(SectionName, L"button2",		joysticks[port].names[MAPN_BUTTON2]);
		host.ConfigSaveStr(SectionName, L"button3",	    joysticks[port].names[MAPN_BUTTON3]);		
		host.ConfigSaveStr(SectionName, L"button4",		joysticks[port].names[MAPN_BUTTON4]);
		host.ConfigSaveStr(SectionName, L"button5",		joysticks[port].names[MAPN_BUTTON5]);
		host.ConfigSaveStr(SectionName, L"button6",		joysticks[port].names[MAPN_BUTTON6]);
		host.ConfigSaveStr(SectionName, L"start",		joysticks[port].names[MAPN_START]);		
		host.ConfigSaveStr(SectionName, L"coin",		joysticks[port].names[MAPN_COIN]);		
		host.ConfigSaveStr(SectionName, L"Service1",	joysticks[port].names[MAPN_SERVICE1]);		
		host.ConfigSaveStr(SectionName, L"Service2",	joysticks[port].names[MAPN_SERVICE2]);		
		host.ConfigSaveStr(SectionName, L"Test1",		joysticks[port].names[MAPN_TEST1]);		
		host.ConfigSaveStr(SectionName, L"Test2",		joysticks[port].names[MAPN_TEST2]);	
#else
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
#endif			
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

#ifdef BUILD_NAOMI
	for (int port=0;port<2;port++)
	{		
		wsprintf(SectionName, L"PuruPuru_NAOMI_%i", port+1);

		host.ConfigLoadStr(SectionName, L"d_up",		joysticks[port].names[MAPN_D_UP], L"-1");
		host.ConfigLoadStr(SectionName, L"d_down",		joysticks[port].names[MAPN_D_DOWN], L"-1");
		host.ConfigLoadStr(SectionName, L"d_left",		joysticks[port].names[MAPN_D_LEFT], L"-1");
		host.ConfigLoadStr(SectionName, L"d_right",		joysticks[port].names[MAPN_D_RIGHT], L"-1");
		host.ConfigLoadStr(SectionName, L"button1",		joysticks[port].names[MAPN_BUTTON1], L"-1");
		host.ConfigLoadStr(SectionName, L"button2",		joysticks[port].names[MAPN_BUTTON2], L"-1");
		host.ConfigLoadStr(SectionName, L"button3",	    joysticks[port].names[MAPN_BUTTON3], L"-1");		
		host.ConfigLoadStr(SectionName, L"button4",		joysticks[port].names[MAPN_BUTTON4], L"-1");
		host.ConfigLoadStr(SectionName, L"button5",		joysticks[port].names[MAPN_BUTTON5], L"-1");
		host.ConfigLoadStr(SectionName, L"button6",		joysticks[port].names[MAPN_BUTTON6], L"-1");
		host.ConfigLoadStr(SectionName, L"start",		joysticks[port].names[MAPN_START], L"-1");		
		host.ConfigLoadStr(SectionName, L"coin",		joysticks[port].names[MAPN_COIN], L"-1");		
		host.ConfigLoadStr(SectionName, L"Service1",	joysticks[port].names[MAPN_SERVICE1], L"-1");		
		host.ConfigLoadStr(SectionName, L"Service2",	joysticks[port].names[MAPN_SERVICE2], L"-1");		
		host.ConfigLoadStr(SectionName, L"Test1",		joysticks[port].names[MAPN_TEST1], L"-1");		
		host.ConfigLoadStr(SectionName, L"Test2",		joysticks[port].names[MAPN_TEST2], L"-1");		
#else
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
#endif				
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
