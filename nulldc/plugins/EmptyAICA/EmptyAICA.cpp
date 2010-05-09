// nullAICA.cpp : Defines the entry point for the DLL application.
//

#include "EmptyAICA.h"
#include "aica_hax.h"
#include "aica_hle.h"


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    return TRUE;
}


void cfgdlg(PluginType type,void* window)
{
	//printf("null AICA plugin [h4x0rs olny kthx]:No config kthx , however i will show ya some hle info :D\n");
	//MessageBox((HWND)window,"Nothing to configure","nullAICA plugin",MB_OK | MB_ICONINFORMATION);
	//ARM_Katana_Driver_Info();
}

s32 FASTCALL PluginLoad(emu_info* param)
{
	return rv_ok;
}

void FASTCALL PluginUnload()
{
}
s32 FASTCALL Init(aica_init_params* param)
{
	aica_ram=param->aica_ram;
	init_mem();
	InitHLE();

	return rv_ok;
}
void FASTCALL Term()
{
	term_mem();
	TermHLE();
}
void FASTCALL Reset(bool Manual)
{
	ResetHLE();
}

//Give to the emu pointers for the PowerVR interface
void EXPORT_CALL dcGetInterface(plugin_interface* info)
{
#define c info->common
#define a info->aica

	info->InterfaceVersion=PLUGIN_I_F_VERSION;

	wcscpy(c.Name,L"Empty Aica Plugin [no sound/reduced compat] [" _T(__DATE__) L"]");

	c.InterfaceVersion=AICA_PLUGIN_I_F_VERSION;
	c.Type=Plugin_AICA;

	c.Load=PluginLoad;
	c.Unload=PluginUnload;

	a.Init=Init;
	a.Reset=Reset;
	a.Term=Term;

	a.Update=UpdateAICA;

	a.ReadMem_aica_reg=ReadMem_reg;
	a.WriteMem_aica_reg=WriteMem_reg;
	
/*
	info->Init=dcInit;
	info->Term=dcTerm;
	info->Reset=dcReset;

	info->ThreadInit=dcThreadInit;
	info->ThreadTerm=dcThreadTerm;
	info->ShowConfig=cfgdlg;
	info->Type=PluginType::AICA;

	info->InterfaceVersion.full=AICA_PLUGIN_I_F_VERSION;

	info->ReadMem_aica_ram=ReadMem_ram;
	info->WriteMem_aica_ram=WriteMem_ram;
	info->ReadMem_aica_reg=ReadMem_reg;
	info->WriteMem_aica_reg=WriteMem_reg;
	info->UpdateAICA=UpdateAICA;*/
}