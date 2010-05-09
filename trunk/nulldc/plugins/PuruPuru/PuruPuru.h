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

#include <windows.h>
#include <tchar.h>

#include "..\..\nullDC\plugins\plugin_header.h"
#include "config.h"

#include <SDL.h>
#pragma comment(lib, "SDL.lib")

//////////////////////////////////////////////////////////////////////////////////////////
// Define
// ¯¯¯¯¯¯

#define INPUT_VERSION	"0.1"
#define INPUT_STATE		"PUBLIC RELEASE"
#define RELDAY			"01"
#define RELMONTH		"10"
#define RELYEAR			"2008"
#define THANKYOU		"`plot`, Absolute0, Aprentice, Bositman, Brice, ChaosCode, CKemu, CoDeX, Dave2001, dn, ector, F|RES, Florin, Gent, Gigaherz, Hacktarux, JegHegy, Linker, Linuzappz, Martin64, Muad, Knuckles, Raziel, Refraction, Rudy_x, Shadowprince, Snake785, Saqib, vEX, yaz0r, Zilmar and Zenogais."

//////////////////////////////////////////////////////////////////////////////////////////
// Structures
// ¯¯¯¯¯¯¯¯¯¯

struct CONTROLLER_STATE{	// DC PAD INFO/STATE
	int buttons[7];			// Amount of buttons (A B X Y L-Trigger R-Trigger Start) might need to change the triggers buttons
	int dpad;				// 1 HAT (8 directions + neutral)
	int dpad2[4];			// d-pad using buttons
	int axis[2];			// 1 Axis (Main)
	int halfpress;			// ...
	SDL_Joystick *joy;		// SDL joystick device
};

struct CONTROLLER_MAPPING{	// GC PAD MAPPING
	int buttons[7];			// Amount of buttons (A B X Y L-Trigger R-Trigger Start) might need to change the triggers buttons
	int dpad;				// 1 HAT (8 directions + neutral)
	int dpad2[4];			// d-pad using buttons
	int axis[2];			// 1 Axis (Main)
	int enabled;			// Pad attached?
	int deadzone;			// Deadzone... what else?
	int halfpress;			// Halfpress... you know, like not fully pressed ;)...
	int ID;					// SDL joystick device ID
	int controllertype;		// Joystick, Joystick no hat or a keyboard (perhaps a mouse later)
	int eventnum;			// Linux Event Number, Can't be found dynamically yet
};

struct CONTROLLER_INFO{		// CONNECTED WINDOWS DEVICES INFO
	int NumAxes;			// Amount of Axes
	int NumButtons;			// Amount of Buttons
	int NumBalls;			// Amount of Balls
	int NumHats;			// Amount of Hats (POV)
	const char *Name;		// Joypad/stickname	
	int ID;					// SDL joystick device ID
	SDL_Joystick *joy;		// SDL joystick device
};

enum
{
	CTL_MAIN_X = 0,
	CTL_MAIN_Y
};

enum
{
	CTL_L_SHOULDER = 0,
	CTL_R_SHOULDER,
	CTL_A_BUTTON,
	CTL_B_BUTTON,
	CTL_X_BUTTON,
	CTL_Y_BUTTON,
	CTL_START	
};

enum
{
	CTL_D_PAD_UP = 0,
	CTL_D_PAD_DOWN,
	CTL_D_PAD_LEFT,
	CTL_D_PAD_RIGHT
};

enum
{
	CTL_TYPE_JOYSTICK = 0,
	CTL_TYPE_JOYSTICK_NO_HAT,
	CTL_TYPE_JOYSTICK_XBOX360,
	CTL_TYPE_KEYBOARD
};

//////////////////////////////////////////////////////////////////////////////////////////
// Custom Functions
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯

// Common
s32 FASTCALL Load(emu_info* emu);
void FASTCALL Unload();

// Maple
s32 FASTCALL CreateMain(maple_device_instance* inst,u32 id,u32 flags,u32 rootmenu);
s32 FASTCALL CreateSub(maple_subdevice_instance* inst,u32 id,u32 flags,u32 rootmenu);
s32 FASTCALL Init(void* data,u32 id,maple_init_params* params);
void FASTCALL Term(void* data,u32 id);
void FASTCALL Destroy(void* data,u32 id);

// Other
u32 FASTCALL ControllerDMA(void* device_instance, u32 Command,u32* buffer_in, u32 buffer_in_len, u32* buffer_out, u32& buffer_out_len);
void GetJoyState(int controller);
void EXPORT_CALL ConfigMenuCallback(u32 id,void* w,void* p);

void SaveConfig();
void LoadConfig();
int Search_Devices();