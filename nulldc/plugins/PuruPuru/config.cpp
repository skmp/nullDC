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

extern u32 current_port;
extern bool emulator_running;

extern CONTROLLER_INFO_SDL		*joyinfo;
extern CONTROLLER_INFO_XINPUT	 xoyinfo[4];
extern CONTROLLER_MAPPING joysticks[4];
extern HINSTANCE PuruPuru_hInst;

extern bool canSDL;
extern bool canXInput;

//HINSTANCE config_hInst;
//HWND config_HWND;

static const wchar* ControllerType[] =
{
	L"Joystick (SDL)",	
	L"Joystick (XInput)",	
	L"Keyboard-Only"		
};

//////////////////////////////////////////////////////////////////////////////////////////
// Config dialog functions
// -----------------------

INT_PTR CALLBACK OpenConfig( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	int Beep = GetSystemMetrics( SPI_SETBEEP );
	
	switch( uMsg )
	{
	case WM_INITDIALOG:

		// Disables the DING sound when using the keyboard...			
		SystemParametersInfo( SPI_SETBEEP , FALSE , NULL , 0);

		TCITEM tci; 
		tci.mask = TCIF_TEXT | TCIF_IMAGE;
		tci.iImage = -1; 
		tci.pszText = current_port == 0 ? L"->Controller 1<-" : L"Controller 1"; 
		TabCtrl_InsertItem(GetDlgItem(hDlg,IDC_PORTTAB), 0, &tci); 
		tci.pszText = current_port == 1 ? L"->Controller 2<-" : L"Controller 2"; 
		TabCtrl_InsertItem(GetDlgItem(hDlg,IDC_PORTTAB), 1, &tci); 
		tci.pszText = current_port == 2 ? L"->Controller 3<-" : L"Controller 3"; 
		TabCtrl_InsertItem(GetDlgItem(hDlg,IDC_PORTTAB), 2, &tci); 
		tci.pszText = current_port == 3 ? L"->Controller 4<-" : L"Controller 4"; 
		TabCtrl_InsertItem(GetDlgItem(hDlg,IDC_PORTTAB), 3, &tci); 

		TabCtrl_SetCurSel(GetDlgItem(hDlg,IDC_PORTTAB),current_port);		

		// Dummy Keyb-only combobox
		SendMessage(GetDlgItem(hDlg, IDC_JOYNAME_KEY), CB_ADDSTRING, 0, (LPARAM)L"Keyboard-Only");
		SendMessage(GetDlgItem(hDlg, IDC_JOYNAME_KEY), CB_SETCURSEL, 0, 0);		
		ComboBox_Enable(GetDlgItem(hDlg, IDC_JOYNAME_KEY), FALSE);

		//if(emulator_running)
		if(emulator_running)
		{
			ComboBox_Enable(GetDlgItem(hDlg, IDC_JOYNAME_SDL), FALSE);
			ComboBox_Enable(GetDlgItem(hDlg, IDC_JOYNAME_XINPUT), FALSE);
			ComboBox_Enable(GetDlgItem(hDlg, IDC_CONTROLTYPE), FALSE);				
		}
		else
		{
			ComboBox_Enable(GetDlgItem(hDlg, IDC_JOYNAME_SDL), TRUE);
			ComboBox_Enable(GetDlgItem(hDlg, IDC_JOYNAME_XINPUT), TRUE);
			ComboBox_Enable(GetDlgItem(hDlg, IDC_CONTROLTYPE), TRUE);				
		}

		{	// Control type selection and deadzone control.
			HWND ControlType = GetDlgItem(hDlg, IDC_CONTROLTYPE);				
			SendMessage(ControlType, CB_ADDSTRING, 0, (LPARAM)ControllerType[CTL_TYPE_JOYSTICK_SDL]);				
			SendMessage(ControlType, CB_ADDSTRING, 0, (LPARAM)ControllerType[CTL_TYPE_JOYSTICK_XINPUT]);
			SendMessage(ControlType, CB_ADDSTRING, 0, (LPARAM)ControllerType[CTL_TYPE_KEYBOARD]);
				
			wchar buffer[8];				
			ControlType = GetDlgItem(hDlg, IDC_DEADZONE);
			SendMessage(ControlType, CB_RESETCONTENT, 0, 0);
			for(int x = 1; x <= 100; x++)
			{				
				wsprintf(buffer, L"%d %%", x);
				SendMessage(ControlType, CB_ADDSTRING, 0, (LPARAM)buffer);				
			}
		}

		SetControllerAll(hDlg, current_port);
						
		// Search for devices and add the to the device list
		if( canSDL )
		{
			HWND CB = GetDlgItem(hDlg, IDC_JOYNAME_SDL);

			for(int x = 0; x < SDL_NumJoysticks(); x++)
			{					
				SendMessage(CB, CB_ADDSTRING, 0, (LPARAM)joyinfo[x].Name);					
			}	

			SendMessage(CB, CB_SETCURSEL, 0, joysticks[current_port].ID);
		}
		else
		{
			HWND CB = GetDlgItem(hDlg, IDC_JOYNAME_SDL);				
			SendMessage(CB, CB_ADDSTRING, 0, (LPARAM)L"No Joystick detected!");
			SendMessage(CB, CB_SETCURSEL, 0, 0);
			ComboBox_Enable(CB, FALSE);
		}

		if( canXInput )
		{
			HWND CB = GetDlgItem(hDlg, IDC_JOYNAME_XINPUT);

			for(int x = 0; x < 4; x++)
			{									
				wchar temp[512];				
				
				if(xoyinfo[x].connected)				
					swprintf(temp, sizeof(temp), L"XInput[%d]: Connected", x);	
				else
					swprintf(temp, sizeof(temp), L"XInput[%d]: Not connected", x);						

				SendMessage(CB, CB_ADDSTRING, 0, (LPARAM)temp);
			}

			SendMessage(CB, CB_SETCURSEL, 0, joysticks[current_port].ID);
		}
		else
		{
			HWND CB = GetDlgItem(hDlg, IDC_JOYNAME_XINPUT);				
			SendMessage(CB, CB_ADDSTRING, 0, (LPARAM)L"No Joystick detected!");
			SendMessage(CB, CB_SETCURSEL, 0, 0);
			ComboBox_Enable(CB, FALSE);
		}
	break;

	case WM_NOTIFY:
		{
			if ( ((LPNMHDR)lParam)->idFrom==IDC_PORTTAB && ((LPNMHDR)lParam)->code == TCN_SELCHANGE  )
			{
				current_port = TabCtrl_GetCurSel(GetDlgItem(hDlg,IDC_PORTTAB));
				SetControllerAll(hDlg, current_port);				
			}
			
			return true;
		}
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
			
			case IDOK:
			{
				GetControllerAll(hDlg, current_port);
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			}
			
			case IDC_JOYNAME_SDL:
			{
				// Selected a different joystick
				if(HIWORD(wParam) == CBN_SELCHANGE)
				{
					joysticks[current_port].ID = (int)SendMessage(GetDlgItem(hDlg, IDC_JOYNAME_SDL), CB_GETCURSEL, 0, 0);						
				}
				return TRUE;
			}
			break;

			case IDC_JOYNAME_XINPUT:
			{
				// Selected a different joystick
				if(HIWORD(wParam) == CBN_SELCHANGE)
				{
					joysticks[current_port].ID = (int)SendMessage(GetDlgItem(hDlg, IDC_JOYNAME_XINPUT), CB_GETCURSEL, 0, 0);						
				}
				return TRUE;
			}
			break;

			case IDC_CONTROLTYPE:
			{
				// Selected a different joystick
				if(HIWORD(wParam) == CBN_SELCHANGE)
				{
					joysticks[current_port].controllertype = (int)SendMessage(GetDlgItem(hDlg, IDC_CONTROLTYPE), CB_GETCURSEL, 0, 0);					
					UpdateVisibleItems(hDlg, joysticks[current_port].controllertype);
				}
				return TRUE;
			}
			break;

			case IDC_KEY:
			{
				joysticks[current_port].keys = IsDlgButtonChecked(hDlg, IDC_KEY) == BST_CHECKED? 1:0;
			}
			break;

			case IDC_SHOULDERL:
			case IDC_SHOULDERR:
			case IDC_A:
			case IDC_B:
			case IDC_X:
			case IDC_Y:				
			case IDC_START:
			case IDC_HALFPRESS:												
			case IDC_DPAD_UP:
			case IDC_DPAD_DOWN:
			case IDC_DPAD_LEFT:
			case IDC_DPAD_RIGHT:
			case IDC_MX_L:
			case IDC_MX_R:
			case IDC_MY_U:
			case IDC_MY_D:
			{
				switch(joysticks[current_port].controllertype)
				{
					case CTL_TYPE_JOYSTICK_SDL:		GetInputSDL(hDlg, LOWORD(wParam), current_port); break;
					case CTL_TYPE_JOYSTICK_XINPUT:	GetInputXInput(hDlg, LOWORD(wParam), current_port); break;
					case CTL_TYPE_KEYBOARD:			GetInputKey(hDlg, LOWORD(wParam), current_port); break;
				}
								
				return TRUE;
			}
			break;

			case IDABOUT:
			{
				// yay an about box ;p
				OpenAbout(PuruPuru_hInst, hDlg);
				return TRUE;
			}
			break;
		}			
	break;
					
	case WM_CLOSE:
	case WM_DESTROY:	
		
		// Restore it.
		SystemParametersInfo( SPI_SETBEEP , Beep , NULL , 0);
		
		KillTimer(hDlg,0);
		EndDialog(hDlg,0);
	return true;

	default: break;
	}	
	return FALSE;
}

