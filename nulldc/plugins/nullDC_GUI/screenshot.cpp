//---------------------------------------------------------------------------
//
//    Screenshot - Author: Michael F?tsch; Date: May 31, 2000
//
//---------------------------------------------------------------------------

#include <windows.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "CPicture.h"

//---------------------------------------------------------------------------

// Helper function to retrieve current position of file pointer:
inline int GetFilePointer(HANDLE FileHandle)
{
	return SetFilePointer(FileHandle, 0, 0, FILE_CURRENT);
}
//---------------------------------------------------------------------------

// Helper macro to return from function when error occurs:
#define ERROR_BREAK(x) throw (int)(x);

// Screenshot
//    -> FileName: Name of file to save screenshot to
//    -> lpDDS: DirectDraw surface to capture
//    <- Result: Success
//
bool Screenshot(LPCTSTR FileName, HWND hwnd)
{
	if (!FileName || !hwnd) return false;

	bool Success=false;
	HDC SurfDC=NULL;        // GDI-compatible device context for the surface
	HBITMAP OffscrBmp=NULL; // bitmap that is converted to a DIB
	HDC OffscrDC=NULL;      // offscreen DC that we can select OffscrBmp into

	CPicture sshot;
	try
	{
		RECT windowRect;
		if (!GetWindowRect(hwnd, &windowRect))
					ERROR_BREAK(0);

		if (GetWindowLong(hwnd,GWL_STYLE)&WS_MAXIMIZE && GetWindowLong(hwnd,GWL_STYLE)&WS_BORDER)
		{
			int ax=GetSystemMetrics(SM_CXSIZEFRAME);
			int ay=GetSystemMetrics(SM_CYSIZEFRAME);
			windowRect.top+=ay;
			windowRect.bottom-=ay;
			windowRect.left+=ax;
			windowRect.right-=ax;
			printf("Maximised window fixup %d %d\n",ax,ay);
		}

		int Width = windowRect.right- windowRect.left;
		int Height = windowRect.bottom-windowRect.top;
		
		if (Width<=0 || Height<=0)
			return false;

		SurfDC=GetDC(GetDesktopWindow());
		// Create a GDI-compatible device context for the surface:
		if (SurfDC==0) ERROR_BREAK(1);

		// We need an HBITMAP to convert it to a DIB:
		if ((OffscrBmp = CreateCompatibleBitmap(SurfDC, Width, Height)) == NULL)
			ERROR_BREAK(2);

		// The bitmap is empty, so let's copy the contents of the surface to it.
		// For that we need to select it into a device context. We create one.
		if ((OffscrDC = CreateCompatibleDC(SurfDC)) == NULL) ERROR_BREAK(3);
		// Select OffscrBmp into OffscrDC:
		SelectObject(OffscrDC, OffscrBmp);
		// Now we can copy the contents of the surface to the offscreen bitmap:
		BitBlt(OffscrDC, 0, 0, Width, Height, SurfDC, windowRect.left, windowRect.top, SRCCOPY);

		// We don't need SurfDC anymore. Free it:
		//lpDDS->ReleaseDC(SurfDC); SurfDC = NULL;
		ReleaseDC(GetDesktopWindow(),SurfDC);SurfDC=NULL;

		if (!sshot.Load(OffscrBmp,NULL))
			ERROR_BREAK(3);


		if (!sshot.Save(FileName))
			ERROR_BREAK(4);
		
		Success = true;
	}
	catch (int &errorcode)
	{
		//char Buf[100];
		//wsprintf(Buf, "Screenshot error #%i", errorcode);
		//OutputDebugString(Buf);
		printf ("Screenshot error #%i", errorcode);
	}
	catch (...)
	{
		printf("Screenshot error");
		//OutputDebugString("Screenshot error");
	}

	sshot.Free();
	if (SurfDC) ReleaseDC(hwnd,SurfDC);
	if (OffscrDC) DeleteDC(OffscrDC);
	if (OffscrBmp) DeleteObject(OffscrBmp);
	
	return Success;
}
//---------------------------------------------------------------------------
