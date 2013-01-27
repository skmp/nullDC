/*
**	emuWinUI.cpp
*/

#include "nullDC_GUI.h"
#include "CPicture.h"
#include "hwbrk.h"
#include <math.h>

/////////////////////////////
#include "CtrlMemView.h"
#include "CtrlDisAsmView.h"
#include "resource.h"
//////////////////////////////////

#include "screenshot.h"

u32 PowerVR_menu;
u32 GDRom_menu;
u32 Aica_menu;
u32 Arm_menu;
u32 Maple_menu;
u32 Maple_menu_ports[4][6];
u32 ExtDev_menu;
u32 Debug_menu;

/// i dont like it but ....
CtrlMemView *cMemView;
CtrlDisAsmView *cDisView;
/////////////////////////

HWND g_hWnd;
HINSTANCE g_hInst;

TCHAR g_szFileName[MAX_PATH];

INT_PTR CALLBACK DlgProc( HWND, UINT, WPARAM, LPARAM );
INT_PTR CALLBACK ArmDlgProc( HWND, UINT, WPARAM, LPARAM );
INT_PTR CALLBACK DlgProcModal_about( HWND, UINT, WPARAM, LPARAM );
INT_PTR CALLBACK DlgProcModal_config( HWND, UINT, WPARAM, LPARAM );
LRESULT CALLBACK WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
INT_PTR CALLBACK PluginDlgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
INT_PTR CALLBACK ProfilerProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

HWND ProfilerWindow;
#pragma warning(disable: 4311)
#pragma warning(disable: 4312)

void* GetWindowPtr(HWND hWnd,int nIndex)
{
#ifdef X64
	return (void*)GetWindowLongPtr(hWnd,nIndex);
#else
	return (void*)GetWindowLongPtr(hWnd,nIndex);
#endif
}

void SetWindowPtr( HWND hWnd,int nIndex,void* dwNewLong)
{
#ifdef X64
	SetWindowLongPtr(hWnd,nIndex,(LONG_PTR)dwNewLong);
#else
	SetWindowLongPtr(hWnd,nIndex,(LONG)dwNewLong);
#endif
}

#pragma warning(default: 4311)
#pragma warning(default: 4312)


 int power(int base, int n)
 {
    int i, p;
 
    if (n == 0)
       return 1;
 
    p = 1;
    for (i = 1; i <= n; ++i)
       p = p * base;
    return p;
 }

 int htoi(wchar s[]) {
 
	size_t len;
	int  value = 1, digit = 0,  total = 0;
    int c, x, y, i = 0;
    wchar hexchars[] = L"abcdef"; /* Helper string to find hex digit values */
 
    /* Test for 0s, '0x', or '0X' at the beginning and move on */
 
    if (s[i] == '0')
    {
       i++;
       if (s[i] == 'x' || s[i] == 'X')
       {
          i++;
       }
    }
 
    len = wcslen(s);
 
    for (x = i; x < (int)len; x++)
    {
       c = tolower(s[x]);
       if (c >= '0' && c <= '9')
       {
          digit = c - '0';
       } 
       else if (c >= 'a' && c <= 'f')
       {
          for (y = 0; hexchars[y] != '\0'; y++)
          {
             if (c == hexchars[y])
             {
                digit = y + 10;
             }
          }
       } else {
          return 0; /* Return 0 if we get something unexpected */
       }
       value = power(16, (int)(len-x-1));
       total += value * digit;
    }
    return total;
 }
 
void InitMenu();
HMENU GetHMenu();
void ToggleFullscreen(HWND hWnd);
bool uiInit()
{
	g_hInst =(HINSTANCE)hMod;
	WNDCLASS wc;
	memset(&wc,0,sizeof(wc));
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hbrBackground	= (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
	wc.hIcon			= LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_NDC_ICON));
	wc.hInstance		= g_hInst;
	wc.lpfnWndProc		= WndProc;
	wc.lpszClassName	= L"ndc_main_window";
	wc.lpszMenuName		= 0;
	wc.style			= CS_VREDRAW | CS_HREDRAW ;

	if( !RegisterClass(&wc) ) {
		MessageBox( NULL, L"Couldn't Register ndc_main_window Class !",L"ERROR",MB_ICONERROR );
		return false;
	}

	InitCommonControls();

	InitMenu();

	g_hWnd = CreateWindow( L"ndc_main_window", emu_name, WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT, 640,480, NULL, NULL, g_hInst, NULL );
	

	if( !IsWindow(g_hWnd) ) {
		MessageBox( NULL, L"Couldn't Create nullDC Window!",L"ERROR",MB_ICONERROR );
		return false;
	}
	
	SetMenu(g_hWnd,GetHMenu());
	
	RECT r= { 0,0,640,480};
	AdjustWindowRectEx(&r,GetWindowLong(g_hWnd,GWL_STYLE),GetMenu(g_hWnd)!=NULL,GetWindowLong(g_hWnd,GWL_EXSTYLE));

	if(settings.AlwaysOnTop)
		SetWindowPos( g_hWnd, HWND_TOPMOST, 0, 0, r.right - r.left, r.bottom - r.top,SWP_NOMOVE | SWP_SHOWWINDOW);
	else
		SetWindowPos( g_hWnd, NULL, 0, 0, r.right - r.left, r.bottom - r.top,SWP_NOZORDER | SWP_NOMOVE) ;

	ShowWindow(g_hWnd,emu.nCmdShow);

	if(settings.AlwaysOnTop)
		SetFocus(g_hWnd);

	if (settings.Fullscreen)
	{
		ToggleFullscreen(g_hWnd);
	}

	MSG msg;
	while( PeekMessage( &msg, NULL, 0, 0 ,TRUE) != 0)
	{
		TranslateMessage(&msg); 
		DispatchMessage(&msg); 
	}
	
	return true;
}

void uiTerm()
{
	DestroyWindow(g_hWnd);
}

void uiMain() {
	static MSG msg;

	LoadAccelerators(g_hInst,NULL);

	while (1) {
		if (PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
			TranslateMessage(&msg); 
			DispatchMessage(&msg); 
			if (msg.message == WM_QUIT) {
				return;
			}
		}
	}
}


void uiGetFN(TCHAR *szFileName, TCHAR *szParse)
{
	static OPENFILENAME ofn;
	static TCHAR szFile[MAX_PATH];    
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize		= sizeof(OPENFILENAME);
	ofn.hwndOwner		= g_hWnd;
	ofn.lpstrFile		= szFileName;
	ofn.nMaxFile		= MAX_PATH;
	ofn.lpstrFilter		= szParse;
	ofn.nFilterIndex	= 1;
	ofn.nMaxFileTitle	= 128;
	ofn.lpstrFileTitle	= szFile;
	ofn.lpstrInitialDir	= NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if(GetOpenFileName(&ofn)<=0)
		printf("GetISOfname() Failed !\n");
}


#define WM_GOTOPC WM_USER+0xC0D3

///////////////////
HWND hDebugger;


//Dynamic menu code :)
struct _MenuItem;
struct MenuStrip
{
	_MenuItem* owner;
	HMENU hmenu;
	vector<u32> items;

	bool Empty()  { return items.size()==0; }
	void AddItem(u32 id,u32 pos);
	void RemoveItem(u32 id);

	void Delete();
	MenuStrip(_MenuItem* p=0)
	{
		owner=p;
		hmenu=0;
	}
	~MenuStrip()
	{
		Delete();
	}
};
struct _MenuItem
{
	MenuStrip* owner;
	MenuStrip submenu;
	u32 gmid;	//menu item id
	u32 nid;	//notification id
	wchar* txt;
	void* puser;
	MenuItemSelectedFP* handler;
	u32 Style;
	void* hbitmap;

	_MenuItem(wchar* text,u32 id,u32 gid)
	{
		owner=0;
		submenu.owner=this;
		gmid=gid;
		nid=id;
		txt=text;
		puser=0;
		handler=0;
		Style=0;
		hbitmap=0;
	}

	void Insert(MenuStrip* menu,u32 pos);
	void Remove(HMENU menu);

	void Update();
	~_MenuItem();
	void Create(wchar* text);

	void AddChild(u32 id);
	void RemoveChild(u32 id);
	void Clicked(void* hWnd);
};
vector<_MenuItem*> MenuItems;
MenuStrip MainMenu;

void MenuStrip::AddItem(u32 id,u32 pos)
{
	if (hmenu==0)
	{
		if (owner!=0)
			hmenu=CreatePopupMenu();
		else
			hmenu=CreateMenu();
	}
	items.push_back(id);

	MenuItems[id]->Insert(this,pos);
	if (owner)
		owner->AddChild(id);
	DrawMenuBar(g_hWnd);
}
void MenuStrip::RemoveItem(u32 id)
{
	verify(hmenu!=0);
	MenuItems[id]->Remove(hmenu);

	for (size_t i=0;i<items.size();i++)
	{
		if (items[i]==id)
		{
			items.erase(items.begin()+i);
			break;
		}
	}
	if (owner)
		owner->RemoveChild(id);
	DrawMenuBar(g_hWnd);
}

void MenuStrip::Delete()
{
	while(items.size())
	{
		RemoveItem(items[0]);
	}
	if (hmenu)
		DestroyMenu(hmenu);

	hmenu=0;
}


void _MenuItem::Clicked(void* hWnd)
{
	if (handler)
		handler(gmid,hWnd,puser);
}
void _MenuItem::Insert(MenuStrip* menu,u32 pos)
{
	MENUITEMINFO mif;
	memset(&mif,0,sizeof(mif));
	mif.cbSize=sizeof(mif);
	mif.fMask=MIIM_ID|MIIM_STRING|MIIM_STATE;
	mif.dwTypeData=txt;
	mif.wID=nid;
	if (handler==0)
		mif.fState=MFS_GRAYED;

	owner=menu;
	InsertMenuItem(owner->hmenu,pos,TRUE,&mif);
}
void _MenuItem::Remove(HMENU menu)
{
	DeleteMenu(menu,nid,MF_BYCOMMAND);
	owner=0;
}