bool GetInputXInput(HWND hDlg, int buttonid, int controller)
{	
	buttonid += 1000;			

	wchar format[128];	

	bool plus = false;
	int pad = joysticks[controller].ID;

	bool waiting = true;
	bool succeed = false;
	int pressed = 0;
	int threshold = 10000;

	int counter1 = 0;
	int counter2 = 10;
	
	wsprintf(format, L"[%d]", counter2);
	SetDlgItemText(hDlg, buttonid, format);		

	while(waiting)
	{					
		XInputGetState(pad, &xoyinfo[pad].state);				

		// AXIS
		if(xoyinfo[pad].state.Gamepad.sThumbLX >  threshold) 
		{ 
			wsprintf(format, L"LX+"); 
			succeed = true;
			waiting = false; 
			break;
		}
		else if(xoyinfo[pad].state.Gamepad.sThumbLX < -threshold)
		{
			wsprintf(format, L"LX-");		
			succeed = true;
			waiting = false;
		}
		
		if(xoyinfo[pad].state.Gamepad.sThumbLY >  threshold) 
		{
			wsprintf(format, L"LY+");
			succeed = true;
			waiting = false;
		}
		else if(xoyinfo[pad].state.Gamepad.sThumbLY < -threshold)
		{
			wsprintf(format, L"LY-");	
			succeed = true;
			waiting = false;
		}

		if(xoyinfo[pad].state.Gamepad.sThumbRX >  threshold)
		{
			wsprintf(format, L"RX+");
			succeed = true;
			waiting = false;
		}
		else if(xoyinfo[pad].state.Gamepad.sThumbRX < -threshold) 
		{
			wsprintf(format, L"RX-");	
			succeed = true;
			waiting = false;
		}

		if(xoyinfo[pad].state.Gamepad.sThumbRY >  threshold) 
		{
			wsprintf(format, L"RY+");
			succeed = true;
			waiting = false;
		}
		else if(xoyinfo[pad].state.Gamepad.sThumbRY < -threshold)
		{
			wsprintf(format, L"RY-");		
			succeed = true;
			waiting = false;
		}

		// TRIGGERS
		if(xoyinfo[pad].state.Gamepad.bLeftTrigger > 50)
		{
			wsprintf(format, L"LT");		
			succeed = true;
			waiting = false;			
		}

		if(xoyinfo[pad].state.Gamepad.bRightTrigger > 50)
		{
			wsprintf(format, L"RT");		
			succeed = true;
			waiting = false;			
		}

		// BUTTONS | HAT	
		switch(xoyinfo[pad].state.Gamepad.wButtons)
		{
			case XINPUT_GAMEPAD_DPAD_UP:
				{
					wsprintf(format, L"UP");		
					succeed = true;
					waiting = false;
				} break;
			case XINPUT_GAMEPAD_DPAD_DOWN:
				{
					wsprintf(format, L"DOWN");		
					succeed = true;
					waiting = false;
				} break;
			case XINPUT_GAMEPAD_DPAD_LEFT:       
				{
					wsprintf(format, L"LEFT");		
					succeed = true;
					waiting = false;
				} break;
			case XINPUT_GAMEPAD_DPAD_RIGHT:       
				{
					wsprintf(format, L"RIGHT");		
					succeed = true;
					waiting = false;
				} break;
			case XINPUT_GAMEPAD_START:      
				{
					wsprintf(format, L"START");		
					succeed = true;
					waiting = false;
				} break;
			case XINPUT_GAMEPAD_BACK:           
				{
					wsprintf(format, L"BACK");		
					succeed = true;
					waiting = false;
				} break;
			case XINPUT_GAMEPAD_LEFT_THUMB:
				{
					wsprintf(format, L"LS");		
					succeed = true;
					waiting = false;
				} break;
			case XINPUT_GAMEPAD_RIGHT_THUMB:      
				{
					wsprintf(format, L"RS");		
					succeed = true;
					waiting = false;
				} break;
			case XINPUT_GAMEPAD_LEFT_SHOULDER:
				{
					wsprintf(format, L"LB");		
					succeed = true;
					waiting = false;
				} break;
			case XINPUT_GAMEPAD_RIGHT_SHOULDER:   
				{
					wsprintf(format, L"RB");		
					succeed = true;
					waiting = false;
				} break;
			case XINPUT_GAMEPAD_A:  
				{
					wsprintf(format, L"A.");		
					succeed = true;
					waiting = false;
				} break;
			case XINPUT_GAMEPAD_B:               
				{
					wsprintf(format, L"B.");		
					succeed = true;
					waiting = false;
				} break;
			case XINPUT_GAMEPAD_X:               
				{
					wsprintf(format, L"X.");		
					succeed = true;
					waiting = false;
				} break;
			case XINPUT_GAMEPAD_Y:               
				{
					wsprintf(format, L"Y.");		
					succeed = true;
					waiting = false;
				} break;
		}

		// KEYBOARD
		if(joysticks[controller].keys)
		{
			for(int k = 1; k < 255; k++)
			{
				if(GetAsyncKeyState(k)>>12)
				{
					pressed = k;
					waiting = false;
					succeed = true;

					wsprintf(format, L"K%d", pressed);

					break;
				}
			}
		}

		counter1++;
		if(counter1==100)
		{
			counter1=0;
			counter2--;
			
			wsprintf(format, L"[%d]", counter2);
			SetDlgItemText(hDlg, buttonid, format);

			if(counter2<0)
				waiting = false;
		}	
		Sleep(10);
	}
			
	if(!succeed) wsprintf(format, L"-1");

	SetDlgItemText(hDlg, buttonid, format);	

	return true;
}

