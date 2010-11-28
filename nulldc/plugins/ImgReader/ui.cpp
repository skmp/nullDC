#include <windows.h>
#include <windowsx.h>

#include "ImgReader.h"
#include "resource.h"

wchar SelectedFile[512];
wchar FileFormats[512];
int SelectedFileValid=-1;

extern HINSTANCE hInstance;
u32 fileflags;

bool GetFile_(HWND hwnd)
{
	static OPENFILENAME ofn;
	static TCHAR szFile[512]=L"";    
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize		= sizeof(OPENFILENAME);
	ofn.hwndOwner		= hwnd;
	ofn.lpstrFile		= SelectedFile;
	ofn.nMaxFile		= MAX_PATH;
	ofn.lpstrFilter		= FileFormats;
	ofn.nFilterIndex	= 1;
	ofn.nMaxFileTitle	= 128;
	ofn.lpstrFileTitle	= szFile;
	ofn.lpstrInitialDir	= NULL;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

	if(GetOpenFileName(&ofn)<=0)
	{
		DWORD err= CommDlgExtendedError();
		if (err==FNERR_INVALIDFILENAME)
		{
			SelectedFile[0]=0;
			if(GetOpenFileName(&ofn)<=0)
				return false;
			else
				return true;
		}
		return false;
	}
	return true;
}

INT_PTR CALLBACK DlgProcModal_SelectFile( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg )
	{
	case WM_INITDIALOG:
		{
			SelectedFileValid=-2;

			Edit_SetText(GetDlgItem(hWnd,IDC_IMGPATH),SelectedFile);
			if (fileflags&1)
				EnableWindow(GetDlgItem(hWnd,IDC_NODISK),false);
			if (fileflags&2)
			{
				EnableWindow(GetDlgItem(hWnd,IDCANCEL),false);
				HMENU hMenu =GetSystemMenu(hWnd, false);
				DeleteMenu(hMenu, 1, 1024);
			}
		}
		return true;

	case WM_COMMAND:

		switch( LOWORD(wParam) )
		{
		case IDOK:
			SelectedFileValid=1;
			Edit_GetText(GetDlgItem(hWnd,IDC_IMGPATH),SelectedFile,512);
			EndDialog(hWnd,0);
			return true;

		case IDCANCEL:
			if (fileflags&2)
				return true;
			SelectedFileValid=-1;
			EndDialog(hWnd,0);
			return true;

		case IDC_NODISK:
			if (fileflags&1)
				return true;
			SelectedFileValid=0;
			EndDialog(hWnd,0);
			return true;

		case IDC_BROWSE:
			ShowWindow(hWnd,SW_HIDE);
			if (GetFile_(hWnd))
			{
				Edit_SetText(GetDlgItem(hWnd,IDC_IMGPATH),SelectedFile);
				SendMessage(hWnd,WM_COMMAND,IDOK,0);
			}
			ShowWindow(hWnd,SW_SHOW);
			return true;

		default: break;
		}
		return false;

	case WM_CLOSE:
			SendMessage(hWnd,WM_COMMAND,IDCANCEL,0);
			return true;
		
	case WM_DESTROY:
		EndDialog(hWnd,0);
		return true;

	default: break;
	}

	return false;
}

int GetFile(TCHAR *szFileName, TCHAR *szParse,u32 flags)
{
	fileflags=flags;
	if (szParse==0)
		szParse=L"CD/GD Images (*.cdi;*.mds;*.nrg;*.gdi;*.chd) \0*.cdi;*.mds;*.nrg;*.gdi;*.chd\0\0";
	wcscpy(SelectedFile,szFileName);
	
	TCHAR* dst=FileFormats;
	while(szParse[0]!=0 || szParse[1]!=0)
		*dst++=*szParse++;
	*dst++=0;
	*dst++=0;

	DialogBox(hInstance,MAKEINTRESOURCE(IDD_FILESELECT),(HWND)emu.GetRenderTarget(),DlgProcModal_SelectFile);
	
	if (SelectedFileValid==1)
	{
		wcscpy(szFileName,SelectedFile);
	}
	
	return SelectedFileValid;
}