void _MenuItem::Update()
{
	MENUITEMINFO mif;
	memset(&mif,0,sizeof(mif));
	mif.cbSize=sizeof(mif);
	mif.fMask=MIIM_SUBMENU | MIIM_STATE | MIIM_FTYPE | MIIM_BITMAP | MIIM_STRING;
	mif.hSubMenu=submenu.hmenu;

	mif.dwTypeData=txt;

	mif.fType = MFT_STRING;

	if (Style & MIS_Bitmap)
	{
		mif.hbmpItem=(HBITMAP)hbitmap;
	}
	if (Style & MIS_Radiocheck)
	{
		mif.fType|=MFT_RADIOCHECK;
	}
	
	if (Style & MIS_Seperator)
	{
		mif.fType|=MFT_SEPARATOR;
	}
	
	if (Style&MIS_Checked)
	{
		mif.fState|=MFS_CHECKED;
	}
	if (Style&MIS_Grayed)
	{
		mif.fState|=MFS_GRAYED;
	}
	
	if (submenu.Empty() && handler==0)
	{
		mif.fState|=MFS_GRAYED;
	}

	SetMenuItemInfo(owner->hmenu,nid,FALSE,&mif);
}
_MenuItem::~_MenuItem()
{
	if (txt)
		free(txt);
	if (owner)
	owner->RemoveItem(gmid);
	MenuItems[gmid]=0;
}
void _MenuItem::Create(wchar* text)
{
	//smth
}
void _MenuItem::AddChild(u32 id)
{
	//submenu.AddItem(id,pos);

	//update item info
	Update();
}
void _MenuItem::RemoveChild(u32 id)
{
	//submenu.RemoveItem(id);
	if (submenu.Empty())
		submenu.Delete();

	//update item info
	Update();

	//delete MenuItems[id];
}
u32 CreateMenuItem(const wchar* text,MenuItemSelectedFP* handler , void* puser)
{
	u32 gmid = MenuItems.size();
	_MenuItem* t=new _MenuItem(_tcsdup(text),gmid+10,gmid);
	t->puser=puser;
	t->handler=handler;
	MenuItems.push_back(t);
	return gmid;
}
u32 EXPORT_CALL AddMenuItem(u32 parent,s32 pos,const wchar* text,MenuItemSelectedFP* handler ,u32 checked)
{
	u32 rv= CreateMenuItem(text,handler,0);
	
	if (parent==0)
		MainMenu.AddItem(rv,pos);
	else
	{
		MenuItems[parent]->submenu.AddItem(rv,(u32)pos);
	}

	SetMenuItemStyle(rv,checked?(u32)MIS_Checked:0,(u32)MIS_Checked);
	
	return rv;
}
void EXPORT_CALL SetMenuItemStyle(u32 id,u32 style,u32 mask)
{
	MenuItems[id]->Style= (MenuItems[id]->Style & (~mask))|style;
	MenuItems[id]->Update();
}
void EXPORT_CALL GetMenuItem(u32 id,MenuItem* info,u32 mask)
{
	if (mask & MIM_Bitmap)
		info->Bitmap=MenuItems[id]->hbitmap;

	if (mask & MIM_Handler)
		info->Handler=MenuItems[id]->handler;

	if (mask & MIM_PUser)
		info->PUser=MenuItems[id]->puser;

	if (mask & MIM_Style)
		info->Style=MenuItems[id]->Style;

	if (mask & MIM_Text)
		info->Text=MenuItems[id]->txt;
}
void EXPORT_CALL SetMenuItem(u32 id,MenuItem* info,u32 mask)
{
	if (mask & MIM_Bitmap)
		MenuItems[id]->hbitmap=info->Bitmap;

	if (mask & MIM_Handler)
		MenuItems[id]->handler=info->Handler;

	if (mask & MIM_PUser)
		MenuItems[id]->puser=info->PUser;

	if (mask & MIM_Style)
		MenuItems[id]->Style=info->Style;

	if (mask & MIM_Text)
	{
		if (MenuItems[id]->txt)
			free(MenuItems[id]->txt);
		MenuItems[id]->txt=_tcsdup(info->Text);
	}

	MenuItems[id]->Update();
}

void EXPORT_CALL DeleteAllMenuItemChilds(u32 id)
{
	MenuItems[id]->submenu.Delete();
}
void EXPORT_CALL SetMenuItemHandler(u32 id,MenuItemSelectedFP* h)
{
	MenuItems[id]->handler=h;
	MenuItems[id]->Update();
}
void EXPORT_CALL SetMenuItemBitmap(u32 id,void* hbmp)
{
	MenuItems[id]->hbitmap=hbmp;
	MenuItems[id]->Update();
}
u32 EXPORT_CALL GetMenuItemStyle(u32 id)
{
	return MenuItems[id]->Style;
}
void* EXPORT_CALL GetMenuItemBitmap(u32 id)
{
	return MenuItems[id]->hbitmap;
}

void EXPORT_CALL DeleteMenuItem(u32 id)
{
	if (id==0)
		return;

	delete MenuItems[id];
}


//Wow , that was quite big :p

//Nice helper :)
void AddSeperator(u32 menu)
{
	AddMenuItem(menu,-1,0,0,0);
}
#define MENU_HANDLER(name) void EXPORT_CALL name (u32 id,void* hWnd,void* stuff)
///////Menu Handlers\\\\\\\

MENU_HANDLER( HandleMenu1 )
{
	DeleteMenuItem(id);
}
MENU_HANDLER( HandleMenu0 )
{
	msgboxf(L"Menu %d -- not implemented",MBX_ICONEXCLAMATION,id);
}

//File 
MENU_HANDLER(Handle_File_OpenBin)
{
	if (!EmuStarted())	//if cpu is stoped
	{
		OPENFILENAME ofn;
		TCHAR szFile[128];

		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize		= sizeof(OPENFILENAME);
		ofn.hwndOwner		= (HWND)hWnd;
		ofn.lpstrFile		= g_szFileName;
		ofn.nMaxFile		= MAX_PATH;
		ofn.lpstrFilter		= L"All(.BIN\\.ELF)\0*.BIN;*.ELF\0Binary\0*.BIN\0Elf\0*.ELF\0All\0*.*\0";
		ofn.nFilterIndex	= 1;
		ofn.nMaxFileTitle	= 128;
		ofn.lpstrFileTitle	= szFile;
		ofn.lpstrInitialDir	= NULL;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		if(GetOpenFileName(&ofn)>0)
		{
			if (EmuInit())
			{
				EmuReset(false);
				if(!EmuLoadBinary(g_szFileName,0x10000))
					return;
				EmuSetPatch(patch_resets_Misc,patch_resets_Misc);//mwhaha
				
				Sh4SetRegister(reg_sr,0x70000000);
				Sh4SetRegister(reg_gbr,0x8c000000);
				Sh4SetRegister(reg_pc,0x8c008300);
				EmuStart();
			}
		}
	}
	//add warn message
}
MENU_HANDLER(Handle_File_LoadBin)
{
	bool resume=false;
	if (EmuStarted())
	{
		EmuStop();
		resume=true;
	}

	OPENFILENAME ofn;
	TCHAR szFile[128];

	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize		= sizeof(OPENFILENAME);
	ofn.hwndOwner		= (HWND)hWnd;
	ofn.lpstrFile		= g_szFileName;
	ofn.nMaxFile		= MAX_PATH;
	ofn.lpstrFilter		= L"All(.BIN\\.ELF)\0*.BIN;*.ELF\0Binary\0*.BIN\0Elf\0*.ELF\0All\0*.*\0";
	ofn.nFilterIndex	= 1;
	ofn.nMaxFileTitle	= 128;
	ofn.lpstrFileTitle	= szFile;
	ofn.lpstrInitialDir	= NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

	if(GetOpenFileName(&ofn)>0)
	{
		if(!EmuLoadBinary(g_szFileName,0x10000))
			return;
	}
	
	if (resume)
		EmuStart();
}
MENU_HANDLER(Handle_File_BootHLE)
{
	if (EmuInit())
	{
		EmuReset(false); //do a hard reset

		if (EmuBootHLE()==false) //try to load binary using hle boot
		{
			MessageBox((HWND)hWnd,L"Failed to find ip.bin/bootfile\nTry to boot using the Normal boot method.",L"HLE Boot Error",MB_ICONEXCLAMATION | MB_OK);
			return;
		}

		//that needs to be disabled for many games to run
		EmuSetPatch(0,patch_resets_Misc);//mwhaha
		//setup some basic regs (should be moved to BootHLE)
		Sh4SetRegister(reg_sr,0x70000000);
		Sh4SetRegister(reg_gbr,0x8c000000);
		Sh4SetRegister(reg_pc,0x8c008300);
		//Start Emulation
		EmuStart();
	}
}
MENU_HANDLER(Handle_File_Exit)
{
	SendMessage((HWND)hWnd, WM_CLOSE, 0,0);
}
//System
MENU_HANDLER( Handle_System_Start)
{
	EmuStart();
}
MENU_HANDLER( Handle_System_Stop)
{
	EmuStop();
}
MENU_HANDLER( Handle_System_Reset)
{
	bool resume=false;
	if (EmuStarted())
	{
		EmuStop();
		resume=true;
	}
	EmuReset(false);
	EmuSetPatch(patch_all,0);

	if (resume)
		EmuStart();
	//msgboxf("Reset is not implemented",MBX_ICONERROR);

	/*
	sh4_cpu->Stop();
	printf(">>\tDreamcast Reset\n");
	sh4_cpu->Reset(false);//do a hard reset
	Sh4SetRegister(reg_pc,0xA0000000);
	DisablePatch(patch_all);
	*/
}
//Debug
MENU_HANDLER( Handle_Debug_Sh4Debugger)
{
	if (hDebugger !=0 && IsWindow(hDebugger))
	{
		SetFocus(hDebugger);
	}
	else
	{
		if(!EmuInit())
		{
			msgboxf(L"Init failed , cannot open debugger",MBX_ICONEXCLAMATION);
			return;
		}
		if (!EmuStarted())
		{
			EmuStart();
			EmuStop();
		}
		//EmuReset(false);

		CtrlMemView::init();
		CtrlDisAsmView::init();

		hDebugger = CreateDialog( g_hInst, MAKEINTRESOURCE(IDD_SH4DEBUG), NULL, DlgProc);
		if( !IsWindow(hDebugger) )
		{
			MessageBox( (HWND)hWnd, L"Couldn't open Sh4 debugger",L"",MB_OK );
		}
	}
}

//Options
MENU_HANDLER( Handle_Options_Config)
{
	DialogBox(g_hInst,MAKEINTRESOURCE(IDD_CONFIG),(HWND)hWnd,DlgProcModal_config);
}
MENU_HANDLER( Handle_Options_AutoHideMenu)
{
	if (settings.AutoHideMenu)
		settings.AutoHideMenu=0;
	else
		settings.AutoHideMenu=1;

	SaveSettings();
	SetMenuItemStyle(id,settings.AutoHideMenu?(u32)MIS_Checked:0,(u32)MIS_Checked);
}

MENU_HANDLER( Handle_Options_AlwaysOnTop )
{
	settings.AlwaysOnTop = !settings.AlwaysOnTop;
	RECT r= { 0,0,640,480};
	AdjustWindowRectEx(&r,GetWindowLong(g_hWnd,GWL_STYLE),GetMenu(g_hWnd)!=NULL,GetWindowLong(g_hWnd,GWL_EXSTYLE));

	if(settings.AlwaysOnTop)
	{
		SetWindowPos( g_hWnd, HWND_TOPMOST, 0, 0, r.right - r.left, r.bottom - r.top,SWP_NOMOVE | SWP_SHOWWINDOW);
		SetFocus(g_hWnd);
	}
	else
		SetWindowPos( g_hWnd,HWND_NOTOPMOST, 0, 0, r.right - r.left, r.bottom - r.top,SWP_NOZORDER | SWP_NOMOVE ) ;

	ShowWindow(g_hWnd,emu.nCmdShow);
	SaveSettings();
	SetMenuItemStyle(id,settings.AlwaysOnTop?(u32)MIS_Checked:0,(u32)MIS_Checked);
}