bool GetInputKey(HWND hDlg, int buttonid, int controller)
{		
	buttonid += 1000;

	wchar format[128];

	bool waiting = true;
	bool succeed = false;
	int pressed = 0;

	int counter1 = 0;
	int counter2 = 10;
	
	wsprintf(format, L"[%d]", counter2);
	SetDlgItemText(hDlg, buttonid, format);	

	while(waiting)
	{													

		for(int k = 1; k < 255; k++)
		{												
			if(GetAsyncKeyState(k)>>12)
			{				
				pressed = k;
				waiting = false;
				succeed = true;
				break;
			}
		}

		counter1++;
		if(counter1==100)
		{
			counter1=0;
			counter2--;
			
			wsprintf(format, L"[%d]", counter2);
			SetDlgItemText(hDlg, buttonid, format);

			if(counter2<0)
				waiting = false;
		}	
		Sleep(10);
	}
			
	if(succeed)
		wsprintf(format, L"K%d", pressed);	
	else
		wsprintf(format, L"-1", pressed);
	
	SetDlgItemText(hDlg, buttonid, format);

	return true;
}

bool GetInputSDL(HWND hDlg, int buttonid, int controller)
{	
	buttonid += 1000;
		
	SDL_Joystick *joy;
	joy=SDL_JoystickOpen(joysticks[controller].ID);

	wchar format[128];

	int buttons = SDL_JoystickNumButtons(joy);
	int hats = SDL_JoystickNumHats(joy);
	int axes = SDL_JoystickNumAxes(joy);
	Sint16 value;

	bool KEY = false;
	bool HAT = false;
	bool AXIS = false;
	bool plus = false;

	bool waiting = true;
	bool succeed = false;
	int pressed = 0;

	int counter1 = 0;
	int counter2 = 10;
	
	wsprintf(format, L"[%d]", counter2);
	SetDlgItemText(hDlg, buttonid, format);
	
	while(waiting)
	{			
		SDL_JoystickUpdate();		

		// AXIS
		for(int b = 0; b < axes; b++)
		{		
			value = SDL_JoystickGetAxis(joy, b);
			if(value > 10000)
			{
				pressed = b;	
				plus = true;
				waiting = false;
				succeed = true;
				AXIS = true;
				break;
			}
			else if ( value < -10000 )
			{
				pressed = b;	
				waiting = false;
				succeed = true;
				AXIS = true;
				break;
			}
		}
		
		// BUTTONS
		for(int b = 0; b < buttons; b++)
		{			
			if(SDL_JoystickGetButton(joy, b))
			{
				pressed = b;	
				waiting = false;
				succeed = true;
				break;
			}			
		}

		// HATS
		for(int b = 0; b < hats; b++)
		{			
			switch (SDL_JoystickGetHat(joy, b))
			{
				case SDL_HAT_LEFT:
					{
						pressed = SDL_HAT_LEFT;	
						waiting = false;
						succeed = true;
						HAT = true;
						break;
					}
				case SDL_HAT_RIGHT:
					{
						pressed = SDL_HAT_RIGHT;	
						waiting = false;
						succeed = true;
						HAT = true;
						break;
					}
				case SDL_HAT_UP:
					{
						pressed = SDL_HAT_UP;	
						waiting = false;
						succeed = true;
						HAT = true;
						break;
					}
				case SDL_HAT_DOWN:
					{
						pressed = SDL_HAT_DOWN;	
						waiting = false;
						succeed = true;
						HAT = true;
						break;
					}
			}

		}

		if(joysticks[controller].keys)
		{
			for(int k = 1; k < 255; k++)
			{
				if(GetAsyncKeyState(k)>>12)
				{
					pressed = k;
					waiting = false;
					succeed = true;
					KEY = true;
					break;
				}
			}
		}

		counter1++;
		if(counter1==100)
		{
			counter1=0;
			counter2--;
			
			wsprintf(format, L"[%d]", counter2);
			SetDlgItemText(hDlg, buttonid, format);

			if(counter2<0)
				waiting = false;
		}	
		Sleep(10);
	}
			
	if(succeed)
	{
		if(HAT) wsprintf(format, L"H%d", pressed);

		else if (AXIS)
		{
			if(plus)
				wsprintf(format, L"A%d+", pressed);
			else
				wsprintf(format, L"A%d-", pressed);
		}
		
		else if (KEY) wsprintf(format, L"K%d", pressed);
		
		else    wsprintf(format, L"B%d", pressed);

		
	}
	else
		wsprintf(format, L"-1", pressed);
	SetDlgItemText(hDlg, buttonid, format);

	if(SDL_JoystickOpened(joysticks[controller].ID))
		SDL_JoystickClose(joy);

	return true;
}

