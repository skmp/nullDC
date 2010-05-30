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

extern u32 current_port;
extern bool emulator_running;

extern CONTROLLER_INFO	*joyinfo;
extern CONTROLLER_MAPPING joysticks[4];
extern HINSTANCE PuruPuru_hInst;
//HINSTANCE config_hInst;
//HWND config_HWND;

static const wchar* ControllerType[] =
{
	L"Joystick (SDL)",	
	L"Joystick (Xinput)",	// Perhaps just for the triggers...
	L"Keyboard (Win)"		// SDL for keys is meh... 
};

//////////////////////////////////////////////////////////////////////////////////////////
// Config dialog functions
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯


// NOT pretty... 
int tabOrder( int tab )
{
	tab = tab - current_port;
	if (tab < 0) tab = 4 + tab;

	return tab;
}


int OpenConfig(HINSTANCE hInst, HWND _hParent)
{
	//config_hInst = hInst;
	//config_HWND = _hParent;
	
	PROPSHEETPAGE psp[4];
    PROPSHEETHEADER psh;

    psp[tabOrder(0)].dwSize = sizeof(PROPSHEETPAGE);
    psp[tabOrder(0)].dwFlags = PSP_USETITLE;
    psp[tabOrder(0)].hInstance = hInst;
    psp[tabOrder(0)].pszTemplate = MAKEINTRESOURCE( joysticks[0].enabled? IDD_CONFIG_ON: IDD_CONFIG_OFF );
    psp[tabOrder(0)].pszIcon = NULL;
    psp[tabOrder(0)].pfnDlgProc = (DLGPROC)ControllerTab1;
    psp[tabOrder(0)].pszTitle = current_port == 0? L"->Controller 1<-" : L"Controller 1"; 
    psp[tabOrder(0)].lParam = 0;	

    psp[tabOrder(1)].dwSize = sizeof(PROPSHEETPAGE);
    psp[tabOrder(1)].dwFlags = PSP_USETITLE;
    psp[tabOrder(1)].hInstance = hInst;
    psp[tabOrder(1)].pszTemplate = MAKEINTRESOURCE( joysticks[1].enabled? IDD_CONFIG_ON: IDD_CONFIG_OFF );
    psp[tabOrder(1)].pszIcon = NULL;
    psp[tabOrder(1)].pfnDlgProc = (DLGPROC)ControllerTab2;
    psp[tabOrder(1)].pszTitle = current_port == 1? L"->Controller 2<-" : L"Controller 2"; 
    psp[tabOrder(1)].lParam = 0;
    
	psp[tabOrder(2)].dwSize = sizeof(PROPSHEETPAGE);
    psp[tabOrder(2)].dwFlags = PSP_USETITLE;
    psp[tabOrder(2)].hInstance = hInst;
    psp[tabOrder(2)].pszTemplate = MAKEINTRESOURCE( joysticks[2].enabled? IDD_CONFIG_ON: IDD_CONFIG_OFF );
    psp[tabOrder(2)].pszIcon = NULL;
    psp[tabOrder(2)].pfnDlgProc = (DLGPROC)ControllerTab3;
    psp[tabOrder(2)].pszTitle = current_port == 2? L"->Controller 3<-" : L"Controller 3"; 
    psp[tabOrder(2)].lParam = 0;

	psp[tabOrder(3)].dwSize = sizeof(PROPSHEETPAGE);
    psp[tabOrder(3)].dwFlags = PSP_USETITLE;
    psp[tabOrder(3)].hInstance = hInst;
    psp[tabOrder(3)].pszTemplate = MAKEINTRESOURCE( joysticks[3].enabled? IDD_CONFIG_ON: IDD_CONFIG_OFF );
    psp[tabOrder(3)].pszIcon = NULL;
    psp[tabOrder(3)].pfnDlgProc = (DLGPROC)ControllerTab4;
    psp[tabOrder(3)].pszTitle = current_port == 3? L"->Controller 4<-" : L"Controller 4"; 
    psp[tabOrder(3)].lParam = 0;

    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW | PSH_NOCONTEXTHELP;	
    psh.hwndParent = _hParent;
    psh.hInstance = hInst;
    psh.pszIcon = NULL;
    #ifndef _DEBUG
		psh.pszCaption = L"Configure: PuruPuru v" _T(INPUT_VERSION) L" Input Plugin";
	#else
		psh.pszCaption = L"Configure: PuruPuru v" _T(INPUT_VERSION) L" (Debug) Input Plugin";		
	#endif
    psh.nPages = sizeof(psp) / sizeof(PROPSHEETPAGE);
    psh.ppsp = (LPCPROPSHEETPAGE) &psp;
	
	return (int)(PropertySheet(&psh));
}

