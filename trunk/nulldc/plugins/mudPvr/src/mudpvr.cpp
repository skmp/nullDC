/*
	Plugin entrypoints and exports
	GUI
*/
#include "mudPvr.h"
#include "ta.h"

emu_info emu;


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}

void EXPORT_CALL handle_About(u32 id,void* w,void* p)
{
	MessageBox((HWND)w,L"Made by mudlord.\nThanks to nullDC Team.",L"About nullDC PVR (DX11)...",MB_ICONINFORMATION);
}



void EXPORT_CALL EventHandler(u32 id,void*p)
{
	
}

//called when plugin is unloaded by emulator
void FASTCALL Unload()
{

}

//called when plugin is used by emulator (first time setup)
s32 FASTCALL Load(emu_info* emu_inf)
{
	return rv_ok;

}


void FASTCALL ResetPvr(bool Manual)
{
}

//called when entering sh4 thread , from the new thread context (for any thread speciacific init)
s32 FASTCALL InitPvr(pvr_init_params* param)
{
	return rv_ok;
}

//called when exiting from sh4 thread , from the new thread context (for any thread speciacific de init) :P
void FASTCALL TermPvr()
{
}

u32 FASTCALL ReadPvrRegister(u32 addr,u32 size)
{
	return 0;
}

void FASTCALL WritePvrRegister(u32 paddr,u32 data,u32 size)
{
}

void FASTCALL spgUpdatePvr(u32 cycles)
{
}

void FASTCALL vramLockCB (vram_block* block,u32 addr)
{
}

void EXPORT_CALL dcGetInterface(plugin_interface* info)
{
	#define c  info->common
	#define p info->pvr
	info->InterfaceVersion=PLUGIN_I_F_VERSION;
	c.Type=Plugin_PowerVR;
	c.InterfaceVersion=PVR_PLUGIN_I_F_VERSION;
	wcscpy(c.Name,L"nullDC PowerVR (D3D11) -- " L" [" _T(__DATE__) L"]");
	c.Load=Load;
	c.Unload=Unload;
	c.EventHandler=EventHandler;
	p.Init=InitPvr;
	p.Reset=ResetPvr;
	p.Term=TermPvr;
	p.ReadReg=ReadPvrRegister;
	p.WriteReg=WritePvrRegister;
	p.UpdatePvr=spgUpdatePvr;
	p.TaDMA=TASplitter::Dma;
	p.TaSQ=TASplitter::SQ;
	p.LockedBlockWrite=vramLockCB;

	#undef c
	#undef p
}

//Helper functions
float GetSeconds()
{
	//return timeGetTime()/1000.0f;
	return 0.0;
}

int msgboxf(wchar* text,unsigned int type,...)
{
	va_list args;
	wchar temp[2048];
	va_start(args, type);
	vswprintf(temp,2048, text, args);
	va_end(args);
	return MessageBox(NULL,temp,L"nullDC",type | MB_TASKMODAL);
}