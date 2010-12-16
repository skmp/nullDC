#pragma once

#include "nullDC\plugins\plugin_header.h"
#include "nullDC\plugins\gui_plugin_header.h"

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>

#include <string.h>
#include <vector>

using namespace std;

#include "emuWinUI.h"

//vars
extern gui_emu_info emu;
extern wchar emu_name[128];

//helper stuff
#define verify(x) if((x)==false){ msgboxf(L"Verify Failed  : " _T(#x) L"\n in %s -> %s : %d \n",MBX_ICONERROR,_T(__FUNCTION__),_T(__FILE__),__LINE__); dbgbreak;}
#define die(reason) { msgboxf(L"Fatal error : %s\n in %s -> %s : %d \n",MBX_ICONERROR,reason,_T(__FUNCTION__),_T(__FILE__),__LINE__); dbgbreak;}
#define fverify verify

//functions
u32 ReadMem32(u32 addr);
u16 ReadMem16(u32 addr);
u8 ReadMem8(u32 addr);
void WriteMem32(u32 addr,u32 data);
void WriteMem16(u32 addr,u16 data);
void WriteMem8(u32 addr,u8 data);

enum Sh4RegType
{
	//GPR , bank 0
	r0=0,
	r1=1,
	r2=2,
	r3=3,
	r4=4,
	r5=5,
	r6=6,
	r7=7,
	r8=8,
	r9=9,
	r10=10,
	r11=11,
	r12=12,
	r13=13,
	r14=14,
	r15=15,

	//GPR , bank 1
	r0_Bank=16,
	r1_Bank=17,
	r2_Bank=18,
	r3_Bank=19,
	r4_Bank=20,
	r5_Bank=21,
	r6_Bank=22,
	r7_Bank=23,

	//Misc regs
	reg_gbr=24,
	reg_ssr=25,
	reg_spc=26,
	reg_sgr=27,
	reg_dbr=28,
	reg_vbr=29,
	reg_mach=30,
	reg_macl=31,
	reg_pr=32,
	reg_fpul=33,
    reg_pc=34,
	reg_sr=35,
	reg_fpscr=36,

	//FPU gpr , bank 0
	fr_0=37,
	fr_1=fr_0+1,
	fr_2=fr_0+2,
	fr_3=fr_0+3,
	fr_4=fr_0+4,
	fr_5=fr_0+5,
	fr_6=fr_0+6,
	fr_7=fr_0+7,
	fr_8=fr_0+8,
	fr_9=fr_0+9,
	fr_10=fr_0+10,
	fr_11=fr_0+11,
	fr_12=fr_0+12,
	fr_13=fr_0+13,
	fr_14=fr_0+14,
	fr_15=fr_0+15,

	//FPU gpr , bank 1
	xf_0=fr_15+1,
	xf_1=xf_0+1,
	xf_2=xf_0+2,
	xf_3=xf_0+3,
	xf_4=xf_0+4,
	xf_5=xf_0+5,
	xf_6=xf_0+6,
	xf_7=xf_0+7,
	xf_8=xf_0+8,
	xf_9=xf_0+9,
	xf_10=xf_0+10,
	xf_11=xf_0+11,
	xf_12=xf_0+12,
	xf_13=xf_0+13,
	xf_14=xf_0+14,
	xf_15=xf_0+15,

	//special regs , used _olny_ on rec
	reg_ftrv=xf_15+1,
	reg_xmtrx=reg_ftrv+1,
	//sr_T=xmtrx+1,
	//sr_Q=sr_T+1,
	//sr_S=sr_Q+1,
	//sr_M=sr_S+1,
	
	dr_0=reg_xmtrx+1,
	dr_7=dr_0+7,

	xd_0=dr_7+1,
	xd_7=xd_0+7,
	reg_pc_temp=xd_7+1,
	reg_sr_T=reg_pc_temp+1,

	sh4_reg_count,

	NoReg=-1
};


#define MBX_OK                       0x00000000L
#define MBX_OKCANCEL                 0x00000001L
#define MBX_ABORTRETRYIGNORE         0x00000002L
#define MBX_YESNOCANCEL              0x00000003L
#define MBX_YESNO                    0x00000004L
#define MBX_RETRYCANCEL              0x00000005L


#define MBX_ICONHAND                 0x00000010L
#define MBX_ICONQUESTION             0x00000020L
#define MBX_ICONEXCLAMATION          0x00000030L
#define MBX_ICONASTERISK             0x00000040L


#define MBX_USERICON                 0x00000080L
#define MBX_ICONWARNING              MBX_ICONEXCLAMATION
#define MBX_ICONERROR                MBX_ICONHAND


#define MBX_ICONINFORMATION          MBX_ICONASTERISK
#define MBX_ICONSTOP                 MBX_ICONHAND

#define MBX_DEFBUTTON1               0x00000000L
#define MBX_DEFBUTTON2               0x00000100L
#define MBX_DEFBUTTON3               0x00000200L

#define MBX_DEFBUTTON4               0x00000300L


#define MBX_APPLMODAL                0x00000000L
#define MBX_SYSTEMMODAL              0x00001000L
#define MBX_TASKMODAL                0x00002000L

#define MBX_HELP                     0x00004000L // Help Button


#define MBX_NOFOCUS                  0x00008000L
#define MBX_SETFOREGROUND            0x00010000L
#define MBX_DEFAULT_DESKTOP_ONLY     0x00020000L

#define MBX_TOPMOST                  0x00040000L
#define MBX_RIGHT                    0x00080000L
#define MBX_RTLREADING               0x00100000L

#define MBX_RV_OK                1
#define MBX_RV_CANCEL            2
#define MBX_RV_ABORT             3
#define MBX_RV_RETRY             4
#define MBX_RV_IGNORE            5
#define MBX_RV_YES               6
#define MBX_RV_NO                7
#define dbgbreak __debugbreak()

#define BPT_OPCODE		0x8A00

void DissasembleOpcode(u16 opcode,u32 pc,wchar* Dissasm);

u32 Sh4GetRegister(Sh4RegType reg);
void Sh4SetRegister(Sh4RegType reg,u32 value);

int GetSymbName(u32 address,wchar *szDesc,bool bUseUnkAddress);
int msgboxf(wchar* text,unsigned int type,...);

bool EmuStarted();

bool EmuInit();

void EmuStart();
void EmuStop();
void EmuStep();
void EmuSkip();

void EmuSetPatch(u32 Value,u32 Mask);
void EmuReset(bool Manual);

bool EmuBootHLE();
bool EmuLoadBinary(wchar* file,u32 address);

bool EmuSelectPlugins();
void EmuStartProfiler();
void EmuStopProfiler();

u32 GetSettingI(u32 id);
void SetSettingI(u32 id,u32 v);

#define patch_syscall_system		32
#define patch_syscall_font			16
#define patch_syscall_flashrom		8
#define patch_syscall_GDrom_misc	4
#define patch_syscall_GDrom_HLE		2
#define patch_resets_Misc			1
#define patch_all					0xFFFFFFFF

extern HWND g_hWnd;
extern HINSTANCE g_hInst;
extern HMODULE hMod;

struct Settings_Struct
{
	bool AutoHideMenu;
	bool Fullscreen;
	bool AlwaysOnTop;
	bool pad0;
};

extern Settings_Struct settings;

void SaveSettings();
void LoadSettings();