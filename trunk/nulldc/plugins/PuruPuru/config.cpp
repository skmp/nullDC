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
//
#include "config.h"

//////////////////////////////////////////////////////////////////////////////////////////
// Variables
// ---------

extern u32 current_port;
extern bool emulator_running;
extern HINSTANCE PuruPuru_hInst;

extern Supported_Status Support;

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

#ifdef BUILD_NAOMI
		tci.pszText = current_port == 0 ? L"->Controller 1<-" : L"Controller 1"; 
		TabCtrl_InsertItem(GetDlgItem(hDlg,IDC_PORTTAB), 0, &tci); 
		tci.pszText = current_port == 1 ? L"->Controller 2<-" : L"Controller 2"; 
		TabCtrl_InsertItem(GetDlgItem(hDlg,IDC_PORTTAB), 1, &tci); 
#elif defined BUILD_DREAMCAST	
		tci.pszText = current_port == 0 ? L"->Controller 1<-" : L"Controller 1"; 
		TabCtrl_InsertItem(GetDlgItem(hDlg,IDC_PORTTAB), 0, &tci); 
		tci.pszText = current_port == 1 ? L"->Controller 2<-" : L"Controller 2"; 
		TabCtrl_InsertItem(GetDlgItem(hDlg,IDC_PORTTAB), 1, &tci); 
		tci.pszText = current_port == 2 ? L"->Controller 3<-" : L"Controller 3"; 
		TabCtrl_InsertItem(GetDlgItem(hDlg,IDC_PORTTAB), 2, &tci); 
		tci.pszText = current_port == 3 ? L"->Controller 4<-" : L"Controller 4"; 
		TabCtrl_InsertItem(GetDlgItem(hDlg,IDC_PORTTAB), 3, &tci); 
#endif
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
			
			// Dreadzone Dropbox
			wchar buffer[8];				
			ControlType = GetDlgItem(hDlg, IDC_DEADZONE);
			SendMessage(ControlType, CB_RESETCONTENT, 0, 0);
			for(int x = 1; x <= 100; x++)
			{				
				wsprintf(buffer, L"%d %%", x);
				SendMessage(ControlType, CB_ADDSTRING, 0, (LPARAM)buffer);				
			}

			// Pakku Intensity Dropbox							
			ControlType = GetDlgItem(hDlg, IDC_PAKKU);
			SendMessage(ControlType, CB_RESETCONTENT, 0, 0);
			for(int x = 1; x <= 100; x++)
			{				
				wsprintf(buffer, L"%d %%", 2*x);
				SendMessage(ControlType, CB_ADDSTRING, 0, (LPARAM)buffer);				
			}
		}

		SetControllerAll(hDlg, current_port);
						
		// Search for devices and add the to the device list
		if( Support.SDL )
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

		if( Support.XInput )
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
				GetControllerAll(hDlg, current_port);
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

#ifdef BUILD_NAOMI
			case IDC_NBUTTON1:
			case IDC_NBUTTON2:
			case IDC_NBUTTON3:
			case IDC_NBUTTON4:
			case IDC_NBUTTON5:
			case IDC_NBUTTON6:
			case IDC_COIN:
			case IDC_SERVICE1:
			case IDC_SERVICE2:
			case IDC_TEST1:
			case IDC_TEST2:
#elif defined BUILD_DREAMCAST
			case IDC_SHOULDERL:
			case IDC_SHOULDERR:
			case IDC_A:
			case IDC_B:
			case IDC_X:
			case IDC_Y:							
			case IDC_HALFPRESS:												
			case IDC_MX_L:
			case IDC_MX_R:
			case IDC_MY_U:
			case IDC_MY_D:
