#include "nullDC_GUI.h"

#include <windows.h>
#include <math.h>
#include <tchar.h>
#include <stdio.h>

#include "resource.h"


#include "CtrlDisAsmView.h"
//#include "Debugger_MemoryDlg.h"


SH4DebugInterface di;



wchar CtrlDisAsmView::szClassName[] = L"CtrlDisAsmView";

void CtrlDisAsmView::init()
{
    WNDCLASSEX wc;
    
    wc.cbSize         = sizeof(wc);
    wc.lpszClassName  = szClassName;
	wc.hInstance      = g_hInst;
    wc.lpfnWndProc    = CtrlDisAsmView::wndProc;
    wc.hCursor        = LoadCursor (NULL, IDC_ARROW);
    wc.hIcon          = 0;
    wc.lpszMenuName   = 0;
    wc.hbrBackground  = (HBRUSH)GetSysColorBrush(COLOR_WINDOW);
    wc.style          = 0;
    wc.cbClsExtra     = 0;
	wc.cbWndExtra     = sizeof( CtrlDisAsmView * );
    wc.hIconSm        = 0;
	
    RegisterClassEx(&wc);
}

void CtrlDisAsmView::deinit()
{
	//UnregisterClass(szClassName, hInst)
}

BOOL CopyTextToClipboard(HWND hwnd, TCHAR *text)
{
	OpenClipboard(hwnd);
	EmptyClipboard();
	HANDLE hglbCopy = GlobalAlloc(GMEM_MOVEABLE, (wcslen(text) + 1) * sizeof(wchar)); 
	if (hglbCopy == NULL) 
	{ 
		CloseClipboard(); 
		return FALSE; 
	} 

	// Lock the handle and copy the text to the buffer. 

	wchar *lptstrCopy = (wchar *)GlobalLock(hglbCopy); 
	wcscpy(lptstrCopy,text);
	//lptstrCopy[wcslen(text)] =  0;    // null character 
	GlobalUnlock(hglbCopy); 
	SetClipboardData(CF_UNICODETEXT,hglbCopy);
	CloseClipboard();
	return TRUE;
}

