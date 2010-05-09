#include "debugger_ui.h"
INT_PTR CALLBACK DebuggerDlgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg )
	{
	case WM_INITDIALOG:
		{
			
		}
		return true;

	case WM_COMMAND:

		switch( LOWORD(wParam) )
		{
		case IDOK:
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


void ShowDebugger(HWND parent)
{
	DialogBox(hinst,MAKEINTRESOURCE(IDD_DEBUGGER),parent,DebuggerDlgProc);
}