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

Supported_Status Support;

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
	//printf("PuruPuru -> GetInterface\n");	
	
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

	u8 id = 0;
#ifdef BUILD_DREAMCAST	
	wcscpy_s(info->maple.devices[id].Name, L"PuruPuru Dreamcast Controller v" _T(INPUT_VERSION) L" by Falcon4ever [" _T(__DATE__) L"]");	
	info->maple.devices[id].Type = MDT_Main;	
	info->maple.devices[id].Flags = MDTF_Sub0 | MDTF_Sub1 | MDTF_Hotplug;
	id++; // PuruPuru Pack
	wcscpy_s(info->maple.devices[id].Name, L"PuruPuru Pakku v" _T(INPUT_VERSION) L" by Falcon4ever [" _T(__DATE__) L"]");
	info->maple.devices[id].Type = MDT_Sub;
	info->maple.devices[id].Flags = MDTF_Hotplug;	
#elif defined BUILD_NAOMI
	wcscpy_s(info->maple.devices[id].Name, L"PuruPuru NAOMI JAMMA Controller v" _T(INPUT_VERSION) L" by Falcon4ever [" _T(__DATE__) L"]");
	info->maple.devices[id].Type = MDT_Main;
	info->maple.devices[id].Flags = 0;
#endif 

	id++;//EOL marker
	info->maple.devices[id].Type = MDT_EndOfList;	// wtf is this raz?	
}

//////////////////////////////////////////////////////////////////////////////////////////
// Common Input Plugin Functions
// -----------------------------

// Init SDL and check Joystick settings.
void Init_Update()
{
	if(SDL_Init(SDL_INIT_JOYSTICK ) < 0)
	{		
		MessageBoxA(NULL, SDL_GetError(), "Could not initialize SDL!", MB_ICONERROR);				
	}
	else
	{						
		if(SDL_Init(SDL_INIT_HAPTIC) < 0)
		{
			MessageBoxA(NULL, SDL_GetError(), "Could not initialize SDL Haptic!", MB_ICONERROR);			
		}
		else
		{
			Support.numHaptic = SDL_NumHaptics();
			if(Support.numHaptic>0)
				Support.Haptic = true;
		}
		
		int joys = Search_Devices();

		if(joys == 0) wprintf(L"PuruPuru: No SDL Joystick Found!");
	}

	LoadConfig();

	for(int i=0; i<4; i++)
	{
		switch(joysticks[i].controllertype)
		{
		case CTL_TYPE_JOYSTICK_SDL:
			  {
				  if(!SDL_JoystickOpened(joysticks[i].ID))
					  joysticks[i].controllertype = CTL_TYPE_KEYBOARD;
			  }
			  break;
		case CTL_TYPE_JOYSTICK_XINPUT:
			  {
				  if(!xoyinfo[joysticks[i].ID].connected)
					  joysticks[i].controllertype = CTL_TYPE_KEYBOARD;
			  }		
			  break;
		}
	}
}

// Load plugin
// -----------
// Notes: Called when plugin is loaded by the emu, the param has some handy functions so i make a copy of it ;).
s32 FASTCALL Load(emu_info* emu)
{	
	//printf("PuruPuru -> Load\n");	

	memcpy(&host, emu, sizeof(host));
	memset(keyboard_map, 0, sizeof(keyboard_map));
	memset(&Support,0,sizeof(Support));

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

	Init_Update();

	return rv_ok;
}

