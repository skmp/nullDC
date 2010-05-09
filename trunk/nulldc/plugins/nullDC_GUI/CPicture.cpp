#include <windows.h>
#include <ole2.h>
#include <comutil.h> 
#include <Gdiplus.h>
#include <Gdiplusflat.h>
#include "CPicture.h"


#define HIMETRIC_INCH   2540    // HIMETRIC units per inch

using namespace Gdiplus;

struct GdiPlus_T
{
	HMODULE gdip;
    ULONG_PTR Token;
	
	typedef GpStatus WINGDIPAPI
	GdipSaveImageToFileFP(GpImage *image, GDIPCONST WCHAR* filename,
                    GDIPCONST CLSID* clsidEncoder, 
                    GDIPCONST EncoderParameters* encoderParams);
	typedef GpStatus WINGDIPAPI GdipCreateBitmapFromHBITMAPFP(HBITMAP hbm, HPALETTE hpal, GpBitmap** bitmap);

	typedef GpStatus WINGDIPAPI
GdipGetImageEncodersSizeFP(UINT *numEncoders, UINT *size);

typedef GpStatus WINGDIPAPI
GdipGetImageEncodersFP(UINT numEncoders,
                     UINT size,
                     ImageCodecInfo *encoders);
typedef void WINGDIPAPI
GdipFreeFP(void* ptr);

 typedef Status WINAPI GdiplusStartupFP(
    OUT ULONG_PTR *token,
    const GdiplusStartupInput *input,
    OUT GdiplusStartupOutput *output);

typedef VOID WINAPI GdiplusShutdownFP(ULONG_PTR token);

	bool Loaded() { return gdip!=0; }
	GdipSaveImageToFileFP* GdipSaveImageToFile;
	GdipCreateBitmapFromHBITMAPFP* GdipCreateBitmapFromHBITMAP;
	GdipGetImageEncodersSizeFP* GdipGetImageEncodersSize;
	GdipGetImageEncodersFP* GdipGetImageEncoders;
	GdipFreeFP* GdipFree;

	GdiplusStartupFP* GdiplusStartup;
	GdiplusShutdownFP* GdiplusShutdown;

	void GdiPlus_Init()
	{
		gdip=LoadLibrary(L"gdiplus.dll");
		
		if (Loaded())
		{
			GdipSaveImageToFile=(GdipSaveImageToFileFP*)GetProcAddress(gdip,"GdipSaveImageToFile");
			GdipCreateBitmapFromHBITMAP=(GdipCreateBitmapFromHBITMAPFP*)GetProcAddress(gdip,"GdipCreateBitmapFromHBITMAP");
			GdipGetImageEncodersSize=(GdipGetImageEncodersSizeFP*)GetProcAddress(gdip,"GdipGetImageEncodersSize");
			GdipGetImageEncoders=(GdipGetImageEncodersFP*)GetProcAddress(gdip,"GdipGetImageEncoders");
			GdipFree=(GdipFreeFP*)GetProcAddress(gdip,"GdipFree");
			GdiplusStartup=(GdiplusStartupFP*)GetProcAddress(gdip,"GdiplusStartup");
			GdiplusShutdown=(GdiplusShutdownFP*)GetProcAddress(gdip,"GdiplusShutdown");

			//DONE WITH DLL LOADING !
			//Init gdi plus now ..
			GdiplusStartupInput StartupInput;//leave it to defaults :)
			if (GdiplusStartup(&Token,&StartupInput,0)!=Ok)
			{
				FreeLibrary(gdip);
				gdip=0;
			}
		}
	}
	void GdiPlus_Term()
	{
		if (Loaded())
		{
			GdiplusShutdown(Token);
		}
	}
} gdiPlus;

