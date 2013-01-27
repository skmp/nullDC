#include "nullDC_GUI.h"

HMODULE hMod;
wchar emu_name[128];
gui_emu_info emu;
Settings_Struct settings;

void SaveSettings()
{
	emu.ConfigSaveInt(L"nullDC_GUI",L"AutoHideMenu",settings.AutoHideMenu);
	emu.ConfigSaveInt(L"nullDC_GUI",L"Fullscreen",settings.Fullscreen);
	emu.ConfigSaveInt(_T("nullDC_GUI"),_T("AlwaysOnTop"),settings.AlwaysOnTop);
}

void LoadSettings()
{
	settings.AutoHideMenu=emu.ConfigLoadInt(L"nullDC_GUI",L"AutoHideMenu",1) ? true : false;
	settings.Fullscreen=emu.ConfigLoadInt(L"nullDC_GUI",L"Fullscreen",0) ? true : false;
	settings.AlwaysOnTop=emu.ConfigLoadInt(L"nullDC_GUI",L"AlwaysOnTop",0) ? true : false;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		hMod=hModule; 
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}
int EXPORT_CALL guiMsgBox(wchar* text,int type)
{
	return MessageBox(NULL,text,emu_name,type|MB_TASKMODAL);
}
void gdipInit();
void gdipTerm();
s32 EXPORT_CALL Load(gui_emu_info* emui)
{
	memcpy(&emu,emui,sizeof(emu));
	
	emu.ConfigLoadStr(L"emu",L"FullName",emu_name,L"");
	LoadSettings();
	if (!uiInit())
		return rv_serror;
	gdipInit();
	return rv_ok;
}
void EXPORT_CALL GetMenuIDs(MenuIDList* mid)
{
	mid->PowerVR=PowerVR_menu;
	mid->GDRom=GDRom_menu;
	mid->Aica=Aica_menu;
	mid->Arm=Arm_menu;
	mid->Maple=Maple_menu;
	
	for (int i=0;i<4;i++) for (int j=0;j<6;j++)
		mid->Maple_port[i][j] =Maple_menu_ports[i][j];

	mid->ExtDev=ExtDev_menu;
	mid->Debug=Debug_menu;
}
void EXPORT_CALL Unload()
{
	uiTerm();
	gdipTerm();
}
void EXPORT_CALL Mainloop()
{
	uiMain();
}

void* EXPORT_CALL GetRTWH() { return g_hWnd; }
void EXPORT_CALL gDeleteAllMenuItemChilds(u32 id)
{
	DeleteAllMenuItemChilds(id);
}
void EXPORT_CALL EventHandler(u32 nid,void* p)
{
	//lota work no ?
}
void EXPORT_CALL ndcGetInterface(gui_plugin_info* info)
{
	info->InterfaceVersion=GuiPluginInterfaceVersion;
	wcscpy(info->Name,L"Default Gui");
	
	info->Load=Load;
	info->Unload=Unload;
	info->Mainloop=Mainloop;

	info->MsgBox=guiMsgBox;
	info->AddMenuItem=AddMenuItem;
	info->SetMenuItemStyle=SetMenuItemStyle;
	info->SetMenuItem=SetMenuItem;
	info->GetMenuItem=GetMenuItem;
	info->DeleteMenuItem=DeleteMenuItem;
	info->GetMenuIDs=GetMenuIDs;
	info->GetRenderTarget=GetRTWH;
	info->SelectPluginsGui=SelectPluginsGui;
	info->DeleteAllMenuItemChilds=gDeleteAllMenuItemChilds;

	info->EventHandler=EventHandler;
}
u32 ReadMem32(u32 addr)
{
	u32 rv;
	emu.dbgReadMem(addr,4,&rv);
	return rv;
}
u16 ReadMem16(u32 addr)
{
	u16 rv;
	emu.dbgReadMem(addr,2,&rv);
	return rv;
}
u8 ReadMem8(u32 addr)
{
	u8 rv;
	emu.dbgReadMem(addr,1,&rv);
	return rv;
}

void WriteMem32(u32 addr,u32 data)
{
	emu.dbgWriteMem(addr,4,&data);
}
void WriteMem16(u32 addr,u16 data)
{
	emu.dbgWriteMem(addr,2,&data);
}
void WriteMem8(u32 addr,u8 data)
{
	emu.dbgWriteMem(addr,1,&data);
}


bool EmuStarted()
{
	return emu.EmuStarted();
}

bool EmuInit()
{
	return emu.EmuInit();
}

void EmuStart()
{
	emu.EmuStart();
}
void EmuStop()
{
	emu.EmuStop();
}
void EmuStep()
{
	emu.EmuStep();
}
void EmuSkip()
{
	emu.EmuSkip();
}

void EmuSetPatch(u32 Value,u32 Mask)
{
	emu.EmuSetPatch(Value,Mask);
}
void EmuReset(bool Manual)
{
	emu.EmuReset(Manual);
}

bool EmuBootHLE()
{
	return emu.EmuBootHLE();
}
bool EmuLoadBinary(wchar* file,u32 address)
{
	return emu.EmuLoadBinary(file,address);
}

bool EmuSelectPlugins()
{
	return emu.EmuSelectPlugins();
}
void EmuStartProfiler()
{
	emu.EmuStartProfiler();
}
void EmuStopProfiler()
{
	emu.EmuStopProfiler();
}

void DissasembleOpcode(u16 opcode,u32 pc,wchar* Dissasm)
{
	emu.DissasembleOpcode(opcode,pc,Dissasm);
}

u32 Sh4GetRegister(Sh4RegType reg)
{
	return emu.Sh4GetRegister(reg);
}
void Sh4SetRegister(Sh4RegType reg,u32 value)
{
	emu.Sh4SetRegister(reg,value);
}

int GetSymbName(u32 address,wchar *szDesc,bool bUseUnkAddress)
{
	return emu.GetSymbName(address,szDesc,bUseUnkAddress);
}
int msgboxf(wchar* text,unsigned int type,...)
{
	va_list args;

	wchar temp[2048];
	va_start(args, type);
	vswprintf(temp, text, args);
	va_end(args);

	return guiMsgBox(temp,type | MB_TASKMODAL);
}

u32 GetSettingI(u32 id)
{
	int t;

	if (emu.GetSetting(id,&t)!=rv_ok)
	{
		dbgbreak;
	}
	return t;
}
void SetSettingI(u32 id,u32 v)
{
	if (emu.SetSetting(id,&v)!=rv_ok)
	{
		dbgbreak;
	}
}