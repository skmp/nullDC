#include "nullDC_GUI.h"
#include <windows.h>
#include <math.h>
#include <stdio.h>

#include "resource.h"

//#include "dc/mem/sh4_mem.h"

//#include "Debugger_MemChecksdlg.h"
//#include "Debugger_SymbolMap.h"

//#include "DbgHelp.h"
#include "CtrlMemView.h"



wchar CtrlMemView::szClassName[] = L"CtrlMemView";


void CtrlMemView::init()
{
    WNDCLASSEX wc;
    wc.cbSize         = sizeof(wc);
    wc.lpszClassName  = szClassName;
    wc.hInstance      = g_hInst;
    wc.lpfnWndProc    = CtrlMemView::wndProc;
    wc.hCursor        = LoadCursor (NULL, IDC_ARROW);
    wc.hIcon          = 0;
    wc.lpszMenuName   = 0;
    wc.hbrBackground  = (HBRUSH)GetSysColorBrush(COLOR_WINDOW);
    wc.style          = 0;
    wc.cbClsExtra     = 0;
	wc.cbWndExtra     = sizeof( CtrlMemView * );
    wc.hIconSm        = 0;
	
    RegisterClassEx(&wc);
}

void CtrlMemView::deinit()
{
	//UnregisterClass(szClassName, hInst)
}

extern BOOL CopyTextToClipboard(HWND hwnd, TCHAR *text);

LRESULT CALLBACK CtrlMemView::wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CtrlMemView *ccp = CtrlMemView::getFrom(hwnd);
	static bool lmbDown=false,rmbDown=false;
    switch(msg)
    {
	case WM_COMMAND:
	/*	switch(LOWORD(wParam))
		{
		case ID_MEMVIEW_DUMP:
			MessageBox(hwnd,"This feature has not been implemented.","Sorry",0);
			break;
		case ID_MEMVIEW_GOTOINDISASM:
			CDisasm::Goto(ccp->selection);
			break;

		case ID_MEMVIEW_ADDMEMORYBREAKPOINT:
			{
#ifdef LOGGING
				CMemChecksDlg::Show(true);
				TMemCheck mc;
				_u32 addr = ccp->getSelection();
				int fun = Debugger_GetSymbolNum(addr);
				int st,end;
				if (fun!=-1)
				{
					st = Debugger_GetSymbolAddr(fun);
					end = st + Debugger_GetSymbolSize(fun)-4;
					if (end<st) end=st; //for smaller than 4 bytes symbols
				}
				else
				{
					st=addr;
					end=addr;
				}
				mc.iStartAddress = st;
				mc.iEndAddress = end;
				mc.bRange=(end-st)>4;
				mc.bBreak=true;
				mc.bLog=true;
				mc.bOnRead=true;
				mc.bOnWrite=true;
				CMemChecksDlg::AddNewCheck(mc);
#else
				MessageBox(hwnd,"This build does not support this feature.","Speed build restrictions",0);
#endif
			}
			break;
		case ID_MEMVIEW_COPYVALUE:
			{
				char temp[24];
				sprintf(temp,"%08x",CMemory::ReadUnchecked_U32(ccp->selection));
				CopyTextToClipboard(hwnd,temp);
			}
			break;
		}*/
		break;
    case WM_NCCREATE:
        // Allocate a new CustCtrl structure for this window.
        ccp = new CtrlMemView(hwnd);

		ccp->setDebugger(&di);
		ccp->setMode(MV_NORMAL);
		
        // Continue with window creation.
        return ccp != NULL;
		
		// Clean up when the window is destroyed.
    case WM_NCDESTROY:
        delete ccp;
        break;
	case WM_SETFONT:
		break;
	case WM_SIZE:
		ccp->redraw();
		break;
	case WM_PAINT:
		ccp->onPaint(wParam,lParam);
		break;	
	case WM_VSCROLL:
		ccp->onVScroll(wParam,lParam);
		break;
	case WM_ERASEBKGND:
		return FALSE;
	case WM_KEYDOWN:
		ccp->onKeyDown(wParam,lParam);
		break;
	case WM_LBUTTONDOWN: SetFocus(hwnd); lmbDown=true; ccp->onMouseDown(wParam,lParam,1); break;
	case WM_RBUTTONDOWN: rmbDown=true; ccp->onMouseDown(wParam,lParam,2); break;
	case WM_MOUSEMOVE:   ccp->onMouseMove(wParam,lParam,(lmbDown?1:0) | (rmbDown?2:0)); break;
	case WM_LBUTTONUP:   lmbDown=false; ccp->onMouseUp(wParam,lParam,1); break;
	case WM_RBUTTONUP:   rmbDown=false; ccp->onMouseUp(wParam,lParam,2); break;
	case WM_SETFOCUS:
		SetFocus(hwnd);
		ccp->hasFocus=true;
		ccp->redraw();
		break;
	case WM_KILLFOCUS:
		ccp->hasFocus=false;
		ccp->redraw();
		break;
    default:
        break;
    }
	
    return DefWindowProc(hwnd, msg, wParam, lParam);
}