MENU_HANDLER( Handle_Options_Fullscreen )
{
	if (settings.Fullscreen)
		settings.Fullscreen=0;
	else
		settings.Fullscreen=1;

	ToggleFullscreen((HWND)hWnd);
	SaveSettings();
	SetMenuItemStyle(id,settings.Fullscreen?(u32)MIS_Checked:0,(u32)MIS_Checked);
}

MENU_HANDLER( Handle_Options_SelectPlugins)
{
	EmuSelectPlugins();
}
//Profiler
MENU_HANDLER( Handle_Profiler_Show)
{
	if (ProfilerWindow !=0 && IsWindow(ProfilerWindow))
		SetFocus(ProfilerWindow);
	else
	{
		ProfilerWindow = CreateDialog(g_hInst,MAKEINTRESOURCE(IDD_PROFILER),g_hWnd,ProfilerProc);

		if( !IsWindow(ProfilerWindow) )
		{
			MessageBox( (HWND)hWnd, L"Couldn't create profiler window",L"",MB_OK );
			ProfilerWindow=0;
		}
		ShowWindow(ProfilerWindow, SW_SHOW);
	}
}
MENU_HANDLER( Handle_Profiler_Enable )
{
	if (GetMenuItemStyle(id) & MIS_Checked)
	{
		EmuStopProfiler();
		SetMenuItemStyle(id,0,(u32)MIS_Checked);
	}
	else
	{
		EmuStartProfiler();
		SetMenuItemStyle(id,(u32)MIS_Checked,(u32)MIS_Checked);
	}
}
//Help
MENU_HANDLER( Handle_Help_About )
{
	DialogBox(g_hInst,MAKEINTRESOURCE(IDD_ABOUT),(HWND)hWnd,DlgProcModal_about);
}
template<u32 sffid>
MENU_HANDLER( Handle_Option_Bool_Template )
{
	SetSettingI(sffid,(GetSettingI(sffid)^1)&1);
}

u32 gui_fullscreen_mid;
u32 rec_cpp_mid;
u32 rec_safe_mid;
u32 rec_enb_mid;
u32 rec_ufpu_mid;
u32 ct_menu[4];
u32 sr_menu[4];
u32 bf_menu[5];


void UpdateMenus()
{
	if(GetSettingI(NDCS_DYNAREC_ENABLED))
	{
		SetMenuItemStyle(rec_enb_mid,(u32)MIS_Checked,(u32)MIS_Checked);
		SetMenuItemStyle(rec_cpp_mid,0,(u32)MIS_Grayed);
		SetMenuItemStyle(rec_safe_mid,0,(u32)MIS_Grayed);
		SetMenuItemStyle(rec_ufpu_mid,0,(u32)MIS_Grayed);
	}
	else
	{
		SetMenuItemStyle(rec_enb_mid,0,(u32)MIS_Checked);
		SetMenuItemStyle(rec_cpp_mid,(u32)MIS_Grayed,(u32)MIS_Grayed);
		SetMenuItemStyle(rec_safe_mid,(u32)MIS_Grayed,(u32)MIS_Grayed);
		SetMenuItemStyle(rec_ufpu_mid,(u32)MIS_Grayed,(u32)MIS_Grayed);
	}


	SetMenuItemStyle(rec_cpp_mid,GetSettingI(NDCS_DYNAREC_CPPASS)?(u32)MIS_Checked:0,(u32)MIS_Checked);
	SetMenuItemStyle(rec_safe_mid,GetSettingI(NDCS_DYNAREC_SAFEMODE)?(u32)MIS_Checked:0,(u32)MIS_Checked);
	SetMenuItemStyle(rec_ufpu_mid,GetSettingI(NDCS_DYNAREC_UCFPU)?(u32)MIS_Checked:0,(u32)MIS_Checked);

	for (int i=0;i<4;i++)
	{
		SetMenuItemStyle(ct_menu[i],GetSettingI(NDCS_DREAMCAST_CABLE)==(u32)i?(u32)MIS_Checked:0,(u32)MIS_Checked);
	}
	
	for (int i=0;i<4;i++)
	{
		SetMenuItemStyle(sr_menu[i],GetSettingI(NDCS_DREAMCAST_REGION)==(u32)i?(u32)MIS_Checked:0,(u32)MIS_Checked);
	}
	
	for (int i=0;i<5;i++)
	{
		SetMenuItemStyle(bf_menu[i],GetSettingI(NDCS_DREAMCAST_BROADCAST)==(u32)i?(u32)MIS_Checked:0,(u32)MIS_Checked);
	}
}
MENU_HANDLER( Handle_Option_EnableCP )
{
	Handle_Option_Bool_Template<NDCS_DYNAREC_CPPASS>(id,hWnd,stuff);
	UpdateMenus();
}
MENU_HANDLER( Handle_Option_EnableSAFE )
{
	Handle_Option_Bool_Template<NDCS_DYNAREC_SAFEMODE>(id,hWnd,stuff);
	UpdateMenus();
}
MENU_HANDLER( Handle_Option_EnableRec )
{
	Handle_Option_Bool_Template<NDCS_DYNAREC_ENABLED>(id,hWnd,stuff);
	UpdateMenus();
}

MENU_HANDLER( Handle_Option_UnderclockFpu )
{
	Handle_Option_Bool_Template<NDCS_DYNAREC_UCFPU>(id,hWnd,stuff);
	UpdateMenus();
}

MENU_HANDLER( Handle_Option_Cable_Type )
{

	for (int i=0;i<4;i++)
		if (ct_menu[i]==id)
			SetSettingI(NDCS_DREAMCAST_CABLE,i);
	
	UpdateMenus();
}

MENU_HANDLER( Handle_Option_System_Region )
{

	for (int i=0;i<4;i++)
		if (sr_menu[i]==id)
			SetSettingI(NDCS_DREAMCAST_REGION,i);
	
	UpdateMenus();
}

MENU_HANDLER( Handle_Option_Broadcast_Format )
{

	for (int i=0;i<5;i++)
		if (bf_menu[i]==id)
			SetSettingI(NDCS_DREAMCAST_BROADCAST,i);
	
	UpdateMenus();
}

//Create the menus and set the handlers :)
void CreateBasicMenus()
{
	u32 menu_file=AddMenuItem(0,-1,L"File",0,0);
	u32 menu_system=AddMenuItem(0,-1,L"System",0,0);
	u32 menu_options=AddMenuItem(0,-1,L"Options",0,0);
	u32 menu_debug=AddMenuItem(0,-1,L"Debug",0,0);
	u32 menu_profiler=AddMenuItem(0,-1,L"Profiler",0,0);
	u32 menu_help=AddMenuItem(0,-1,L"Help",0,0);

	//File menu
	AddMenuItem(menu_file,-1,L"Normal Boot",Handle_System_Start,0);
	//AddMenuItem(menu_file,-1,L"Hle GDROM boot",Handle_File_BootHLE,0);
	AddSeperator(menu_file);
	AddMenuItem(menu_file,-1,L"Open bin/elf",Handle_File_OpenBin,0);
	AddMenuItem(menu_file,-1,L"Load bin/elf",Handle_File_LoadBin,0);
	AddSeperator(menu_file);
	AddMenuItem(menu_file,-1,L"Exit",Handle_File_Exit,0);


	//System Menu
	AddMenuItem(menu_system,-1,L"Start",Handle_System_Start,0);
	AddMenuItem(menu_system,-1,L"Stop",Handle_System_Stop,0);
	AddMenuItem(menu_system,-1,L"Reset",Handle_System_Reset,0);

	//Options Menu
	u32 menu_setts=AddMenuItem(menu_options,-1,L"nullDC Settings",Handle_Options_Config,0);
	gui_fullscreen_mid=AddMenuItem(menu_setts,-1,L"Fullscreen",Handle_Options_Fullscreen,settings.Fullscreen);
	AddMenuItem(menu_setts,-1,L"Always on top",Handle_Options_AlwaysOnTop,settings.AlwaysOnTop); 
	AddMenuItem(menu_setts,-1,L"Auto Hide Menu",Handle_Options_AutoHideMenu,settings.AutoHideMenu);
		AddSeperator(menu_setts);
		AddMenuItem(menu_setts,-1,L"Show",Handle_Options_Config,0);
		AddSeperator(menu_setts);
		rec_enb_mid=AddMenuItem(menu_setts,-1,L"Enable Dynarec",Handle_Option_EnableRec,0);
		rec_safe_mid=AddMenuItem(menu_setts,-1,L"Accurate Mode",Handle_Option_EnableSAFE,0);
		rec_cpp_mid=AddMenuItem(menu_setts,-1,L"Enable CP pass",Handle_Option_EnableCP,0);
		rec_ufpu_mid=AddMenuItem(menu_setts,-1,L"Underclock FPU",Handle_Option_UnderclockFpu,0);
		AddSeperator(menu_setts);
		u32 cable_type=AddMenuItem(menu_setts,-1,L"Cable Type",0,0);
			ct_menu[0]=AddMenuItem(cable_type,-1,L"VGA(0) (RGB)",Handle_Option_Cable_Type,0);
			ct_menu[1]=AddMenuItem(cable_type,-1,L"VGA(1) (RGB)",Handle_Option_Cable_Type,0);
			ct_menu[2]=AddMenuItem(cable_type,-1,L"TV     (RGB)",Handle_Option_Cable_Type,0);
			ct_menu[3]=AddMenuItem(cable_type,-1,L"TV     (VBS/Y+S/C)",Handle_Option_Cable_Type,0);
		u32 system_region=AddMenuItem(menu_setts,-1,L"System Region",0,0);
			sr_menu[0]=AddMenuItem(system_region,-1,L"JAP",Handle_Option_System_Region,0);
			sr_menu[1]=AddMenuItem(system_region,-1,L"USA",Handle_Option_System_Region,0);
			sr_menu[2]=AddMenuItem(system_region,-1,L"EUR",Handle_Option_System_Region,0);
			sr_menu[3]=AddMenuItem(system_region,-1,L"Default",Handle_Option_System_Region,0);
		u32 broadcast_format=AddMenuItem(menu_setts,-1,L"Broadcast Format",0,0);
			bf_menu[0]=AddMenuItem(broadcast_format,-1,L"NTSC",Handle_Option_Broadcast_Format,0);
			bf_menu[1]=AddMenuItem(broadcast_format,-1,L"PAL",Handle_Option_Broadcast_Format,0);
			bf_menu[2]=AddMenuItem(broadcast_format,-1,L"PAL_M",Handle_Option_Broadcast_Format,0);
			bf_menu[3]=AddMenuItem(broadcast_format,-1,L"PAL_N",Handle_Option_Broadcast_Format,0);
			bf_menu[4]=AddMenuItem(broadcast_format,-1,L"Default",Handle_Option_Broadcast_Format,0);

	AddMenuItem(menu_options,-1,L"Select Plugins",Handle_Options_SelectPlugins,0);
	AddSeperator(menu_options);
	PowerVR_menu = AddMenuItem(menu_options,-1,L"PowerVR",0,0);
	GDRom_menu = AddMenuItem(menu_options,-1,L"GDRom",0,0);
	Aica_menu = AddMenuItem(menu_options,-1,L"Aica",0,0);
	Arm_menu = AddMenuItem(menu_options,-1,L"Arm",0,0);
	Maple_menu = AddMenuItem(menu_options,-1,L"Maple",0,0);
	ExtDev_menu = AddMenuItem(menu_options,-1,L"ExtDevice",0,0);

	//Maple Menu
	Maple_menu_ports[0][5]=AddMenuItem(Maple_menu,-1,L"Port A",0,0);
	Maple_menu_ports[1][5]=AddMenuItem(Maple_menu,-1,L"Port B",0,0);
	Maple_menu_ports[2][5]=AddMenuItem(Maple_menu,-1,L"Port C",0,0);
	Maple_menu_ports[3][5]=AddMenuItem(Maple_menu,-1,L"Port D",0,0);

	//Debug
	Debug_menu=menu_debug;
	AddMenuItem(menu_debug,-1,L"Debugger",Handle_Debug_Sh4Debugger,0);

	//Profiler
	AddMenuItem(menu_profiler,-1,L"Enable",Handle_Profiler_Enable,0);
	AddMenuItem(menu_profiler,-1,L"Show",Handle_Profiler_Show,0);

	//Help
	AddMenuItem(menu_help,-1,L"About",Handle_Help_About,0);

	//Update menu ticks to match settings :)
	UpdateMenus();
}
void InitMenu()
{
	MenuItems.push_back(0);

	CreateBasicMenus();
}
HMENU GetHMenu()
{
	return MainMenu.hmenu;
}

