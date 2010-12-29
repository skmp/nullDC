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

#include <windows.h>
#include <stdio.h>
#include <tchar.h>

#include "..\..\nullDC\plugins\plugin_header.h"
#include "resource.h"

#include <commctrl.h>   // includes the common control header
#pragma comment(lib, "comctl32.lib")

#include <SDL.h>
#include <SDL_haptic.h>
#pragma comment(lib, "SDL.lib")

#include <XInput.h>
#pragma comment(lib, "Xinput.lib")

//////////////////////////////////////////////////////////////////////////////////////////
// Define
// ------

#define INPUT_VERSION	"0.1"
#define INPUT_STATE		"PUBLIC RELEASE"
#define RELDAY			"01"
#define RELMONTH		"10"
#define RELYEAR			"2008"
#define THANKYOU		"`plot`, Absolute0, Aprentice, Bositman, Brice, ChaosCode, CKemu, CoDeX, Dave2001, dn, ector, F|RES, Florin, Gent, Gigaherz, Hacktarux, JegHegy, Linker, Linuzappz, Martin64, Muad, Knuckles, Raziel, Refraction, Rudy_x, Shadowprince, Snake785, Saqib, vEX, yaz0r, Zilmar and Zenogais."

//////////////////////////////////////////////////////////////////////////////////////////
// Structures
// ----------

struct CONTROLLER_STATE		// Dreamcast/NAOMI device information
{
	int control[16];		// Axis, buttons, dpad, triggers... + NAOMI
	
};

struct CONTROLLER_MAPPING   // Player's controller information
{	
	wchar names[16][64];	// Names from config file.

	int control[16];		// All of it too.		
	int ID;					// Joystick device ID
	int controllertype;		// XInput, SDL, or just Keyboard	
	int deadzone;			// Deadzone... what else?	

	int pakku_intensity;    // Strength of vibration.
	int pakku_length;		// Length of each vibration command.
	
	bool enabled;			// Pad attached?	
	bool halfpress;			// Half the analog values
	bool keys;				// Using keyboard
};

struct CONTROLLER_INFO_SDL	// SDL Controllers information
{
	wchar Name[512];		// Joypad/stick name		
	
	SDL_Joystick *joy;		// SDL joystick device	
	SDL_Haptic *rumble;		// SDL Haptic device, opened from joy.
	
	int NumAxes;			// Amount of Axes
	int NumButtons;			// Amount of Buttons
	int NumBalls;			// Amount of Balls
	int NumHats;			// Amount of Hats (POV)
		
	bool canRumble;			// Haptic support.
};

struct CONTROLLER_INFO_XINPUT
{
    XINPUT_STATE state;
    bool connected;
};

struct Supported_Status
{
	bool XInput;
	bool SDL;
	bool Haptic;

	int numJoy;
	int numHaptic;
};

struct _NaomiState
{
	BYTE Cmd;
	BYTE Mode;
	BYTE Node;
};

// ENUMS

enum eController
{
#ifdef BUILD_NAOMI

	CTLN_D_UP = 0,
	CTLN_D_DOWN,
	CTLN_D_LEFT,
	CTLN_D_RIGHT,

	CTLN_BUTTON1,
	CTLN_BUTTON2,
	CTLN_BUTTON3,
	CTLN_BUTTON4,
	CTLN_BUTTON5,
	CTLN_BUTTON6,

	CTLN_START,
	CTLN_COIN,
	
	CTLN_SERVICE1,
	CTLN_SERVICE2,
	CTLN_TEST1,
	CTLN_TEST2	
#else
	CTL_MAIN_X = 0,
	CTL_MAIN_Y,

	CTL_L_SHOULDER,
	CTL_R_SHOULDER,

	CTL_A_BUTTON,
	CTL_B_BUTTON,
	CTL_X_BUTTON,
	CTL_Y_BUTTON,
	CTL_START,

	CTL_D_PAD_UP,
	CTL_D_PAD_DOWN,
	CTL_D_PAD_LEFT,
	CTL_D_PAD_RIGHT,
#endif
};

enum eControllerX
{
	X360_LX,
	X360_LY,
	X360_RX,
	X360_RY,
	X360_LT,
	X360_RT,
	X360_UP,
	X360_DOWN,
	X360_LEFT,
	X360_RIGHT,
	X360_START,
	X360_BACK,
	X360_LS,
	X360_RS,
	X360_LB,
	X360_RB,
	X360_A,
	X360_B,
	X360_X,
	X360_Y
};

enum eControllerType
{
	CTL_TYPE_JOYSTICK_SDL,	
	CTL_TYPE_JOYSTICK_XINPUT,
	CTL_TYPE_KEYBOARD
};

enum
{
	AXIS,	
	TRIGGER,
	DIGITAL		
};

enum eInputType
{
	inAXIS_0 = 1, // Axis-, Axis+
	inAXIS_1,
	inBUTTON,
	inHAT,
	inTRIGGER,
	inKEY
}; 

enum eMappedController
{
#ifdef BUILD_NAOMI

	MAPN_D_UP = 0,
	MAPN_D_DOWN,
	MAPN_D_LEFT,
	MAPN_D_RIGHT,

	MAPN_BUTTON1,
	MAPN_BUTTON2,
	MAPN_BUTTON3,
	MAPN_BUTTON4,
	MAPN_BUTTON5,
	MAPN_BUTTON6,

	MAPN_START,
	MAPN_COIN,
	
	MAPN_SERVICE1,
	MAPN_SERVICE2,
	MAPN_TEST1,
	MAPN_TEST2	
#else	
	MAP_LT = 0,
	MAP_RT,
	MAP_A,
	MAP_B,
	MAP_X,
	MAP_Y,
	MAP_START,
	MAP_D_UP,
	MAP_D_DOWN,
	MAP_D_LEFT,
	MAP_D_RIGHT,
	MAP_A_XL,
	MAP_A_XR,
	MAP_A_YU,
	MAP_A_YD,
	MAP_HALF
#endif
};

//////////////////////////////////////////////////////////////////////////////////////////
// Custom Functions
// ----------------

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
#ifdef BUILD_DREAMCAST
u32 FASTCALL ControllerDMA(void* device_instance, u32 Command,u32* buffer_in, u32 buffer_in_len, u32* buffer_out, u32& buffer_out_len);
u32 FASTCALL PakkuDMA(void* device_instance, u32 Command, u32* buffer_in, u32 buffer_in_len, u32* buffer_out, u32& buffer_out_len);
// Rumble functions
void Start_Vibration(int port);
void Stop_Vibration(int port);
#elif defined BUILD_NAOMI
u32 FASTCALL ControllerDMA_NAOMI(void* device_instance,u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len);
#endif

void GetJoyStatus(int controller);
void Names2Control(int port);

void EXPORT_CALL ConfigMenuCallback(u32 id,void* w,void* p);

void SaveConfig();
void LoadConfig();
int Search_Devices();

//Globals
extern emu_info host;
extern CONTROLLER_MAPPING joysticks[4];
extern CONTROLLER_STATE joystate[4];
extern CONTROLLER_INFO_SDL	*joyinfo;
extern CONTROLLER_INFO_XINPUT xoyinfo[4];
extern bool keyboard_map[256];

//Config Function
INT_PTR CALLBACK OpenConfig( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