CtrlMemView *CtrlMemView::getFrom(HWND hwnd)
{
    return (CtrlMemView *)GetWindowPtr(hwnd, 0);
}




CtrlMemView::CtrlMemView(HWND _wnd)
{
	wnd=_wnd;
	SetWindowPtr(wnd, 0, this);
	SetWindowLong(wnd, GWL_STYLE, GetWindowLong(wnd,GWL_STYLE) | WS_VSCROLL);
	SetScrollRange(wnd, SB_VERT, -1,1,TRUE);
	font = CreateFont(16,0,0,0,FW_DONTCARE,FALSE,FALSE,FALSE,DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH, L"Courier New");
	curAddress=0;
	rowHeight=16;
	align=4;
	selecting=false;
	mode=MV_SYMBOLS;
}


CtrlMemView::~CtrlMemView()
{
	DeleteObject(font);
}



//Yeah this truly turned into a mess with the latest additions.. but it sure looks nice ;)
void CtrlMemView::onPaint(WPARAM wParam, LPARAM lParam)
{
	GetClientRect(wnd, &rect);
	PAINTSTRUCT ps;
	HDC hdc;
	hdc = BeginPaint(wnd, &ps);
	int width = rect.right;
	int numRows=(rect.bottom/rowHeight)/2+1;
	SetBkMode(hdc, TRANSPARENT);
	HPEN nullPen=CreatePen(0,0,0xFFFFFF);
	HPEN currentPen=CreatePen(0,0,0);
	HPEN selPen=CreatePen(0,0,0x808080);
	LOGBRUSH lbr;
	lbr.lbHatch=0; lbr.lbStyle=0; 
	lbr.lbColor=0xFFFFFF;
	HBRUSH nullBrush=CreateBrushIndirect(&lbr);
	lbr.lbColor=0xFFEfE8;
	HBRUSH currentBrush=CreateBrushIndirect(&lbr);
	lbr.lbColor=0x70FF70;
	HBRUSH pcBrush=CreateBrushIndirect(&lbr);
	HPEN oldPen=(HPEN)SelectObject(hdc,nullPen);
	HBRUSH oldBrush=(HBRUSH)SelectObject(hdc,nullBrush);
   	HFONT oldFont = (HFONT)SelectObject(hdc,(HGDIOBJ)font);

	int i;
	for (i=-numRows; i<=numRows; i++)
	{
		unsigned int address=curAddress + i*align;

		int rowY1 = rect.bottom/2 + rowHeight*i - rowHeight/2;
		int rowY2 = rect.bottom/2 + rowHeight*i + rowHeight/2;

		wchar temp[256];
		swprintf(temp,L"%08x",address);

		//SelectObject(hdc,currentBrush);
		Rectangle(hdc,0,rowY1,16,rowY2);

		if(selecting && address == (u32)selection)
			SelectObject(hdc,selPen);
		else
			SelectObject(hdc,i==0 ? currentPen : nullPen);

		Rectangle(hdc,16,rowY1,width,rowY2);
		SelectObject(hdc,nullBrush);
		SetTextColor(hdc,0x600000);
		TextOut(hdc,17,rowY1,temp,(int)wcslen(temp));
		SetTextColor(hdc,0x000000);
		
		switch(mode)
		{
		case MV_NORMAL:
			swprintf(temp, L"%08X  %08X  %08X  %08X",
				ReadMem32((address+0)), ReadMem32((address+4)), ReadMem32((address+8)), ReadMem32((address+12)));
			TextOut(hdc,90,rowY1,temp,(int)wcslen(temp));

			SetTextColor(hdc,0x0033BB22);
			((u32*)temp)[0] = ReadMem32(address);
			((u32*)temp)[1] = ReadMem32(address+4);
			((u32*)temp)[2] = ReadMem32(address+8);
			((u32*)temp)[3] = ReadMem32(address+12);
			((u32*)temp)[4] = 0x00000000;
			TextOut(hdc,420,rowY1,temp,(int)wcslen(temp));
		break;

		case MV_SYMBOLS:
			SetTextColor(hdc,0x0000FF);
			//int fn = address&3;//Debugger_GetSymbolNum(address);
			swprintf(temp,L"MV_SYMBOLS !!!");
	//		sprintf(temp, "%s (0x%x b)", Debugger_GetSymbolName(fn),Debugger_GetSymbolSize(fn));
			TextOut(hdc,200,rowY1,temp,(int)wcslen(temp));

			SetTextColor(hdc,0x0000000);
			UINT value = 0xBADC0D3;//ReadMem(address,4);//CMemory::ReadUnchecked_U32(address);
			swprintf(temp, L"%08x", value );
		//	sprintf(temp, "%08x [%s]", value, Debugger_GetSymbolName(Debugger_GetSymbolNum(value)));
			TextOut(hdc,70,rowY1,temp,(int)wcslen(temp));
		break;
		}
	}

	SelectObject(hdc,oldFont);
	SelectObject(hdc,oldPen);
	SelectObject(hdc,oldBrush);
	
	DeleteObject(nullPen);
	DeleteObject(currentPen);
	DeleteObject(selPen);

	DeleteObject(nullBrush);
	DeleteObject(pcBrush);
	DeleteObject(currentBrush);
	
	EndPaint(wnd, &ps);
}