void UpdateVisibleItems(HWND hDlg, int controllertype)
{	
	if(controllertype == CTL_TYPE_JOYSTICK_SDL) 
	{
		ShowWindow(GetDlgItem(hDlg, IDC_JOYNAME_SDL), TRUE);
		ShowWindow(GetDlgItem(hDlg, IDC_JOYNAME_XINPUT), FALSE);
		ShowWindow(GetDlgItem(hDlg, IDC_JOYNAME_KEY), FALSE);
		ShowWindow(GetDlgItem(hDlg, IDC_KEY), TRUE);
	}
	else if (controllertype == CTL_TYPE_JOYSTICK_XINPUT)
	{		
		ShowWindow(GetDlgItem(hDlg, IDC_JOYNAME_XINPUT), TRUE);
		ShowWindow(GetDlgItem(hDlg, IDC_JOYNAME_SDL), FALSE);		
		ShowWindow(GetDlgItem(hDlg, IDC_JOYNAME_KEY), FALSE);
		ShowWindow(GetDlgItem(hDlg, IDC_KEY), TRUE);
	}
	else
	{
		ShowWindow(GetDlgItem(hDlg, IDC_JOYNAME_KEY), TRUE);
		ShowWindow(GetDlgItem(hDlg, IDC_JOYNAME_SDL), FALSE);
		ShowWindow(GetDlgItem(hDlg, IDC_JOYNAME_XINPUT), FALSE);			
		ShowWindow(GetDlgItem(hDlg, IDC_KEY), FALSE);
	}
}