s32 mouse_hidden=2;
bool mouse_visible=true;
bool mouseCapture = false;

void SetMouseState(HWND hWnd,bool visible)
{
	if (visible==mouse_visible)
		return;
	RECT r;
	GetClientRect(hWnd,&r);
	if (visible)
	{
		ShowCursor(true);
		SetMenu(hWnd,GetHMenu());
	}
	else
	{
		ShowCursor(false);
		if (settings.AutoHideMenu)
			SetMenu(hWnd,NULL);
	}

	if (GetWindowLong(hWnd,GWL_STYLE) & WS_CAPTION)
	{
		AdjustWindowRectEx(&r,GetWindowLong(hWnd,GWL_STYLE),GetMenu(hWnd)!=NULL,GetWindowLong(hWnd,GWL_EXSTYLE));
		SetWindowPos( hWnd, NULL, 0, 0, r.right - r.left, r.bottom - r.top,SWP_NOZORDER | SWP_NOMOVE ) ;
	}
	mouse_visible=visible;
}

void ToggleFullscreen(HWND hWnd)
{
	HMONITOR hmon=MonitorFromWindow(hWnd,0);
	MONITORINFO mi;
	mi.cbSize=sizeof(mi);
	GetMonitorInfo(hmon,&mi);
	static RECT oldrect;

	if (GetWindowLong(hWnd,GWL_STYLE)&WS_CAPTION)
	{
		GetClientRect(hWnd,&oldrect);
		RECT pos;
		GetWindowRect(hWnd,&pos);
		oldrect.top=pos.top;
		oldrect.bottom+=pos.top;
		oldrect.left=pos.left;
		oldrect.right+=pos.left;

		SetWindowLong(hWnd,GWL_STYLE,(GetWindowLong(hWnd,GWL_STYLE)&~WS_OVERLAPPEDWINDOW) | WS_POPUP);
		SetWindowPos(hWnd,HWND_TOPMOST,mi.rcMonitor.left,mi.rcMonitor.top,mi.rcMonitor.right-mi.rcMonitor.left,mi.rcMonitor.bottom-mi.rcMonitor.top,0);
	}
	else
	{
		SetWindowLong(hWnd,GWL_STYLE,(GetWindowLong(hWnd,GWL_STYLE)&~WS_POPUP) | WS_OVERLAPPEDWINDOW);
		RECT r=oldrect;
		AdjustWindowRectEx(&r,GetWindowLong(hWnd,GWL_STYLE),GetMenu(hWnd)!=NULL,GetWindowLong(hWnd,GWL_EXSTYLE));
		SetWindowPos( hWnd, NULL, oldrect.left, oldrect.top, r.right - r.left, r.bottom - r.top,SWP_NOZORDER) ;
	}
}
static CPicture ndclogo;
LRESULT CALLBACK WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	static RECT rc;
	//printf("msg %X\n",uMsg);
	//OleSavePictureFile
	switch(uMsg)
	{
	case WM_PAINT:
		//ndclogo
		{
			if (EmuStarted())
				break;
			PAINTSTRUCT ps; 
			HDC hdc; 
			RECT rc;
			hdc = BeginPaint(hWnd, &ps); 
			GetClientRect(hWnd, &rc); 
			int width=(rc.right-rc.left);
			int height=(rc.bottom-rc.top);
			SIZE hm=ndclogo.GetImageSize(hdc);
			long hmWidth=hm.cx,hmHeight=hm.cy; // HIMETRIC units
			//ndclogo.GetHIMETRICSize(hmWidth, hmHeight);

			int h=(int)(hmHeight*width/(float)hmWidth+0.5f);
			int w=(int)(hmWidth*height/(float)hmHeight+0.5f);
			
			if (h>height)
			{
				h=height;
			}
			else
			{
				w=width;
			}
			int dx=width-w;
			int dy=height-h;
			
			rc.left=0;
			rc.right=hmWidth;

			rc.top=0;
			rc.bottom=hmHeight;

			SetStretchBltMode(hdc,HALFTONE);

			HDC memDC = CreateCompatibleDC ( hdc );
			HBITMAP memBM = CreateCompatibleBitmap ( hdc, hmWidth, hmHeight );
			SelectObject ( memDC, memBM );
			ndclogo.Render(memDC,&rc);
			StretchBlt(hdc,dx/2,dy/2,w,h,memDC,0,0,hmWidth,hmHeight,SRCCOPY);

			//LEFT
			StretchBlt(hdc,0,0,dx/2,height,memDC,0,0,1,hmHeight,SRCCOPY);

			//RIGHT
			StretchBlt(hdc,dx/2+w,0,dx-dx/2,height,memDC,hmWidth-1,0,1,hmHeight,SRCCOPY);

			//TOP
			StretchBlt(hdc,0,0,width,dy/2,memDC,0,0,hmWidth,1,SRCCOPY);

			//BOTTOM
			StretchBlt(hdc,0,dy/2+h,width,dy-dy/2,memDC,0,hmHeight-1,hmWidth,1,SRCCOPY);

			DeleteObject(memBM);
			DeleteDC(memDC);

			/*
			SetMapMode(hdc, MM_ANISOTROPIC); 
			SetWindowExtEx(hdc, 100, 100, NULL); 
			SetViewportExtEx(hdc, rc.right, rc.bottom, NULL); 
			Polyline(hdc, aptStar, 6); 
			*/
			
			EndPaint(hWnd, &ps); 
			return 0L; 
		}
		break;
	case WM_ERASEBKGND:
		{
			
			return TRUE;
		}
		break;

	case WM_TIMER:
		{
			POINT cp;
			RECT cl;
			GetClientRect(hWnd,&cl);

			GetCursorPos(&cp);
			if (GetActiveWindow()==hWnd && WindowFromPoint(cp)==hWnd && ScreenToClient(hWnd,&cp) && cp.x>cl.left && cp.y>cl.top && cp.x<cl.right && cp.y<cl.bottom)
			{
				if (mouse_hidden>0)
					mouse_hidden-=9;
				if (mouse_hidden<=0)
				{
					mouse_hidden=0;
					SetMouseState(hWnd,false);
				}
				//printf("TIMER %d\n",mouse_hidden);
				
			}
			else if (mouse_hidden==0)
			{
				//mouse_hidden=10;
				//SetMouseState(hWnd,true);
			}
		} 
		break;
	case WM_SIZE:
		{
			NDC_WINDOW_RECT r = { LOWORD(lParam),HIWORD(lParam) };
			if (r.height>0 && r.width>0)
				emu.BroardcastEvent((u32)MT_All,NDE_GUI_RESIZED,&r,0);
		}
		break;
	case WM_CREATE:
		{
			InitCommonControls();
			ndclogo.Load(g_hInst,IDR_NDC_LOGO);
			mouse_hidden=30;
			SetTimer(hWnd,0,1000,0);
		}
		break;
	case WM_MOUSEMOVE:
		{
			int xPos = GET_X_LPARAM(lParam); 
			int yPos = GET_Y_LPARAM(lParam);
			static int oldX=-65537,oldY=-65537;
			if (oldX==xPos && oldY==yPos)
				break;
			oldX=xPos;
			oldY=yPos;
			//printf("Mouse move %d,%d,%d\n",mouse_hidden,oldX,oldY);
			mouse_hidden++;
			if (!mouseCapture && mouse_hidden>=30)
			{
				mouse_hidden=30;

				SetMouseState(hWnd,true);
			}
		}
		break;
	case WM_COMMAND:
		{
			for (size_t i=1;i<MenuItems.size();i++)
			{
				if (MenuItems[i] && MenuItems[i]->nid==LOWORD(wParam))
				{
					MenuItems[i]->Clicked(hWnd);
					break;
				}
			}
			if (IDCANCEL==LOWORD(wParam))
				PostQuitMessage(0);
			break;
			//printf("Menu item %d selected\n",LOWORD(wParam));
		}
		break;

	case WM_SYSCOMMAND:
		if (SC_SCREENSAVE == wParam || SC_MONITORPOWER == wParam)
			return 0;	//disable Screen Saver/Monitor turn off
		break;

	case WM_CLOSE:
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_SYSKEYDOWN:
		{
			if (LOWORD(wParam)==VK_RETURN)
			{
				Handle_Options_Fullscreen(gui_fullscreen_mid,hWnd,0);
				return 0;
			}
		}
		break;
	case WM_MENUCHAR:
		// A menu is active and the user presses a key that does not correspond to any mnemonic or accelerator key
		// So just ignore and don't beep
		if (LOWORD(wParam)==VK_RETURN)
			return MAKELRESULT(0,MNC_CLOSE);
		break;
	case WM_KEYDOWN:
		{
			if(wParam == VK_SCROLL)
			{
				if (!mouseCapture)
				{
					ShowCursor(false);
					mouseCapture = true;
				}
				else
				{
					ShowCursor(true);
					mouseCapture = false;
				}
			}
			
			int val = (int)wParam;
			switch(val)
			{
			case VK_ESCAPE:
				//SendMessage(hWnd,WM_CLOSE,0,0);
				break;
			case VK_F7:
				{
					emu.EmuResetCaches();
				}
				break;
			case VK_F8:
				{
					wchar fn[512];
					wchar ep[512];
					
					emu.ConfigLoadStr(L"emu",L"AppPath",ep,0);
					int i;
					i=0;

					while( i >=0)
					{
						
						swprintf(fn,L"%sscreenshot_%d.bmp",ep,i);

						
						FILE* tf=_tfopen(fn,L"rb");
						if (tf)
							fclose(tf);
						else
							break;
						i++;
						
					}

					
					if (Screenshot(fn,g_hWnd))
						wprintf(L"Screenshot saved to %s\n",fn);
					else
						wprintf(L"failed to save screenshot to \"%s\"\n",fn);
				}
				break;

			}
		}
		break;
	default: break;
	}

	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}
