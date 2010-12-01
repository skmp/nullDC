#include "config_ui.h"
#include <CommCtrl.h>
void UpdateMenuSelections();
u32 config_scmi=0,config_stami=0,config_stami2=0;
void Cofnig_UpdateMenuSelections()
{
	eminf.SetMenuItemStyle(config_stami,settings.LimitFPS==1?MIS_Checked:0,MIS_Checked);
	eminf.SetMenuItemStyle(config_stami2,settings.LimitFPS==2?MIS_Checked:0,MIS_Checked);
}
INT_PTR CALLBACK ConfigDlgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{		
	switch( uMsg )
	{
	case WM_INITDIALOG:
		{
			CheckDlgButton(hWnd,IDC_HWMIX,settings.HW_mixing!=0?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hWnd,IDC_GFOCUS,settings.GlobalFocus!=0?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hWnd,IDC_FSYNC,settings.LimitFPS!=0?BST_CHECKED:BST_UNCHECKED);

			bool hideo=true;
			if (settings.BufferSize==884)
			{
				CheckDlgButton(hWnd,IDC_RADIO1,BST_CHECKED);
			}
			else if (settings.BufferSize==1024)
			{
				CheckDlgButton(hWnd,IDC_RADIO2,BST_CHECKED);
			}
			else if (settings.BufferSize==1536)
			{
				CheckDlgButton(hWnd,IDC_RADIO3,BST_CHECKED);
			}
			else if (settings.BufferSize==1768)
			{
				CheckDlgButton(hWnd,IDC_RADIO4,BST_CHECKED);
			}
			else if (settings.BufferSize==2048)
			{
				CheckDlgButton(hWnd,IDC_RADIO5,BST_CHECKED);
			}
			else if (settings.BufferSize==4096)
			{
				CheckDlgButton(hWnd,IDC_RADIO6,BST_CHECKED);
			}
			else
			{
				CheckDlgButton(hWnd,IDC_RADIO7,BST_CHECKED);
				char temp[512];
				sprintf(temp,"%d",settings.BufferSize);
				SetDlgItemTextA(hWnd,IDC_RADIO7,temp);
				hideo=false;
			}
			if (hideo)
				ShowWindow(GetDlgItem(hWnd,IDC_RADIO7), SW_HIDE);			
			
			HWND hSlider = GetDlgItem(hWnd,IDC_SLIDER1);

			SendMessage(hSlider, TBM_SETRANGE, 0, MAKELONG(0, 100));				
		    SendMessage(hSlider, TBM_SETTICFREQ, 25, 0);
			SendMessage(hSlider, TBM_SETPOS, 1, 100 - settings.Volume);			
		}
		return true;

	case WM_COMMAND:

		switch( LOWORD(wParam) )
		{
		case IDOK:
			{
				if (IsDlgButtonChecked(hWnd,IDC_RADIO1))
				{
					settings.BufferSize=884;
				}
				else if (IsDlgButtonChecked(hWnd,IDC_RADIO2))
				{
					settings.BufferSize=1024;
				}
				else if (IsDlgButtonChecked(hWnd,IDC_RADIO3))
				{
					settings.BufferSize=1536;
				}
				else if (IsDlgButtonChecked(hWnd,IDC_RADIO4))
				{
					settings.BufferSize=1768;
				}
				else if (IsDlgButtonChecked(hWnd,IDC_RADIO5))
				{
					settings.BufferSize=2048;
				}
				else if (IsDlgButtonChecked(hWnd,IDC_RADIO6))
				{
					settings.BufferSize=4096;
				}

				settings.HW_mixing=IsDlgButtonChecked(hWnd,IDC_HWMIX);
				settings.GlobalFocus=IsDlgButtonChecked(hWnd,IDC_GFOCUS);
				settings.LimitFPS=IsDlgButtonChecked(hWnd,IDC_FSYNC);

				HWND hSlider = GetDlgItem(hWnd,IDC_SLIDER1);
				settings.Volume = 100 - SendMessage(hSlider,TBM_GETPOS,0,0);
				
				SaveSettings();
			}
		case IDCANCEL:
			EndDialog(hWnd,0);
			return true;

		default: break;
		}
		return false;

	case WM_CLOSE:
	case WM_DESTROY:
		EndDialog(hWnd,0);
		return true;

	default: break;
	}

	return false;
}



void ShowConfig(HWND parent)
{
	DialogBox(hinst,MAKEINTRESOURCE(IDD_SETTINGS),parent,ConfigDlgProc);
}