// Set dialog items
// ----------------
void SetControllerAll(HWND hDlg, int controller)
{		
	SendMessage(GetDlgItem(hDlg, IDC_JOYNAME_SDL), CB_SETCURSEL, joysticks[controller].ID, 0);
	SendMessage(GetDlgItem(hDlg, IDC_JOYNAME_XINPUT), CB_SETCURSEL, joysticks[controller].ID, 0);

	if( joysticks[controller].enabled )
	{
		ShowWindow(GetDlgItem(hDlg, IDC_CONFIG_ON), TRUE);
		ShowWindow(GetDlgItem(hDlg, IDC_CONFIG_OFF), FALSE);
	}
	else
	{
		ShowWindow(GetDlgItem(hDlg, IDC_CONFIG_ON), FALSE);
		ShowWindow(GetDlgItem(hDlg, IDC_CONFIG_OFF), TRUE);
	}
	
	SetButton(hDlg, IDTEXT_SHOULDERL,	joysticks[controller].names[MAP_LT]);
	SetButton(hDlg, IDTEXT_SHOULDERR,	joysticks[controller].names[MAP_RT]);
	SetButton(hDlg, IDTEXT_A,			joysticks[controller].names[MAP_A]);
	SetButton(hDlg, IDTEXT_B,			joysticks[controller].names[MAP_B]);
	SetButton(hDlg, IDTEXT_X,			joysticks[controller].names[MAP_X]);
	SetButton(hDlg, IDTEXT_Y,			joysticks[controller].names[MAP_Y]);

	SetButton(hDlg, IDTEXT_START,		joysticks[controller].names[MAP_START]);
	
	SetButton(hDlg, IDTEXT_HALFPRESS,	joysticks[controller].names[MAP_HALF]);
	
	SetButton(hDlg, IDTEXT_MX_L,		joysticks[controller].names[MAP_A_XL]);
	SetButton(hDlg, IDTEXT_MX_R,		joysticks[controller].names[MAP_A_XR]);
	SetButton(hDlg, IDTEXT_MY_U,		joysticks[controller].names[MAP_A_YU]);	
	SetButton(hDlg, IDTEXT_MY_D,		joysticks[controller].names[MAP_A_YD]);

	SendMessage(GetDlgItem(hDlg, IDC_CONTROLTYPE), CB_SETCURSEL, joysticks[controller].controllertype, 0);	
	SendMessage(GetDlgItem(hDlg, IDC_DEADZONE), CB_SETCURSEL, joysticks[controller].deadzone, 0);	

	UpdateVisibleItems(hDlg, joysticks[controller].controllertype);

	if(joysticks[controller].keys == 1) CheckDlgButton(hDlg,IDC_KEY, BST_CHECKED);
	else CheckDlgButton(hDlg,IDC_KEY, BST_UNCHECKED);

	SetButton(hDlg, IDTEXT_DPAD_UP,		joysticks[controller].names[MAP_D_UP]);
	SetButton(hDlg, IDTEXT_DPAD_DOWN,	joysticks[controller].names[MAP_D_DOWN]);
	SetButton(hDlg, IDTEXT_DPAD_LEFT,	joysticks[controller].names[MAP_D_LEFT]);
	SetButton(hDlg, IDTEXT_DPAD_RIGHT,	joysticks[controller].names[MAP_D_RIGHT]);	
	
}