bool plgdlg_cancel=false;
bool EXPORT_CALL SelectPluginsGui()
{
	DialogBox(g_hInst,MAKEINTRESOURCE(IDD_PLUGIN_SELECT),g_hWnd,PluginDlgProc);
	return !plgdlg_cancel;
}
INT_PTR CALLBACK DlgProcModal_about( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg )
	{
	case WM_INITDIALOG:
		{
			Edit_SetText(GetDlgItem(hWnd,IDC_THXLIST),emu.GetAboutText());
			Static_SetText(GetDlgItem(hWnd,IDC_NDC_VER),emu_name);
		}
		return true;

	case WM_COMMAND:
		switch( LOWORD(wParam) )
		{
		case IDOK:
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

INT_PTR CALLBACK DlgProcModal_config( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg )
	{
	case WM_INITDIALOG:
		{
			CheckDlgButton(hWnd,IDC_REC,GetSettingI(NDCS_DYNAREC_ENABLED)!=0?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hWnd,IDC_REC_CPP,GetSettingI(NDCS_DYNAREC_CPPASS)!=0?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hWnd,IDC_REC_UFPU,GetSettingI(NDCS_DYNAREC_UCFPU)!=0?BST_CHECKED:BST_UNCHECKED);
		}
		return true;

	case WM_COMMAND:

		switch( LOWORD(wParam) )
		{
		case IDOK:
			{
				SetSettingI(NDCS_DYNAREC_ENABLED,(BST_CHECKED==IsDlgButtonChecked(hWnd,IDC_REC)) ? 1 : 0);
				SetSettingI(NDCS_DYNAREC_CPPASS,(BST_CHECKED==IsDlgButtonChecked(hWnd,IDC_REC_CPP)) ? 1 : 0);
				SetSettingI(NDCS_DYNAREC_UCFPU,(BST_CHECKED==IsDlgButtonChecked(hWnd,IDC_REC_UFPU)) ? 1 : 0);

				UpdateMenus();
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


INT_PTR CALLBACK ChildDlgProc( HWND hChild, UINT uMsg, WPARAM wParam, LPARAM lParan )
{
	switch( uMsg ) {
	case WM_INITDIALOG:	return true;
	default: break;
	}
	return false;
}


//////////////

void RefreshDebugger(HWND);	// *TEMP*

/////////////////////////////////////////// TAB CTRL STUFF /////////////////////////////////

#define C_PAGES 3 
 
typedef struct tag_dlghdr { 
    HWND hTab;       // tab control 
    HWND hDisplay;   // current child dialog box 
    RECT rcDisplay;     // display rectangle for the tab control 

    DLGTEMPLATE *apRes[C_PAGES]; 
} DLGHDR; 

// DoLockDlgRes - loads and locks a dialog box template resource. 
// Returns the address of the locked resource. 
// lpszResName - name of the resource 
 
DLGTEMPLATE * WINAPI DoLockDlgRes(LPCWSTR lpszResName) 
{ 
    HRSRC hrsrc = FindResource(g_hInst, lpszResName, RT_DIALOG); 
    HGLOBAL hglb = LoadResource(g_hInst, hrsrc); 
    return (DLGTEMPLATE *) LockResource(hglb); 
} 


// OnChildDialogInit - Positions the child dialog box to fall 
//     within the display area of the tab control. 


/////////////////////////////////////////////////////////////////////////////////////
#define YOFFS 20
#define BUTTONOFFS 25;

void WINAPI OnSizeTab(HWND hDlg) 
{
    DLGHDR *pHdr = (DLGHDR *) GetWindowPtr(hDlg, GWLP_USERDATA); 
	////////////////////////////////////////////////////////////

	RECT rcTab, rcDlg;
	GetWindowRect( hDlg, &rcDlg );
	GetWindowRect( pHdr->hTab, &rcTab );
	
	UINT y = rcTab.top - rcDlg.top - YOFFS;
	UINT x = rcTab.left - rcDlg.left;
	UINT w = rcTab.right - rcTab.left;
	UINT h = rcTab.bottom - rcTab.top - BUTTONOFFS;

	HWND hCust = GetDlgItem(pHdr->hDisplay, IDC_VIEW);
	SetWindowPos( hCust, HWND_TOP, 0, 0, w-8, h, SWP_SHOWWINDOW );
	SetWindowPos( pHdr->hDisplay, HWND_TOP, x, y, w, h, SWP_SHOWWINDOW );
} 

// OnSelChanged - processes the TCN_SELCHANGE notification. 
// hwndDlg - handle to the parent dialog box. 
 
VOID WINAPI OnSelChanged(HWND hDlg) 
{ 
    DLGHDR *pHdr = (DLGHDR *) GetWindowPtr( hDlg, GWLP_USERDATA); 

    int iSel = TabCtrl_GetCurSel(pHdr->hTab); 
 
    // Destroy the current child dialog box, if any. 
    if (pHdr->hDisplay != NULL) 
        DestroyWindow(pHdr->hDisplay); 
 
    // Create the new child dialog box. 
    pHdr->hDisplay = CreateDialogIndirect(g_hInst, pHdr->apRes[iSel], hDlg, ChildDlgProc); 

	switch( iSel ) {
	case 0: cDisView = CtrlDisAsmView::getFrom(GetDlgItem(pHdr->hDisplay,IDC_VIEW)); cMemView=NULL; break;
	case 1: cMemView = CtrlMemView::getFrom(GetDlgItem(pHdr->hDisplay,IDC_VIEW)); cDisView=NULL; break;
	case 2:
	default: cDisView = NULL; cMemView = NULL; break;
	}
	OnSizeTab(hDlg);
} 

void WINAPI OnDestroyTab(HWND hDlg)
{
    DLGHDR *pHdr = (DLGHDR *) GetWindowPtr( hDlg, GWLP_USERDATA); 

	if( IsWindow(pHdr->hDisplay) )
		DestroyWindow( pHdr->hDisplay );
	if( IsWindow(pHdr->hTab) )
		DestroyWindow( pHdr->hTab );
}

void WINAPI OnInitTab(HWND hDlg)
{
	TCITEM tci; 
	DLGHDR *pHdr = (DLGHDR *) LocalAlloc(LPTR, sizeof(DLGHDR)); 

	SetWindowPtr(hDlg, GWLP_USERDATA, pHdr);

	pHdr->hTab = GetDlgItem(hDlg, IDC_VIEWSEL);
	
	// Add a tab for each of the three child dialog boxes. 
    tci.mask = TCIF_TEXT | TCIF_IMAGE; 
    tci.iImage = -1; 
    tci.pszText = L"DisAsm"; 
    TabCtrl_InsertItem(pHdr->hTab, 0, &tci); 
    tci.pszText = L"Memory"; 
    TabCtrl_InsertItem(pHdr->hTab, 1, &tci); 
    tci.pszText = L"PVR List"; 
    TabCtrl_InsertItem(pHdr->hTab, 2, &tci); 
 
    // Lock the resources for the three child dialog boxes. 
    pHdr->apRes[0] = DoLockDlgRes(MAKEINTRESOURCE(IDD_DISVIEW)); 
    pHdr->apRes[1] = DoLockDlgRes(MAKEINTRESOURCE(IDD_MEMVIEW__)); 
    pHdr->apRes[2] = DoLockDlgRes(MAKEINTRESOURCE(IDD_PVRVIEW)); 
 
    OnSelChanged(hDlg);
}


////////////////////////////// END TAB CTRL	///////////////////////////////////////////////

INT_PTR CALLBACK DlgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	static int iSel=0;
	static wchar szAddr[32];
	static DWORD dwIndex=0, dwAddr=0, i=0;


	switch( uMsg )
	{
	case WM_INITDIALOG:

		if( IsWindow(GetDlgItem(hWnd,IDC_VIEWSEL)) )
		{
			OnInitTab(hWnd);
			OnSizeTab(hWnd);	/// have to do this or postsizemsg

			RefreshDebugger(hWnd);
		}
		if( IsWindow( GetDlgItem(hWnd, IDC_MEMSEL) ) )
		{
		//	SendDlgItemMessage(hWnd, IDC_MEMSEL, CB_SETITEMDATA, 0xBADC0D3, 0x666 );
		//	for( i=0; i<MMAP_NUM_BLOCKS; i++ ) {
		//		dwIndex = SendDlgItemMessage( hWnd, IDC_MEMSEL, CB_ADDSTRING, 0, (LPARAM)MemBlock[i].szDesc );
		//		SendDlgItemMessage( hWnd, IDC_MEMSEL, CB_SETITEMDATA, dwIndex, i );
		//	}
		}
		return true;

	case WM_COMMAND: 

		switch (LOWORD(wParam)) 
		{
		case IDC_FIND:
			MessageBox( hWnd, L"FIND",L"", MB_OK );
			return true;

		case IDB_ADDMBP:
			{
				GetDlgItemText( hWnd, IDC_MBPADDR, szAddr, 32 );
				u32 addr=htoi(szAddr);
				//DWORD old;
				//VirtualProtect((void*)addr,4,PAGE_NOACCESS,&old);
				HANDLE s=SetHardwareBreakpoint(emu.EmuThread,HWBRK_TYPE_READWRITE,HWBRK_SIZE_4,(void*)addr);
				printf("MBP @ 0x%08X=%08X\n",addr,s);
			}
			break;
		case IDB_REMOVEMBP:
			{
			GetDlgItemText( hWnd, IDC_ADDR, szAddr, 32 );
			//u32 addr=htoi(szAddr);
			//DWORD old;
			//VirtualProtect((void*)addr,4,PAGE_READWRITE,&old);
			}
			break;
		/////////////////////////////
		case IDB_STOP:
			EmuStop();
			goto refresh;

		case IDB_START:
			EmuStart();
			goto refresh;

		case IDB_STEP:
			EmuStep();
			goto refresh;

		case IDB_SKIP:
			EmuSkip();
			goto refresh;

		case IDB_GOTO:
			GetDlgItemText( hWnd, IDC_ADDR, szAddr, 32 );
			cDisView->gotoAddr(htoi(szAddr));
			return true;	// dont goto refresh .. will goto pc instead of IDC_GOTO

		//////////////////////////////

			/*
		case IDC_MEMSEL:
			if( HIWORD(wParam) == CBN_SELCHANGE )
			{
				dwIndex = SendDlgItemMessage( hWnd, IDC_MEMSEL, CB_GETCURSEL, 0,0 );
				i = SendDlgItemMessage( hWnd, IDC_MEMSEL, CB_GETITEMDATA, dwIndex, 0 );
				MemViewUpdate( i );	// can just pass this to MV_UPDATE

				SendMessage( hWnd, WM_SIZE, 0, 0 );
				SendDlgItemMessage( hWnd, IDC_CUSTOM, MV_UPDATE, 0,0 );
			}
			return true;
			*/

			////////////

		case IDC_XF:
		case IDC_FPSEL1:
		case IDC_FPSEL2:
		case IDC_FPSEL3:
			RefreshDebugger(hWnd);
			return true;

		default: break;
		}
		break;


	case WM_NOTIFY:
		switch( ((LPNMHDR)lParam)->idFrom )
		{
		case IDC_VIEWSEL:
			if( ((LPNMHDR)lParam)->code == TCN_SELCHANGE )
			{
				OnSelChanged(hWnd);
				goto refresh;
			}
			break;
		}
		break;
		

	case WM_CLOSE:
	case WM_DESTROY:			// for mem view dlg
		OnDestroyTab(hWnd);
		DestroyWindow(hWnd);
		return TRUE; 

	case WM_MOVE:
	case WM_SIZE:
	//	OnSizeTab(hWnd);
	//	return TRUE;
		goto refresh;

	case WM_GOTOPC:
		goto refresh;
	}

	return false;

refresh:
	
	if( cDisView != NULL ) cDisView->gotoPC();
	if( cMemView != NULL ) cMemView->gotoAddr(Sh4GetRegister(reg_pc));
	RefreshDebugger(hWnd);
	return true;
}


///////////////////////////////////////////////////////////////////////

wchar szBuf[256];	// more than big enough for this
wchar *xs( u32 x ) { swprintf(szBuf, L"%X", x); return szBuf; }
wchar *fls( float f ) { swprintf(szBuf, L"%f", f); return szBuf; }
wchar *x8s( u32 x ) { swprintf(szBuf, L"%08X", x); return szBuf; }

f32 GetXf(u32 r)
{
	u32 temp=Sh4GetRegister((Sh4RegType)(xf_0+r));
	return *(float*)&temp;
}
f32 GetFr(u32 r)
{
	u32 temp=Sh4GetRegister((Sh4RegType)(fr_0+r));
	return *(float*)&temp;
}

f64 dbGetXD(u32 r)
{
	double t;
	((u32*)(&t))[1]=Sh4GetRegister((Sh4RegType)(xf_0+(r<<1)));
	((u32*)(&t))[0]=Sh4GetRegister((Sh4RegType)(xf_0+(r<<1)+1));
	return t;
}
f64 dbGetDR(u32 r)
{
	double t;
	((u32*)(&t))[1]=Sh4GetRegister((Sh4RegType)(fr_0+(r<<1)));
	((u32*)(&t))[0]=Sh4GetRegister((Sh4RegType)(fr_0+(r<<1)+1));
	return t;
}
union StatusReg
{
	struct
	{
		u32 T		:1;//<<0
		u32 S		:1;//<<1
		u32 rsvd0	:2;//<<2
		u32 IMASK	:4;//<<4
		u32 Q		:1;//<<8
		u32 M		:1;//<<9
		u32 rsvd1	:5;//<<10
		u32 FD		:1;//<<15
		u32 rsvd2	:12;//<<16
		u32 BL		:1;//<<28
		u32 RB		:1;//<<29
		u32 MD		:1;//<<20
		u32 rsvd3	:1;//<<31
	};
	u32 full;
};
void RefreshDebugger(HWND hDlg)
{
	
	SetDlgItemText( hDlg, IDS_PC, x8s(Sh4GetRegister(reg_pc) ));
	SetDlgItemText( hDlg, IDS_PR, x8s(Sh4GetRegister(reg_pr) ));

	SetDlgItemText( hDlg, IDS_GBR, x8s(Sh4GetRegister(reg_gbr) ));
	SetDlgItemText( hDlg, IDS_SSR, x8s(Sh4GetRegister(reg_ssr) ));
	SetDlgItemText( hDlg, IDS_SPC, x8s(Sh4GetRegister(reg_spc) ));
	SetDlgItemText( hDlg, IDS_VBR, x8s(Sh4GetRegister(reg_vbr) ));
	SetDlgItemText( hDlg, IDS_SGR, x8s(Sh4GetRegister(reg_sgr) ));
	SetDlgItemText( hDlg, IDS_DBR, x8s(Sh4GetRegister(reg_dbr) ));

	SetDlgItemText( hDlg, IDS_MACH, x8s(Sh4GetRegister(reg_mach) ));
	SetDlgItemText( hDlg, IDS_MACL, x8s(Sh4GetRegister(reg_macl) ));
	SetDlgItemText( hDlg, IDS_FPUL, x8s(Sh4GetRegister(reg_fpul) ));
	SetDlgItemText( hDlg, IDS_FPSCR, x8s(Sh4GetRegister(reg_fpscr) ));

	StatusReg sr;

	sr.full=Sh4GetRegister(reg_sr);

	SetDlgItemText( hDlg, IDS_SR_T, (sr.T?L"1":L"0") );
	SetDlgItemText( hDlg, IDS_SR_S, (sr.S?L"1":L"0") );
	SetDlgItemText( hDlg, IDS_SR_M, (sr.M?L"1":L"0") );
	SetDlgItemText( hDlg, IDS_SR_Q, (sr.Q?L"1":L"0") );
	SetDlgItemText( hDlg, IDS_SR_BL, (sr.BL?L"1":L"0") );
	SetDlgItemText( hDlg, IDS_SR_RB, (sr.RB?L"1":L"0") );
	SetDlgItemText( hDlg, IDS_SR_MD, (sr.MD?L"1":L"0") );
	SetDlgItemText( hDlg, IDS_SR_FD, (sr.FD?L"1":L"0") );
	SetDlgItemText( hDlg, IDS_SR_IMASK, xs(sr.IMASK) );

	wchar tbuff[1024*80];
	/**
	GetCallStackText(&tbuff[0]);
	SetDlgItemText( hDlg, IDC_CALLSTACK,&tbuff[0] );
	**/


	for( u32 i=0; i<16; i++ )
		SetDlgItemText( hDlg, IDS_R0+i, x8s(Sh4GetRegister((Sh4RegType)(r0+i)) ));

	for( u32 i=0; i<8; i++ )
		SetDlgItemText( hDlg, IDS_B0+i, x8s(Sh4GetRegister((Sh4RegType)(r0_Bank+i)) ));


	////////////////////////////////////////////////////////
	u32 fpm =	IsDlgButtonChecked(hDlg, IDC_XF) ? (4) : (0);
	fpm |=	IsDlgButtonChecked(hDlg, IDC_FPSEL3) ? (3) :
			IsDlgButtonChecked(hDlg, IDC_FPSEL2) ? (2) : (1) ;

	wchar szFPU[1024];
	swprintf(szFPU, L"FPU Regs\n========\nFPSCR: %08X \nFPUL: %08X\n", Sh4GetRegister(reg_fpscr), Sh4GetRegister(reg_fpul));

	switch( fpm &3 )	// IDC_FPSEL
	{
	case 3:	// Vector: XMTX | FV
		if(fpm&4) {
			swprintf(szBuf, L"XMTX\n"
				L"(%.5f\t%.5f\t%.5f\t%.5f)\n" L"(%.5f\t%.5f\t%.5f\t%.5f)\n"
				L"(%.5f\t%.5f\t%.5f\t%.5f)\n" L"(%.5f\t%.5f\t%.5f\t%.5f)\n\n",
				GetXf(0), GetXf(4), GetXf(8), GetXf(12), GetXf(1), GetXf(5), GetXf(9), GetXf(13),
				GetXf(2), GetXf(6), GetXf(10), GetXf(14), GetXf(3), GetXf(7), GetXf(11), GetXf(15) );
			_tcscat(szFPU,szBuf);
		} else {
			swprintf(szBuf,
				L"FV0 :\n%.5f\t%.5f\t%.5f\t%.5f\n" L"FV4 :\n%.5f\t%.5f\t%.5f\t%.5f\n"
				L"FV8 :\n%.5f\t%.5f\t%.5f\t%.5f\n" L"FV12:\n%.5f\t%.5f\t%.5f\t%.5f\n\n",
				GetFr(0), GetFr(1), GetFr(2), GetFr(3),	GetFr(4), GetFr(5), GetFr(6), GetFr(7),
				GetFr(9), GetFr(9), GetFr(10), GetFr(11),GetFr(12), GetFr(13), GetFr(14), GetFr(15) );
			_tcscat(szFPU,szBuf);
		}
	break;

	case 2:	// Double: XD | DR
		for( u32 i=0; i<8; i++ ) {
			swprintf(szBuf, L" %s%02d :%+G \n", (fpm&4)?L"XD":L"DR", (i), ((fpm&4)? dbGetXD(i) : dbGetDR(i)) );
			wcscat(szFPU,szBuf);
		}
	break;

	case 1:	// Single: XF | FR
	default:	// default to single precision mode | wtf is the first entry non working for ?
		for( u32 i=0; i<16; i+=2 ) {
			swprintf(szBuf, L" %s%02d :%+G \n", ((fpm&4)?L"XF":L"FR"), (i+0), ((fpm&4)? GetXf(i+0) : GetFr(i+0)) );	_tcscat(szFPU,szBuf);
			swprintf(szBuf, L" %s%02d :%+G \n", ((fpm&4)?L"XF":L"FR"), (i+1), ((fpm&4)? GetXf(i+1) : GetFr(i+1)) );	_tcscat(szFPU,szBuf);
		}
	break;
	}

	SetDlgItemText( hDlg, IDS_FPUREGS, szFPU );
	///////////////////////////////////////////

//	SetDlgItemText( hDlg, IDS_CCR, x8s(ccr) );
//	SetDlgItemText( hDlg, IDS_QACR0, x8s(g_dwQACR0) );
//	SetDlgItemText( hDlg, IDS_QACR1, x8s(g_dwQACR1) );

//	SetDlgItemText( hDlg, IDS_MMUCR, x8s(g_dwMMUCR) );

}



inline static void RefreshArmDbg(void)
{

}
BOOL CopyTextToClipboard(HWND hwnd, TCHAR *text);
INT_PTR CALLBACK ProfilerProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg )
	{
	case WM_INITDIALOG:
		SetTimer(hWnd,0,500,0);
		return TRUE;

	case WM_TIMER:
		{
			nullDCPerfomanceInfo ndcpi;
			emu.GetPerformanceInfo(&ndcpi);
			
			
			wchar text_[2048];
			wchar* text=text_;

#define strcatf text+=swprintf
			if (ndcpi.TBP.Valid)
			{
				strcatf(text,L"\r\nTBP: Enabled, look at the console for the profiler output ..\r\n");
				//the console has more usefull info ;p
			}
			else
			{
				strcatf(text,L"\r\nTBP: Disabled\r\n");
			}

			if (ndcpi.Dynarec.Runtime.Lookups.Valid)
			{
				strcatf(text,L"\r\nRuntime dynarec lookup profiling enabled\r\n");
				strcatf(text,L"%d full lookups , %d fast lookups , ratio %.2f%%\r\n",ndcpi.Dynarec.Runtime.Lookups.FullLookupDelta,ndcpi.Dynarec.Runtime.Lookups.FastLookupDelta
														,ndcpi.Dynarec.Runtime.Lookups.FullLookupDelta/(float)ndcpi.Dynarec.Runtime.Lookups.LookupDelta*100);
				strcatf(text,L"%d blocks TOTAL , (100%%)\r\n",ndcpi.Dynarec.Runtime.Execution.TotalBlocks);
				strcatf(text,L" |- %d blocks staticaly linked , ratio %.2f%%\r\n",ndcpi.Dynarec.Runtime.Execution.TotalBlocks-ndcpi.Dynarec.Runtime.Lookups.LookupDelta-ndcpi.Dynarec.Runtime.RetCache.Count,
					(ndcpi.Dynarec.Runtime.Execution.TotalBlocks-ndcpi.Dynarec.Runtime.Lookups.LookupDelta-ndcpi.Dynarec.Runtime.RetCache.Count)/(float)ndcpi.Dynarec.Runtime.Execution.TotalBlocks*100);
				strcatf(text,L" |- %d blocks ret cache , ratio %.2f%%\r\n",ndcpi.Dynarec.Runtime.RetCache.Count,
					(ndcpi.Dynarec.Runtime.RetCache.Count)/(float)ndcpi.Dynarec.Runtime.Execution.TotalBlocks*100);
				strcatf(text,L" |- %d blocks from BM (total), ratio %.2f%%\r\n",ndcpi.Dynarec.Runtime.Lookups.LookupDelta,
					(ndcpi.Dynarec.Runtime.Lookups.LookupDelta)/(float)ndcpi.Dynarec.Runtime.Execution.TotalBlocks*100);
				strcatf(text,L" |  |- %d blocks from BM cache, ratio %.2f%%\r\n",ndcpi.Dynarec.Runtime.Lookups.FastLookupDelta,
					(ndcpi.Dynarec.Runtime.Lookups.FastLookupDelta)/(float)ndcpi.Dynarec.Runtime.Execution.TotalBlocks*100);
				strcatf(text,L" |  |- %d blocks from BM slow, ratio %.2f%%\r\n",ndcpi.Dynarec.Runtime.Lookups.FullLookupDelta,
					(ndcpi.Dynarec.Runtime.Lookups.FullLookupDelta)/(float)ndcpi.Dynarec.Runtime.Execution.TotalBlocks*100);
			}
			else
			{
				strcatf(text,L"\r\nRuntime dynarec lookup profiling disabled\r\n");
			}
			if (ndcpi.Dynarec.Runtime.Execution.Valid)
			{
				strcatf(text,L"\r\nRuntime dynarec execution profiling enabled\r\n");
				strcatf(text,L"%d Manual blocks , %d locked blocks , ratio %.2f%%\r\n",ndcpi.Dynarec.Runtime.Execution.ManualBlocks,ndcpi.Dynarec.Runtime.Execution.LockedBlocks,
				ndcpi.Dynarec.Runtime.Execution.ManualBlocks/(float)ndcpi.Dynarec.Runtime.Execution.TotalBlocks*100);
			}
			else
			{
				strcatf(text,L"\r\nRuntime dynarec execution profiling disabled\r\n");
			}
			strcatf(text,L"\r\nDynarec Translation Cache stats:\r\n");
			strcatf(text,L"%.2f MB src size, %.2f ratio\r\n",ndcpi.Dynarec.CodeGen.SrcCodeSize/1024.0f/1024.0f,ndcpi.Dynarec.CodeGen.CodeSize/(float)ndcpi.Dynarec.CodeGen.SrcCodeSize);
			strcatf(text,L"%.2f MB size, %d blocks, %.1f B avg size\r\n",ndcpi.Dynarec.CodeGen.CodeSize/1024.0f/1024.0f,ndcpi.Dynarec.CodeGen.TotalBlocks,
														ndcpi.Dynarec.CodeGen.CodeSize/(float)ndcpi.Dynarec.CodeGen.TotalBlocks);
			strcatf(text,L"%.3f ms compile time, %.3f E/block, %.3f MB/s\r\n",(float)ndcpi.Dynarec.CodeGen.TotalCompileTime/1000.f,ndcpi.Dynarec.CodeGen.TotalCompileTime/(float)ndcpi.Dynarec.CodeGen.TotalBlocks,
																				ndcpi.Dynarec.CodeGen.SrcCodeSize/(float)ndcpi.Dynarec.CodeGen.TotalCompileTime);
			
			strcatf(text,L"%d Manual blocks , %d locked blocks , ratio %.2f%%\r\n",ndcpi.Dynarec.CodeGen.ManualBlocks,ndcpi.Dynarec.CodeGen.LockedBlocks,
				ndcpi.Dynarec.CodeGen.ManualBlocks/(float)ndcpi.Dynarec.CodeGen.TotalBlocks*100);
			
			
			if (ndcpi.Dynarec.Runtime.RetCache.Valid)
			{
				strcatf(text,L"\r\nRuntime dynarec ret cache stats enabled\r\n");
				strcatf(text,L"%d rets missmatched , %d rets matched , ratio %.2f%%\r\n",ndcpi.Dynarec.Runtime.RetCache.Misses,ndcpi.Dynarec.Runtime.RetCache.Hits
														,ndcpi.Dynarec.Runtime.RetCache.Misses/(float)ndcpi.Dynarec.Runtime.RetCache.Count*100);
			}
			else
			{
				strcatf(text,L"\r\nRuntime dynarec ret cache stats disabled\r\n");
			}
			/**
			sprintf(text,"Block manager : \r\ntracking %d blocks , %d kb TCH\r\n"

						 "%d Manual blocks , %d locked blocks , ratio %f\r\n"
						 "%d full lookups , %d fast lookups , ratio %f\r\n"
						 "%d ret cache miss , %d ret cache hits , ratio %f\r\n",
						 stats.block_count,stats.cache_size/1024,
						 stats.manual_blocks,stats.locked_blocks,stats.manual_blocks/(float)(stats.locked_blocks+stats.manual_blocks),
						 stats.full_lookups,stats.fast_lookups,stats.full_lookups/(float)(stats.fast_lookups+stats.full_lookups),
						 ret_cache_total-ret_cache_hits,ret_cache_hits,(ret_cache_total-ret_cache_hits)/(float)(ret_cache_total),
						 0);
						 ret_cache_total=ret_cache_hits=0;
			**/
			SetDlgItemText(hWnd,IDC_PROFTEXT,text_);
		}
		return TRUE;
	

	case WM_COMMAND:
		switch( LOWORD(wParam) )
		{
		case IDC_COPY:
			{
				wchar text_[2048*8];
				GetDlgItemText(hWnd,IDC_PROFTEXT,text_,sizeof(text_)/sizeof(text_[0]));
				CopyTextToClipboard(hWnd,text_);
			}
			return true;

		default: break;
		}
		return false;

	case WM_CLOSE:
	case WM_DESTROY:
		EndDialog(hWnd,0);
		ProfilerWindow=NULL;
		return TRUE;

	default: return FALSE;
	}


}
INT_PTR CALLBACK ArmDlgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg )
	{
	case WM_INITDIALOG:
		return TRUE;

	default: return FALSE;
	}


}
wchar SelectedPlugin_Aica[512]={0};
wchar SelectedPlugin_ARM[512]={0};
wchar SelectedPlugin_ExtDev[512]={0};
wchar SelectedPlugin_Pvr[512]={0};
wchar SelectedPlugin_Gdr[512]={0};
wchar SelectedPlugin_maple[4][6][512]={0};