#endif		
			case IDC_START:
			case IDC_DPAD_UP:
			case IDC_DPAD_DOWN:
			case IDC_DPAD_LEFT:
			case IDC_DPAD_RIGHT:
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
	int threshold = 16384;

	int counter1 = 0;
	int counter2 = 10;
	
	wsprintf(format, L"[%d]", counter2);
	SetDlgItemText(hDlg, buttonid, format);		

	// To get the "always pressed" keys.
	unsigned char keycheck[255] = {0};
	for(int i=0; i<255; i++) 
	keycheck[i] = GetAsyncKeyState(i) >> 14;


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
		if(xoyinfo[pad].state.Gamepad.bLeftTrigger > 128)
		{
			wsprintf(format, L"LT");		
			succeed = true;
			waiting = false;			
		}

		if(xoyinfo[pad].state.Gamepad.bRightTrigger > 128)
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
			for(int k = 0; k < 255; k++)
			{
				unsigned char key = GetAsyncKeyState(k) >> 14;				

				if(key && !keycheck[k])
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

	// To get the "always pressed" keys.
	unsigned char keycheck[255] = {0};
	for(int i=0; i<255; i++) 
	keycheck[i] = GetAsyncKeyState(i) >> 14;

	while(waiting)
	{													

		for(int k = 1; k < 255; k++)
		{												
			unsigned char key = GetAsyncKeyState(k) >> 14;				

			if(key && !keycheck[k])
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

	int threshold = 16384;
	
	wsprintf(format, L"[%d]", counter2);
	SetDlgItemText(hDlg, buttonid, format);
	
	// To get the "always pressed" keys.
	unsigned char keycheck[255] = {0};
	for(int i=0; i<255; i++) 
	keycheck[i] = GetAsyncKeyState(i) >> 14;

	while(waiting)
	{			
		SDL_JoystickUpdate();		

		// AXIS
		for(int b = 0; b < axes; b++)
		{		
			value = SDL_JoystickGetAxis(joy, b);
			if(value > threshold)
			{
				pressed = b;	
				plus = true;
				waiting = false;
				succeed = true;
				AXIS = true;
				break;
			}
			else if ( value < -threshold )
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
				unsigned char key = GetAsyncKeyState(k) >> 14;				

				if(key && !keycheck[k])
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
#ifdef BUILD_DREAMCAST
	ShowWindow(GetDlgItem(hDlg, IDC_CONFIG_ON), FALSE);
	ShowWindow(GetDlgItem(hDlg, IDC_CONFIG_ON2), FALSE);
	ShowWindow(GetDlgItem(hDlg, IDC_PAKKU), FALSE);
	ShowWindow(GetDlgItem(hDlg, IDC_PAKKU_TEXT), FALSE);
#endif
	
	switch(controllertype)
	{
	case CTL_TYPE_JOYSTICK_SDL: 
		{
			ShowWindow(GetDlgItem(hDlg, IDC_JOYNAME_SDL), TRUE);
			ShowWindow(GetDlgItem(hDlg, IDC_JOYNAME_XINPUT), FALSE);
			ShowWindow(GetDlgItem(hDlg, IDC_JOYNAME_KEY), FALSE);
			ShowWindow(GetDlgItem(hDlg, IDC_KEY), TRUE);
#ifdef BUILD_DREAMCAST			
			if(joyinfo[joysticks[current_port].ID].canRumble)
			{
				ShowWindow(GetDlgItem(hDlg, IDC_CONFIG_ON2), TRUE);			
				ShowWindow(GetDlgItem(hDlg, IDC_PAKKU), TRUE);
				ShowWindow(GetDlgItem(hDlg, IDC_PAKKU_TEXT), TRUE);	
			}
			else
				ShowWindow(GetDlgItem(hDlg, IDC_CONFIG_ON), TRUE);
#endif
		}
		break;

	case CTL_TYPE_JOYSTICK_XINPUT:
		{
			ShowWindow(GetDlgItem(hDlg, IDC_JOYNAME_XINPUT), TRUE);
			ShowWindow(GetDlgItem(hDlg, IDC_JOYNAME_SDL), FALSE);		
			ShowWindow(GetDlgItem(hDlg, IDC_JOYNAME_KEY), FALSE);
			ShowWindow(GetDlgItem(hDlg, IDC_KEY), TRUE);
#ifdef BUILD_DREAMCAST			
			ShowWindow(GetDlgItem(hDlg, IDC_CONFIG_ON2), TRUE);
			ShowWindow(GetDlgItem(hDlg, IDC_PAKKU), TRUE);
			ShowWindow(GetDlgItem(hDlg, IDC_PAKKU_TEXT), TRUE);
#endif
		}
		break;

	case CTL_TYPE_KEYBOARD: 
		{
			ShowWindow(GetDlgItem(hDlg, IDC_JOYNAME_KEY), TRUE);
			ShowWindow(GetDlgItem(hDlg, IDC_JOYNAME_SDL), FALSE);
			ShowWindow(GetDlgItem(hDlg, IDC_JOYNAME_XINPUT), FALSE);			
			ShowWindow(GetDlgItem(hDlg, IDC_KEY), FALSE);
#ifdef BUILD_DREAMCAST
			ShowWindow(GetDlgItem(hDlg, IDC_CONFIG_ON), TRUE);
#endif
		}
		break;
	}				
	
}

// Set dialog items
// ----------------
void SetControllerAll(HWND hDlg, int controller)
{		
	SendMessage(GetDlgItem(hDlg, IDC_JOYNAME_SDL), CB_SETCURSEL, joysticks[controller].ID, 0);
	SendMessage(GetDlgItem(hDlg, IDC_JOYNAME_XINPUT), CB_SETCURSEL, joysticks[controller].ID, 0);

#ifdef BUILD_DREAMCAST
	ShowWindow(GetDlgItem(hDlg, IDC_CONFIG_ON), FALSE);
	ShowWindow(GetDlgItem(hDlg, IDC_CONFIG_ON2), FALSE);
	ShowWindow(GetDlgItem(hDlg, IDC_CONFIG_OFF), FALSE);

	if( joysticks[controller].enabled )
	{
		switch(joysticks[controller].controllertype)
		{
		case CTL_TYPE_JOYSTICK_SDL: 
			{
				if(joyinfo[joysticks[controller].ID].canRumble)
					ShowWindow(GetDlgItem(hDlg, IDC_CONFIG_ON2), TRUE);
				else
					ShowWindow(GetDlgItem(hDlg, IDC_CONFIG_ON), TRUE);
			}
		case CTL_TYPE_JOYSTICK_XINPUT: ShowWindow(GetDlgItem(hDlg, IDC_CONFIG_ON2), TRUE); break;
		case CTL_TYPE_KEYBOARD: ShowWindow(GetDlgItem(hDlg, IDC_CONFIG_ON), TRUE); break;
		}			

		ShowWindow(GetDlgItem(hDlg, IDC_CONFIG_OFF), FALSE);
	}
	else			
		ShowWindow(GetDlgItem(hDlg, IDC_CONFIG_OFF), TRUE);	

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

	SetButton(hDlg, IDTEXT_DPAD_UP,		joysticks[controller].names[MAP_D_UP]);
	SetButton(hDlg, IDTEXT_DPAD_DOWN,	joysticks[controller].names[MAP_D_DOWN]);
	SetButton(hDlg, IDTEXT_DPAD_LEFT,	joysticks[controller].names[MAP_D_LEFT]);
	SetButton(hDlg, IDTEXT_DPAD_RIGHT,	joysticks[controller].names[MAP_D_RIGHT]);	

	SendMessage(GetDlgItem(hDlg, IDC_DEADZONE), CB_SETCURSEL, joysticks[controller].deadzone, 0);	
	SendMessage(GetDlgItem(hDlg, IDC_PAKKU), CB_SETCURSEL, (joysticks[controller].pakku_intensity>>1)-1, 0);
#elif defined BUILD_NAOMI
	SetButton(hDlg, IDTEXT_SERVICE1,	joysticks[controller].names[MAPN_SERVICE1]);
	SetButton(hDlg, IDTEXT_SERVICE2,	joysticks[controller].names[MAPN_SERVICE2]);
	SetButton(hDlg, IDTEXT_TEST1,		joysticks[controller].names[MAPN_TEST1]);
	SetButton(hDlg, IDTEXT_TEST2,		joysticks[controller].names[MAPN_TEST2]);

	SetButton(hDlg, IDTEXT_NBUTTON1,	joysticks[controller].names[MAPN_BUTTON1]);
	SetButton(hDlg, IDTEXT_NBUTTON2,	joysticks[controller].names[MAPN_BUTTON2]);
	SetButton(hDlg, IDTEXT_NBUTTON3,	joysticks[controller].names[MAPN_BUTTON3]);	
	SetButton(hDlg, IDTEXT_NBUTTON4,	joysticks[controller].names[MAPN_BUTTON4]);
	SetButton(hDlg, IDTEXT_NBUTTON5,	joysticks[controller].names[MAPN_BUTTON5]);
	SetButton(hDlg, IDTEXT_NBUTTON6,	joysticks[controller].names[MAPN_BUTTON6]);
	
	SetButton(hDlg, IDTEXT_START,		joysticks[controller].names[MAPN_START]);	
	SetButton(hDlg, IDTEXT_COIN,		joysticks[controller].names[MAPN_COIN]);

	SetButton(hDlg, IDTEXT_DPAD_UP,		joysticks[controller].names[MAPN_D_UP]);
	SetButton(hDlg, IDTEXT_DPAD_DOWN,	joysticks[controller].names[MAPN_D_DOWN]);
	SetButton(hDlg, IDTEXT_DPAD_LEFT,	joysticks[controller].names[MAPN_D_LEFT]);
	SetButton(hDlg, IDTEXT_DPAD_RIGHT,	joysticks[controller].names[MAPN_D_RIGHT]);	
#endif
	SendMessage(GetDlgItem(hDlg, IDC_CONTROLTYPE), CB_SETCURSEL, joysticks[controller].controllertype, 0);		
	UpdateVisibleItems(hDlg, joysticks[controller].controllertype);

	if(joysticks[controller].keys == 1) CheckDlgButton(hDlg,IDC_KEY, BST_CHECKED);
	else CheckDlgButton(hDlg,IDC_KEY, BST_UNCHECKED);	
}

// Get dialog items
// ----------------
void GetControllerAll(HWND hDlg, int controller)
{
	if(joysticks[controller].controllertype == CTL_TYPE_JOYSTICK_SDL)
		joysticks[controller].ID = (int)SendMessage(GetDlgItem(hDlg, IDC_JOYNAME_SDL), CB_GETCURSEL, 0, 0); 
	else if(joysticks[controller].controllertype == CTL_TYPE_JOYSTICK_XINPUT)
		joysticks[controller].ID = (int)SendMessage(GetDlgItem(hDlg, IDC_JOYNAME_XINPUT), CB_GETCURSEL, 0, 0); 

#ifdef BUILD_NAOMI

	GetButton(hDlg, IDTEXT_SERVICE1,	joysticks[controller].names[MAPN_SERVICE1]);
	GetButton(hDlg, IDTEXT_SERVICE2,	joysticks[controller].names[MAPN_SERVICE2]);
	GetButton(hDlg, IDTEXT_TEST1,		joysticks[controller].names[MAPN_TEST1]);
	GetButton(hDlg, IDTEXT_TEST2,		joysticks[controller].names[MAPN_TEST2]);

	GetButton(hDlg, IDTEXT_NBUTTON1,	joysticks[controller].names[MAPN_BUTTON1]);
	GetButton(hDlg, IDTEXT_NBUTTON2,	joysticks[controller].names[MAPN_BUTTON2]);
	GetButton(hDlg, IDTEXT_NBUTTON3,	joysticks[controller].names[MAPN_BUTTON3]);	
	GetButton(hDlg, IDTEXT_NBUTTON4,	joysticks[controller].names[MAPN_BUTTON4]);
	GetButton(hDlg, IDTEXT_NBUTTON5,	joysticks[controller].names[MAPN_BUTTON5]);
	GetButton(hDlg, IDTEXT_NBUTTON6,	joysticks[controller].names[MAPN_BUTTON6]);
	
	GetButton(hDlg, IDTEXT_START,		joysticks[controller].names[MAPN_START]);	
	GetButton(hDlg, IDTEXT_COIN,		joysticks[controller].names[MAPN_COIN]);

	GetButton(hDlg, IDTEXT_DPAD_UP,		joysticks[controller].names[MAPN_D_UP]);
	GetButton(hDlg, IDTEXT_DPAD_DOWN,	joysticks[controller].names[MAPN_D_DOWN]);
	GetButton(hDlg, IDTEXT_DPAD_LEFT,	joysticks[controller].names[MAPN_D_LEFT]);
	GetButton(hDlg, IDTEXT_DPAD_RIGHT,	joysticks[controller].names[MAPN_D_RIGHT]);	
#else
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

	joysticks[controller].deadzone = (int)SendMessage(GetDlgItem(hDlg, IDC_DEADZONE), CB_GETCURSEL, 0, 0);
	joysticks[controller].pakku_intensity = (int)(SendMessage(GetDlgItem(hDlg, IDC_PAKKU), CB_GETCURSEL, 0, 0)+1)<<1;
#endif
	joysticks[current_port].keys = IsDlgButtonChecked(hDlg, IDC_KEY) == BST_CHECKED? 1:0;
	joysticks[controller].controllertype = (int)SendMessage(GetDlgItem(hDlg, IDC_CONTROLTYPE), CB_GETCURSEL, 0, 0); 		
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

