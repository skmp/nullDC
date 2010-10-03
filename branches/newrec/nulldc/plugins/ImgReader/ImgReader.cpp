// nullGDR.cpp : Defines the entry point for the DLL application.
//

#include "ImgReader.h"
//Get a copy of the operators for structs ... ugly , but works :)
#include "common.h"

HINSTANCE hInstance;
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		hInstance=(HINSTANCE)hModule;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}
_setts settings;
int cfgGetInt(wchar* key,int def)
{
	return emu.ConfigLoadInt(L"ImageReader",key,def);
}
void cfgSetInt(wchar* key,int v)
{
	emu.ConfigSaveInt(L"ImageReader",key,v);
}
void cfgGetStr(wchar* key,wchar* v,const wchar*def)
{
	emu.ConfigLoadStr(L"ImageReader",key,v,def);
}
void cfgSetStr(wchar* key,const wchar* v)
{
	emu.ConfigSaveStr(L"ImageReader",key,v);
}
void LoadSettings()
{
	settings.PatchRegion=cfgGetInt(L"PatchRegion",0)!=0;
	settings.LoadDefaultImage=cfgGetInt(L"LoadDefaultImage",0)!=0;
	cfgGetStr(L"DefaultImage",settings.DefaultImage,L"defualt.gdi");
	cfgGetStr(L"LastImage",settings.LastImage,L"c:\\game.gdi");
}
void SaveSettings()
{
	cfgSetInt(L"PatchRegion",settings.PatchRegion);
	cfgSetInt(L"LoadDefaultImage",settings.LoadDefaultImage);
	cfgSetStr(L"DefaultImage",settings.DefaultImage);
	cfgSetStr(L"LastImage",settings.LastImage);
}
#define PLUGIN_NAME "Image Reader plugin by drk||Raziel & GiGaHeRz [" __DATE__ "]"
#define PLUGIN_NAMEW L"Image Reader plugin by drk||Raziel & GiGaHeRz [" _T(__DATE__) L"]"

void FASTCALL GetSessionInfo(u8* out,u8 ses);

void FASTCALL DriveReadSubChannel(u8 * buff, u32 format, u32 len)
{
	if (format==0)
	{
		memcpy(buff,q_subchannel,len);
	}
}

void FASTCALL DriveReadSector(u8 * buff,u32 StartSector,u32 SectorCount,u32 secsz)
{
	GetDriveSector(buff,StartSector,SectorCount,secsz);
	//if (CurrDrive)
	//	CurrDrive->ReadSector(buff,StartSector,SectorCount,secsz);
}

void FASTCALL DriveGetTocInfo(u32* toc,u32 area)
{
	
		GetDriveToc(toc,(DiskArea)area);
}
//TODO : fix up
u32 FASTCALL DriveGetDiscType()
{
	if (disc)
		return disc->type;
	else
		return NullDriveDiscType;
}

void FASTCALL GetSessionInfo(u8* out,u8 ses)
{
	GetDriveSessionInfo(out,ses);
}
emu_info emu;
wchar emu_name[512];
void EXPORT_CALL handle_PatchRegion(u32 id,void* w,void* p)
{
	if (settings.PatchRegion)
		settings.PatchRegion=0;
	else
		settings.PatchRegion=1;

	emu.SetMenuItemStyle(id,settings.PatchRegion?MIS_Checked:0,MIS_Checked);

	SaveSettings();
}