int IDC_maple[6]=
{
	IDC_MAPLESUB0,
	IDC_MAPLESUB1,
	IDC_MAPLESUB2,
	IDC_MAPLESUB3,
	IDC_MAPLESUB4,
	IDC_MAPLEMAIN,
};

int current_maple_port=0;
void InitMaplePorts(HWND hw)
{
	int idef=0;
	int i2 = ComboBox_AddString(hw,L"A"); 
	ComboBox_SetItemData(hw, i2, 0); 
	idef=i2;

	i2 = ComboBox_AddString(hw,L"B"); 
	ComboBox_SetItemData(hw, i2, 1); 

	i2 = ComboBox_AddString(hw,L"C"); 
	ComboBox_SetItemData(hw, i2, 2); 

	i2 = ComboBox_AddString(hw,L"D"); 
	ComboBox_SetItemData(hw, i2, 3); 
	
	ComboBox_SetCurSel(hw,idef);
}
void SetSelected(HWND hw,wchar* selected)
{
	int item_count=ComboBox_GetCount(hw);
	for (int i=0;i<item_count;i++)
	{
		wchar * it=(wchar*)ComboBox_GetItemData(hw,i);
		if (wcsicmp(it,selected)==0)
		{
			ComboBox_SetCurSel(hw,i);
			return;
		}
	}
	ComboBox_SetCurSel(hw,0);
}