// Get dialog items
// ----------------
void GetControllerAll(HWND hDlg, int controller)
{
	if(joysticks[controller].controllertype == CTL_TYPE_JOYSTICK_SDL)
		joysticks[controller].ID = (int)SendMessage(GetDlgItem(hDlg, IDC_JOYNAME_SDL), CB_GETCURSEL, 0, 0); 
	else if(joysticks[controller].controllertype == CTL_TYPE_JOYSTICK_XINPUT)
		joysticks[controller].ID = (int)SendMessage(GetDlgItem(hDlg, IDC_JOYNAME_XINPUT), CB_GETCURSEL, 0, 0); 

	GetButton(hDlg, IDTEXT_SHOULDERL,		joysticks[controller].names[MAP_LT]);
	GetButton(hDlg, IDTEXT_SHOULDERR,		joysticks[controller].names[MAP_RT]);
	GetButton(hDlg, IDTEXT_A,				joysticks[controller].names[MAP_A]);
	GetButton(hDlg, IDTEXT_B,				joysticks[controller].names[MAP_B]);
	GetButton(hDlg, IDTEXT_X,				joysticks[controller].names[MAP_X]);
	GetButton(hDlg, IDTEXT_Y,				joysticks[controller].names[MAP_Y]);

	GetButton(hDlg, IDTEXT_START,			joysticks[controller].names[MAP_START]);
	
	GetButton(hDlg, IDTEXT_HALFPRESS,		joysticks[controller].names[MAP_HALF]);
		
	GetButton(hDlg, IDTEXT_DPAD_UP,			joysticks[controller].names[MAP_D_UP]);
	GetButton(hDlg, IDTEXT_DPAD_DOWN,		joysticks[controller].names[MAP_D_DOWN]);
	GetButton(hDlg, IDTEXT_DPAD_LEFT,		joysticks[controller].names[MAP_D_LEFT]);
	GetButton(hDlg, IDTEXT_DPAD_RIGHT,		joysticks[controller].names[MAP_D_RIGHT]);	

	GetButton(hDlg, IDTEXT_MX_L,			joysticks[controller].names[MAP_A_XL]);
	GetButton(hDlg, IDTEXT_MX_R,			joysticks[controller].names[MAP_A_XR]);
	GetButton(hDlg, IDTEXT_MY_U,			joysticks[controller].names[MAP_A_YU]);
	GetButton(hDlg, IDTEXT_MY_D,			joysticks[controller].names[MAP_A_YD]);
	
	joysticks[current_port].keys = IsDlgButtonChecked(hDlg, IDC_KEY) == BST_CHECKED? 1:0;
	joysticks[controller].controllertype = (int)SendMessage(GetDlgItem(hDlg, IDC_CONTROLTYPE), CB_GETCURSEL, 0, 0); 
	joysticks[controller].deadzone = (int)SendMessage(GetDlgItem(hDlg, IDC_DEADZONE), CB_GETCURSEL, 0, 0);
	
}