// Unload plugin
// -------------
// Notes: Called when plugin is unloaded by emulator (only if Load was called before).
void FASTCALL Unload()
{
	//printf("PuruPuru -> Unload\n");
	
	if (oldptr!=0)
	{
		SetWindowLongPtrW((HWND)host.GetRenderTarget(),GWLP_WNDPROC,(LONG_PTR)oldptr);
		oldptr=0;
	}

#ifdef BUILD_DREAMCAST
	for(int i=0; i<4; i++) Stop_Vibration(i);
#endif

	for(int i=0; i<Support.numJoy; i++)	
	{
		if(joyinfo[i].rumble != NULL) SDL_HapticClose(joyinfo[i].rumble);
		if(joyinfo[i].joy != NULL) SDL_JoystickClose(joyinfo[i].joy);		
	}

	delete [] joyinfo;

	SDL_Quit();
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
	//printf("PuruPuru -> CreateMain [%d]\n", inst->port >> 6);

	// Enable pad on port.
	joysticks[(inst->port >> 6)].enabled = 1;

	if (id!=0)
		return rv_error;

	inst->data = inst;

#ifdef BUILD_DREAMCAST
	inst->dma = ControllerDMA;
	host.AddMenuItem(rootmenu, -1, L"Dreamcast Controller", 0, 0);

#elif defined BUILD_NAOMI
	inst->dma = ControllerDMA_NAOMI;
	host.AddMenuItem(rootmenu, -1, L"NAOMI JAMMA", 0, 0);
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
	int port = inst->port>>6;	
	//printf("PuruPuru -> CreateSub [%d]\n", port);

#ifdef BUILD_DREAMCAST
	inst->data = inst;
	inst->dma  = PakkuDMA;

	if(joysticks[port].controllertype == CTL_TYPE_JOYSTICK_SDL && Support.Haptic)
		host.AddMenuItem(rootmenu, -1, L"Puru Puru Pakku (SDL)", 0, 0);
	else if(joysticks[port].controllertype == CTL_TYPE_JOYSTICK_XINPUT)
		host.AddMenuItem(rootmenu, -1, L"Puru Puru Pakku (XInput)", 0, 0);		
	else
		host.AddMenuItem(rootmenu, -1, L"No Rumble Device", 0, 0);
#endif

	return rv_ok;
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
	emulator_running = TRUE;
	u32 port = ((maple_device_instance*)data)->port >> 6;
	printf("PuruPuru -> Init [%d]\n", port);

	switch(id)
	{
	case 0: // Dreamcast/NAOMI Controller
		{
			// Do nothing, already covered from settings and Load().
		}
		break;
#ifdef BUILD_DREAMCAST
	case 1: // PuruPuru Pakku
		{
			if(joysticks[port].controllertype != CTL_TYPE_KEYBOARD)
			{
				if(joysticks[port].controllertype == CTL_TYPE_JOYSTICK_SDL && Support.Haptic)
					Start_Vibration(port);
				else if(joysticks[port].controllertype == CTL_TYPE_JOYSTICK_XINPUT)
					Start_Vibration(port);
			}
		}
		break;
#endif
	}


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
	u32 port = ((maple_device_instance*)data)->port >> 6;
	//printf("PuruPuru -> Term [%d]\n", port);

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
	//printf("PuruPuru -> Destroy\n");

#ifdef BUILD_DREAMCAST
	// Disable on Hot-Unplug
	u32 port = ((maple_device_instance*)data)->port >> 6;
	
	switch(id)
	{
	case 0: // DC Controller
		{
			joysticks[port].enabled = 0; 
			int joyID = joysticks[port].ID;

			if(joyinfo[joyID].rumble != NULL)
			{
				SDL_HapticClose(joyinfo[joyID].rumble);
				joyinfo[joyID].rumble = NULL;
			}
			if(joyinfo[joyID].joy != NULL) 	
			{
				SDL_JoystickClose(joyinfo[joyID].joy);
				joyinfo[joyID].joy = NULL;
			}
		} 
		break;
	case 1: // Puru Pack
		{
			Stop_Vibration(port);
		} 
		break;
	}
#endif
	
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
	//printf("PuruPuru -> ConfigMenuCallback\n");

	current_port = ((maple_device_instance*)p)->port >> 6;

	LoadConfig();	// load settings	

#ifdef BUILD_DREAMCAST
	if( DialogBox(PuruPuru_hInst, MAKEINTRESOURCE(IDD_CONFIG_DC), (HWND)w, OpenConfig) )
	{
		SaveConfig();
	}
#elif defined BUILD_NAOMI
	if( DialogBox(PuruPuru_hInst, MAKEINTRESOURCE(IDD_CONFIG_NAOMI), (HWND)w, OpenConfig) )
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
#elif defined BUILD_DREAMCAST
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
		host.ConfigSaveInt(SectionName, L"pakku_intensity",	joysticks[port].pakku_intensity);		
		host.ConfigSaveInt(SectionName, L"pakku_length",	joysticks[port].pakku_length);	
#endif			
		host.ConfigSaveInt(SectionName, L"keyboard",		joysticks[port].keys);		
		host.ConfigSaveInt(SectionName, L"joy_id",			joysticks[port].ID);
		host.ConfigSaveInt(SectionName, L"controllertype",	joysticks[port].controllertype);		
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
#elif defined BUILD_DREAMCAST
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
		joysticks[port].pakku_intensity	= host.ConfigLoadInt(SectionName, L"pakku_intensity", 100);
		joysticks[port].pakku_intensity = max(0, min(joysticks[port].pakku_intensity, 200));
		joysticks[port].pakku_length	= host.ConfigLoadInt(SectionName, L"pakku_length", 175);
#endif				
		joysticks[port].keys			= host.ConfigLoadInt(SectionName, L"keyboard",		 0) == 0? false:true;
		joysticks[port].ID				= host.ConfigLoadInt(SectionName, L"joy_id",		 0);
		joysticks[port].controllertype	= host.ConfigLoadInt(SectionName, L"controllertype", 0);		

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
	Support.numJoy = SDL_NumJoysticks();

	wprintf(L"\nPuruPuru -> SDL: %i Joystick(s) detected:\n", Support.numJoy);

	if(Support.numJoy > 0) 
	{		
		Support.SDL = true;
		
		if(joyinfo)
		{
			delete [] joyinfo;
			joyinfo = new CONTROLLER_INFO_SDL [Support.numJoy];
		}
		else
		{
			joyinfo = new CONTROLLER_INFO_SDL [Support.numJoy];
		}
	
		for(int i = 0; i < Support.numJoy; i++ )
		{
			joyinfo[i].joy			= SDL_JoystickOpen(i);
			joyinfo[i].rumble		= NULL;						
			joyinfo[i].NumAxes		= SDL_JoystickNumAxes(joyinfo[i].joy);
			joyinfo[i].NumButtons	= SDL_JoystickNumButtons(joyinfo[i].joy);
			joyinfo[i].NumBalls		= SDL_JoystickNumBalls(joyinfo[i].joy);
			joyinfo[i].NumHats		= SDL_JoystickNumHats(joyinfo[i].joy);	
			
			AnsiToWide( joyinfo[i].Name, SDL_JoystickName(i) );
			
			wprintf(L"\t[%d] %s. ", i, joyinfo[i].Name);
			
			if(Support.Haptic)
			{
				if(SDL_JoystickIsHaptic(joyinfo[i].joy) == 1)
				{
					wprintf(L"Rumble supported.");					
					joyinfo[i].canRumble = true;
				}
				else				
					wprintf(L"Rumble not supported.");						
				
			}
		
			wprintf(L"\n");
		}
			wprintf(L"\n");
	}	

	ZeroMemory( xoyinfo, sizeof( CONTROLLER_INFO_XINPUT ) * 4 );

	DWORD status;    

	int xjoy = 0;

	for( int i = 0; i < 4; i++ )
    {        
        status = XInputGetState( i, &xoyinfo[i].state );

        if( status == ERROR_SUCCESS )
		{            
			xoyinfo[i].connected = true;
			Support.XInput = true;
			xjoy++;
		}			
        else
            xoyinfo[i].connected = false;
    }  	    	

	if(xjoy == 0) wprintf(L"PuruPuru: No XInput Joystick Found!");

	return Support.numJoy;
}