void AddItemsToCB(PluginInfoList* list,HWND hw,wchar* selected)
{
		while(list)
		{
			wchar temp[512];
			wchar* dll=_tcsrchr(list->dll,L'\\');  
			if (dll==0)
				dll=list->dll;
			
			swprintf(temp,L"%s (%s)",list->Name,dll);
			
			size_t dll_len=wcslen(dll);
			wchar* lp = (wchar *)malloc((dll_len+1)*sizeof(wchar)); 
			_tcscpy(lp,dll);
			int i2 = ComboBox_AddString(hw, temp); 
			ComboBox_SetItemData(hw, i2, lp); 
			list=list->next;
		}
		SetSelected(hw,selected);
}

void AddMapleItemsToCB(PluginInfoList* list,HWND hw,wchar* selected)
{
		wchar temp[512]=L"None";
		wchar dll[512]=L"NULL";

		wchar* lp = (wchar * )malloc(5*sizeof(wchar)); 
		wcscpy(lp,dll);
		
		int i2 = ComboBox_AddString(hw, temp); 
		ComboBox_SetItemData(hw, i2, lp); 

		AddItemsToCB(list,hw,selected);
}

void GetCurrent(HWND hw,wchar* dest)
{
	int sel=ComboBox_GetCurSel(hw);
	wchar* source=(wchar*)ComboBox_GetItemData(hw,sel);
	if (source==0 || source==(((wchar*)(-1))))
		source=L"";
	wcscpy(dest,source);
}