// Get text from static text item
// ------------------------------
void GetButton(HWND hDlg, int item, wchar* Receiver)
{	
	GetDlgItemText(hDlg, item, Receiver, 8);	
}

// Set text in static text item
// ----------------------------
void SetButton(HWND hDlg, int item, wchar* value)
{	
	SetDlgItemText(hDlg, item, value);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Config dialog functions
// -----------------------

// OpenAbout
// ---------
void OpenAbout(HINSTANCE abouthInstance, HWND _hParent)
{
	DialogBox(abouthInstance,MAKEINTRESOURCE(IDD_ABOUT), _hParent, AboutDlg);
}

// AboutDlg
// --------
BOOL CALLBACK AboutDlg(HWND abouthWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	wchar format[512];	
	
	switch (message)
	{		
		case WM_INITDIALOG:			
			wsprintf(format, L"PuruPuru v" _T(INPUT_VERSION) L" by Falcon4ever\n"
			L"Builddate: " _T(__DATE__) L"\n"
			L"www.multigesture.net");			
			SetDlgItemText(abouthWnd, IDC_ABOUT1, format);

			wsprintf(format, _T(INPUT_STATE));
			SetDlgItemText(abouthWnd, IDC_STATEI, format);
			
			wsprintf(format, _T(THANKYOU));
			SetDlgItemText(abouthWnd, IDC_ABOUT3, format);
		break;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK) 
			{
				EndDialog(abouthWnd, LOWORD(wParam));
				return TRUE;
			}
		break;
	}
    return FALSE;
}