// Create Tab
// ¯¯¯¯¯¯¯¯¯¯

BOOL APIENTRY ControllerTab1(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
	return ControllerTab(hDlg, message, wParam, lParam, 0);	
}

BOOL APIENTRY ControllerTab2(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
	return ControllerTab(hDlg, message, wParam, lParam, 1);	
}

BOOL APIENTRY ControllerTab3(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
	return ControllerTab(hDlg, message, wParam, lParam, 2);	
}

BOOL APIENTRY ControllerTab4(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
	return ControllerTab(hDlg, message, wParam, lParam, 3);	
}

// Create Controller Tab
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯

BOOL ControllerTab(HWND hDlg, UINT message, UINT wParam, LONG lParam, int controller)
{						

	switch (message)
	{
		case WM_INITDIALOG:

			// Prevent user from changing the joystick while emulation is running			
			// Is used automatically now, so disable and just show.
			Button_Enable(GetDlgItem(hDlg, IDC_JOYATTACH), FALSE);									

			//if(emulator_running)
			if(emulator_running)
			{
				ComboBox_Enable(GetDlgItem(hDlg, IDC_JOYNAME), FALSE);
				ComboBox_Enable(GetDlgItem(hDlg, IDC_CONTROLTYPE), FALSE);				
			}
			else
			{
				ComboBox_Enable(GetDlgItem(hDlg, IDC_JOYNAME), TRUE);				
				ComboBox_Enable(GetDlgItem(hDlg, IDC_CONTROLTYPE), TRUE);				
			}
			
			
			// Search for devices and add the to the device list
			if( SDL_NumJoysticks() > 0 )
			{
				HWND CB = GetDlgItem(hDlg, IDC_JOYNAME);

				for(int x = 0; x < SDL_NumJoysticks(); x++)
				{					
					SendMessage(CB, CB_ADDSTRING, 0, (LPARAM)joyinfo[x].Name);					
				}
				
				CB = GetDlgItem(hDlg, IDC_CONTROLTYPE);				
				SendMessage(CB, CB_ADDSTRING, 0, (LPARAM)ControllerType[CTL_TYPE_JOYSTICK_SDL]);				
				//SendMessage(CB, CB_ADDSTRING, 0, (LPARAM)ControllerType[CTL_TYPE_JOYSTICK_XINPUT]);
				//SendMessage(CB, CB_ADDSTRING, 0, (LPARAM)ControllerType[CTL_TYPE_KEYBOARD]);
				
				wchar buffer[8];				
				CB = GetDlgItem(hDlg, IDC_DEADZONE);
				SendMessage(CB, CB_RESETCONTENT, 0, 0);
				for(int x = 1; x <= 100; x++)
				{				
					wsprintf(buffer, L"%d %%", x);
					SendMessage(CB, CB_ADDSTRING, 0, (LPARAM)buffer);				
				}

				SetControllerAll(hDlg, controller);
				return TRUE;
			}
			else
			{
				HWND CB = GetDlgItem(hDlg, IDC_JOYNAME);				
				SendMessage(CB, CB_ADDSTRING, 0, (LPARAM)L"No Joystick detected!");
				SendMessage(CB, CB_SETCURSEL, 0, 0);
				return FALSE;
			}			
		break;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_JOYNAME:
				{
					// Selected a different joystick
					if(HIWORD(wParam) == CBN_SELCHANGE)
					{
						joysticks[controller].ID = (int)SendMessage(GetDlgItem(hDlg, IDC_JOYNAME), CB_GETCURSEL, 0, 0);						
					}
					return TRUE;
				}
				break;

				case IDC_CONTROLTYPE:
				{
					// Selected a different joystick
					if(HIWORD(wParam) == CBN_SELCHANGE)
					{
						joysticks[controller].controllertype = (int)SendMessage(GetDlgItem(hDlg, IDC_CONTROLTYPE), CB_GETCURSEL, 0, 0);
						UpdateVisibleItems(hDlg, joysticks[controller].controllertype);
					}
					return TRUE;
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
				{
					GetButtons(hDlg, LOWORD(wParam), controller);
					return TRUE;
				}
				break;
				
				case IDC_DPAD_UP:
				case IDC_DPAD_DOWN:
				case IDC_DPAD_LEFT:
				case IDC_DPAD_RIGHT:
				{
					GetHats(hDlg, LOWORD(wParam), controller);
					return TRUE;
				}
				break;
				
				case IDC_MX:
				case IDC_MY:
				{
					GetAxis(hDlg, LOWORD(wParam), controller);
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
				
		case WM_DESTROY:			
			GetControllerAll(hDlg, controller);
		break;
	}	
	return FALSE;	
}

// Wait for button press
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
bool GetButtons(HWND hDlg, int buttonid, int controller)
{
	buttonid += 1000;
		
	SDL_Joystick *joy;
	joy=SDL_JoystickOpen(joysticks[controller].ID);

	wchar format[128];
	int buttons = SDL_JoystickNumButtons(joy);
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
		wsprintf(format, L"%d", pressed);
	else
		wsprintf(format, L"-1", pressed);
	SetDlgItemText(hDlg, buttonid, format);

	if(SDL_JoystickOpened(joysticks[controller].ID))
		SDL_JoystickClose(joy);

	return true;
}

// Wait for D-Pad
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯
bool GetHats(HWND hDlg, int buttonid, int controller)
{
	buttonid += 1000;

	SDL_Joystick *joy;
	joy=SDL_JoystickOpen(joysticks[controller].ID);

	wchar format[128];
	int hats = SDL_JoystickNumHats(joy);
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
		for(int b = 0; b < hats; b++)
		{			
			switch (SDL_JoystickGetHat(joy, b))
			{
				case SDL_HAT_LEFT:
					{
						pressed = SDL_HAT_LEFT;	
						waiting = false;
						succeed = true;
						break;
					}
				case SDL_HAT_RIGHT:
					{
						pressed = SDL_HAT_RIGHT;	
						waiting = false;
						succeed = true;
						break;
					}
				case SDL_HAT_UP:
					{
						pressed = SDL_HAT_UP;	
						waiting = false;
						succeed = true;
						break;
					}
				case SDL_HAT_DOWN:
					{
						pressed = SDL_HAT_DOWN;	
						waiting = false;
						succeed = true;
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
		wsprintf(format, L"%d", pressed);
	else
		wsprintf(format, L"-1", pressed);
	SetDlgItemText(hDlg, buttonid, format);

	if(SDL_JoystickOpened(joysticks[controller].ID))
		SDL_JoystickClose(joy);

	return true;
}

// Wait for Analog
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
bool GetAxis(HWND hDlg, int buttonid, int controller)
{
	buttonid += 1000;

	SDL_Joystick *joy;
	joy=SDL_JoystickOpen(joysticks[controller].ID);

	wchar format[128];
	int axes = SDL_JoystickNumAxes(joy);
	bool waiting = true;
	bool succeed = false;
	int pressed = 0;
	Sint16 value;
	
	int counter1 = 0;
	int counter2 = 10;
	
	wsprintf(format, L"[%d]", counter2);
	SetDlgItemText(hDlg, buttonid, format);

	while(waiting)
	{		
		SDL_JoystickUpdate();
		for(int b = 0; b < axes; b++)
		{		
			value = SDL_JoystickGetAxis(joy, b);
			if(value < -10000 || value > 10000)
			{
				pressed = b;	
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
		wsprintf(format, L"%d", pressed);
	else
		wsprintf(format, L"-1", pressed);
	SetDlgItemText(hDlg, buttonid, format);

	if(SDL_JoystickOpened(joysticks[controller].ID))
		SDL_JoystickClose(joy);

	return true;
}

void UpdateVisibleItems(HWND hDlg, int controllertype)
{	
	if(controllertype == CTL_TYPE_KEYBOARD)	
	{
		ComboBox_Enable(GetDlgItem(hDlg, IDC_JOYNAME), FALSE);		
	}
	else
	{
		// ComboBox_Enable(GetDlgItem(hDlg, IDC_JOYNAME), TRUE);

		ShowWindow(GetDlgItem(hDlg, IDTEXT_DPAD_DOWN), TRUE);
		ShowWindow(GetDlgItem(hDlg, IDTEXT_DPAD_LEFT), TRUE);
		ShowWindow(GetDlgItem(hDlg, IDTEXT_DPAD_RIGHT), TRUE);
		ShowWindow(GetDlgItem(hDlg, IDC_DPAD_DOWN), TRUE);
		ShowWindow(GetDlgItem(hDlg, IDC_DPAD_LEFT), TRUE);
		ShowWindow(GetDlgItem(hDlg, IDC_DPAD_RIGHT), TRUE);
		ShowWindow(GetDlgItem(hDlg, IDC_DPAD_TEXT1), TRUE);
		ShowWindow(GetDlgItem(hDlg, IDC_DPAD_TEXT2), TRUE);
		ShowWindow(GetDlgItem(hDlg, IDC_DPAD_TEXT3), TRUE);
		ShowWindow(GetDlgItem(hDlg, IDC_DPAD_TEXT4), TRUE);
	}
}

// Set dialog items
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
void SetControllerAll(HWND hDlg, int controller)
{	
	SendMessage(GetDlgItem(hDlg, IDC_JOYNAME), CB_SETCURSEL, joysticks[controller].ID, 0);

	SetButton(hDlg, IDTEXT_SHOULDERL, joysticks[controller].buttons[CTL_L_SHOULDER]);
	SetButton(hDlg, IDTEXT_SHOULDERR, joysticks[controller].buttons[CTL_R_SHOULDER]);
	SetButton(hDlg, IDTEXT_A, joysticks[controller].buttons[CTL_A_BUTTON]);
	SetButton(hDlg, IDTEXT_B, joysticks[controller].buttons[CTL_B_BUTTON]);
	SetButton(hDlg, IDTEXT_X, joysticks[controller].buttons[CTL_X_BUTTON]);
	SetButton(hDlg, IDTEXT_Y, joysticks[controller].buttons[CTL_Y_BUTTON]);

	SetButton(hDlg, IDTEXT_START, joysticks[controller].buttons[CTL_START]);
	
	SetButton(hDlg, IDTEXT_HALFPRESS, joysticks[controller].halfpress);
	
	SetButton(hDlg, IDTEXT_MX, joysticks[controller].axis[CTL_MAIN_X]);
	SetButton(hDlg, IDTEXT_MY, joysticks[controller].axis[CTL_MAIN_Y]);

	SendDlgItemMessage(hDlg, IDC_JOYATTACH, BM_SETCHECK, joysticks[controller].enabled, 0);

	SendMessage(GetDlgItem(hDlg, IDC_CONTROLTYPE), CB_SETCURSEL, joysticks[controller].controllertype, 0);	
	SendMessage(GetDlgItem(hDlg, IDC_DEADZONE), CB_SETCURSEL, joysticks[controller].deadzone, 0);	

	UpdateVisibleItems(hDlg, joysticks[controller].controllertype);

	SetButton(hDlg, IDTEXT_DPAD_UP, joysticks[controller].dpad[CTL_D_PAD_UP]);
	SetButton(hDlg, IDTEXT_DPAD_DOWN, joysticks[controller].dpad[CTL_D_PAD_DOWN]);
	SetButton(hDlg, IDTEXT_DPAD_LEFT, joysticks[controller].dpad[CTL_D_PAD_LEFT]);
	SetButton(hDlg, IDTEXT_DPAD_RIGHT, joysticks[controller].dpad[CTL_D_PAD_RIGHT]);		
}

// Get dialog items
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
void GetControllerAll(HWND hDlg, int controller)
{
	
	joysticks[controller].ID = (int)SendMessage(GetDlgItem(hDlg, IDC_JOYNAME), CB_GETCURSEL, 0, 0); 

	joysticks[controller].buttons[CTL_L_SHOULDER] = GetButton(hDlg, IDTEXT_SHOULDERL);
	joysticks[controller].buttons[CTL_R_SHOULDER] = GetButton(hDlg, IDTEXT_SHOULDERR);
	joysticks[controller].buttons[CTL_A_BUTTON] = GetButton(hDlg, IDTEXT_A);
	joysticks[controller].buttons[CTL_B_BUTTON] = GetButton(hDlg, IDTEXT_B);
	joysticks[controller].buttons[CTL_X_BUTTON] = GetButton(hDlg, IDTEXT_X);
	joysticks[controller].buttons[CTL_Y_BUTTON] = GetButton(hDlg, IDTEXT_Y);

	joysticks[controller].buttons[CTL_START] = GetButton(hDlg, IDTEXT_START);
	
	joysticks[controller].halfpress = GetButton(hDlg, IDTEXT_HALFPRESS);
		
	joysticks[controller].dpad[CTL_D_PAD_UP] = GetButton(hDlg, IDTEXT_DPAD_UP);
	joysticks[controller].dpad[CTL_D_PAD_DOWN] = GetButton(hDlg, IDTEXT_DPAD_DOWN);
	joysticks[controller].dpad[CTL_D_PAD_LEFT] = GetButton(hDlg, IDTEXT_DPAD_LEFT);
	joysticks[controller].dpad[CTL_D_PAD_RIGHT] = GetButton(hDlg, IDTEXT_DPAD_RIGHT);	

	joysticks[controller].axis[CTL_MAIN_X] = GetButton(hDlg, IDTEXT_MX);
	joysticks[controller].axis[CTL_MAIN_Y] = GetButton(hDlg, IDTEXT_MY);
	
	joysticks[controller].controllertype = (int)SendMessage(GetDlgItem(hDlg, IDC_CONTROLTYPE), CB_GETCURSEL, 0, 0); 
	joysticks[controller].deadzone = (int)SendMessage(GetDlgItem(hDlg, IDC_DEADZONE), CB_GETCURSEL, 0, 0);
	
}

// Get text from static text item
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
int GetButton(HWND hDlg, int item)
{
	wchar format[8];
	GetDlgItemText(hDlg, item, format, sizeof(format));	
	return _tstoi(format);	// wide atoi...	
}

// Set text in static text item
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
void SetButton(HWND hDlg, int item, int value)
{
	wchar format[8];
	wsprintf(format, _T("%d"), value);
	SetDlgItemText(hDlg, item, format);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Config dialog functions
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯

// OpenAbout
// ¯¯¯¯¯¯¯¯¯
void OpenAbout(HINSTANCE abouthInstance, HWND _hParent)
{
	DialogBox(abouthInstance,MAKEINTRESOURCE(IDD_ABOUT), _hParent, AboutDlg);
}

// AboutDlg
// ¯¯¯¯¯¯¯¯
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