void SetMapleMain_Mask(wchar* plugin,HWND hWnd)
{
	if (wcscmp(plugin,L"NULL")==0)
	{
		for (int j=0;j<5;j++)
		{
			SetSelected(GetDlgItem(hWnd,IDC_maple[j]),L"NULL");
			ComboBox_Enable(GetDlgItem(hWnd,IDC_maple[j]),FALSE);
		}
	}
	else
	{
		PluginInfoList* lst=emu.GetMapleDeviceList(MDT_Main);
		PluginInfoList* ci=lst;
		while(ci)
		{
			if (_tcsicmp(plugin,ci->dll)==0)
			{
				for (int j=0;j<5;j++)
				{
					if (ci->Flags & (1<<j))
					{
						ComboBox_Enable(GetDlgItem(hWnd,IDC_maple[j]),TRUE);
					}
					else
					{
						SetSelected(GetDlgItem(hWnd,IDC_maple[j]),L"NULL");
						ComboBox_Enable(GetDlgItem(hWnd,IDC_maple[j]),FALSE);
					}
				}
				break;
			}
			ci=ci->next;
		}
		
		if (ci==0)
		{
			for (int j=0;j<5;j++)
			{
				SetSelected(GetDlgItem(hWnd,IDC_maple[j]),L"NULL");
				ComboBox_Enable(GetDlgItem(hWnd,IDC_maple[j]),FALSE);
			}
		}
		emu.FreePluginList(lst);
	}
}
void UpdateMapleSelections(HWND hw,HWND hWnd)
{
	LRESULT new_port=TabCtrl_GetCurSel(hw);

	//save selected ones
	if (current_maple_port!=-1)
	{
		for (int j=0;j<6;j++)
		{
			GetCurrent(GetDlgItem(hWnd,IDC_maple[j]),SelectedPlugin_maple[current_maple_port][j]);
		}
	}
	//load new ones
	for (int j=0;j<6;j++)
	{
		SetSelected(GetDlgItem(hWnd,IDC_maple[j]),SelectedPlugin_maple[new_port][j]);
	}
	SetMapleMain_Mask(SelectedPlugin_maple[new_port][5],hWnd);
	current_maple_port=new_port;
}
void SaveMaple()
{
	u32 base=NDCS_PLUGIN_MAPLE_0_0;
	for (int i=0;i<4;i++)
	{
		for (int j=0;j<6;j++)
		{
			emu.SetSetting(base,SelectedPlugin_maple[i][j]);
			base++;
		}
	}
}

void LoadMaple()
{
	u32 base=NDCS_PLUGIN_MAPLE_0_0;
	for (int i=0;i<4;i++)
	{
		for (int j=0;j<6;j++)
		{
			emu.GetSetting(base,SelectedPlugin_maple[i][j]);
			base++;
		}
	}
}
INT_PTR CALLBACK PluginDlgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg )
	{
	case WM_INITDIALOG:
		{

		TCITEM tci; 
		tci.mask = TCIF_TEXT | TCIF_IMAGE; 
		tci.iImage = -1; 
		tci.pszText = L"Port A"; 
		TabCtrl_InsertItem(GetDlgItem(hWnd,IDC_MAPLETAB), 0, &tci); 
		tci.pszText = L"Port B"; 
		TabCtrl_InsertItem(GetDlgItem(hWnd,IDC_MAPLETAB), 1, &tci); 
		tci.pszText = L"Port C"; 
		TabCtrl_InsertItem(GetDlgItem(hWnd,IDC_MAPLETAB), 2, &tci); 
		tci.pszText = L"Port D"; 
		TabCtrl_InsertItem(GetDlgItem(hWnd,IDC_MAPLETAB), 3, &tci); 

		current_maple_port=-1;
		
		PluginInfoList* pvr= emu.GetPluginList(Plugin_PowerVR);	
		PluginInfoList* gdrom= emu.GetPluginList(Plugin_GDRom);
		PluginInfoList* aica= emu.GetPluginList(Plugin_AICA);
		PluginInfoList* arm= emu.GetPluginList(Plugin_ARM);
		PluginInfoList* extdev= emu.GetPluginList(Plugin_ExtDevice);

		PluginInfoList* MapleMain=emu.GetMapleDeviceList(MDT_Main);
		PluginInfoList* MapleSub=emu.GetMapleDeviceList(MDT_Sub);

		wchar temp[512];

		
		emu.GetSetting(NDCS_PLUGIN_PVR,temp);
		AddItemsToCB(pvr,GetDlgItem(hWnd,IDC_C_PVR),temp);
		GetCurrent(GetDlgItem(hWnd,IDC_C_PVR),SelectedPlugin_Pvr);

		emu.GetSetting(NDCS_PLUGIN_GDR,temp);
		AddItemsToCB(gdrom,GetDlgItem(hWnd,IDC_C_GDR),temp);
		GetCurrent(GetDlgItem(hWnd,IDC_C_GDR),SelectedPlugin_Gdr);

		emu.GetSetting(NDCS_PLUGIN_AICA,temp);
		AddItemsToCB(aica,GetDlgItem(hWnd,IDC_C_AICA),temp);
		GetCurrent(GetDlgItem(hWnd,IDC_C_AICA),SelectedPlugin_Aica);

		emu.GetSetting(NDCS_PLUGIN_ARM,temp);
		AddItemsToCB(arm,GetDlgItem(hWnd,IDC_C_ARM),temp);
		GetCurrent(GetDlgItem(hWnd,IDC_C_ARM),SelectedPlugin_ARM);

		emu.GetSetting(NDCS_PLUGIN_EXTDEV,temp);
		AddItemsToCB(extdev,GetDlgItem(hWnd,IDC_C_EXTDEV),temp);
		GetCurrent(GetDlgItem(hWnd,IDC_C_EXTDEV),SelectedPlugin_ExtDev);

		AddMapleItemsToCB(MapleMain,GetDlgItem(hWnd,IDC_MAPLEMAIN),L"NULL");

		AddMapleItemsToCB(MapleSub,GetDlgItem(hWnd,IDC_MAPLESUB0),L"NULL");
		AddMapleItemsToCB(MapleSub,GetDlgItem(hWnd,IDC_MAPLESUB1),L"NULL");
		AddMapleItemsToCB(MapleSub,GetDlgItem(hWnd,IDC_MAPLESUB2),L"NULL");
		AddMapleItemsToCB(MapleSub,GetDlgItem(hWnd,IDC_MAPLESUB3),L"NULL");
		AddMapleItemsToCB(MapleSub,GetDlgItem(hWnd,IDC_MAPLESUB4),L"NULL");

		emu.FreePluginList(pvr);
		emu.FreePluginList(gdrom);
		emu.FreePluginList(aica);
		emu.FreePluginList(MapleMain);
		emu.FreePluginList(MapleSub);

		LoadMaple();
		InitMaplePorts(GetDlgItem(hWnd,IDC_MAPLETAB));
		UpdateMapleSelections(GetDlgItem(hWnd,IDC_MAPLETAB),hWnd);
		
		}
		return true;
		
	case WM_COMMAND:

		switch( LOWORD(wParam) )
		{
		case IDC_C_AICA:
			if (HIWORD(wParam)==CBN_SELCHANGE)
				GetCurrent(GetDlgItem(hWnd,IDC_C_AICA),SelectedPlugin_Aica);
			break;
		case IDC_C_ARM:
			if (HIWORD(wParam)==CBN_SELCHANGE)
				GetCurrent(GetDlgItem(hWnd,IDC_C_ARM),SelectedPlugin_ARM);
			break;
		case IDC_C_GDR:
			if (HIWORD(wParam)==CBN_SELCHANGE)
				GetCurrent(GetDlgItem(hWnd,IDC_C_GDR),SelectedPlugin_Gdr);
			break;
		case IDC_C_PVR:
			if (HIWORD(wParam)==CBN_SELCHANGE)
				GetCurrent(GetDlgItem(hWnd,IDC_C_PVR),SelectedPlugin_Pvr);
			break;
		case IDC_MAPLEPORT:
			if (HIWORD(wParam)==CBN_SELCHANGE)
				UpdateMapleSelections(GetDlgItem(hWnd,IDC_MAPLETAB),hWnd);
			break;
		case IDC_MAPLEMAIN:
			if (HIWORD(wParam)==CBN_SELCHANGE)
			{
				wchar temp[512];
				GetCurrent(GetDlgItem(hWnd,IDC_MAPLEMAIN),temp);
				SetMapleMain_Mask(temp,hWnd);
			}
			break;

		case IDC_C_EXTDEV:
			if (HIWORD(wParam)==CBN_SELCHANGE)
				GetCurrent(GetDlgItem(hWnd,IDC_C_EXTDEV),SelectedPlugin_ExtDev);
			break;

		case IDOK:
			//save settings
			emu.SetSetting(NDCS_PLUGIN_PVR,SelectedPlugin_Pvr);
			emu.SetSetting(NDCS_PLUGIN_GDR,SelectedPlugin_Gdr);
			emu.SetSetting(NDCS_PLUGIN_AICA,SelectedPlugin_Aica);
			emu.SetSetting(NDCS_PLUGIN_ARM,SelectedPlugin_ARM);
			emu.SetSetting(NDCS_PLUGIN_EXTDEV,SelectedPlugin_ExtDev);

			UpdateMapleSelections(GetDlgItem(hWnd,IDC_MAPLEPORT),hWnd);
			SaveMaple();
			plgdlg_cancel=false;
			EndDialog(hWnd,0);
			return true;

		case IDCANCEL://close plugin
			plgdlg_cancel=true;
			EndDialog(hWnd,0);
			return true;

		default: break;
		}
		return false;
	case WM_NOTIFY:
		{
			if ( ((LPNMHDR)lParam)->idFrom==IDC_MAPLETAB && 
				 ((LPNMHDR)lParam)->code == TCN_SELCHANGE  )
			{
				UpdateMapleSelections(GetDlgItem(hWnd,IDC_MAPLETAB),hWnd);
			}
			return true;
		}
	case WM_CLOSE:
	case WM_DESTROY:
		EndDialog(hWnd,0);
		return true;

	default: break;
	}

	return false;
}