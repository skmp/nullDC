// nullDC.cpp : Makes magic cookies
//

//initialse Emu
#include "types.h"
#include "dc/mem/_vmem.h"
#include "stdclass.h"
#include "dc/dc.h"
#include "gui/base.h"
#include "config/config.h"
#define _WIN32_WINNT 0x0500 
#include <windows.h>
#include "plugins/plugin_manager.h"
#include "serial_ipc/serial_ipc_client.h"
#include "cl/cl.h"
#include "emitter/emitter.h"

__settings settings;

BOOL CtrlHandler( DWORD fdwCtrlType ) 
{ 
  switch( fdwCtrlType ) 
  { 
	  case CTRL_SHUTDOWN_EVENT: 
	  case CTRL_LOGOFF_EVENT: 
	  // Pass other signals to the next handler. 
    case CTRL_BREAK_EVENT: 
	  // CTRL-CLOSE: confirm that the user wants to exit. 
    case CTRL_CLOSE_EVENT: 
    // Handle the CTRL-C signal. 
    case CTRL_C_EVENT: 
		SendMessageA((HWND)GetRenderTargetHandle(),WM_CLOSE,0,0); //FIXME
      return( TRUE );
    default: 
      return FALSE; 
  } 
} 

//Simple command line bootstrap
int RunDC(int argc, wchar* argv[])
{

	if(settings.dynarec.Enable)
	{
		sh4_cpu=Get_Sh4Recompiler();
		log("Using Recompiler\n");
	}
	else
	{
		sh4_cpu=Get_Sh4Interpreter();
		log("Using Interpreter\n");
	}
	
	if (settings.emulator.AutoStart)
		Start_DC();

	GuiLoop();

	Term_DC();
	Release_Sh4If(sh4_cpu);
	return 0;
}


void EnumPlugins()
{
	EnumeratePlugins();

	vector<PluginLoadInfo>* pvr= GetPluginList(Plugin_PowerVR);
	vector<PluginLoadInfo>* gdrom= GetPluginList(Plugin_GDRom);
	vector<PluginLoadInfo>* aica= GetPluginList(Plugin_AICA);
	vector<PluginLoadInfo>* arm= GetPluginList(Plugin_ARM);
	vector<PluginLoadInfo>* maple= GetPluginList(Plugin_Maple);
	vector<PluginLoadInfo>* extdev= GetPluginList(Plugin_ExtDevice);

	log("PowerVR plugins :\n");
	for (u32 i=0;i<pvr->size();i++)
	{
		wprintf(L"*\tFound %s\n" ,(*pvr)[i].Name);
	}

	log("\nGDRom plugins :\n");
	for (u32 i=0;i<gdrom->size();i++)
	{
		wprintf(L"*\tFound %s\n" ,(*gdrom)[i].Name);
	}

	
	log("\nAica plugins :\n");
	for (u32 i=0;i<aica->size();i++)
	{
		wprintf(L"*\tFound %s\n" ,(*aica)[i].Name);
	}

	log("\nArm plugins :\n");
	for (u32 i=0;i<arm->size();i++)
	{
		wprintf(L"*\tFound %s\n" ,(*arm)[i].Name);
	}

	log("\nMaple plugins :\n");
	for (u32 i=0;i<maple->size();i++)
	{
		wprintf(L"*\tFound %s\n" ,(*maple)[i].Name);
	}
	log("\nExtDevice plugins :\n");
	for (u32 i=0;i<extdev->size();i++)
	{
		wprintf(L"*\tFound %s\n" ,(*extdev)[i].Name);
	}

	delete pvr;
	delete gdrom;
	delete aica;
	delete maple;
	delete extdev;
}

int main___(int argc,wchar* argv[])
{
	if(ParseCommandLine(argc,argv))
	{
		log("\n\n(Exiting due to command line, without starting nullDC)\n");
		return 69;
	}

	if(!cfgOpen())
	{
		msgboxf(_T("Unable to open config file"),MBX_ICONERROR);
		return -4;
	}
	LoadSettings();

	if (settings.emulator.NoConsole)
	{
		FreeConsole();
	}

	if (!CreateGUI())
	{
		msgboxf(_T("Creating GUI failed\n"),MBX_ICONERROR);
		return -1;
	}
	int rv= 0;

	wchar* temp_path=GetEmuPath(_T("data\\"));

	bool lrf=LoadRomFiles(temp_path);

	free(temp_path);

	if (!lrf)
	{
		rv=-3;
		goto cleanup;
	}

	wchar * currpath=GetEmuPath(L"");
	SetCurrentDirectory(currpath);
	free(currpath);

	EnumPlugins();

	while (!plugins_Load())
	{
		if (!plugins_Select())
		{
			msgboxf(L"Unable to load plugins -- exiting\n",MBX_ICONERROR);
			rv = -2;
			goto cleanup;
		}
	}
	
	rv = RunDC(argc,argv);
	
cleanup:
	DestroyGUI();
	
	SaveSettings();
	return rv;
}

