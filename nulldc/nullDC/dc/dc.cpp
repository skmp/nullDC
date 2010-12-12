//dc.cpp
//emulation driver - interface
#include "mem/sh4_mem.h"
#include "sh4/sh4_opcode_list.h"
#include "pvr/pvr_if.h"
#include "mem/sh4_internal_reg.h"
#include "aica/aica_if.h"
#include "maple/maple_if.h"
#include "dc.h"
#include "config/config.h"
#include "profiler/profiler.h"
#include <string.h>
#include <windows.h>

bool dc_inited=false;
bool dc_reseted=false;
bool dc_ingore_init=false;
bool dc_running=false;
void* hEmuThread;
void _Reset_DC(bool Manual);

enum emu_thread_state_t
{
	EMU_IDLE,
	EMU_CPU_START,
	EMU_SOFTRESET,
	EMU_NOP,
	EMU_QUIT,
	EMU_INIT,
	EMU_TERM,
	EMU_RESET,
	EMU_RESET_MANUAL,
};
enum emu_thread_rv_t
{
	RV_OK = 1,
	RV_ERROR=2,

	RV_EXEPTION=-2,
	RV_WAIT =-1,
};


volatile emu_thread_state_t emu_thread_state=EMU_IDLE;
volatile emu_thread_rv_t emu_thread_rv=RV_WAIT;

cThread* emu_thread=0;