void gdipInit() { gdiPlus.GdiPlus_Init(); }
void gdipTerm() { gdiPlus.GdiPlus_Term(); }
////////////////////////////////////////////////////////////////
// CPicture implementation
//
CPicture::CPicture()
{
	this->m_spIPicture=0;
}
CPicture::~CPicture()
{
	Free();
}
//////////////////
// Load from resource. Looks for "IMAGE" type.
//
BOOL CPicture::Load(HINSTANCE hInst, UINT nIDRes)
{
    // find resource in resource file
    HRSRC hRsrc = ::FindResource(hInst, MAKEINTRESOURCE(nIDRes), L"jpeg"); // type
    if ( !hRsrc )
        return FALSE;

    // load resource into memory
    DWORD len = ::SizeofResource(hInst, hRsrc);
    HGLOBAL hResData = ::LoadResource(hInst, hRsrc);
    if ( !hResData )
        return FALSE;

    HGLOBAL hGlobal = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_NODISCARD, len);
    if ( !hGlobal )
    {
        ::FreeResource(hResData);
        return FALSE;
    }

    char* pDest = reinterpret_cast<char*> ( ::GlobalLock(hGlobal) );
    char* pSrc = reinterpret_cast<char*> ( ::LockResource(hResData) );
    if (!pSrc || !pDest)
    {
        ::GlobalFree(hGlobal);
        ::FreeResource(hResData);
        return FALSE;
    }

    ::CopyMemory(pDest,pSrc,len);
    ::FreeResource(hResData);
    ::GlobalUnlock(hGlobal);


    // don't delete memory on object's release
    IStream* pStream = NULL;
    if ( ::CreateStreamOnHGlobal(hGlobal,FALSE,&pStream) != S_OK )
    {
        ::GlobalFree(hGlobal);
        return FALSE;
    }

    // create memory file and load it
    BOOL bRet = Load(pStream);
	pStream->Release();
    ::GlobalFree(hGlobal);

    return bRet;
}

//////////////////
// Load from path name.
//
BOOL CPicture::Load(LPCTSTR pszPathName)
{
    HANDLE hFile = ::CreateFile(pszPathName, 
                                FILE_READ_DATA,
                                FILE_SHARE_READ,
                                NULL, 
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL);
    if ( !hFile )
        return FALSE;

    DWORD len = ::GetFileSize( hFile, NULL); // only 32-bit of the actual file size is retained
    if (len == 0)
        return FALSE;

    HGLOBAL hGlobal = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_NODISCARD, len);
    if ( !hGlobal )
    {
        ::CloseHandle(hFile);
        return FALSE;
    }

    char* lpBuffer = reinterpret_cast<char*> ( ::GlobalLock(hGlobal) );
    DWORD dwBytesRead = 0;

    while ( ::ReadFile(hFile, lpBuffer, 4096, &dwBytesRead, NULL) )
    {
        lpBuffer += dwBytesRead;
        if (dwBytesRead == 0)
            break;
        dwBytesRead = 0;
    }

    ::CloseHandle(hFile);

	
    ::GlobalUnlock(hGlobal);


    // don't delete memory on object's release
    IStream* pStream = NULL;
    if ( ::CreateStreamOnHGlobal(hGlobal,FALSE,&pStream) != S_OK )
    {
        ::GlobalFree(hGlobal);
        return FALSE;
    }

    // create memory file and load it
    BOOL bRet = Load(pStream);
	pStream->Release();
    ::GlobalFree(hGlobal);

    return bRet;
}
BOOL CPicture::Select(HDC hDC, HDC* newhDC,OLE_HANDLE *hBmp)
{
	return SUCCEEDED(m_spIPicture->SelectPicture(hDC,newhDC,hBmp));
}
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
   UINT  num = 0;          // number of image encoders
   UINT  size = 0;         // size of the image encoder array in bytes

   ImageCodecInfo* pImageCodecInfo = NULL;

   gdiPlus.GdipGetImageEncodersSize(&num, &size);
   if(size == 0)
      return -1;  // Failure

   pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
   if(pImageCodecInfo == NULL)
      return -1;  // Failure

   gdiPlus.GdipGetImageEncoders(num, size, pImageCodecInfo);

   for(UINT j = 0; j < num; ++j)
   {
      if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
      {
         *pClsid = pImageCodecInfo[j].Clsid;
         free(pImageCodecInfo);
         return j;  // Success
      }    
   }

   free(pImageCodecInfo);
   return -1;  // Failure
}
BOOL CPicture::Save(const wchar_t* to)
{
	if (gdiPlus.Loaded())
	{
		OLE_HANDLE handle=(OLE_HANDLE)bmph;
		bool rv=false;
		if (bmph!=0  /*SUCCEEDED(m_spIPicture->get_Handle(&handle))*/) //get_Handle returns allways 1 as handle :|
		{
			GpBitmap* bmp;
			if (gdiPlus.GdipCreateBitmapFromHBITMAP((HBITMAP)handle,0,&bmp)==Ok)
			{
				//(GpImage *image, GDIPCONST WCHAR* filename,
				//      GDIPCONST CLSID* clsidEncoder, 
				//    GDIPCONST EncoderParameters* encoderParams);

				CLSID pngClsid;
				GetEncoderClsid(L"image/png", &pngClsid);
				if (gdiPlus.GdipSaveImageToFile(bmp,to,&pngClsid,0)==Ok)
					rv=true;
				gdiPlus.GdipFree(bmp);
			}
			//DeleteObject((HGDIOBJ)handle);
		}
		return rv;
	}
	else
	{
		IPictureDisp*pDisp;
		if (SUCCEEDED(m_spIPicture->QueryInterface(IID_IPictureDisp,(PVOID*) &pDisp)))
		{
			BSTR str=SysAllocString(to);
			OleSavePictureFile(pDisp, str);
			SysFreeString(str);
			pDisp->Release();
			return true;
		}
		return false;
	}
}
BOOL CPicture::Load(HBITMAP hBmp,HPALETTE hPal,bool own)
{
    Free();
    
	PICTDESC pic;
	memset(&pic,0,sizeof(pic));
	pic.cbSizeofstruct=sizeof(pic);
	pic.picType=1;
	pic.bmp.hbitmap=hBmp;
	pic.bmp.hpal=hPal;
	HRESULT hr = OleCreatePictureIndirect(&pic,IID_IDispatch,own,(void**)&m_spIPicture);
	bmph=hBmp;

    return hr == S_OK;
}
//////////////////
// Load from stream (IStream). This is the one that really does it: call
// OleLoadPicture to do the work.
//
BOOL CPicture::Load(IStream* pstm)
{
    Free();

    HRESULT hr = OleLoadPicture(pstm, 0, FALSE,
                                IID_IPicture, (void**)&m_spIPicture);

	bmph=0;
	m_spIPicture->get_Handle((OLE_HANDLE*)&bmph);
    return hr == S_OK;
}