int _tmain(int argc, wchar* argv[])
{
	x86_caps.detect();
	if (!_vmem_reserve())
	{
		msgboxf(L"Unable to reserve nullDC memory ...",MBX_OK | MBX_ICONERROR);
		return -5;
	}
	int rv=0;

	__try
	{
		rv=main___(argc,argv);
	}
	__except( ExeptionHandler( GetExceptionCode(), (GetExceptionInformation()) ) )
	{

	}

	_vmem_release();
	return rv;
}

extern int nCmdShow;
int CALLBACK WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShowCmd)
{
	int argc=0;
	nCmdShow=nCmdShowCmd;
	wchar* cmd_line=GetCommandLineW();
	wchar** argv=CommandLineToArgvW(cmd_line,&argc);
	if(wcsstr(cmd_line,L"NoConsole")==0)
	{
		if (AllocConsole())
		{
			freopen("CON","w",stdout);
			freopen("CON","w",stderr);
			freopen("CON","r",stdin);
		}
		SetConsoleCtrlHandler( (PHANDLER_ROUTINE) CtrlHandler, TRUE );
	}
	
	return _tmain(argc,argv);
}

void LoadSettings()
{
	settings.dynarec.Enable=cfgLoadInt(L"nullDC",L"Dynarec.Enabled",1)!=0;
	settings.dynarec.CPpass=cfgLoadInt(L"nullDC",L"Dynarec.DoConstantPropagation",1)!=0;
	settings.dynarec.Safe=cfgLoadInt(L"nullDC",L"Dynarec.SafeMode",1)!=0;
	settings.dynarec.UnderclockFpu=cfgLoadInt(L"nullDC",L"Dynarec.UnderclockFpu",0)!=0;
	
	settings.dreamcast.cable=cfgLoadInt(L"nullDC",L"Dreamcast.Cable",3);
	settings.dreamcast.RTC=cfgLoadInt(L"nullDC",L"Dreamcast.RTC",GetRTC_now());

	settings.dreamcast.region=cfgLoadInt(L"nullDC",L"Dreamcast.Region",3);
	settings.dreamcast.broadcast=cfgLoadInt(L"nullDC",L"Dreamcast.Broadcast",4);

	settings.emulator.AutoStart=cfgLoadInt(L"nullDC",L"Emulator.AutoStart",0)!=0;
	settings.emulator.NoConsole=cfgLoadInt(L"nullDC",L"Emulator.NoConsole",0)!=0;

	//make sure values are valid
	settings.dreamcast.cable=min(max(settings.dreamcast.cable,0),3);
	settings.dreamcast.region=min(max(settings.dreamcast.region,0),3);
	settings.dreamcast.broadcast=min(max(settings.dreamcast.broadcast,0),4);
}
void SaveSettings()
{
	cfgSaveInt(L"nullDC",L"Dynarec.Enabled",settings.dynarec.Enable);
	cfgSaveInt(L"nullDC",L"Dynarec.DoConstantPropagation",settings.dynarec.CPpass);
	cfgSaveInt(L"nullDC",L"Dynarec.SafeMode",settings.dynarec.Safe);
	cfgSaveInt(L"nullDC",L"Dynarec.UnderclockFpu",settings.dynarec.UnderclockFpu);
	cfgSaveInt(L"nullDC",L"Dreamcast.Cable",settings.dreamcast.cable);
	cfgSaveInt(L"nullDC",L"Dreamcast.RTC",settings.dreamcast.RTC);
	cfgSaveInt(L"nullDC",L"Dreamcast.Region",settings.dreamcast.region);
	cfgSaveInt(L"nullDC",L"Dreamcast.Broadcast",settings.dreamcast.broadcast);
	cfgSaveInt(L"nullDC",L"Emulator.AutoStart",settings.emulator.AutoStart);
	cfgSaveInt(L"nullDC",L"Emulator.NoConsole",settings.emulator.NoConsole);
}