void CtrlMemView::onVScroll(WPARAM wParam, LPARAM lParam)
{
	RECT rect;
	GetClientRect(this->wnd, &rect);
	int page=(rect.bottom/rowHeight)/2-1;

	switch (wParam & 0xFFFF)
	{
	case SB_LINEDOWN:
		curAddress+=align;
		break;
	case SB_LINEUP:
		curAddress-=align;
		break;
	case SB_PAGEDOWN:
		curAddress+=page*align;
		break;
	case SB_PAGEUP:
		curAddress-=page*align;
		break;
	default:
		return;
	}
	redraw();
}

void CtrlMemView::onKeyDown(WPARAM wParam, LPARAM lParam)
{
	RECT rect;
	GetClientRect(this->wnd, &rect);
	int page=(rect.bottom/rowHeight)/2-1;

	switch (wParam & 0xFFFF)
	{
	case VK_DOWN:
		curAddress+=align;
		break;
	case VK_UP:
		curAddress-=align;
		break;
	case VK_NEXT:
		curAddress+=page*align;
		break;
	case VK_PRIOR:
		curAddress-=page*align;
		break;
	default:
		return;
	}
	redraw();
}


void CtrlMemView::redraw()
{
	InvalidateRect(wnd, NULL, FALSE);
	UpdateWindow(wnd); 
}


void CtrlMemView::onMouseDown(WPARAM wParam, LPARAM lParam, int button)
{	
	int x = LOWORD(lParam); 
	int y = HIWORD(lParam); 
	if (x>16)
	{
		oldSelection=selection;
		selection=yToAddress(y);
		SetCapture(wnd);
		bool oldselecting=selecting;
		selecting=true;
		if (!oldselecting || (selection!=oldSelection))
			redraw();
	}
}

void CtrlMemView::onMouseUp(WPARAM wParam, LPARAM lParam, int button)
{
	if (button==2)
	{
		//popup menu?
		POINT pt;
		GetCursorPos(&pt);
	//	TrackPopupMenuEx(GetSubMenu(g_hPopupMenus,0),TPM_RIGHTBUTTON,pt.x,pt.y,wnd,0);
		return;
	}
	int x = LOWORD(lParam); 
	int y = HIWORD(lParam); 
	if (x>16)
	{
		curAddress=yToAddress(y);
		selecting=false;
		ReleaseCapture();
		redraw();
	}
}

void CtrlMemView::onMouseMove(WPARAM wParam, LPARAM lParam, int button)
{
	if (button&1)
	{
		int x = LOWORD(lParam); 
		int y = (signed short)HIWORD(lParam); 
		if (x>16)
		{
			if (y<0)
			{
				curAddress-=align;
				redraw();
			}
			else if (y>rect.bottom)
			{
				curAddress+=align;
				redraw();
			}
			else
				onMouseDown(wParam,lParam,1);
		}
	}
}	


int CtrlMemView::yToAddress(int y)
{
	int ydiff=y-rect.bottom/2-rowHeight/2;
	ydiff=(int)(floorf((float)ydiff / (float)rowHeight))+1;
	return curAddress + ydiff * align;
}