void EXPORT_CALL handle_UseDefImg(u32 id,void* w,void* p)
{
	if (settings.LoadDefaultImage)
		settings.LoadDefaultImage=0;
	else
		settings.LoadDefaultImage=1;

	emu.SetMenuItemStyle(id,settings.LoadDefaultImage?MIS_Checked:0,MIS_Checked);

	SaveSettings();
}
void EXPORT_CALL handle_SelDefImg(u32 id,void* w,void* p)
{
	if (GetFile(settings.DefaultImage,0,1)==1)//no "no disk"
	{
		SaveSettings();
	}
}
void EXPORT_CALL handle_About(u32 id,void* w,void* p)
{
	MessageBox((HWND)w,L"Made by drk||Raziel & GiGaHeRz",L"About ImageReader...",MB_ICONINFORMATION);
}
void EXPORT_CALL handle_SwitchDisc(u32 id,void* w,void* p)
{
	//msgboxf("This feature is not yet implemented",MB_ICONWARNING);
	//return;
	TermDrive();
	
	NullDriveDiscType=Busy;
	DriveNotifyEvent(DiskChange,0);
	Sleep(150);	//busy for a bit

	NullDriveDiscType=Open;
	DriveNotifyEvent(DiskChange,0);
	Sleep(150); //tray is open

	while(!InitDrive(2))//no "cancel"
		msgboxf(L"Init Drive failed, disc must be valid for swap",0x00000010L);

	DriveNotifyEvent(DiskChange,0);
	//new disc is in
}
//called when plugin is used by emu (you should do first time init here)
s32 FASTCALL Load(emu_info* emu_inf)
{
	if (emu_inf==0)
		return rv_ok;
	memcpy(&emu,emu_inf,sizeof(emu));

	emu.ConfigLoadStr(L"emu",L"shortname",emu_name,0);
	
	LoadSettings();

	emu.AddMenuItem(emu.RootMenu,-1,L"Swap Disc",handle_SwitchDisc,settings.LoadDefaultImage);
	emu.AddMenuItem(emu.RootMenu,-1,0,0,0);
	emu.AddMenuItem(emu.RootMenu,-1,L"Use Default Image",handle_UseDefImg,settings.LoadDefaultImage);
	emu.AddMenuItem(emu.RootMenu,-1,L"Select Default Image",handle_SelDefImg,0);
	emu.AddMenuItem(emu.RootMenu,-1,L"Patch GDROM region",handle_PatchRegion,settings.PatchRegion);
	emu.AddMenuItem(emu.RootMenu,-1,0,0,0);
	emu.AddMenuItem(emu.RootMenu,-1,L"About",handle_About,0);
	
	
	return rv_ok;
}

//called when plugin is unloaded by emu , olny if dcInitGDR is called (eg , not called to enumerate plugins)
void FASTCALL Unload()
{
	
}

//It's suposed to reset everything (if not a manual reset)
void FASTCALL ResetGDR(bool Manual)
{
	DriveNotifyEvent(DiskChange,0);
}

//called when entering sh4 thread , from the new thread context (for any thread speciacific init)
s32 FASTCALL InitGDR(gdr_init_params* prm)
{
	DriveNotifyEvent=prm->DriveNotifyEvent;
	if (!InitDrive())
		return rv_serror;
	DriveNotifyEvent(DiskChange,0);
	LoadSettings();
	return rv_ok;
}

//called when exiting from sh4 thread , from the new thread context (for any thread speciacific de init) :P
void FASTCALL TermGDR()
{
	TermDrive();
}

//Give to the emu pointers for the gd rom interface
void EXPORT_CALL dcGetInterface(plugin_interface* info)
{
#define c info->common
#define g info->gdr
	
	info->InterfaceVersion=PLUGIN_I_F_VERSION;

	c.Type=Plugin_GDRom;
	c.InterfaceVersion=GDR_PLUGIN_I_F_VERSION;

	wcscpy(c.Name,PLUGIN_NAMEW);
	
	c.Load=Load;
	c.Unload=Unload;
	
	
	g.Init=InitGDR;
	g.Term=TermGDR;
	g.Reset=ResetGDR;
	
	g.GetDiscType=DriveGetDiscType;
	g.GetToc=DriveGetTocInfo;
	g.ReadSector=DriveReadSector;
	g.GetSessionInfo=GetSessionInfo;
	g.ReadSubChannel=DriveReadSubChannel;
	g.ExeptionHanlder=0;
}