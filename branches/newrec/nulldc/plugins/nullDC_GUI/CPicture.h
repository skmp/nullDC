#pragma once
#include <windows.h>
#include <ole2.h>
#include <olectl.h>
//////////////////
// Picture object--encapsulates IPicture
//
class CPicture 
{
public:
	CPicture();
	~CPicture();

	// Load frm various sosurces
	BOOL Load(HINSTANCE hInst, UINT nIDRes);
	BOOL Load(LPCTSTR pszPathName);
	BOOL CPicture::Load(HBITMAP hBmp,HPALETTE hPal,bool own=false);
	BOOL Load(IStream* pstm);
	BOOL Save(const wchar_t* to);
	BOOL CPicture::Select(HDC hDC, HDC* newhDC,OLE_HANDLE *hBmp);
	// render to device context
	BOOL Render(HDC dc, RECT* rc, LPCRECT prcMFBounds=NULL) const;

	SIZE GetImageSize(HDC dc=NULL) const;

	operator IPicture*() {
		return m_spIPicture;
	}

	void GetHIMETRICSize(OLE_XSIZE_HIMETRIC& cx, OLE_YSIZE_HIMETRIC& cy) const {
		cx = cy = 0;
		const_cast<CPicture*>(this)->m_hr = m_spIPicture->get_Width(&cx);
		const_cast<CPicture*>(this)->m_hr = m_spIPicture->get_Height(&cy);
	}

	void Free() {
		if (m_spIPicture) {
			m_spIPicture->Release();
		}
		m_spIPicture=0;
	}

protected:

	void SetHIMETRICtoDP(HDC hdc, SIZE* sz) const;

	IPicture* m_spIPicture;  // ATL smart pointer to IPicture
	HBITMAP bmph;
	HRESULT m_hr;  // last error code
};