u32 THREADCALL emulation_thead(void* ptar);
struct _______initialzzz__
{	
	_______initialzzz__()
	{
		emu_thread = new cThread(emulation_thead,0);
		emu_thread->Start();
	}
} init_ctorz;
emu_thread_rv_t emu_rtc(emu_thread_state_t cmd)
{
	static MSG msg;

	emu_thread_rv=RV_WAIT;
	emu_thread_state=cmd;
	while(emu_thread_rv==RV_WAIT && cmd!=EMU_SOFTRESET)
	{
		Sleep(10);
		while( PeekMessage(&msg, NULL, 0,0, PM_REMOVE) != 0 )
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return emu_thread_rv;
}
//This is the emulation thread =P
u32 THREADCALL emulation_thead(void* ptar)
{
	emu_thread_state=EMU_IDLE;
	emu_thread_rv=RV_OK;

	DuplicateHandle(GetCurrentProcess(), 
		GetCurrentThread(), 
		GetCurrentProcess(),
		&(HANDLE)hEmuThread, 
		0,
		FALSE,
		DUPLICATE_SAME_ACCESS);
	init_Profiler(hEmuThread);

	while (emu_thread_state!=EMU_QUIT)
	{
		__try
		{
			while(emu_thread_state!=EMU_QUIT)
			{
				switch(emu_thread_state)
				{
				case EMU_IDLE:
					Sleep(100);
					break;

				case EMU_NOP:
					emu_thread_state=EMU_IDLE;

					emu_thread_rv=RV_OK;
					break;

				case EMU_CPU_START:
					emu_thread_state=EMU_IDLE;
					emu_thread_rv=RV_OK;
					sh4_cpu->Run();
					break;
				case EMU_SOFTRESET:
					emu_thread_state=EMU_CPU_START;
					_Reset_DC(true);
					break;

				case EMU_INIT:
					emu_thread_state=EMU_IDLE;

					
					if (!plugins_Init())
					{ 
						//log("Emulation thread : Plugin init failed\n"); 	
						plugins_Term();
						emu_thread_rv=RV_ERROR;
						break;
					}
					sh4_cpu->Init();


					mem_Init();
					pvr_Init();
					aica_Init();
					mem_map_defualt();

					emu_thread_rv=RV_OK;
					break;

				case EMU_TERM:
					emu_thread_state=EMU_IDLE;

					aica_Term();
					pvr_Term();
					mem_Term();
					sh4_cpu->Term();
					plugins_Term();
					

					emu_thread_rv=RV_OK;
					break;

				case EMU_RESET:
					emu_thread_state=EMU_IDLE;

					_Reset_DC(false);

					//when we boot from ip.bin , it's nice to have it seted up
					sh4_cpu->SetRegister(reg_gbr,0x8c000000);
					sh4_cpu->SetRegister(reg_sr,0x700000F0);
					sh4_cpu->SetRegister(reg_fpscr,0x0004001);

					emu_thread_rv=RV_OK;
					break;

				case EMU_RESET_MANUAL:
					emu_thread_state=EMU_IDLE;

					_Reset_DC(true);
					
					//when we boot from ip.bin , it's nice to have it seted up
					sh4_cpu->SetRegister(reg_gbr,0x8c000000);
					sh4_cpu->SetRegister(reg_sr,0x700000F0);
					sh4_cpu->SetRegister(reg_fpscr,0x0004001);

					emu_thread_rv=RV_OK;
					break;

				}
			}

		}
		__except( (EXCEPTION_CONTINUE_EXECUTION==ExeptionHandler( GetExceptionCode(), (GetExceptionInformation()))) ? EXCEPTION_CONTINUE_EXECUTION : EXCEPTION_CONTINUE_SEARCH )
		{
			log("Unhandled exeption ; Emulation thread halted...\n");
			emu_thread_rv= RV_EXEPTION;
		}
	}
	emu_thread_rv=RV_OK;
	term_Profiler();
	CloseHandle(hEmuThread);

	return 0;
}
//called from the new thread
/*void ThreadCallback_DC(bool start)
{
	//call Thread initialisers
	if (start)
	{
		//plugins_ThreadInit();
		GDB_BOOT_TEST();
	}
	//call Thread terminators
	else
	{
		//plugins_ThreadTerm();
	}
}*/


//Init mainly means allocate
//Reset is called before first run
//Init is called olny once
//When Init is called , cpu interface and all plugins configurations myst be finished
//Plugins/Cpu core must not change after this call is made.
bool Init_DC()
{
	if (dc_inited)
		return true;
	/*
	char temp_dll[512];
	//nullDC_PowerVR_plugin* pvrplg=new nullDC_PowerVR_plugin();
	//nullDC_GDRom_plugin* gdrplg=new nullDC_GDRom_plugin();
	//nullDC_AICA_plugin* aicaplg=new nullDC_AICA_plugin();
	//nullDC_ExtDevice_plugin* extdevplg=new nullDC_ExtDevice_plugin();

	//load the selected plugins
	cfgLoadStr("nullDC_plugins","Current_PVR",temp_dll);
	SetPlugin(temp_dll,PluginType::PowerVR);

	cfgLoadStr("nullDC_plugins","Current_GDR",temp_dll);
	SetPlugin(temp_dll,PluginType::GDRom);

	cfgLoadStr("nullDC_plugins","Current_AICA",temp_dll);
	SetPlugin(temp_dll,PluginType::AICA);

	cfgLoadStr("nullDC_plugins","Current_ExtDevice",temp_dll);
	SetPlugin(temp_dll,PluginType::ExtDevice);
	*/
	if (!plugins_Load())
		return false;

	if (emu_rtc(EMU_INIT)!=RV_OK)
		return false;


	dc_inited=true;
	return true;
}
void _Reset_DC(bool Manual)
{
	plugins_Reset(Manual);
	sh4_cpu->Reset(Manual);
	mem_Reset(Manual);
	pvr_Reset(Manual);
	aica_Reset(Manual);
}
bool SoftReset_DC()
{
	if (sh4_cpu->IsCpuRunning())
	{
		sh4_cpu->Stop();
		emu_rtc(EMU_SOFTRESET);
		return true;
	}
	else
		return false;
}
bool Reset_DC(bool Manual)
{
	if (!dc_inited || sh4_cpu->IsCpuRunning())
		return false;

	if (Manual)
		emu_rtc(EMU_RESET_MANUAL);
	else
		emu_rtc(EMU_RESET);

	dc_reseted=true;
	return true;
}

void Term_DC()
{
	if (dc_inited)
	{
		Stop_DC();
		emu_rtc(EMU_TERM);
		emu_rtc(EMU_QUIT);
		dc_inited=false;
	}
}

void Start_DC()
{
	if (!sh4_cpu->IsCpuRunning())
	{
		if (!dc_inited)
		{
			if (!Init_DC())
				return;
		}

		if (!dc_reseted)
			Reset_DC(false);//hard reset kthx

		u32 test = emu_rtc(EMU_CPU_START);
		verify(test==RV_OK);
	}
}
void Stop_DC()
{
	if (dc_inited)//sh4_cpu may not be inited ;)
	{
		if (sh4_cpu->IsCpuRunning())
		{
			sh4_cpu->Stop();
			emu_rtc(EMU_NOP);
		}
	}
}

bool IsDCInited()
{
	return dc_inited;
}