//////////////////
// Render to device context. Covert to HIMETRIC for IPicture.
//
// prcMFBounds : NULL if dc is not a metafile dc
//
BOOL CPicture::Render(HDC dc, RECT* rc, LPCRECT prcMFBounds) const
{

    if ( !rc || (rc->right == rc->left && rc->bottom == rc->top) ) 
    {
          SIZE sz = GetImageSize(dc);
          rc->right = sz.cx;
          rc->bottom = sz.cy;
    }

    long hmWidth,hmHeight; // HIMETRIC units
    GetHIMETRICSize(hmWidth, hmHeight);

    m_spIPicture->Render(dc, 
                        rc->left, rc->top, 
                        rc->right - rc->left, rc->bottom - rc->top,
                        0, hmHeight, hmWidth, -hmHeight, prcMFBounds);

    return TRUE;
}

//////////////////
// Get image size in pixels. Converts from HIMETRIC to device coords.
//
SIZE CPicture::GetImageSize(HDC dc) const
{
    SIZE sz = {0,0};

    if (!m_spIPicture)
         return sz;
	
    LONG hmWidth, hmHeight; // HIMETRIC units
    m_spIPicture->get_Width(&hmWidth);
    m_spIPicture->get_Height(&hmHeight);

    sz.cx = hmWidth;
    sz.cy = hmHeight;

    if ( dc == NULL ) 
    {
        HDC dcscreen = ::GetWindowDC(NULL);

        SetHIMETRICtoDP(dcscreen,&sz); // convert to pixels
    } 
    else 
    {
        SetHIMETRICtoDP(dc,&sz);
    }
    return sz;
}


void CPicture::SetHIMETRICtoDP(HDC hdc, SIZE* sz) const
{
    int nMapMode;
    if ( (nMapMode = ::GetMapMode(hdc)) < MM_ISOTROPIC && nMapMode != MM_TEXT)
    {
        // when using a constrained map mode, map against physical inch
		
        ::SetMapMode(hdc,MM_HIMETRIC);
        POINT pt;
        pt.x = sz->cx;
        pt.y = sz->cy;
        ::LPtoDP(hdc,&pt,1);
        sz->cx = pt.x;
        sz->cy = pt.y;
        ::SetMapMode(hdc, nMapMode);
    }
    else
    {
        // map against logical inch for non-constrained mapping modes
        int cxPerInch, cyPerInch;
        cxPerInch = ::GetDeviceCaps(hdc,LOGPIXELSX);
        cyPerInch = ::GetDeviceCaps(hdc,LOGPIXELSY);
        sz->cx = MulDiv(sz->cx, cxPerInch, HIMETRIC_INCH);
        sz->cy = MulDiv(sz->cy, cyPerInch, HIMETRIC_INCH);
    }

    POINT pt;
    pt.x = sz->cx;
    pt.y = sz->cy;
    ::DPtoLP(hdc,&pt,1);
    sz->cx = pt.x;
    sz->cy = pt.y;

}
