// nullAICA.cpp : Defines the entry point for the DLL application.
//

#include "vbaARM.h"
#include "aica.h"
#include "arm7.h"
#include "mem.h"
#include <tchar.h>

arm_init_params arm_params;
emu_info eminf;
HINSTANCE hinst;
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	hinst=(HINSTANCE)hModule;
    return TRUE;
}
void EXPORT_CALL handle_About(u32 id,void* w,void* p)
{
	MessageBoxA((HWND)w,"Made by the VBA Team\r\nPorted by drk||Raziel","About the VBA ARM Core...",MB_ICONINFORMATION);
}
s32 FASTCALL OnLoad(emu_info* em)
{
	memcpy(&eminf,em,sizeof(eminf));

	LoadSettings();
	eminf.AddMenuItem(em->RootMenu,-1,L"About",handle_About,0);
	return rv_ok;
}

void FASTCALL OnUnload()
{
}

//called when plugin is used by emu (you should do first time init here)
s32 FASTCALL Init(arm_init_params* initp)
{
	memcpy(&arm_params,initp,sizeof(arm_params));

	init_mem();
	arm_Init();

	return rv_ok;
}

//called when plugin is unloaded by emu , olny if dcInit is called (eg , not called to enumerate plugins)
void FASTCALL Term()
{
	term_mem();
	//arm7_Term ?
}

//It's suposed to reset anything 
void FASTCALL Reset(bool Manual)
{
	arm_Reset();
}

void FASTCALL SetResetState(u32 state)
{
	arm_SetEnabled(state==0);
}
//Give to the emu pointers for the PowerVR interface
EXPORT void EXPORT_CALL dcGetInterface(plugin_interface* info)
{
	info->InterfaceVersion=PLUGIN_I_F_VERSION;

#define c info->common
#define a info->arm

	wcscpy(c.Name,L"VBA ARM Sound Cpu Core [" _T(__DATE__) L"]");

	c.InterfaceVersion=ARM_PLUGIN_I_F_VERSION;
	c.Type=Plugin_ARM;

	c.Load=OnLoad;
	c.Unload=OnUnload;

	a.Init=Init;
	a.Reset=Reset;
	a.Term=Term;

	a.Update=UpdateARM;
	a.ArmInterruptChange=ArmInterruptChange;
	a.ExeptionHanlder=0;
	a.SetResetState=SetResetState;
}

int cfgGetInt(char* key,int def)
{
	wchar t[512];
	mbstowcs(t,key,512);
	return eminf.ConfigLoadInt(L"nullAica",t,def);
}
void cfgSetInt(char* key,int def)
{
	wchar t[512];
	mbstowcs(t,key,512);
	eminf.ConfigSaveInt(L"nullAica",t,def);
}

void LoadSettings()
{
	//load default settings before init
	//settings.BufferSize=cfgGetInt("BufferSize",1024);
}

void SaveSettings()
{
	
}