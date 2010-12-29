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
#include <Windowsx.h>

//////////////////////////////////////////////////////////////////////////////////////////
// Config dialog functions
// -----------------------

bool GetInputSDL(HWND hDlg, int buttonid, int controller);
bool GetInputXInput(HWND hDlg, int buttonid, int controller);
bool GetInputKey(HWND hDlg, int buttonid, int controller);

void UpdateVisibleItems(HWND hDlg, int controllertype);

void GetControllerAll(HWND hDlg, int controller);
void SetControllerAll(HWND hDlg, int controller);

void GetButton(HWND hDlg, int item, wchar* Receiver);
void SetButton(HWND hDlg, int item, wchar* value);

void OpenAbout(HINSTANCE abouthInstance, HWND _hParent);

BOOL CALLBACK AboutDlg(HWND abouthWnd, UINT message, WPARAM wParam, LPARAM lParam);