LRESULT CALLBACK CtrlDisAsmView::wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CtrlDisAsmView *ccp = CtrlDisAsmView::getFrom(hwnd);
	static bool lmbDown=false,rmbDown=false;
    switch(msg)
    {
	case WM_COMMAND:
/*		switch (wParam)
		{
		case ID_DISASM_GOTOINMEMORYVIEW:
			CMemoryDlg::Goto(ccp->selection);
			break;
		case ID_DISASM_ADDHLE:
			break;
		case ID_DISASM_TOGGLEBREAKPOINT:
			ccp->debugger->toggleBreakpoint(ccp->selection);
			ccp->redraw();
			break;
		case ID_DISASM_COPYINSTRUCTIONDISASM:
			CopyTextToClipboard(hwnd, ccp->debugger->disasm(ccp->selection));
			break;
		case ID_DISASM_COPYINSTRUCTIONHEX:
			{
				char temp[24];
				sprintf(temp,"%08x",ccp->debugger->readMemory(ccp->selection));
				CopyTextToClipboard(hwnd,temp);
			}
			break;
		case ID_DISASM_RUNTOHERE:
			{
				ccp->debugger->setBreakpoint(ccp->selection);
				ccp->debugger->runToBreakpoint();
				ccp->redraw();
			}
			break;
		}*/
		break;
		
    case WM_NCCREATE:
        // Allocate a new CustCtrl structure for this window.
        ccp = new CtrlDisAsmView(hwnd);

		ccp->setDebugger(&di);
		
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


CtrlDisAsmView *CtrlDisAsmView::getFrom(HWND hwnd)
{
    return (CtrlDisAsmView *)GetWindowPtr(hwnd, 0);
}




CtrlDisAsmView::CtrlDisAsmView(HWND _wnd)
{
	wnd=_wnd;
	SetWindowPtr(wnd, 0, this);
	SetWindowLong(wnd, GWL_STYLE, GetWindowLong(wnd,GWL_STYLE) | WS_VSCROLL);
	SetScrollRange(wnd, SB_VERT, -1,1,TRUE);
	font = CreateFont(16,0,0,0,FW_DONTCARE,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,
		L"Courier New");
	curAddress=0;
	rowHeight=16;
	align=2;
	selecting=false;
	showHex=false;
}


CtrlDisAsmView::~CtrlDisAsmView()
{
	DeleteObject(font);
}

void fillRect(HDC hdc, RECT *rect, COLORREF colour)
{
    COLORREF oldcr = SetBkColor(hdc, colour);
    ExtTextOut(hdc, 0, 0, ETO_OPAQUE, rect, L"", 0, 0);
    SetBkColor(hdc, oldcr);
}



//Yeah this truly turned into a mess with the latest additions.. but it sure looks nice ;)
void CtrlDisAsmView::onPaint(WPARAM wParam, LPARAM lParam)
{
	struct branch
	{
		int src,dst,srcAddr;
	};
	branch branches[256];
	int numBranches=0;


	GetClientRect(wnd, &rect);
	PAINTSTRUCT ps;
	HDC hdc;
	
	hdc = BeginPaint(wnd, &ps);
	// TODO: Add any drawing code here...
	int width = rect.right;
	int numRows=(rect.bottom/rowHeight)/2+1;
	//numRows=(numRows&(~1)) + 1;
	SetBkMode(hdc, TRANSPARENT);
	DWORD bgColor = 0xffffff;
	HPEN nullPen=CreatePen(0,0,bgColor);
	HPEN currentPen=CreatePen(0,0,0);
	HPEN selPen=CreatePen(0,0,0x808080);

	LOGBRUSH lbr;
	lbr.lbHatch=0; lbr.lbStyle=0; 
	lbr.lbColor=bgColor;
	HBRUSH nullBrush=CreateBrushIndirect(&lbr);
	lbr.lbColor=0xFFEfE8;
	HBRUSH currentBrush=CreateBrushIndirect(&lbr);
	lbr.lbColor=0x70FF70;
	HBRUSH pcBrush=CreateBrushIndirect(&lbr);

	HPEN oldPen=(HPEN)SelectObject(hdc,nullPen);
	HBRUSH oldBrush=(HBRUSH)SelectObject(hdc,nullBrush);

   
	HFONT oldFont = (HFONT)SelectObject(hdc,(HGDIOBJ)font);
	HICON breakPoint = (HICON)LoadIcon(hMod,(LPCWSTR)IDI_BRKPT);
	HICON breakPointDisable = (HICON)LoadIcon(hMod,(LPCWSTR)IDI_BRKPTDIS);
	int i;
	for (i=-numRows; i<=numRows; i++)
	{
		unsigned int address=curAddress + i*align;

		int rowY1 = rect.bottom/2 + rowHeight*i - rowHeight/2;
		int rowY2 = rect.bottom/2 + rowHeight*i + rowHeight/2;
		wchar temp[256];
		swprintf(temp,L"%08X",address);

		TCHAR desc[256]=L"";
		wcscpy(desc,debugger->getDescription(address));	// do this before getColor()
		lbr.lbColor=debugger->getColor(address);

		//SelectObject(hdc,currentBrush);
		SelectObject(hdc,nullPen);
		Rectangle(hdc,0,rowY1,16,rowY2);

		if (selecting && address == (u32)selection)
			SelectObject(hdc,selPen);
		else
			SelectObject(hdc,i==0 ? currentPen : nullPen);

		HBRUSH mojsBrush=CreateBrushIndirect(&lbr);
		SelectObject(hdc,mojsBrush);

		if (address == debugger->getPC())
			SelectObject(hdc,pcBrush);
		//else
		//	SelectObject(hdc,i==0 ? currentBrush : nullBrush);

		Rectangle(hdc,16,rowY1,width,rowY2);
		SelectObject(hdc,currentBrush);
		DeleteObject(mojsBrush);
		SetTextColor(hdc,0x600000);
		TextOut(hdc,17,rowY1,temp,(int)wcslen(temp));
		SetTextColor(hdc,0x000000);
		
		TCHAR *dis = debugger->disasm(address);
		TCHAR *dis2=_tcschr(dis,'\t');
		if (dis2)
		{
			*dis2=0;
			dis2++;
			wchar *mojs=wcsstr(dis2,L"0x8");
			if (mojs)
			for (int i=0; i<8; i++)
			{
				bool found=false;
				for (int j=0; j<22; j++)
				{
					if (mojs[i+2]==L"0123456789ABCDEFabcdef"[j])
						found=true;
				}
				if (!found)
				{
					mojs=0;
					break;
				}
			}
			if (mojs)
			{
				int offs;
				swscanf(mojs+2,L"%08X",&offs);
				branches[numBranches].src=rowY1 + rowHeight/2;
				branches[numBranches].srcAddr=address/align;
				branches[numBranches++].dst=(int)(rowY1+((__int64)offs-(__int64)address)*rowHeight/align + rowHeight/2);
			//	sprintf(desc,"-->%s", debugger->getDescription(offs));
				SetTextColor(hdc,0x600060);
			}
			else
				SetTextColor(hdc,0x000000);
			TextOut(hdc,198,rowY1,dis2,(int)wcslen(dis2));
		}

		SetTextColor(hdc,0x007000);
		TextOut(hdc,90,rowY1,dis,(int)wcslen(dis));

		SetTextColor(hdc,0x0000FF);
		//char temp[256];
		//UnDecorateSymbolName(desc,temp,255,UNDNAME_COMPLETE);
		if (wcslen(desc))
			TextOut(hdc,320,rowY1,desc,(int)wcslen(desc));

		if (debugger->isBreakpoint(address))
		{
			DrawIconEx(hdc,2,rowY1,breakPoint,32,32,0,0,DI_NORMAL);
		}
	}
	SelectObject(hdc,currentPen);
	for (i=0; i<numBranches; i++)
	{
		int x=250+(branches[i].srcAddr%9)*8;
		MoveToEx(hdc,x-2,branches[i].src,0);

		if (branches[i].dst<rect.bottom+200 && branches[i].dst>-200)
		{
			LineTo(hdc,x+2,branches[i].src);
			LineTo(hdc,x+2,branches[i].dst);
			LineTo(hdc,x-4,branches[i].dst);
			
			MoveToEx(hdc,x,branches[i].dst-4,0);
			LineTo(hdc,x-4,branches[i].dst);
			LineTo(hdc,x+1,branches[i].dst+5);
		}
		else
		{
			LineTo(hdc,x+4,branches[i].src);
			//MoveToEx(hdc,x+2,branches[i].dst-4,0);
			//LineTo(hdc,x+6,branches[i].dst);
			//LineTo(hdc,x+1,branches[i].dst+5);
		}
		//LineTo(hdc,x,branches[i].dst+4);

		//LineTo(hdc,x-2,branches[i].dst);
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
	
	DestroyIcon(breakPoint);
	DestroyIcon(breakPointDisable);
	
	EndPaint(wnd, &ps);
}



void CtrlDisAsmView::onVScroll(WPARAM wParam, LPARAM lParam)
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

void CtrlDisAsmView::onKeyDown(WPARAM wParam, LPARAM lParam)
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


void CtrlDisAsmView::redraw()
{
	InvalidateRect(wnd, NULL, FALSE);
	UpdateWindow(wnd); 
}


void CtrlDisAsmView::onMouseDown(WPARAM wParam, LPARAM lParam, int button)
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
	else
	{
		debugger->toggleBreakpoint(yToAddress(y));
		redraw();
	}
}

void CtrlDisAsmView::onMouseUp(WPARAM wParam, LPARAM lParam, int button)
{
	if (button==2)
	{
		//popup menu?
		POINT pt;
		GetCursorPos(&pt);
	//	TrackPopupMenuEx(GetSubMenu(g_hPopupMenus,2),TPM_RIGHTBUTTON,pt.x,pt.y,wnd,0);
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

void CtrlDisAsmView::onMouseMove(WPARAM wParam, LPARAM lParam, int button)
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


int CtrlDisAsmView::yToAddress(int y)
{
	int ydiff=y-rect.bottom/2-rowHeight/2;
	ydiff=(int)(floorf((float)ydiff / (float)rowHeight))+1;
	return curAddress + ydiff * align;
}
