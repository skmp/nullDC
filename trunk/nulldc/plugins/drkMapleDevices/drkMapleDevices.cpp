// drkMapleDevices.cpp : Defines the entry point for the DLL application.
//

/*
	Ever wondered how REALLY bad code looks like ? just keep on reading ...
*/

// Works with normal keyboards just the same.
//#define _HAS_LGLCD_ 1

#include "nullDC\plugins\plugin_header.h"
#include <memory.h>
#include <math.h>
#include <random>

emu_info host;

#define _WIN32_WINNT 0x501

#include <windowsx.h>
#include <winsock2.h>
#include <windows.h>
#include <time.h>

#include <ws2tcpip.h>

#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>

#include <string.h>
#include <commctrl.h>
#include "resource.h"

#ifdef _HAS_LGLCD_

	#include <lglcd.h>
	#pragma comment(lib, "lgLcd.lib")

#endif

RAWINPUTDEVICE Rid[2]; // Raw mouse and keyboard	

static std::mt19937 random_dev;
u16 kcode[4]={0xFFFF,0xFFFF,0xFFFF,0xFFFF};
u32 vks[4]={0};
s8 joyx[4]={0},joyy[4]={0};
s8 joy2x[4]={0},joy2y[4]={0};
u8 rt[4]={0},lt[4]={0};
u32 mo_buttons = 0xFFFFFFFF;
s32 mo_x_delta = 0;
s32 mo_y_delta = 0;
s32 mo_wheel_delta = 0;

#ifdef _HAS_LGLCD_

	DWORD gA0;
	DWORD gB0;
	DWORD gC0;
	DWORD gD0;
	int gA0temp;
	int gB0temp;
	int gC0temp;
	int gD0temp;
	int gused;
	int gA0used;
	int gB0used;
	int gC0used;
	int gD0used;
	lgLcdOpenContext openContextA0;
	lgLcdConnectContext connectContextA0;
	lgLcdOpenContext openContextB0;
	lgLcdConnectContext connectContextB0;
	lgLcdOpenContext openContextC0;
	lgLcdConnectContext connectContextC0;
	lgLcdOpenContext openContextD0;
	lgLcdConnectContext connectContextD0;

#endif

#define mo_Middle (1<<0) 
#define mo_Right (1<<1)
#define mo_Left (1<<2)
#define mo_Thumb (1<<3)
//1 Right button (B) 
//2 Left button (A) 
//3 Thumb button (START) 
#define dbgbreak {while(1) __noop;}
#define verify(x) if((x)==false){ printf("Verify Failed  : " #x "\n in %s -> %s : %d \n",__FUNCTION__,__FILE__,__LINE__); dbgbreak;}
#pragma pack(1)
char testJoy_strName[64] = "Dreamcast Controller\0";
char testJoy_strName_nul[64] = "Null Dreamcast Controler\0";
char testJoy_strName_net[64] = "Net Dreamcast Controler\0";
char testJoy_strName_vmu[64] = "Visual Memory\0";
char testJoy_strName_kbd[64] = "Emulated Dreamcast Keyboard\0";
char testJoy_strName_mouse[64] = "Emulated Dreamcast Mouse\0";
char testJoy_strName_dreameye_1[64] = "Dreamcast Camera Flash  Devic\0";
char testJoy_strName_dreameye_2[64] = "Dreamcast Camera Flash LDevic\0";
char testJoy_strName_mic[64] = "MicDevice for Dreameye\0";
char testJoy_strBrand[64] = "Faked by drkIIRaziel && ZeZu , made for nullDC\0";
char testJoy_strBrand_2[64] = "Produced By or Under License From SEGA ENTERPRISES,LTD.\0";

#define key_CONT_C  (1 << 0)
#define key_CONT_B  (1 << 1)
#define key_CONT_A  (1 << 2)
#define key_CONT_START  (1 << 3)
#define key_CONT_DPAD_UP  (1 << 4)
#define key_CONT_DPAD_DOWN  (1 << 5)
#define key_CONT_DPAD_LEFT  (1 << 6)
#define key_CONT_DPAD_RIGHT  (1 << 7)
#define key_CONT_Z  (1 << 8)
#define key_CONT_Y  (1 << 9)
#define key_CONT_X  (1 << 10)
#define key_CONT_D  (1 << 11)
#define key_CONT_DPAD2_UP  (1 << 12)
#define key_CONT_DPAD2_DOWN  (1 << 13)
#define key_CONT_DPAD2_LEFT  (1 << 14)
#define key_CONT_DPAD2_RIGHT  (1 << 15)

#define key_CONT_ANALOG_UP  (1 << 16)
#define key_CONT_ANALOG_DOWN  (1 << 17)
#define key_CONT_ANALOG_LEFT  (1 << 18)
#define key_CONT_ANALOG_RIGHT  (1 << 19)
#define key_CONT_LSLIDER  (1 << 20)
#define key_CONT_RSLIDER  (1 << 21)

enum NAOMI_KEYS
{
	NAOMI_SERVICE_KEY_1=1<<0 ,
	NAOMI_TEST_KEY_1=1<<1 ,
	NAOMI_SERVICE_KEY_2	=1<<2,
	NAOMI_TEST_KEY_2=1<<3 ,

	NAOMI_START_KEY =1<<4,

	NAOMI_UP_KEY =1<<5,
	NAOMI_DOWN_KEY =1<<6,
	NAOMI_LEFT_KEY =1<<7,
	NAOMI_RIGHT_KEY =1<<8,

	NAOMI_BTN0_KEY =1<<9,
	NAOMI_BTN1_KEY =1<<10,
	NAOMI_BTN2_KEY =1<<11,
	NAOMI_BTN3_KEY =1<<12,
	NAOMI_BTN4_KEY =1<<13,
	NAOMI_BTN5_KEY =1<<14,
	NAOMI_COIN_KEY =1<<15,
};
struct joy_init_resp
{
	u32 ratio;
	u32 mode;
	u32 players;
};

struct joy_init
{
	u32 Version;
	char Name[512];
	u32 port;
};

struct joy_substate
{
	u16 state;
	s8 jy;
	s8 jx;
	u8 r;
	u8 l;
};
struct joy_state
{
	u32 id;
	joy_substate  substates[8];
};

struct _joypad_settings_entry
{
	u8 KC;
	u32 BIT;
	wchar* name;
};
#define D(x) x ,_T( #x)

#ifdef BUILD_NAOMI
_joypad_settings_entry joypad_settings_K[] = 
{
	{VK_F4,D(NAOMI_SERVICE_KEY_1)},
	{VK_F5,D(NAOMI_TEST_KEY_1)},
	{VK_F6,D(NAOMI_SERVICE_KEY_2)},
	{VK_F7,D(NAOMI_TEST_KEY_2)},

	{'1',D(NAOMI_START_KEY)},
	{'5',D(NAOMI_COIN_KEY)},
	
	{VK_UP,D(NAOMI_UP_KEY)},
	{VK_DOWN,D(NAOMI_DOWN_KEY)},
	{VK_LEFT,D(NAOMI_LEFT_KEY)},
	{VK_RIGHT,D(NAOMI_RIGHT_KEY)},

	{'Z',D(NAOMI_BTN0_KEY)},
	{'X',D(NAOMI_BTN1_KEY)},
	{'C',D(NAOMI_BTN2_KEY)},
	{'A',D(NAOMI_BTN3_KEY)},
	{'S',D(NAOMI_BTN4_KEY)},
	{'D',D(NAOMI_BTN5_KEY)},

	{0,0,0},
};
#else
_joypad_settings_entry joypad_settings_K[] = 
{
	{'B',D(key_CONT_C)},
	{'X',D(key_CONT_B)},
	{'V',D(key_CONT_A)},
	{VK_SHIFT,D(key_CONT_START)},
	
	{VK_UP,D(key_CONT_DPAD_UP)},
	{VK_DOWN,D(key_CONT_DPAD_DOWN)},
	{VK_LEFT,D(key_CONT_DPAD_LEFT)},
	{VK_RIGHT,D(key_CONT_DPAD_RIGHT)},

	{'M',D(key_CONT_Z)},
	{'Z',D(key_CONT_Y)},
	{'C',D(key_CONT_X)},
	{0,D(key_CONT_DPAD2_UP)},
	{0,D(key_CONT_DPAD2_DOWN)},
	{0,D(key_CONT_DPAD2_LEFT)},
	{0,D(key_CONT_DPAD2_RIGHT)},

	{'I',D(key_CONT_ANALOG_UP)},
	{'K',D(key_CONT_ANALOG_DOWN)},
	{'J',D(key_CONT_ANALOG_LEFT)},
	{'L',D(key_CONT_ANALOG_RIGHT)},

	{'A',D(key_CONT_LSLIDER)},
	{'S',D(key_CONT_RSLIDER)},
	{0,0,0},
};
#endif
_joypad_settings_entry joypad_settings[4][32];
#undef D

enum MapleFunctionID
{
	MFID_0_Input		= 0x01000000,		//DC Controller, Lightgun buttons, arcade stick .. stuff like that
	MFID_1_Storage		= 0x02000000,		//VMU , VMS
	MFID_2_LCD			= 0x04000000,		//VMU
	MFID_3_Clock		= 0x08000000,		//VMU
	MFID_4_Mic			= 0x10000000,		//DC Mic (, dreameye too ?)
	MFID_5_ARGun		= 0x20000000,		//Artificial Retina gun ? seems like this one was never developed or smth -- i only remember of lightguns
	MFID_6_Keyboard		= 0x40000000,		//DC Keyboard
	MFID_7_LightGun		= 0x80000000,		//DC Lightgun
	MFID_8_Vibration	= 0x00010000,		//Puru Puru
	MFID_9_Mouse		= 0x00020000,		//DC Mouse
	MFID_10_StorageExt	= 0x00040000,		//Storage ? propably never used
	MFID_11_Camera		= 0x00080000,		//DreamEye
};
enum MapleDeviceCommand
{
	MDC_DeviceRequest	=0x01,			//7 words.Note : Initialises device
	MDC_AllStatusReq	=0x02,			//7 words + device depedant ( seems to be 8 words)
	MDC_DeviceReset		=0x03,			//0 words
	MDC_DeviceKill		=0x04,			//0 words
	MDC_DeviceStatus    =0x05,			//Same as MDC_DeviceRequest ?
	MDC_DeviceAllStatus =0x06,			//Same as MDC_AllStatusReq ?

	//various Functions
	MDCF_GetCondition=0x09,				//FT
	MDCF_GetMediaInfo=0x0A,				//FT,PT,3 pad
	MDCF_BlockRead   =0x0B,				//FT,PT,Phase,Block #
	MDCF_BlockWrite  =0x0C,				//FT,PT,Phase,Block #,data ...
	MDCF_GetLastError=0x0D,				//FT,PT,Phase,Block #
	MDCF_SetCondition=0x0E,				//FT,data ...
	MDCF_MICControl	 =0x0F,				//FT,MIC data ...
	MDCF_ARGunControl=0x10,				//FT,AR-Gun data ...
};
enum MapleDeviceRV
{
	MDRS_DeviseStatus=0x05,			//28 words
	MDRS_DeviseStatusAll=0x06,		//28 words + device depedant data
	MDRS_DeviceReply=0x07,			//0 words
	MDRS_DataTransfer=0x08,			//FT,depends on the command

	MDRE_UnknownFunction=0xFE,		//0 words
	MDRE_UnknownCmd=0xFD,			//0 words
	MDRE_TransminAgain=0xFC,		//0 words
	MDRE_FileError=0xFB,			//1 word, bitfield
	MDRE_LCDError=0xFA,				//1 word, bitfield
	MDRE_ARGunError=0xF9,			//1 word, bitfield
};

void LoadSettings();
void SaveSettings();
u8 kb_shift          ; //shift keys pressed (bitmask)	//1
u8 kb_led            ; //leds currently lit			//1
u8 kb_key[6]={0}     ; //normal keys pressed			//6
u8 kb_used=0;
wchar kb_map[256];

#ifdef _HAS_LGLCD_

void HandleError(DWORD err, LPCTSTR msg)
{
    if(ERROR_SUCCESS != err)
    {
        LPTSTR lpMsgBuf;
        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            err,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR) &lpMsgBuf,
            0, NULL);
		    //_tprintf(_T("%s: error 0x%08x occurred:\n%s\n"), msg, err, lpMsgBuf);
        LocalFree(lpMsgBuf);
    }
}

#endif

struct VMU_info
{
	u8 data[256*1024];
	char file[512];
	struct {
		HWND handle;
		BYTE data[192];
		WORD bitmap[48*32];
//		BITMAPINFO bmi;
//		bool visible;
	} lcd;
};
BITMAPINFO vmu_bmi;

bool ikbmap=false;
void kb_down(u8 kc)
{
	if (ikbmap==false)
	{
		ikbmap=true;
		void Init_kb_map();
		Init_kb_map();
	}
	if (kc==VK_SHIFT)
		kb_shift|=0x02 | 0x20; //both shifts ;p
	kc=(u8)(kb_map[kc & 0xFF]);
	if (kc==0)
		return;
	if (kb_used<6)
	{
		for (int i=0;i<6;i++)
		{
			if (kb_key[i]==kc)
				return;
		}
		kb_key[kb_used]=kc;
		kb_used++;
	}
}
void kb_up(u8 kc)
{		
	if (kc==VK_SHIFT)
		kb_shift&=~(0x02 | 0x20); //both shifts ;p
	kc=(u8)(kb_map[kc & 0xFF]);
	if (kc==0)
		return;
	if (kb_used>0)
	{
		for (int i=0;i<6;i++)
		{
			if (kb_key[i]==kc)
			{
				kb_used--;
				for (int j=i;j<5;j++)
					kb_key[j]=kb_key[j+1];
				//kb_key[6]=0; // 7th member of 6 sized array... ?
			}
		}
	}
}
typedef INT_PTR CALLBACK dlgp( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

// Event variables
dlgp* oldptr=0;
s32 old_pos_x=0;
s32 old_pos_y=0;

int g_ShowVMU;
bool mouseCapture = false;
float mouseSensitivity;


INT_PTR CALLBACK sch( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{	
		
	switch(uMsg)
	{
	case WM_INPUT:
		{						
			UINT dwSize = 40;
			static BYTE lpb[40];		
			    
			GetRawInputData((HRAWINPUT)lParam, RID_INPUT, 
							lpb, &dwSize, sizeof(RAWINPUTHEADER));
    
			RAWINPUT* raw = (RAWINPUT*)lpb;

			raw->data.mouse.usFlags = MOUSE_MOVE_RELATIVE;
    
			switch(raw->header.dwType)
			{
			case RIM_TYPEMOUSE:
				{
					switch(raw->data.mouse.usButtonFlags)
					{
						// Mouse Buttons Down
						case RI_MOUSE_LEFT_BUTTON_DOWN:   mo_buttons &= ~mo_Left; break;	
						case RI_MOUSE_MIDDLE_BUTTON_DOWN: mo_buttons &= ~mo_Middle; break;
						case RI_MOUSE_RIGHT_BUTTON_DOWN:  mo_buttons &= ~mo_Right; break;

						// Buttons Buttons UP
						case RI_MOUSE_LEFT_BUTTON_UP:   mo_buttons |= mo_Left; break;
						case RI_MOUSE_MIDDLE_BUTTON_UP: mo_buttons |= mo_Middle; break;
						case RI_MOUSE_RIGHT_BUTTON_UP:  mo_buttons |= mo_Right; break;

						// Wheel
						case RI_MOUSE_WHEEL:
						{
							u16 raw_wheel = raw->data.mouse.usButtonData;
						
							if(raw_wheel>>15) mo_wheel_delta = -(u16)~raw_wheel-1;
							else			  mo_wheel_delta =  (u16) raw_wheel;																		
						}
						break;
					
						default: break;
					}
								
					mo_x_delta = (int)(raw->data.mouse.lLastX * mouseSensitivity);
					mo_y_delta = (int)(raw->data.mouse.lLastY * mouseSensitivity);	
				}
				break;

			case RIM_TYPEKEYBOARD:
				{
					u8 key = (u8)(raw->data.keyboard.VKey);
					
					switch(raw->data.keyboard.Message)
					{
					case WM_KEYDOWN:
					//case WM_SYSKEYDOWN:
						{
							if(key == VK_SCROLL) mouseCapture = !mouseCapture;
							
							kb_down(key);

							for (int port=0;port<4;port++)							
							for (int i=0;joypad_settings_K[i].name;i++)							
							if (key==joypad_settings[port][i].KC)
							{
								if (joypad_settings[port][i].BIT<=0x8000)
								{
									kcode[port] &= 0xFFFF -joypad_settings[port][i].BIT;
								}
								else
								{
									vks[port]|=joypad_settings[port][i].BIT;
									switch(joypad_settings[port][i].BIT)
									{
									case key_CONT_ANALOG_UP:
										joyy[port]= -126;
										break;
									case key_CONT_ANALOG_DOWN:
										joyy[port]= +126;
										break;
									case key_CONT_ANALOG_RIGHT:
										joyx[port]= +126;
										break;
									case key_CONT_ANALOG_LEFT:
										joyx[port]= -126;
										break;
									case key_CONT_LSLIDER:
										lt[port]=255;
										break;
									case key_CONT_RSLIDER:
										rt[port]=255;
										break;
									}
								}
							}																					
						}
						break;

					case WM_KEYUP:
					//case WM_SYSKEYUP:
						{
							kb_up(key & 0xFF);

							for (int port=0;port<4;port++)		
							for (int i=0;joypad_settings_K[i].name;i++)							
							if (key==joypad_settings[port][i].KC)
							{
								if (joypad_settings[port][i].BIT<=0x8000)
								{
									kcode[port] |= joypad_settings[port][i].BIT;
								}
								else
								{
									vks[port] &= ~joypad_settings[port][i].BIT;
									if ((vks[port] & (key_CONT_ANALOG_UP|key_CONT_ANALOG_DOWN)) !=(key_CONT_ANALOG_UP|key_CONT_ANALOG_DOWN))
									{
										if (vks[port] & key_CONT_ANALOG_UP)
											joyy[port]=-126;
										else if (vks[port] & key_CONT_ANALOG_DOWN)
											joyy[port]=+126;
										else
											joyy[port]=0;
									}
									if ((vks[port] & (key_CONT_ANALOG_LEFT|key_CONT_ANALOG_RIGHT)) !=(key_CONT_ANALOG_LEFT|key_CONT_ANALOG_RIGHT))
									{
										if (vks[port] & key_CONT_ANALOG_LEFT)
											joyx[port]=-126;
										else if (vks[port] & key_CONT_ANALOG_RIGHT)
											joyx[port]=+126;
										else
											joyx[port]=0;
									}
									switch(joypad_settings[port][i].BIT)
									{
									case key_CONT_LSLIDER:
										lt[port]=0;
										break;
									case key_CONT_RSLIDER:
										rt[port]=0;
										break;
									}
								}
							}														
						} 
						break; // Case WM_KEYUP
					}										
				} 
				break; // Case RIM_TYPEKEYBOARD
			}			  
		}
		break; // Case WM_INPUT
	}
	return oldptr(hWnd,uMsg,wParam,lParam);
}

u32 current_port=0;
bool waiting_key=false;
u32 edited_key=0;
u32 waiting_key_timer=6*4;
#ifdef BUILD_NAOMI
u32 kid_to_did[]=
{
	IDC_NAOMI_SERVICE_KEY_1,
	IDC_NAOMI_TEST_KEY_1,
	IDC_NAOMI_SERVICE_KEY_2,
	IDC_NAOMI_TEST_KEY_2,
	IDC_NAOMI_START_KEY,
	IDC_NAOMI_COIN_KEY,
	IDC_NAOMI_UP_KEY,
	IDC_NAOMI_DOWN_KEY,
	IDC_NAOMI_LEFT_KEY,
	IDC_NAOMI_RIGHT_KEY,
	IDC_NAOMI_BTN0_KEY,
	IDC_NAOMI_BTN1_KEY,
	IDC_NAOMI_BTN2_KEY,
	IDC_NAOMI_BTN3_KEY,
	IDC_NAOMI_BTN4_KEY,
	IDC_NAOMI_BTN5_KEY,
	0,
	0,
	0,
	0,
	0,
	0,
	//IDC_BUTTON22,
};
#else
u32 kid_to_did[]=
{
	IDC_BUTTON1,
	IDC_BUTTON2,
	IDC_BUTTON3,
	IDC_BUTTON4,
	IDC_BUTTON5,
	IDC_BUTTON6,
	IDC_BUTTON7,
	IDC_BUTTON8,
	IDC_BUTTON9,
	IDC_BUTTON10,
	IDC_BUTTON11,
	IDC_BUTTON12,
	IDC_BUTTON13,
	IDC_BUTTON14,
	IDC_BUTTON15,
	IDC_BUTTON16,
	IDC_BUTTON17,
	IDC_BUTTON18,
	IDC_BUTTON19,
	IDC_BUTTON20,
	IDC_BUTTON21,
	IDC_BUTTON22,
	//IDC_BUTTON22,
};
#endif
u8 kbs[256];
const u32 kbratio=20;
void ENABLESHITFACE(HWND hWnd,u32 state)
{
	Static_Enable(hWnd,state);
	for (int kk=0;joypad_settings_K[kk].name;kk++)
	{
		Static_Enable(GetDlgItem(hWnd,kid_to_did[kk]),state);
	}
}
void get_name(int VK,wchar* text)
{
	int scancode = MapVirtualKey(VK,0);
	switch(VK) {
	  case VK_INSERT:
	  case VK_DELETE:
	  case VK_HOME:
	  case VK_END:
	  case VK_NEXT:  // Page down
	  case VK_PRIOR: // Page up
	  case VK_LEFT:
	  case VK_RIGHT:
	  case VK_UP:
	  case VK_DOWN:
		  scancode |= 0x100; // Add extended bit
	}
	GetKeyNameText(scancode*0x10000,text,512);
}
void UpdateKeySelectionNames(HWND hWnd)
{
	wchar temp[512];
	for (int i=0;joypad_settings_K[i].name;i++)
	{
		if (kid_to_did[i]==0)
			continue;
		get_name(joypad_settings[current_port][i].KC,temp);
		Button_SetText(GetDlgItem(hWnd,kid_to_did[i]),temp);
	}
}
INT_PTR CALLBACK ConfigKeysDlgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg )
	{
	case WM_INITDIALOG:
		{
			TCITEM tci; 
			tci.mask = TCIF_TEXT | TCIF_IMAGE; 
			tci.iImage = -1; 
			tci.pszText = L"Port A"; 
			TabCtrl_InsertItem(GetDlgItem(hWnd,IDC_PORTTAB), 0, &tci); 
			tci.pszText = L"Port B"; 
			TabCtrl_InsertItem(GetDlgItem(hWnd,IDC_PORTTAB), 1, &tci); 
			tci.pszText = L"Port C"; 
			TabCtrl_InsertItem(GetDlgItem(hWnd,IDC_PORTTAB), 2, &tci); 
			tci.pszText = L"Port D"; 
			TabCtrl_InsertItem(GetDlgItem(hWnd,IDC_PORTTAB), 3, &tci); 

			TabCtrl_SetCurSel(GetDlgItem(hWnd,IDC_PORTTAB),current_port);

			SetTimer(hWnd,0,1000/kbratio,0);
			Static_SetText(GetDlgItem(hWnd,IDC_STATUS),L"Click a button , then press the key you want to use for it.If you want to use joysticks try the joy2key utility");
			UpdateKeySelectionNames(hWnd);
		}
		return true;
	case WM_NOTIFY:
		{
			if ( ((LPNMHDR)lParam)->idFrom==IDC_PORTTAB && 
				 ((LPNMHDR)lParam)->code == TCN_SELCHANGE  )
			{
				current_port=TabCtrl_GetCurSel(GetDlgItem(hWnd,IDC_PORTTAB));
				UpdateKeySelectionNames(hWnd);
			}
			return true;
		}
	case WM_COMMAND:

		for (int i=0;joypad_settings_K[i].name;i++)
		{
			if (kid_to_did[i]==LOWORD(wParam))
			{
				edited_key=i;
				GetKeyboardState(kbs);
				ENABLESHITFACE(hWnd,0);
				waiting_key_timer=6*kbratio;
				waiting_key=true;
				return true;
			}
		}

		switch( LOWORD(wParam) )
		{

			break;
			
		case IDOK:
			{
			
			}
		case IDCANCEL:
			EndDialog(hWnd,0);
			return true;

		default: break;
		}
		return false;
	case WM_TIMER:
	{
		wchar temp[512];
		if (waiting_key)
		{
			int VK_down=-1;
			u8 temp_kbs[256];
			GetKeyboardState(temp_kbs);
			for (int i=0;i<256;i++)
			{
				if (temp_kbs[i]!=kbs[i] && temp_kbs[i]!=0)
				{
					VK_down=i;
				}
			}

			if (VK_down!=-1)
			{
				waiting_key=false;

				swprintf_s(temp,L"Updated Key Mapping,%d",VK_down);
				Static_SetText(GetDlgItem(hWnd,IDC_STATUS),temp);
				joypad_settings[current_port][edited_key].KC=VK_down;
				SaveSettings();
				UpdateKeySelectionNames(hWnd);
			}	
		}

		if(waiting_key)
		{
			wchar temp[512];
			
			waiting_key_timer--;
			if (waiting_key_timer==0)
			{
				Static_Enable(hWnd,1);
				for (int kk=IDC_BUTTON1;kk<(IDC_BUTTON1+16);kk++)
				{
					Static_Enable(GetDlgItem(hWnd,kk),1);
				}
				waiting_key=false;
				waiting_key_timer=6;

				swprintf_s(temp,L"Timed out while waiting for new key",waiting_key_timer/kbratio);
				Static_SetText(GetDlgItem(hWnd,IDC_STATUS),temp);
			}
			else
			{
				swprintf_s(temp,L"Waiting for key ...%d\n",waiting_key_timer/kbratio);
				Static_SetText(GetDlgItem(hWnd,IDC_STATUS),temp);
			}
		}

		if (!waiting_key)
			ENABLESHITFACE(hWnd,1);
		GetKeyboardState(kbs);
	}
	return true;
	case WM_CLOSE:
	case WM_DESTROY:
		KillTimer(hWnd,0);
		EndDialog(hWnd,0);
		return true;

	default: break;
	}

	return false;
}
void cfgdlg(PluginType type,void* window)
{
	printf("ndcMAPLE :No config kthx\n");
}//called when plugin is used by emu (you should do first time init here)

void Init_kb_map();
s32 FASTCALL Load(emu_info* emu)
{
	memcpy(&host,emu,sizeof(host));
	for (int set=0;set<4;set++)
		memcpy(joypad_settings[set],joypad_settings_K,sizeof(joypad_settings_K));

	random_dev.seed((u32)__rdtsc());

	//maple_init_params* mpi=(maple_init_params*)aparam;
	//handle=mpi->WindowHandle;
	if (oldptr==0)
		oldptr = (dlgp*)SetWindowLongPtr((HWND)host.GetRenderTarget(),GWLP_WNDPROC,(LONG_PTR)sch);
	Init_kb_map();

	LoadSettings();
	
	#ifdef _HAS_LGLCD_

	printf("nullDC Maple Devices: Compiled With Optional Logitech G Series Keyboard Support\n");
		gA0 = gB0 = gC0 = gD0 = lgLcdInit();
		HandleError(gA0, _T("lgLcdInit"));
		HandleError(gB0, _T("lgLcdInit"));
		HandleError(gC0, _T("lgLcdInit"));
		HandleError(gD0, _T("lgLcdInit"));

	#endif

	// Registering RAW Mouse
	Rid[0].usUsagePage = 0x01;
	Rid[0].usUsage = 0x02;
	Rid[0].dwFlags = 0;
	Rid[0].hwndTarget = NULL;

	// Registering RAW Keyboard
	Rid[1].usUsagePage = 0x01;
	Rid[1].usUsage = 0x06;
	Rid[1].dwFlags = 0;
	Rid[1].hwndTarget = NULL;

	RegisterRawInputDevices(Rid, 2, sizeof(Rid[0]));	

	return rv_ok;
}

//called when plugin is unloaded by emu , olny if dcInitPvr is called (eg , not called to enumerate plugins)
void FASTCALL  Unload()
{
	if (oldptr!=0)
	{
		SetWindowLongPtrW((HWND)host.GetRenderTarget(),GWLP_WNDPROC,(LONG_PTR)oldptr);
		oldptr=0;
	}

	#ifdef _HAS_LGLCD_

		gA0 = gB0 = gC0 = gD0 = lgLcdDeInit();
		HandleError(gA0, _T("lgLcdDeInit"));
		HandleError(gB0, _T("lgLcdDeInit"));
		HandleError(gC0, _T("lgLcdDeInit"));
		HandleError(gD0, _T("lgLcdDeInit"));

	#endif

}

/*
//It's suposed to reset anything but vram (vram is set to 0 by emu)
s32 FASTCALL Init(maple_init_params* p)
{
	UpdateConfig();
	//hahah do what ? ahahahahahaha
	return rv_ok;
}

//called when entering sh4 thread , from the new thread context (for any thread speciacific init)
void FASTCALL Reset(bool Manual)
{
	//maby init here ?
}

//called when exiting from sh4 thread , from the new thread context (for any thread speciacific de init) :P
void FASTCALL Term()
{
	//term here ?
}
*/
HMODULE hModule;
HINSTANCE hInstance;
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	::hModule=hModule;
	hInstance=(HINSTANCE)hModule;
    return TRUE;
}
#define sk(num,key)kb_map[key]=0x##num;
void Init_kb_map()
{
	memset(kb_map,0,sizeof(kb_map));
	//Keycode Key 
		//00 No key pressed 
		//01 Too many keys pressed error 
		//02-03 Not used(?) 
		//04-1D Letter keys A-Z (in alphabetic order) 
		for (int i=0x4;i<=0x1D;i++)  kb_map['A'+i-4]=i;
		//1E-27 Number keys 1-0 
		for (int i=0x1E;i<=0x27;i++) kb_map['1'+i-0x1E]=i;
		kb_map['0']=0x27;		
		//28 Enter 
		sk(28,VK_RETURN);
		//29 Esc 
		sk(29,VK_ESCAPE);
		//2A Backspace 
		sk(2A,VK_BACK);
		//2B Tab 
		sk(2B,VK_TAB);
		//2C Space 
		sk(2C,VK_SPACE);
		//2D-2E "-" and "^" (the 2 keys right of the numbers) 		
		sk(2D,VK_OEM_MINUS);
		sk(2E,VK_OEM_PLUS);
		//2F-30 "@" and "[" (the 2 keys right of P) 
		sk(2F,VK_OEM_4);
		sk(30,VK_OEM_6);
		//31 Not used 
		//sk(31,VK_OEM_3); // \ (US) also 64
		//32-34 "]", ";" and ":" (the 3 keys right of L) 
		sk(32,VK_OEM_5);
		sk(33,VK_OEM_1); // :; (US)
		sk(34,VK_OEM_7); // "' (US)
		//35 hankaku/zenkaku / kanji (top left) 
		sk(35,VK_OEM_3); // `~ (US)
		//36-38 ",", "." and "/" (the 3 keys right of M) 
		sk(36,VK_OEM_COMMA);
		sk(37,VK_OEM_PERIOD);
		sk(38,VK_OEM_2);
		//39 Caps Lock 
		sk(39,VK_CAPITAL);
		//3A-45 Function keys F1-F12 
		for (int i=0;i<12;i++) kb_map[VK_F1+i]=0x3A+i;
		//46-4E Control keys above cursor keys 
		sk(46,VK_SNAPSHOT);
		sk(47,VK_SCROLL);
		sk(48,VK_PAUSE);
		sk(49,VK_INSERT);
		sk(4A,VK_HOME);
		sk(4B,VK_PRIOR);
		sk(4C,VK_DELETE);
		sk(4D,VK_END);
		sk(4E,VK_NEXT);
		//4F-52 Cursors
		sk(4F,VK_RIGHT);		
		sk(50,VK_LEFT);		
		sk(51,VK_DOWN);		
		sk(52,VK_UP);
		//53 Num Lock (Numeric keypad) 
		sk(53,VK_NUMLOCK);
		//54 "/" (Numeric keypad) 
		sk(54,VK_DIVIDE);
		//55 "*" (Numeric keypad) 
		sk(55,VK_MULTIPLY);
		//56 "-" (Numeric keypad) 
		sk(56,VK_SUBTRACT);
		//57 "+" (Numeric keypad) 
		sk(57,VK_ADD);
		//58 Enter (Numeric keypad) 
		sk(58,VK_EXECUTE);	//enter ??
		//59-62 Number keys 1-0 (Numeric keypad) 
		sk(59,VK_NUMPAD1);
		sk(5A,VK_NUMPAD2);
		sk(5B,VK_NUMPAD3);
		sk(5C,VK_NUMPAD4);
		sk(5D,VK_NUMPAD5);
		sk(5E,VK_NUMPAD6);
		sk(5F,VK_NUMPAD7);
		sk(60,VK_NUMPAD8);
		sk(61,VK_NUMPAD9);
		sk(62,VK_NUMPAD0);
		//63 "." (Numeric keypad) 
		sk(63,VK_DECIMAL);
		//64 "\" (right of left Shift) 
		sk(64,VK_OEM_5);
		//65 S3 key 				
		//66-86 Not used 
		//8C-FF Not used 
}



u8 GetBtFromSgn(s8 val);

#define w32(data) *(u32*)buffer_out_b=(data);buffer_out_b+=4;buffer_out_len+=4
#define w16(data) *(u16*)buffer_out_b=(data);buffer_out_b+=2;buffer_out_len+=2
#define w8(data) *(u8*)buffer_out_b=(data);buffer_out_b+=1;buffer_out_len+=1

u32 FASTCALL KbdDMA(void* device_instance,u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len)
{
	//printf("ControllerDMA Called 0x%X;Command %d\n",device_instance->port,Command);
	//void testJoy_GotData(u32 header1,u32 header2,u32*data,u32 datalen)
	u8*buffer_out_b=(u8*)buffer_out;

	switch (Command)
	{
		/*typedef struct {
			DWORD		func;//4
			DWORD		function_data[3];//3*4
			u8		area_code;//1
			u8		connector_direction;//1
			char		product_name[30];//30*1
			char		product_license[60];//60*1
			WORD		standby_power;//2
			WORD		max_power;//2
		} maple_devinfo_t;*/
		case 1:
			//header
			//WriteMem32(ptr_out,(u32)(0x05 | //response
			//			(((u16)sendadr << 8) & 0xFF00) |
			//			((((recadr == 0x20) ? 0x20 : 0) << 16) & 0xFF0000) |
			//			(((112/4) << 24) & 0xFF000000))); ptr_out += 4;

			//caps
			//4
			w32(1 << 30);

			//struct data
			//3*4
			w32( 0xfe060f00); 
			w32( 0);
			w32( 0);
			//1	area code
			w8(0xFF);
			//1	direction
			w8(0);
			//30
			for (u32 i = 0; i < 30; i++)
			{
				w8((u8)testJoy_strName_kbd[i]);
				//if (!testJoy_strName[i])
				//	break;
			}
			//ptr_out += 30;

			//60
			for (u32 i = 0; i < 60; i++)
			{
				w8((u8)testJoy_strBrand_2[i]);
				//if (!testJoy_strBrand[i])
				//	break;
			}
			//ptr_out += 60;

			//2
			w16(0x01AE); 

			//2
			w16(0x01F5); 
			return 5;

		/* controller condition structure 
		typedef struct {//8 bytes
		int8 shift          ; shift keys pressed (bitmask)	//1
		int8 led            ; leds currently lit			//1
		int8 key[6]         ; normal keys pressed			//6
		} cont_cond_t;*/
		case 9:


			//header
			//WriteMem32(ptr_out, (u32)(0x08 | // data transfer (response)
			//			(((u16)sendadr << 8) & 0xFF00) |
			//			((((recadr == 0x20) ? 0x20 : 1) << 16) & 0xFF0000) |
			//			(((12 / 4 ) << 24) & 0xFF000000))); ptr_out += 4;
			//caps
			//4
			//WriteMem32(ptr_out, (1 << 24)); ptr_out += 4;
			w32((1 << 30));
			//struct data
			//int8 shift          ; shift keys pressed (bitmask)	//1
			w8(kb_shift);
			//int8 led            ; leds currently lit			//1
			w8(kb_led);
			//int8 key[6]         ; normal keys pressed			//6
			for (int i=0;i<6;i++)
			{
				w8(kb_key[i]);
			}
			
			return 8;

		default:
			printf("unknown MAPLE COMMAND %d\n",Command);
			return 7;
	}
}

u16 mo_cvt(s32 delta)
{
	delta+=0x200;
	if (delta<=0)
		delta=0;
	else if (delta>0x3FF)
		delta=0x3FF;

	return (u16) delta;
}
/*
	(06/01/07)[04:09] <BlueCrab> Device info for port A0:
	(06/01/07)[04:09] <BlueCrab> Functions: 01000000
	(06/01/07)[04:09] <BlueCrab> Function Data 0: 00080000
	(06/01/07)[04:09] <BlueCrab> Function Data 1: 00000000
	(06/01/07)[04:09] <BlueCrab> Function Data 2: 00000000
	(06/01/07)[04:09] <BlueCrab> Area code: FF
	(06/01/07)[04:09] <BlueCrab> Connector direction: 00
	(06/01/07)[04:09] <BlueCrab> Product name: Dreamcast Camera Flash  Devic
	(06/01/07)[04:09] <BlueCrab> Product licence: Produced By or Under License From SEGA ENTERPRISES,LTD.    
	(06/01/07)[04:09] <BlueCrab> Standby power: 07D0
	(06/01/07)[04:10] <BlueCrab> Max power: 0960
*/
u32 FASTCALL DreamEye_mainDMA(maple_device_instance* device_instance,u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len,u32& responce)
{
	//printf("ControllerDMA Called 0x%X;Command %d\n",device_instance->port,Command);
//void testJoy_GotData(u32 header1,u32 header2,u32*data,u32 datalen)
	u8*buffer_out_b=(u8*)buffer_out;

	switch (Command)
	{
		/*typedef struct {
			DWORD		func;//4
			DWORD		function_data[3];//3*4
			u8		area_code;//1
			u8		connector_direction;//1
			char		product_name[30];//30*1
			char		product_license[60];//60*1
			WORD		standby_power;//2
			WORD		max_power;//2
		} maple_devinfo_t;*/
		case 1:
			//header
			//WriteMem32(ptr_out,(u32)(0x05 | //response
			//			(((u16)sendadr << 8) & 0xFF00) |
			//			((((recadr == 0x20) ? 0x20 : 0) << 16) & 0xFF0000) |
			//			(((112/4) << 24) & 0xFF000000))); ptr_out += 4;

			//caps
			//4
			w32(0x01000000);

			//struct data
			//3*4
			w32( 0x00080000); 
			w32( 0);
			w32( 0);
			//1	area code
			w8(0xFF);
			//1	direction
			w8(0);
			//30
			for (u32 i = 0; i < 30; i++)
			{
				w8((u8)testJoy_strName_dreameye_1[i]);
				//if (!testJoy_strName[i])
				//	break;
			}
			//ptr_out += 30;

			//60
			for (u32 i = 0; i < 60; i++)
			{
				w8((u8)testJoy_strBrand_2[i]);
				//if (!testJoy_strBrand[i])
				//	break;
			}
			//ptr_out += 60;

			//2
			w16(0x04FF); 

			//2
			w16(0x0069); 
			return 5;
			/* controller condition structure 
		typedef struct {//8 bytes
		WORD buttons;			///* buttons bitfield	/2
		u8 rtrig;			///* right trigger			/1
		u8 ltrig;			///* left trigger 			/1
		u8 joyx;			////* joystick X 			/1
		u8 joyy;			///* joystick Y				/1
		u8 joy2x;			///* second joystick X 		/1
		u8 joy2y;			///* second joystick Y 		/1
		} cont_cond_t;*/
		case 9:


			//header
			//WriteMem32(ptr_out, (u32)(0x08 | // data transfer (response)
			//			(((u16)sendadr << 8) & 0xFF00) |
			//			((((recadr == 0x20) ? 0x20 : 1) << 16) & 0xFF0000) |
			//			(((12 / 4 ) << 24) & 0xFF000000))); ptr_out += 4;
			//caps
			//4
			//WriteMem32(ptr_out, (1 << 24)); ptr_out += 4;
			w32((1 << 24));
			//struct data
			//2
			w16(0xF7FF); //camera button not pressed
			
			//triger
			//1 R
			w8(0);
			//1 L
			w8(0); 
			//joyx
			//1
			w8(GetBtFromSgn(0));
			//joyy
			//1
			w8(GetBtFromSgn(0));

			//1
			w8(GetBtFromSgn(0)); 
			//1
			w8(GetBtFromSgn(0)); 
			//are these needed ?
			//1
			//WriteMem8(ptr_out, 10); ptr_out += 1;
			//1
			//WriteMem8(ptr_out, 10); ptr_out += 1;

			return 8;
		default:
			printf("DreamEye_mainDMA : unknown MAPLE COMMAND %d \n",Command);
			return 7;
	}
}

/*
	(06/01/07)[04:10] <BlueCrab> Device info for port A1:
	(06/01/07)[04:10] <BlueCrab> Functions: 00080000
	(06/01/07)[04:10] <BlueCrab> Function Data 0: 30A800C0
	(06/01/07)[04:10] <BlueCrab> Function Data 1: 00000000
	(06/01/07)[04:10] <BlueCrab> Function Data 2: 00000000
	(06/01/07)[04:10] <BlueCrab> Area code: FF
	(06/01/07)[04:10] <BlueCrab> Connector direction: 00
	(06/01/07)[04:10] <BlueCrab> Product name: Dreamcast Camera Flash LDevic
	(06/01/07)[04:10] <BlueCrab> Product licence: Produced By or Under License From SEGA ENTERPRISES,LTD.    
	(06/01/07)[04:10] <BlueCrab> Standby power: 0000
	(06/01/07)[04:10] <BlueCrab> Max power: 0000
*/
u32 des_9_count=0;
u32 FASTCALL DreamEye_subDMA(maple_device_instance* device_instance,u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len)
{
	//printf("ControllerDMA Called 0x%X;Command %d\n",device_instance->port,Command);
//void testJoy_GotData(u32 header1,u32 header2,u32*data,u32 datalen)
	u8*buffer_out_b=(u8*)buffer_out;

	switch (Command)
	{
		/*typedef struct {
			DWORD		func;//4
			DWORD		function_data[3];//3*4
			u8		area_code;//1
			u8		connector_direction;//1
			char		product_name[30];//30*1
			char		product_license[60];//60*1
			WORD		standby_power;//2
			WORD		max_power;//2
		} maple_devinfo_t;*/
		case 1:
			//header
			//WriteMem32(ptr_out,(u32)(0x05 | //response
			//			(((u16)sendadr << 8) & 0xFF00) |
			//			((((recadr == 0x20) ? 0x20 : 0) << 16) & 0xFF0000) |
			//			(((112/4) << 24) & 0xFF000000))); ptr_out += 4;

			//caps
			//4
			w32(0x00080000);

			//struct data
			//3*4
			w32( 0x30A800C0); 
			w32( 0);
			w32( 0);
			//1	area code
			w8(0xFF);
			//1	direction
			w8(0);
			//30
			for (u32 i = 0; i < 30; i++)
			{
				w8((u8)testJoy_strName_dreameye_2[i]);
				//if (!testJoy_strName[i])
				//	break;
			}
			//ptr_out += 30;

			//60
			for (u32 i = 0; i < 60; i++)
			{
				w8((u8)testJoy_strBrand_2[i]);
				//if (!testJoy_strBrand[i])
				//	break;
			}
			//ptr_out += 60;

			//2
			w16(0); 

			//2
			w16(0); 
			return 5;

//(07/01/07)[02:36] <BlueCrab> dreameye: replied with 8, size 5
//(07/01/07)[02:36] <BlueCrab> 00080000, 000000D0, 1F000480, 1F000481, C0070094,
//(07/01/07)[02:46] <BlueCrab> ok... to the 6 command 9's after the one with data, it replies identically:
//(07/01/07)[02:46] <BlueCrab> dreameye: replied with 8, size 2
//(07/01/07)[02:46] <BlueCrab> 00080000, 000000D0, 
		case 9:

			//header
			//WriteMem32(ptr_out, (u32)(0x08 | // data transfer (response)
			//			(((u16)sendadr << 8) & 0xFF00) |
			//			((((recadr == 0x20) ? 0x20 : 1) << 16) & 0xFF0000) |
			//			(((12 / 4 ) << 24) & 0xFF000000))); ptr_out += 4;
			//caps
			//4
			w32(0x00080000);
			//struct data
			if (des_9_count==0)
			{
			//4 dwords
			w32(0x000000D0);
			w32(0x1F000480);
			w32(0x1F000481);
			w32(0xC0070094);
			}
			else
			{
				//1 dword
				w32(0x000000D0); //? ok maby ?
			}

			des_9_count++;
			printf("DreamEye_subDMA[0x%x | %d %x] : unknown MAPLE COMMAND %d \n",device_instance->port,device_instance->port>>6,device_instance->port&63,Command);
			printf(" buffer in size : %d\n",buffer_in_len);
			for (u32 i=0;i<buffer_in_len;i+=4)
			{
				printf("%08X ",*buffer_in++);
			}
			printf("\n");
			//getchar();
			return 0x08;
//(07/01/07)[03:04] <BlueCrab> dreameye: replied with 11, size 2
//(07/01/07)[03:04] <BlueCrab>           responding to port B1
//(07/01/07)[03:04] <BlueCrab> 00080000, 000002FF, 
		case 17:
			{
				//responce=17; //? fuck ?
				//caps
				//4
				w32(0x00080000);
				//mmwaaa?
				//w32(0x000002FF);
				w32((u32)random_dev());
			}
			printf("DreamEye_subDMA[0x%x | %d %x] : unknown MAPLE COMMAND %d \n",device_instance->port,device_instance->port>>6,device_instance->port&63,Command);
			printf(" buffer in size : %d\n",buffer_in_len);
			for (u32 i=0;i<buffer_in_len;i+=4)
			{
				printf("%08X ",*buffer_in++);
			}
			printf("\n");
			//getchar();
			return 17;
		default:
			printf("DreamEye_subDMA[0x%x | %d %x] : unknown MAPLE COMMAND %d \n",device_instance->port,device_instance->port>>6,device_instance->port&63,Command);
			printf(" buffer in size : %d\n",buffer_in_len);
			for (u32 i=0;i<buffer_in_len;i+=4)
			{
				printf("%08X ",*buffer_in++);
			}
			printf("\n");
			//getchar();
			return 7;//just ko
	}
}


u32 FASTCALL MicDMA(maple_device_instance* device_instance,u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len)
{
	//printf("ControllerDMA Called 0x%X;Command %d\n",device_instance->port,Command);
//void testJoy_GotData(u32 header1,u32 header2,u32*data,u32 datalen)
	u8*buffer_out_b=(u8*)buffer_out;

	printf("MicDMA[0x%x | %d %x] : unknown MAPLE COMMAND %d \n",device_instance->port,device_instance->port>>6,device_instance->port&63,Command);
	printf(" buffer in size : %d\n",buffer_in_len);

	for (u32 i=0;i<buffer_in_len;i+=4)
	{
		printf("%08X ",*buffer_in++);
	}
	printf("\n");

	switch (Command)
	{
		/*typedef struct {
			DWORD		func;//4
			DWORD		function_data[3];//3*4
			u8		area_code;//1
			u8		connector_direction;//1
			char		product_name[30];//30*1
			char		product_license[60];//60*1
			WORD		standby_power;//2
			WORD		max_power;//2
		} maple_devinfo_t;*/
		case 1:
			//header
			//WriteMem32(ptr_out,(u32)(0x05 | //response
			//			(((u16)sendadr << 8) & 0xFF00) |
			//			((((recadr == 0x20) ? 0x20 : 0) << 16) & 0xFF0000) |
			//			(((112/4) << 24) & 0xFF000000))); ptr_out += 4;

			//caps
			//4
			w32(0x10000000);

			//struct data
			//3*4
			w32( 0x3F000000);  // ?? wii pwns ps3 ...
			w32( 0);
			w32( 0);
			//1	area code
			w8(0xF); //WTF ?
			//1	direction
			w8(1); //WTF ?
			//30
			for (u32 i = 0; i < 30; i++)
			{
				w8((u8)testJoy_strName_mic[i]);
				//if (!testJoy_strName[i])
				//	break;
			}
			//ptr_out += 30;

			//60
			for (u32 i = 0; i < 60; i++)
			{
				w8((u8)testJoy_strBrand_2[i]);
				//if (!testJoy_strBrand[i])
				//	break;
			}
			//ptr_out += 60;

			//2
			w16(0x012C); 

			//2
			w16(0x012C); 
			return 5;
		case 9:
			//caps
			//4
			w32(0x10000000);
			//getchar();
			return 8;

		default:
			return 7;//just ko
	}
}


u32 FASTCALL MouseDMA(void* device_instance,u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len)
{
	//printf("ControllerDMA Called 0x%X;Command %d\n",device_instance->port,Command);
//void testJoy_GotData(u32 header1,u32 header2,u32*data,u32 datalen)
	u8*buffer_out_b=(u8*)buffer_out;

	switch (Command)
	{
		/*typedef struct {
			DWORD		func;//4
			DWORD		function_data[3];//3*4
			u8		area_code;//1
			u8		connector_direction;//1
			char		product_name[30];//30*1
			char		product_license[60];//60*1
			WORD		standby_power;//2
			WORD		max_power;//2
		} maple_devinfo_t;*/
		case 1:
			//header
			//WriteMem32(ptr_out,(u32)(0x05 | //response
			//			(((u16)sendadr << 8) & 0xFF00) |
			//			((((recadr == 0x20) ? 0x20 : 0) << 16) & 0xFF0000) |
			//			(((112/4) << 24) & 0xFF000000))); ptr_out += 4;


			//caps
			//4
			w32(1 << 17);

			//struct data
			//3*4
			w32( 0xfe060f00); 
			w32( 0);
			w32( 0);
			//1	area code
			w8(0xFF);
			//1	direction
			w8(0);
			//30
			for (u32 i = 0; i < 30; i++)
			{
				w8((u8)testJoy_strName_mouse[i]);
				//if (!testJoy_strName[i])
				//	break;
			}
			//ptr_out += 30;

			//60
			for (u32 i = 0; i < 60; i++)
			{
				w8((u8)testJoy_strBrand_2[i]);
				//if (!testJoy_strBrand[i])
				//	break;
			}
			//ptr_out += 60;

			//2
			w16(0x0069); 

			//2
			w16(0x0120); 
			return 5;

		/* controller condition structure 
		typedef struct {//8 bytes
		int32 buttons       ; digital buttons bitfield (little endian)
		int16 axis1         ; horizontal movement (0-$3FF) (little endian)
		int16 axis2         ; vertical movement (0-$3FF) (little endian)
		int16 axis3         ; mouse wheel movement (0-$3FF) (little endian)
		int16 axis4         ; ? movement (0-$3FF) (little endian)
		int16 axis5         ; ? movement (0-$3FF) (little endian)
		int16 axis6         ; ? movement (0-$3FF) (little endian)
		int16 axis7         ; ? movement (0-$3FF) (little endian)
		int16 axis8         ; ? movement (0-$3FF) (little endian)
		} cont_cond_t;*/
		case 9:


			//header
			//WriteMem32(ptr_out, (u32)(0x08 | // data transfer (response)
			//			(((u16)sendadr << 8) & 0xFF00) |
			//			((((recadr == 0x20) ? 0x20 : 1) << 16) & 0xFF0000) |
			//			(((12 / 4 ) << 24) & 0xFF000000))); ptr_out += 4;
			//caps
			//4
			//WriteMem32(ptr_out, (1 << 24)); ptr_out += 4;
			w32(1 << 17);
			//struct data
			
			//int32 buttons       ; digital buttons bitfield (little endian)
			w32(mo_buttons);
			//int16 axis1         ; horizontal movement (0-$3FF) (little endian)
			w16(mo_cvt(mo_x_delta));
			//int16 axis2         ; vertical movement (0-$3FF) (little endian)
			w16(mo_cvt(mo_y_delta));
			//int16 axis3         ; mouse wheel movement (0-$3FF) (little endian)
			w16(mo_cvt(mo_wheel_delta));
			//int16 axis4         ; ? movement (0-$3FF) (little endian)
			w16(mo_cvt(0));
			//int16 axis5         ; ? movement (0-$3FF) (little endian)
			w16(mo_cvt(0));
			//int16 axis6         ; ? movement (0-$3FF) (little endian)
			w16(mo_cvt(0));
			//int16 axis7         ; ? movement (0-$3FF) (little endian)
			w16(mo_cvt(0));
			//int16 axis8         ; ? movement (0-$3FF) (little endian)
			w16(mo_cvt(0));

			mo_x_delta=0;
			mo_y_delta=0;
			mo_wheel_delta = 0;
			return 8;

		default:
			printf("unknown MAPLE COMMAND %d\n",Command);
			return 7;
	}
}
struct _NaomiState
{
	BYTE Cmd;
	BYTE Mode;
	BYTE Node;
};
_NaomiState State;

u32 FASTCALL ControllerDMA(void* device_instance,u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len)
{
	//printf("ControllerDMA Called 0x%X;Command %d\n",device_instance->port,Command);
//void testJoy_GotData(u32 header1,u32 header2,u32*data,u32 datalen)
	u8*buffer_out_b=(u8*)buffer_out;
	u32 port=((maple_device_instance*)device_instance)->port>>6;
	switch (Command)
	{
		/*typedef struct {
			DWORD		func;//4
			DWORD		function_data[3];//3*4
			u8		area_code;//1
			u8		connector_direction;//1
			char		product_name[30];//30*1
			char		product_license[60];//60*1
			WORD		standby_power;//2
			WORD		max_power;//2
		} maple_devinfo_t;*/
		case 1:
			//header
			//WriteMem32(ptr_out,(u32)(0x05 | //response
			//			(((u16)sendadr << 8) & 0xFF00) |
			//			((((recadr == 0x20) ? 0x20 : 0) << 16) & 0xFF0000) |
			//			(((112/4) << 24) & 0xFF000000))); ptr_out += 4;

			//caps
			//4
			w32(1 << 24);

			//struct data
			//3*4
			w32( 0xfe060f00); 
			w32( 0);
			w32( 0);
			//1	area code
			w8(0xFF);
			//1	direction
			w8(0);
			//30
			for (u32 i = 0; i < 30; i++)
			{
				if (testJoy_strName[i]!=0)
				{
					w8((u8)testJoy_strName[i]);
				}
				else
				{
					w8(0x20);
				}
				//if (!testJoy_strName[i])
				//	break;
			}
			//ptr_out += 30;

			//60
			for (u32 i = 0; i < 60; i++)
			{
				if (testJoy_strBrand_2[i]!=0)
				{
					w8((u8)testJoy_strBrand_2[i]);
				}
				else
				{
					w8(0x20);
				}
				//if (!testJoy_strBrand[i])
				//	break;
			}
			//ptr_out += 60;

			//2
			w16(0x01AE); 

			//2
			w16(0x01F4); 
			return 5;

		/* controller condition structure 
		typedef struct {//8 bytes
		WORD buttons;			///* buttons bitfield	/2
		u8 rtrig;			///* right trigger			/1
		u8 ltrig;			///* left trigger 			/1
		u8 joyx;			////* joystick X 			/1
		u8 joyy;			///* joystick Y				/1
		u8 joy2x;			///* second joystick X 		/1
		u8 joy2y;			///* second joystick Y 		/1
		} cont_cond_t;*/
		case 9:


			//header
			//WriteMem32(ptr_out, (u32)(0x08 | // data transfer (response)
			//			(((u16)sendadr << 8) & 0xFF00) |
			//			((((recadr == 0x20) ? 0x20 : 1) << 16) & 0xFF0000) |
			//			(((12 / 4 ) << 24) & 0xFF000000))); ptr_out += 4;
			//caps
			//4
			//WriteMem32(ptr_out, (1 << 24)); ptr_out += 4;
			w32(1 << 24);
			//struct data
			//2
			w16(kcode[port] | 0xF901); 
			
			//triger
			//1 R
			w8(rt[port]);
			//1 L
			w8(lt[port]); 
			//joyx
			//1
			w8(GetBtFromSgn(joyx[port]));
			//joyy
			//1
			w8(GetBtFromSgn(joyy[port]));

			//1
			w8(0x80); 
			//1
			w8(0x80); 
			//are these needed ?
			//1
			//WriteMem8(ptr_out, 10); ptr_out += 1;
			//1
			//WriteMem8(ptr_out, 10); ptr_out += 1;

			return 8;

		default:
			printf("unknown MAPLE COMMAND %d\n",Command);
			return 7;
	}
}

void printState(u32 cmd,u32* buffer_in,u32 buffer_in_len)
{
	printf("Command : 0x%X",cmd);
	if (buffer_in_len>0)
		printf(",Data : %d bytes\n",buffer_in_len);
	else
		printf("\n");
	buffer_in_len>>=2;
	while(buffer_in_len-->0)
	{
		printf("%08X ",*buffer_in++);
		if (buffer_in_len==0)
			printf("\n");
	}
}

char EEPROM[0x100];
bool EEPROM_loaded=false;
u32 FASTCALL ControllerDMA_naomi(void* device_instance,u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len)
{
#define ret(x) { responce=(x); return; }

	//printf("ControllerDMA Called 0x%X;Command %d\n",device_instance->port,Command);
	u8*buffer_out_b=(u8*)buffer_out;
	u8*buffer_in_b=(u8*)buffer_in;
	buffer_out_len=0;
	u32 port=((maple_device_instance*)device_instance)->port>>6;
	switch (Command)
	{
		case 0x86:
			{
				u32 subcode=*(u8*)buffer_in;
				//printf("Naomi 0x86 : %x\n",SubCode);
				switch(subcode)
				{
				case 0x15:
				case 0x33:
					{
						buffer_out[0]=0xffffffff;
						buffer_out[1]=0xffffffff;
						u32 keycode =~kcode[0];
						u32 keycode2=~kcode[1];

						if(keycode&NAOMI_SERVICE_KEY_2)		//Service
							buffer_out[0]&=~(1<<0x1b);

						if(keycode&NAOMI_TEST_KEY_2)		//Test
							buffer_out[0]&=~(1<<0x1a);

						if(State.Mode==0 && subcode!=0x33)	//Get Caps
						{
							buffer_out_b[0x11+1]=0x8E;	//Valid data check
							buffer_out_b[0x11+2]=0x01;
							buffer_out_b[0x11+3]=0x00;
							buffer_out_b[0x11+4]=0xFF;
							buffer_out_b[0x11+5]=0xE0;
							buffer_out_b[0x11+8]=0x01;

							switch(State.Cmd)
							{
								//Reset, in : 2 bytes, out : 0
								case 0xF0:
									break;

								//Find nodes?
								//In addressing Slave address, in : 2 bytes, out : 1
								case 0xF1:
									{
										buffer_out_len=4*4;
									}
									break;

								//Speed Change, in : 2 bytes, out : 0
								case 0xF2:
									break;

								//Name
								//"In the I / O ID" "Reading each slave ID data"
								//"NAMCO LTD.; I / O PCB-1000; ver1.0; for domestic only, no analog input"
								//in : 1 byte, out : max 102
								case 0x10:
									{
										static char ID1[102]="nullDC Team; I/O Plugin-1; ver0.2; for nullDC or other emus";
										buffer_out_b[0x8+0x10]=(BYTE)strlen(ID1)+3;
										for(int i=0;ID1[i]!=0;++i)
										{
											buffer_out_b[0x8+0x13+i]=ID1[i];
										}
									}
									break;

								//CMD Version
								//REV in command|Format command to read the (revision)|One|Two 
								//in : 1 byte, out : 2 bytes
								case 0x11:
									{
										buffer_out_b[0x8+0x13]=0x13;
									}
									break;

								//JVS Version
								//In JV REV|JAMMA VIDEO standard reading (revision)|One|Two 
								//in : 1 byte, out : 2 bytes
								case 0x12:
									{
										buffer_out_b[0x8+0x13]=0x30;
									}
									break;

								//COM Version
								//VER in the communication system|Read a communication system compliant version of |One|Two
								//in : 1 byte, out : 2 bytes
								case 0x13:
									{
										buffer_out_b[0x8+0x13]=0x10;
									}
									break;

								//Features
								//Check in feature |Each features a slave to read |One |6 to
								//in : 1 byte, out : 6 + (?)
								case 0x14:
									{
										unsigned char *FeatPtr=buffer_out_b+0x8+0x13;
										buffer_out_b[0x8+0x9+0x3]=0x0;
										buffer_out_b[0x8+0x9+0x9]=0x1;
										#define ADDFEAT(Feature,Count1,Count2,Count3)	*FeatPtr++=Feature; *FeatPtr++=Count1; *FeatPtr++=Count2; *FeatPtr++=Count3;
										ADDFEAT(1,2,12,0);	//Feat 1=Digital Inputs.  2 Players. 10 bits
										ADDFEAT(2,2,0,0);	//Feat 2=Coin inputs. 2 Inputs
										ADDFEAT(3,2,0,0);	//Feat 3=Analog. 2 Chans

										ADDFEAT(0,0,0,0);	//End of list
									}
									break;

								default:
									printf("unknown CAP %X\n",State.Cmd);
								return 0;
							}
							buffer_out_len=4*4;
						}
						else if(State.Mode==1 || State.Mode==2 || subcode==0x33)	//Get Data
						{
							unsigned char glbl=0x00;
							unsigned char p1_1=0x00;
							unsigned char p1_2=0x00;
							unsigned char p2_1=0x00;
							unsigned char p2_2=0x00;
							static unsigned char LastKey[256];
							static unsigned short coin1=0x0000;
							static unsigned short coin2=0x0000;
							unsigned char Key[256];
							GetKeyboardState(Key);
							
							if(keycode&NAOMI_SERVICE_KEY_1)			//Service ?
								glbl|=0x80;
							if(keycode&NAOMI_TEST_KEY_1)			//Test
								p1_1|=0x40;
							if(keycode&NAOMI_START_KEY)			//start ?
								p1_1|=0x80;
							if(keycode&NAOMI_UP_KEY)			//up
								p1_1|=0x20;
							if(keycode&NAOMI_DOWN_KEY)		//down
								p1_1|=0x10;
							if(keycode&NAOMI_LEFT_KEY)		//left
								p1_1|=0x08;
							if(keycode&NAOMI_RIGHT_KEY)		//right
								p1_1|=0x04;
							if(keycode&NAOMI_BTN0_KEY)			//btn1
								p1_1|=0x02;
							if(keycode&NAOMI_BTN1_KEY)			//btn2
								p1_1|=0x01;
							if(keycode&NAOMI_BTN2_KEY)			//btn3
								p1_2|=0x80;
							if(keycode&NAOMI_BTN3_KEY)			//btn4
								p1_2|=0x40;
							if(keycode&NAOMI_BTN4_KEY)			//btn5
								p1_2|=0x20;
							if(keycode&NAOMI_BTN5_KEY)			//btn6
								p1_2|=0x10;
							
							if(keycode2&NAOMI_TEST_KEY_1)			//Test
								p2_1|=0x40;
							if(keycode2&NAOMI_START_KEY)			//start ?
								p2_1|=0x80;
							if(keycode2&NAOMI_UP_KEY)			//up
								p2_1|=0x20;
							if(keycode2&NAOMI_DOWN_KEY)		//down
								p2_1|=0x10;
							if(keycode2&NAOMI_LEFT_KEY)		//left
								p2_1|=0x08;
							if(keycode2&NAOMI_RIGHT_KEY)		//right
								p2_1|=0x04;
							if(keycode2&NAOMI_BTN0_KEY)			//btn1
								p2_1|=0x02;
							if(keycode2&NAOMI_BTN1_KEY)			//btn2
								p2_1|=0x01;
							if(keycode2&NAOMI_BTN2_KEY)			//btn3
								p2_2|=0x80;
							if(keycode2&NAOMI_BTN3_KEY)			//btn4
								p2_2|=0x40;
							if(keycode2&NAOMI_BTN4_KEY)			//btn5
								p2_2|=0x20;
							if(keycode2&NAOMI_BTN5_KEY)			//btn6
								p2_2|=0x10;

							static bool old_coin =false;
							static bool old_coin2=false;

							if((old_coin==false) && (keycode&NAOMI_COIN_KEY))
								coin1++;
							old_coin = (keycode&NAOMI_COIN_KEY) ? true:false;

							if((old_coin2==false) && (keycode2&NAOMI_COIN_KEY))
								coin2++;
							old_coin2 = (keycode2&NAOMI_COIN_KEY) ? true:false;

							buffer_out_b[0x11+0]=0x00;
							buffer_out_b[0x11+1]=0x8E;	//Valid data check
							buffer_out_b[0x11+2]=0x01;
							buffer_out_b[0x11+3]=0x00;
							buffer_out_b[0x11+4]=0xFF;
							buffer_out_b[0x11+5]=0xE0;
							buffer_out_b[0x11+8]=0x01;

							//memset(OutData+8+0x11,0x00,0x100);

							buffer_out_b[8+0x12+0]=1;
							buffer_out_b[8+0x12+1]=glbl;
							buffer_out_b[8+0x12+2]=p1_1;
							buffer_out_b[8+0x12+3]=p1_2;
							buffer_out_b[8+0x12+4]=p2_1;
							buffer_out_b[8+0x12+5]=p2_2;
							buffer_out_b[8+0x12+6]=1;
							buffer_out_b[8+0x12+7]=coin1>>8;
							buffer_out_b[8+0x12+8]=coin1&0xff;
							buffer_out_b[8+0x12+9]=coin2>>8;
							buffer_out_b[8+0x12+10]=coin2&0xff;
							buffer_out_b[8+0x12+11]=1;
							buffer_out_b[8+0x12+12]=0x00;
							buffer_out_b[8+0x12+13]=0x00;
							buffer_out_b[8+0x12+14]=0x00;
							buffer_out_b[8+0x12+15]=0x00;
							buffer_out_b[8+0x12+16]=0x00;
							buffer_out_b[8+0x12+17]=0x00;
							buffer_out_b[8+0x12+18]=0x00;
							buffer_out_b[8+0x12+19]=0x00;
							buffer_out_b[8+0x12+20]=0x00;

							memcpy(LastKey,Key,sizeof(Key));

							if(State.Mode==1)
							{
								buffer_out_b[0x11+0x7]=19;
								buffer_out_b[0x11+0x4]=19+5;
							}
							else
							{
								buffer_out_b[0x11+0x7]=17;
								buffer_out_b[0x11+0x4]=17-1;
							}

							//OutLen=8+0x11+16;
							buffer_out_len=8+0x12+20;
						}
						/*ID.Keys=0xFFFFFFFF;
						if(GetKeyState(VK_F1)&0x8000)		//Service
						ID.Keys&=~(1<<0x1b);
						if(GetKeyState(VK_F2)&0x8000)		//Test
						ID.Keys&=~(1<<0x1a);
						memcpy(OutData,&ID,sizeof(ID));
						OutData[0x12]=0x8E;
						OutLen=sizeof(ID);
						*/
					}
					return 8;
					
				case 0x17:	//Select Subdevice
					{
						State.Mode=0;
						State.Cmd=buffer_in_b[8];
						State.Node=buffer_in_b[9];
						buffer_out_len=0;
					}
					return (7);
				
				case 0x27:	//Transfer request
					{
						State.Mode=1;
						State.Cmd=buffer_in_b[8];
						State.Node=buffer_in_b[9];
						buffer_out_len=0;
					}
					return (7);
				case 0x21:		//Transfer request with repeat
					{
						State.Mode=2;
						State.Cmd=buffer_in_b[8];
						State.Node=buffer_in_b[9];
						buffer_out_len=0;
					}
					return (7);

				case 0x0B:	//EEPROM write
					{
						int address=buffer_in_b[1];
						int size=buffer_in_b[2];
						//printf("EEprom write %08X %08X\n",address,size);
						//printState(Command,buffer_in,buffer_in_len);
						memcpy(EEPROM+address,buffer_in_b+4,size);

						wchar eeprom_file[512];
						host.ConfigLoadStr(L"emu",L"gamefile",eeprom_file,L"");
						wcscat(eeprom_file,L".eeprom");
						FILE* f=_wfopen(eeprom_file,L"wb");
						if (f)
						{
							fwrite(EEPROM,1,0x80,f);
							fclose(f);
							wprintf(L"SAVED EEPROM to %s\n",eeprom_file);
						}
					}
					return (7);
				case 0x3:	//EEPROM read
					{
						if (!EEPROM_loaded)
						{
							EEPROM_loaded=true;
							wchar eeprom_file[512];
							host.ConfigLoadStr(L"emu",L"gamefile",eeprom_file,L"");
							wcscat(eeprom_file,L".eeprom");
							FILE* f=_wfopen(eeprom_file,L"rb");
							if (f)
							{
								fread(EEPROM,1,0x80,f);
								fclose(f);
								wprintf(L"LOADED EEPROM from %s\n",eeprom_file);
							}
						}
						//printf("EEprom READ ?\n");
						int address=buffer_in_b[1];
						//printState(Command,buffer_in,buffer_in_len);
						memcpy(buffer_out,EEPROM+address,0x80);
						buffer_out_len=0x80;
					}
					return 8;
					//IF I return all FF, then board runs in low res
				case 0x31:
					{
						buffer_out[0]=0xffffffff;
						buffer_out[1]=0xffffffff;
					}
					return (8);
				
				//case 0x3:
				//	break;
				
				//case 0x1:
				//	break;
				default:
					printf("Unknown 0x86 : SubCommand 0x%X - State: Cmd 0x%X Mode :  0x%X Node : 0x%X\n",subcode,State.Cmd,State.Mode,State.Node);
					printState(Command,buffer_in,buffer_in_len);
				}

				return 8;//MAPLE_RESPONSE_DATATRF
			}
			break;
		case 0x82:
			{
				const char *ID="315-6149    COPYRIGHT SEGA E\x83\x00\x20\x05NTERPRISES CO,LTD.  ";
				memset(buffer_out_b,0x20,256);
				memcpy(buffer_out_b,ID,0x38-4);
				buffer_out_len=256;
				return (0x83);
			}
		/*typedef struct {
			DWORD		func;//4
			DWORD		function_data[3];//3*4
			u8		area_code;//1
			u8		connector_direction;//1
			char		product_name[30];//30*1
			char		product_license[60];//60*1
			WORD		standby_power;//2
			WORD		max_power;//2
		} maple_devinfo_t;*/
		case 1:
			{
				//header
				//WriteMem32(ptr_out,(u32)(0x05 | //response
				//			(((u16)sendadr << 8) & 0xFF00) |
				//			((((recadr == 0x20) ? 0x20 : 0) << 16) & 0xFF0000) |
				//			(((112/4) << 24) & 0xFF000000))); ptr_out += 4;


				//caps
				//4
				w32(1 << 24);

				//struct data
				//3*4
				w32( 0xfe060f00); 
				w32( 0);
				w32( 0);
				//1	area code
				w8(0xFF);
				//1	direction
				w8(0);
				//30
				for (u32 i = 0; i < 30; i++)
				{
					if (testJoy_strName[i]!=0)
					{
						w8((u8)testJoy_strName[i]);
					}
					else
					{
						w8(0x20);
					}
					//if (!testJoy_strName[i])
					//	break;
				}
				//ptr_out += 30;

				//60
				for (u32 i = 0; i < 60; i++)
				{
					if (testJoy_strBrand_2[i]!=0)
					{
						w8((u8)testJoy_strBrand_2[i]);
					}
					else
					{
						w8(0x20);
					}
					//if (!testJoy_strBrand[i])
					//	break;
				}
				//ptr_out += 60;

				//2
				w16(0xAE01); 

				//2
				w16(0xF401); 
			}
			return 5;

		/* controller condition structure 
		typedef struct {//8 bytes
		WORD buttons;			///* buttons bitfield	/2
		u8 rtrig;			///* right trigger			/1
		u8 ltrig;			///* left trigger 			/1
		u8 joyx;			////* joystick X 			/1
		u8 joyy;			///* joystick Y				/1
		u8 joy2x;			///* second joystick X 		/1
		u8 joy2y;			///* second joystick Y 		/1
		} cont_cond_t;*/
		case 9:
			{
				//header
				//WriteMem32(ptr_out, (u32)(0x08 | // data transfer (response)
				//			(((u16)sendadr << 8) & 0xFF00) |
				//			((((recadr == 0x20) ? 0x20 : 1) << 16) & 0xFF0000) |
				//			(((12 / 4 ) << 24) & 0xFF000000))); ptr_out += 4;
				//caps
				//4
				//WriteMem32(ptr_out, (1 << 24)); ptr_out += 4;
				w32(1 << 24);
				//struct data
				//2
				w16(kcode[port] | 0xF901); 
				
				//triger
				//1 R
				w8(rt[port]);
				//1 L
				w8(lt[port]); 
				//joyx
				//1
				w8(GetBtFromSgn(joyx[port]));
				//joyy
				//1
				w8(GetBtFromSgn(joyy[port]));

				//1
				w8(0x80); 
				//1
				w8(0x80); 
				//are these needed ?
				//1
				//WriteMem8(ptr_out, 10); ptr_out += 1;
				//1
				//WriteMem8(ptr_out, 10); ptr_out += 1;
			}
			return 8;

		default:
			printf("unknown MAPLE Frame\n");
			printState(Command,buffer_in,buffer_in_len);
			break;
	}
	return 0;
}
joy_state states[4];
SOCKET ConnectSocket = INVALID_SOCKET;
u32 local_port;
u32 send_ratio;
char server_addr[512];
char server_port[512];

#define MSG_WAITALL 0
void setups(SOCKET s)
{
	int flag = 1;
	int result = setsockopt(s,            /* socket affected */
		IPPROTO_TCP,     /* set option at TCP level */
		TCP_NODELAY,     /* name of option */
		(char *) &flag,  /* the cast is historical
						 cruft */
						 sizeof(int));    /* length of option value */
	flag=0;
	u_long t=0;
	ioctlsocket (s,FIONBIO ,&t);
}
u32 sync_counter=0;
u32 next_sync_counter=0;
bool np=false;
int Init_netplay()
{
	WSADATA wsaData;
    if (np)
		return 0;
	np=1;

    struct addrinfo *result = NULL,
                    *ptr = NULL,
                    hints;

    int iResult;
    

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
	iResult = getaddrinfo(server_addr, server_port, &hints, &result);
    if ( iResult != 0 ) {
        printf("getaddrinfo failed: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Attempt to connect to an address until one succeeds
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, 
            ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("Error at socket(): %ld\n", WSAGetLastError());
            freeaddrinfo(result);
            WSACleanup();
            return 1;
        }
		setups(ConnectSocket);
        // Connect to server.
        iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR && WSAEWOULDBLOCK!=WSAGetLastError()) {
			int rr=WSAGetLastError();
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
		Sleep(200);
        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

	joy_init t;
	strcpy(t.Name,"nullDC hookjoy plugin");
	t.port=local_port;
	t.Version=DC_MakeVersion(1,1,0);
    // Send an initial buffer
    iResult = send( ConnectSocket, (char*)&t, (int)sizeof(t), 0 );
    if (iResult == SOCKET_ERROR) {
        printf("send failed: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

	printf("Bytes Sent: %ld\n", iResult);

	joy_init_resp r;

	int dv=recv(ConnectSocket,(char*)&r,sizeof(r),MSG_WAITALL);
	//__asm int 3;
	printf("Server : %d,%d,%d\n",r.ratio,r.mode,r.ratio);
	send_ratio=r.ratio;
    


    // cleanup


    return 0;
}

void net_read()
{
	int rv = recv(ConnectSocket,(char*)&states,sizeof(states),MSG_WAITALL);
	if (rv==0xFFFFFF)
		printf("net_read fail %d\n",WSAGetLastError());
	printf("SYNC1 %d %d\nSYNC2 %d %d %d %d\n",sync_counter,next_sync_counter,states[0].id,states[1].id,states[2].id,states[3].id);
}
void net_send()
{
	//u32* at1=(u32*)0x00aaa150;
	//u32* at2=(u32*)0x00aaa17C;
	
	joy_state t;
	//t.id=*at1+*at2;
	t.substates[0].jx=joyx[local_port];
	t.substates[0].jy=joyy[local_port];
	t.substates[0].l=lt[local_port];
	t.substates[0].r=rt[local_port];
	t.substates[0].state=kcode[local_port];
	int rv = send(ConnectSocket,(char*)&t,sizeof(t),0);
}

void termnet()
{
	closesocket(ConnectSocket);
    WSACleanup();
}
u32 GetMaplePort(u32 addr)
{
	/*
	for (int i=0;i<6;i++)
	{
		if ((1<<i)&addr)
			return i;
	}*/
	return addr>>6;
}

u32 sync_net(u32 port)
{
	if (port==0)
	{
		sync_counter++;
		if (sync_counter==1)
			net_send();
		verify(sync_counter<=next_sync_counter);

		if (sync_counter==next_sync_counter)
		{
			net_read();
			next_sync_counter=sync_counter+send_ratio;
			net_send();
		}
		//printf("%d - %d - %d\n",sync_counter,next_sync_counter,send_ratio);
		verify(sync_counter<next_sync_counter);
	}
	return sync_counter % send_ratio;
}
u32 FASTCALL ControllerDMA_net(void* device_instance,u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len)
{
	//printf("ControllerDMA Called 0x%X;Command %d\n",device_instance->port,Command);
//void testJoy_GotData(u32 header1,u32 header2,u32*data,u32 datalen)
	u8*buffer_out_b=(u8*)buffer_out;
	
	bool islocal=((maple_device_instance*)device_instance)->port==local_port;
	switch (Command)
	{
		/*typedef struct {
			DWORD		func;//4
			DWORD		function_data[3];//3*4
			u8		area_code;//1
			u8		connector_direction;//1
			char		product_name[30];//30*1
			char		product_license[60];//60*1
			WORD		standby_power;//2
			WORD		max_power;//2
		} maple_devinfo_t;*/
		case 1:
			//header
			//WriteMem32(ptr_out,(u32)(0x05 | //response
			//			(((u16)sendadr << 8) & 0xFF00) |
			//			((((recadr == 0x20) ? 0x20 : 0) << 16) & 0xFF0000) |
			//			(((112/4) << 24) & 0xFF000000))); ptr_out += 4;


			//caps
			//4
			w32(1 << 24);

			//struct data
			//3*4
			w32( 0xfe060f00); 
			w32( 0);
			w32( 0);
			//1	area code
			w8(0xFF);
			//1	direction
			w8(0);
			//30
			for (u32 i = 0; i < 30; i++)
			{
				w8((u8)testJoy_strName_net[i]);
				//if (!testJoy_strName[i])
				//	break;
			}
			//ptr_out += 30;

			//60
			for (u32 i = 0; i < 60; i++)
			{
				w8((u8)testJoy_strBrand_2[i]);
				//if (!testJoy_strBrand[i])
				//	break;
			}
			//ptr_out += 60;

			//2
			w16(0x01AE); 

			//2
			w16(0x01F4); 
			return 5;

		/* controller condition structure 
		typedef struct {//8 bytes
		WORD buttons;			///* buttons bitfield	/2
		u8 rtrig;			///* right trigger			/1
		u8 ltrig;			///* left trigger 			/1
		u8 joyx;			////* joystick X 			/1
		u8 joyy;			///* joystick Y				/1
		u8 joy2x;			///* second joystick X 		/1
		u8 joy2y;			///* second joystick Y 		/1
		} cont_cond_t;*/
		case 9:
			{
				u32 aport=GetMaplePort(((maple_device_instance*)device_instance)->port);
				u32 ssf=sync_net(aport);
			/*
				char file[43];
			sprintf(file,"log_%d.raw",aport);
			FILE* printf=fopen(file,"a");
			fseek(printf,0,SEEK_END);
			char* bvvvv=(char*)buffer_out_b;
			*/
			//header
			//WriteMem32(ptr_out, (u32)(0x08 | // data transfer (response)
			//			(((u16)sendadr << 8) & 0xFF00) |
			//			((((recadr == 0x20) ? 0x20 : 1) << 16) & 0xFF0000) |
			//			(((12 / 4 ) << 24) & 0xFF000000))); ptr_out += 4;
			//caps
			//4
			//WriteMem32(ptr_out, (1 << 24)); ptr_out += 4;
			w32((1 << 24));
			//struct data
			//2
			w16(states[aport].substates[ssf].state); 
			
			//triger
			//1 R
			w8(states[aport].substates[ssf].r);
			//1 L
			w8(states[aport].substates[ssf].l); 
			//joyx
			//1
			w8(GetBtFromSgn(states[aport].substates[ssf].jx));
			//joyy
			//1
			w8(GetBtFromSgn(states[aport].substates[ssf].jy));

			//1
			w8(GetBtFromSgn(0)); 
			//1
			w8(GetBtFromSgn(0)); 
			//are these needed ?
			//1
			//WriteMem8(ptr_out, 10); ptr_out += 1;
			//1
			//WriteMem8(ptr_out, 10); ptr_out += 1;
			/*
			fwrite(bvvvv,12,1,printf);
			fclose(printf);
			*/
			}
			return 8;

		default:
			printf("unknown MAPLE COMMAND %d\n",Command);
			return 7;
	}
}

u32 FASTCALL ControllerDMA_nul(void* device_instance,u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len)
{
	//printf("ControllerDMA Called 0x%X;Command %d\n",device_instance->port,Command);
//void testJoy_GotData(u32 header1,u32 header2,u32*data,u32 datalen)
	u8*buffer_out_b=(u8*)buffer_out;

	switch (Command)
	{
		/*typedef struct {
			DWORD		func;//4
			DWORD		function_data[3];//3*4
			u8		area_code;//1
			u8		connector_direction;//1
			char		product_name[30];//30*1
			char		product_license[60];//60*1
			WORD		standby_power;//2
			WORD		max_power;//2
		} maple_devinfo_t;*/
		case 1:
			//header
			//WriteMem32(ptr_out,(u32)(0x05 | //response
			//			(((u16)sendadr << 8) & 0xFF00) |
			//			((((recadr == 0x20) ? 0x20 : 0) << 16) & 0xFF0000) |
			//			(((112/4) << 24) & 0xFF000000))); ptr_out += 4

			//caps
			//4
			w32(1 << 24);

			//struct data
			//3*4
			w32( 0xfe060f00); 
			w32( 0);
			w32( 0);
			//1	area code
			w8(0xFF);
			//1	direction
			w8(0);
			//30
			for (u32 i = 0; i < 30; i++)
			{
				w8((u8)testJoy_strName_nul[i]);
				//if (!testJoy_strName[i])
				//	break;
			}
			//ptr_out += 30;

			//60
			for (u32 i = 0; i < 60; i++)
			{
				w8((u8)testJoy_strBrand_2[i]);
				//if (!testJoy_strBrand[i])
				//	break;
			}
			//ptr_out += 60;

			//2
			w16(0x01AE); 

			//2
			w16(0x01F4); 
			return 5;

		/* controller condition structure 
		typedef struct {//8 bytes
		WORD buttons;			///* buttons bitfield	/2
		u8 rtrig;			///* right trigger			/1
		u8 ltrig;			///* left trigger 			/1
		u8 joyx;			////* joystick X 			/1
		u8 joyy;			///* joystick Y				/1
		u8 joy2x;			///* second joystick X 		/1
		u8 joy2y;			///* second joystick Y 		/1
		} cont_cond_t;*/
		case 9:


			//header
			//WriteMem32(ptr_out, (u32)(0x08 | // data transfer (response)
			//			(((u16)sendadr << 8) & 0xFF00) |
			//			((((recadr == 0x20) ? 0x20 : 1) << 16) & 0xFF0000) |
			//			(((12 / 4 ) << 24) & 0xFF000000))); ptr_out += 4;
			//caps
			//4
			//WriteMem32(ptr_out, (1 << 24)); ptr_out += 4;
			w32((1 << 24));
			//struct data
			//2
			w16(0xFFFF); 
			
			//triger
			//1 R
			w8(0);
			//1 L
			w8(0); 
			//joyx
			//1
			w8(GetBtFromSgn(0));
			//joyy
			//1
			w8(GetBtFromSgn(0));

			//1
			w8(GetBtFromSgn(0)); 
			//1
			w8(GetBtFromSgn(0)); 
			//are these needed ?
			//1
			//WriteMem8(ptr_out, 10); ptr_out += 1;
			//1
			//WriteMem8(ptr_out, 10); ptr_out += 1;

			return 8;

		default:
			printf("unknown MAPLE COMMAND %d\n",Command);
			return 7;
	}
}

u8 GetBtFromSgn(s8 val)
{
	return val+128;
}

typedef struct {
	u16 total_size;
	u16 partition_number;
	u16 system_area_block;
	u16 fat_area_block;
	u16 number_fat_areas_block;
	u16 file_info_block;
	u16 number_info_blocks;
	u8 volume_icon;
	u8 reserved1;
	u16 save_area_block;
	u16 number_of_save_blocks;
	u16 reserverd0;  
}maple_getvmuinfo_t;

#define SWAP32(val) ((u32) ( \
	(((u32) (val) & (u32) 0x000000ffU) << 24) | \
	(((u32) (val) & (u32) 0x0000ff00U) <<  8) | \
	(((u32) (val) & (u32) 0x00ff0000U) >>  8) | \
	(((u32) (val) & (u32) 0xff000000U) >> 24)))

INT_PTR CALLBACK VMULCDProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	VMU_info* dev=(VMU_info*) (GetWindowLongPtr(hWnd,GWLP_USERDATA));
	switch(msg)
	{	
		case WM_CLOSE:
			ShowWindow(hWnd,SW_HIDE);
			break;
		case WM_INITDIALOG:
			{
				SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);
			}
			return TRUE;
		case WM_ERASEBKGND:
			return TRUE;
		case WM_PAINT:
			{
				PAINTSTRUCT ps;
				HDC hdc=BeginPaint(hWnd,&ps);
				int x=0;//(ps.rcPaint.left)/3;
				int y=0;//(ps.rcPaint.top)/3;
				int wx=48;//(ps.rcPaint.right+2)/3-x;
				int wy=32;//(ps.rcPaint.bottom+2)/3-y;
				
				
				SetStretchBltMode(hdc,BLACKONWHITE);

				StretchDIBits(hdc,	x*3,y*3,wx*3,wy*3,
									x  ,y  ,wx  ,wy  ,
									dev->lcd.bitmap,&vmu_bmi,DIB_RGB_COLORS,SRCCOPY);

				EndPaint(hWnd,&ps);
			}
			break;
		//case WM_ACTIVATE:
		//	printf("VMU GOT FOCUS !\n");
		//	break;
	}
	return FALSE;
}

u32 FASTCALL VmuDMA(void* device_instance,u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len)
{
	VMU_info* dev=(VMU_info*)(device_instance);
	u8*buffer_out_b=(u8*)buffer_out;
	//printf("VmuDMA Called for port 0x%X, Command %d\n",device_instance->port,Command);
	switch (Command)
	{
		/*typedef struct {
			DWORD		func;//4
			DWORD		function_data[3];//3*4
			u8		area_code;//1
			u8		connector_direction;//1
			char		product_name[30];//30*1
			char		product_license[60];//60*1
			WORD		standby_power;//2
			WORD		max_power;//2
		} maple_devinfo_t;*/
		case MDC_DeviceRequest:
		{
			//header
			//WriteMem32(ptr_out,(u32)(0x05 | //response
			//			(((u16)sendadr << 8) & 0xFF00) |
			//			((((recadr == 0x20) ? 0x20 : 0) << 16) & 0xFF0000) |
			//			(((112/4) << 24) & 0xFF000000))); ptr_out += 4;

			//caps
			//4
			w32(MFID_1_Storage | MFID_2_LCD | MFID_3_Clock);

			//struct data
			//3*4
			w32( 0x403f7e7e);	//for clock
			w32( 0x00100500);	//for LCD
			w32( 0x00410f00);	//for storage
			//1	area code
			w8(0xFF);
			//1	direction
			w8(0);
			//30
			for (u32 i = 0; i < 30; i++)
			{
				if (testJoy_strName_vmu[i])
				{
					w8((u8)testJoy_strName_vmu[i]);
				}
				else
				{
					w8(' ');
				}
				
				//	break;
			}
			//ptr_out += 30;

			//60
			for (u32 i = 0; i < 60; i++)
			{
				if (testJoy_strBrand_2[i])
				{
					w8((u8)testJoy_strBrand_2[i]);
				}
				else
				{
					w8(' ');
				}
				//if (!testJoy_strBrand[i])
				//	break;
			}
			//ptr_out += 60;

			//2
			w16(0x007c); 

			//2
			w16(0x0082); 
			return MDRS_DeviseStatus;
		}

				//in[0] is function used ?
				//out[0] is function used ?
		case MDCF_GetMediaInfo:
		{
			if (buffer_in[0]& MFID_1_Storage)
			{
				buffer_out[0] = MFID_1_Storage;//is that ok ?
				maple_getvmuinfo_t* vmui = (maple_getvmuinfo_t*)(&buffer_out[1]);
				//ZeroMemory(pMediaInfo,sizeof(TMAPLE_MEDIAINFO));
				memset(vmui,0,sizeof(maple_getvmuinfo_t));
				vmui->total_size = 0xFF;//0x7FFF;//0xFF
				vmui->system_area_block = 0xFF;//0x7FFF;//0xff
				vmui->fat_area_block = 0xfe;//0x7F00;	//0xfe
				vmui->number_fat_areas_block = 1;//256;//1
				vmui->volume_icon = 0x0;//0
				vmui->save_area_block = 0xc8;//?
				vmui->number_of_save_blocks = 0x1f;
				//pMediaInfo->volume_icon = 0x0;
				vmui->file_info_block = 0xfd;//0x7E00;//0xfd
				vmui->number_info_blocks = 0xd;//0x100;//0xd
				vmui->reserverd0 = 0x0000;
				buffer_out_len=4+(sizeof(maple_getvmuinfo_t));
				return MDRS_DataTransfer;//data transfer
			}
			else if (buffer_in[0]&MFID_2_LCD)
			{
				if (buffer_in[1]!=0)
				{
					printf("VMU: MDCF_GetMediaInfo -> bad input |%08X|, returning MDRE_UnknownCmd\n",buffer_in[0]);
					return MDRE_UnknownCmd;
				}
				else
				{
					w32(MFID_2_LCD);

					w8(47);				//X dots -1
					w8(31);				//Y dots -1
					w8(((1)<<4) | (0));		//1 Color, 0 contrast levels
					w8(0);					//Padding
					return MDRS_DataTransfer;
				}
			}
			else
			{
				printf("VMU: MDCF_GetMediaInfo -> Bad function used |%08X|, returning -2\n",buffer_in[0]);
				return MDRE_UnknownFunction;//bad function
			}

		case MDCF_BlockRead:
			if(buffer_in[0]&MFID_1_Storage)
			{
				//VMU_info* dev=(VMU_info*)((*device_instance).data);

				buffer_out[0] = MFID_1_Storage;
				u32 Block = (SWAP32(buffer_in[1]))&0xffff;
				buffer_out[1] = buffer_in[1];
				if (Block>255)
				{
					printf("Block read : %d\n",Block);
					printf("BLOCK READ ERROR\n");
					Block&=255;
				}
				memcpy(&buffer_out[2],(dev->data)+Block*512,512);
				buffer_out_len=(512+8);
				return MDRS_DataTransfer;//data transfer
			}
			else if (buffer_in[0]&MFID_2_LCD)
			{
				buffer_out[0] = MFID_2_LCD;
				buffer_out[1] = buffer_in[1];
				memcpy(&buffer_out[2],(dev->lcd.data),192);
				buffer_out_len=(192+8);
				return MDRS_DataTransfer;//data transfer
			}
			else if (buffer_in[0]&MFID_3_Clock)
			{
				if (buffer_in[1]!=0 || buffer_in_len!=8)
				{
					printf("VMU: Block read: MFID_3_Clock : invalid params \n");
					return MDRE_TransminAgain;		//invalid params
				}
				else
				{
					buffer_out[0] = MFID_3_Clock;
					buffer_out_len=12;
					
					time_t now;
					time(&now);
					tm* timenow=localtime(&now);
					u8* timebuf=(u8*)&buffer_in[1];	//YY M D H M S DotY
					
					timebuf[0]=(timenow->tm_year+1900)%256;
					timebuf[1]=(timenow->tm_year+1900)/256;

					timebuf[2]=timenow->tm_mon+1;
					timebuf[3]=timenow->tm_mday;

					timebuf[4]=timenow->tm_hour;
					timebuf[5]=timenow->tm_min;
					timebuf[6]=timenow->tm_sec;
					timebuf[7]=0;

					printf("VMU: CLOCK Read-> datetime is %04d/%02d/%02d ~ %02d:%02d:%02d!\n",timebuf[0]+timebuf[1]*256,timebuf[2],timebuf[3],timebuf[4],timebuf[5],timebuf[6]);

					return MDRS_DataTransfer;//transfer reply ...
				}
			}
			else 
			{
				printf("VMU: cmd MDCF_BlockRead -> Bad function used, returning -2\n");
				return MDRE_UnknownFunction;//bad function
			}
			break;
		}

		case MDCF_BlockWrite:
		{
			if(buffer_in[0]&MFID_1_Storage)
			{
				u32 Block = (SWAP32(buffer_in[1]))&0xffff;
				u32 Phase = ((SWAP32(buffer_in[1]))>>16)&0xff; 
				//printf("Block wirte : %d:%d , %d bytes\n",Block,Phase,(buffer_in_len-8));
				memcpy(&dev->data[Block*512+Phase*(512/4)],&buffer_in[2],(buffer_in_len-8));
				buffer_out_len=0;
				FILE* f=fopen(dev->file,"wb");
				if (f)
				{
					fwrite(dev->data,1,128*1024,f);
					fclose(f);
				}
				else
				{
					printf("Failed to open %s for saving vmu data\n",dev->file);
					//return MDRE_FileError; -> this also has to return an error bitfield, will do so later on ...
				}
				return MDRS_DeviceReply;//just ko
			}
			else if (buffer_in[0]&MFID_2_LCD)
			{
				memcpy(dev->lcd.data,&buffer_in[2],192);
				//Update lcd window
				if (g_ShowVMU)
				{					
					ShowWindow(dev->lcd.handle,SHOW_OPENNOACTIVATE);
				}
				//if(LCDBitmap)
				{
					const WORD white=0xffff;
					const WORD black=0x0000;

					for(int y=0;y<32;++y)
					{
						WORD *dst=dev->lcd.bitmap+y*48;
						BYTE *src=dev->lcd.data+6*y+5;
						for(int x=0;x<48/8;++x)
						{
							BYTE val=*src;
							for(int m=0;m<8;++m)
							{
								if(val&(1<<(m)))
									*dst++=black;
								else
									*dst++=white;
							}
							--src;
						}
					}
					InvalidateRect(dev->lcd.handle,NULL,FALSE);
				}

	#ifdef _HAS_LGLCD_
				{
					lgLcdBitmap160x43x1 bmp;
					bmp.hdr.Format = LGLCD_BMP_FORMAT_160x43x1;

					const BYTE white=0x00;
					const BYTE black=0xFF;

					//make it all black...
					memset(bmp.pixels,black,sizeof(bmp.pixels));

					//decode from the vmu
					for(int y=0;y<32;++y)
					{
						BYTE *dst=bmp.pixels+5816+((-y)*(48+112)); //ugly way to make things look right :p
						BYTE *src=dev->lcd.data+6*y+5;
						for(int x=0;x<48/8;++x)
						{
							BYTE val=*src;
							for(int m=0;m<8;++m)
							{
								if(val&(1<<(m)))
									*dst++=black;
								else
									*dst++=white;
							}
							--src;
						}
					}
					
					//Set the damned bits
					if (((int)dev->lcd.handle)==gA0temp)
					{
						gA0 = lgLcdUpdateBitmap(openContextA0.device, &bmp.hdr, LGLCD_ASYNC_UPDATE(LGLCD_PRIORITY_ALERT));
					}
					if (((int)dev->lcd.handle)==gB0temp)
					{
						gB0 = lgLcdUpdateBitmap(openContextB0.device, &bmp.hdr, LGLCD_ASYNC_UPDATE(LGLCD_PRIORITY_ALERT));
					}
					if (((int)dev->lcd.handle)==gC0temp)
					{
						gC0 = lgLcdUpdateBitmap(openContextC0.device, &bmp.hdr, LGLCD_ASYNC_UPDATE(LGLCD_PRIORITY_ALERT));
					}
					if (((int)dev->lcd.handle)==gD0temp)
					{
						gD0 = lgLcdUpdateBitmap(openContextD0.device, &bmp.hdr, LGLCD_ASYNC_UPDATE(LGLCD_PRIORITY_ALERT));
					}
					gused=1;
				}
	#endif

				return  MDRS_DeviceReply;//just ko
			}
			else if (buffer_in[0]&MFID_3_Clock)
			{
				if (buffer_in[1]!=0 || buffer_in_len!=16)
					return MDRE_TransminAgain;	//invalid params ...
				else
				{
					u8* timebuf=(u8*)&buffer_in[2];	//YY M D H M S DotY
					printf("VMU: CLOCK Write-> datetime is %04d/%02d/%02d ~ %02d:%02d:%02d! Nothing set tho ...\n",timebuf[0]+timebuf[1]*256,timebuf[2],timebuf[3],timebuf[4],timebuf[5],timebuf[6]);
					return  MDRS_DeviceReply;//ok !
				}
			}
			else
			{
				printf("VMU: cmd MDCF_BlockWrite -> Bad function used, returning MDRE_UnknownFunction\n");
				return  MDRE_UnknownFunction;//bad function
			}
			break;
		}

		case MDCF_GetLastError:
			return MDRS_DeviceReply;//just ko
			break;

		case MDCF_SetCondition:
		{
			if (buffer_in[0]&MFID_3_Clock)
			{
				if (buffer_in[1])
				{
					u8* beepbuf=(u8*)&buffer_in[1];
					printf("BEEP : %d %d | %d %d\n",beepbuf[0],beepbuf[1],beepbuf[2],beepbuf[3]);
				}
				return  MDRS_DeviceReply;//just ko
			}
			else
			{
				printf("VMU: cmd MDCF_SetCondition -> Bad function used, returning MDRE_UnknownFunction\n");
				return MDRE_UnknownFunction;//bad function
			}
			break;
		}
			

		default:
			printf("unknown MAPLE COMMAND %d\n",Command);
			return MDRE_UnknownCmd;
	}

}

void EXPORT_CALL vmu_showwindow(u32 id,void* w,void* p)
{
	if (g_ShowVMU) 
	{
		ShowWindow((HWND)p,SW_HIDE);
		g_ShowVMU=0;
	}
	else 
	{				
		ShowWindow((HWND)p,SHOW_OPENNOACTIVATE);
		g_ShowVMU=1;
	}

	host.SetMenuItemStyle(id,g_ShowVMU?MIS_Checked:0,MIS_Checked);
	
	SaveSettings();	
}

void EXPORT_CALL config_keys(u32 id,void* w,void* p)
{
	maple_device_instance* mdd=(maple_device_instance*)p;
	current_port=mdd->port>>6;
#ifdef BUILD_NAOMI
	DialogBox(hInstance,MAKEINTRESOURCE(IDD_DIALOG1),(HWND)w,ConfigKeysDlgProc);
#else
	DialogBox(hInstance,MAKEINTRESOURCE(IDD_ConfigKeys),(HWND)w,ConfigKeysDlgProc);
#endif
}
s32 FASTCALL CreateMain(maple_device_instance* inst,u32 id,u32 flags,u32 rootmenu)
{
	inst->data=inst;
	wchar temp[512];
	if (id<=1)
	{
		swprintf(temp,sizeof(temp),L"Config keys for Player %d",(inst->port>>6)+1);
		u32 ckid=host.AddMenuItem(rootmenu,-1,temp,config_keys,0);
		MenuItem mi;
		mi.PUser=inst;
		host.SetMenuItem(ckid,&mi,MIM_PUser);
	}
	if (id==0)
	{
#ifdef BUILD_NAOMI
		inst->dma=ControllerDMA_naomi;
#else
		inst->dma=ControllerDMA;
#endif
		swprintf_s(temp,L"Controller[winhook] : 0x%02X",inst->port);
	}
	else if (id==1)
	{
		inst->dma=ControllerDMA_net;
		swprintf_s(temp,L"Controller[winhook,net] : 0x%02X",inst->port);
	}
	else if (id==3)
	{
		inst->dma=KbdDMA;
		swprintf_s(temp,L"Keyboard : 0x%02X",inst->port);
	}
	else if (id==4)
	{
		inst->dma=ControllerDMA_nul;
		swprintf_s(temp,L"Controller [no input] : 0x%02X",inst->port);
	}
	else if (id==5)
	{
		inst->dma=MouseDMA;
		swprintf_s(temp,L"Mouse [winhook] : 0x%02X",inst->port);
	}
	host.AddMenuItem(rootmenu,-1,temp,0,0);
/*
	else if (id==2)
	{
		
	}
	else if (id==3)
	{
		inst->dma=KbdDMA;
		inst->data=0;
	}
	else if (id==4)
	{
		inst->dma=MouseDMA;
		inst->data=0;
	}
	else if ( id==5)
	{
		inst->dma=DreamEye_mainDMA;
		inst->data=0;
	}
	else if ( id==6)
	{
		inst->dma=DreamEye_subDMA;
		inst->data=0;
	}
	else if ( id==7)
	{
		inst->dma=MicDMA;
		inst->data=0;
	}
	else
		return false;
*/
	return rv_ok;
	//printf("Created instance of device %s on port 0x%x\n",dev->name,port);
}


s32 FASTCALL CreateSub(maple_subdevice_instance* inst,u32 id,u32 flags,u32 rootmenu)
{
	wchar wtemp[512];
	swprintf_s(wtemp,L"VMU :vmu_data_port%02X.bin",inst->port);
	host.AddMenuItem(rootmenu,-1,wtemp,0,0);	

	u32 mitem = host.AddMenuItem(rootmenu,-1,L"Show VMU",vmu_showwindow,g_ShowVMU);
	
	inst->data=malloc(sizeof(VMU_info));
	sprintf(((VMU_info*)inst->data)->file,"vmu_data_port%02X.bin",inst->port);
	((VMU_info*)inst->data)->lcd.handle=0;
	FILE* f=fopen(((VMU_info*)inst->data)->file,"rb");
	if (!f)
		f=fopen("data\\vmu_default.bin","rb");
	if (f)
	{
		fread(((VMU_info*)inst->data)->data,1,128*1024,f);
		fclose(f);
	}
	inst->dma=VmuDMA;
	VMU_info* dev=(VMU_info* )inst->data;

	dev->lcd.handle=CreateDialogParam(hInstance,MAKEINTRESOURCE(IDD_LCD),0,VMULCDProc,(LPARAM)dev);

	MenuItem mi;
	mi.PUser=dev->lcd.handle;
	host.SetMenuItem(mitem,&mi,MIM_PUser);
	
	if (vmu_bmi.bmiHeader.biSize==0)
	{
		memset(&vmu_bmi,0,sizeof(BITMAPINFO));
		BITMAPINFO& bmi=vmu_bmi;

		bmi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biBitCount=16;
		bmi.bmiHeader.biCompression=BI_RGB;
		bmi.bmiHeader.biHeight=32;
		bmi.bmiHeader.biWidth=48;
		bmi.bmiHeader.biPlanes=1;
		//memset(&bmi.bmiColors[0],0x00,4);
		//memset(&bmi.bmiColors[1],0xFF,4);
	}

	memset(dev->lcd.bitmap,0xff,32*48*2);

	RECT rc={0,0,48*3,32*3};
	AdjustWindowRectEx(&rc,GetWindowLong(dev->lcd.handle,GWL_STYLE),FALSE,GetWindowLong(dev->lcd.handle,GWL_EXSTYLE));
	static int lastPosX=0;
	static int lastPosY=0;
	SetWindowPos(dev->lcd.handle,NULL,32+lastPosX*192,32+lastPosY*128,rc.right-rc.left,rc.bottom-rc.top,SWP_NOZORDER|SWP_NOACTIVATE);
	lastPosX++;
	if (lastPosX>2)
	{
		lastPosY++;
		lastPosX=0;
	}
	if (lastPosY>4)
		lastPosY=0;
	wchar windowtext[512];
	swprintf_s(windowtext,L"nullDC VMU %c%d",'A'+(inst->port>>6),(int)(log10f((float)(inst->port&31))/log10f(2.0f)));
	SetWindowText(dev->lcd.handle,windowtext);
	EnableWindow(dev->lcd.handle,TRUE);

	#ifdef _HAS_LGLCD_
    
	if ((inst->port>>6)==0 & (inst->port&31)==1)
	{
	gA0temp=((int)dev->lcd.handle);
    //// connect to LCDMon
    // set up connection context
    ZeroMemory(&connectContextA0, sizeof(connectContextA0));
    connectContextA0.appFriendlyName = (dev->lcd.handle,windowtext);
    connectContextA0.isAutostartable = FALSE;
    connectContextA0.isPersistent = FALSE;
    // we don't have a configuration screen
    connectContextA0.onConfigure.configCallback = NULL;
    connectContextA0.onConfigure.configContext = NULL;
    // the "connection" member will be returned upon return
    connectContextA0.connection = LGLCD_INVALID_CONNECTION;
    // and connect
    gA0 = lgLcdConnect(&connectContextA0);
    HandleError(gA0, _T("lgLcdConnect"));

	// now we are connected (and have a connection handle returned),
    // let's enumerate an LCD (the first one, index = 0)
    lgLcdDeviceDesc deviceDescription;
    gA0 = lgLcdEnumerate(connectContextA0.connection, 0, &deviceDescription);
    HandleError(gA0, _T("lgLcdEnumerate"));

    // open it
    ZeroMemory(&openContextA0, sizeof(openContextA0));
    openContextA0.connection = connectContextA0.connection;
    openContextA0.index = 0;
    // we have no softbutton notification callback
    openContextA0.onSoftbuttonsChanged.softbuttonsChangedCallback = NULL;
    openContextA0.onSoftbuttonsChanged.softbuttonsChangedContext = NULL;
    // the "device" member will be returned upon return
    openContextA0.device = LGLCD_INVALID_DEVICE;
    gA0 = lgLcdOpen(&openContextA0);
    HandleError(gA0, _T("lgLcdEnumerate"));
	gA0used=1;

	// coming back from lgLcdOpen, we have a device handle (in openContextA0.device)
    // which we will be using from now on until the program exits
	}
	   
	if ((inst->port>>6)==1 & (inst->port&31)==1)
	{
	gB0temp=((int)dev->lcd.handle);
    //// connect to LCDMon
    // set up connection context
    ZeroMemory(&connectContextB0, sizeof(connectContextB0));
    connectContextB0.appFriendlyName = (dev->lcd.handle,windowtext);
    connectContextB0.isAutostartable = FALSE;
    connectContextB0.isPersistent = FALSE;
    // we don't have a configuration screen
    connectContextB0.onConfigure.configCallback = NULL;
    connectContextB0.onConfigure.configContext = NULL;
    // the "connection" member will be returned upon return
    connectContextB0.connection = LGLCD_INVALID_CONNECTION;
    // and connect
    gB0 = lgLcdConnect(&connectContextB0);
    HandleError(gB0, _T("lgLcdConnect"));

	// now we are connected (and have a connection handle returned),
    // let's enumerate an LCD (the first one, index = 0)
    lgLcdDeviceDesc deviceDescription;
    gB0 = lgLcdEnumerate(connectContextB0.connection, 0, &deviceDescription);
    HandleError(gB0, _T("lgLcdEnumerate"));

    // open it
    ZeroMemory(&openContextB0, sizeof(openContextB0));
    openContextB0.connection = connectContextB0.connection;
    openContextB0.index = 0;
    // we have no softbutton notification callback
    openContextB0.onSoftbuttonsChanged.softbuttonsChangedCallback = NULL;
    openContextB0.onSoftbuttonsChanged.softbuttonsChangedContext = NULL;
    // the "device" member will be returned upon return
    openContextB0.device = LGLCD_INVALID_DEVICE;
    gB0 = lgLcdOpen(&openContextB0);
    HandleError(gB0, _T("lgLcdEnumerate"));
	gB0used=1;

	// coming back from lgLcdOpen, we have a device handle (in openContextB0.device)
    // which we will be using from now on until the program exits
	}
	   
	if ((inst->port>>6)==2 & (inst->port&31)==1)
	{
	gC0temp=((int)dev->lcd.handle);
    //// connect to LCDMon
    // set up connection context
    ZeroMemory(&connectContextC0, sizeof(connectContextC0));
    connectContextC0.appFriendlyName = (dev->lcd.handle,windowtext);
    connectContextC0.isAutostartable = FALSE;
    connectContextC0.isPersistent = FALSE;
    // we don't have a configuration screen
    connectContextC0.onConfigure.configCallback = NULL;
    connectContextC0.onConfigure.configContext = NULL;
    // the "connection" member will be returned upon return
    connectContextC0.connection = LGLCD_INVALID_CONNECTION;
    // and connect
    gC0 = lgLcdConnect(&connectContextC0);
    HandleError(gC0, _T("lgLcdConnect"));

	// now we are connected (and have a connection handle returned),
    // let's enumerate an LCD (the first one, index = 0)
    lgLcdDeviceDesc deviceDescription;
    gC0 = lgLcdEnumerate(connectContextC0.connection, 0, &deviceDescription);
    HandleError(gC0, _T("lgLcdEnumerate"));

    // open it
    ZeroMemory(&openContextC0, sizeof(openContextC0));
    openContextC0.connection = connectContextC0.connection;
    openContextC0.index = 0;
    // we have no softbutton notification callback
    openContextC0.onSoftbuttonsChanged.softbuttonsChangedCallback = NULL;
    openContextC0.onSoftbuttonsChanged.softbuttonsChangedContext = NULL;
    // the "device" member will be returned upon return
    openContextC0.device = LGLCD_INVALID_DEVICE;
    gC0 = lgLcdOpen(&openContextC0);
    HandleError(gC0, _T("lgLcdEnumerate"));
	gC0used=1;

	// coming back from lgLcdOpen, we have a device handle (in openContextC0.device)
    // which we will be using from now on until the program exits
	}
	   
	if ((inst->port>>6)==3 & (inst->port&31)==1)
	{
	gD0temp=((int)dev->lcd.handle);
    //// connect to LCDMon
    // set up connection context
    ZeroMemory(&connectContextD0, sizeof(connectContextD0));
    connectContextD0.appFriendlyName = (dev->lcd.handle,windowtext);
    connectContextD0.isAutostartable = FALSE;
    connectContextD0.isPersistent = FALSE;
    // we don't have a configuration screen
    connectContextD0.onConfigure.configCallback = NULL;
    connectContextD0.onConfigure.configContext = NULL;
    // the "connection" member will be returned upon return
    connectContextD0.connection = LGLCD_INVALID_CONNECTION;
    // and connect
    gD0 = lgLcdConnect(&connectContextD0);
    HandleError(gD0, _T("lgLcdConnect"));

	// now we are connected (and have a connection handle returned),
    // let's enumerate an LCD (the first one, index = 0)
    lgLcdDeviceDesc deviceDescription;
    gD0 = lgLcdEnumerate(connectContextD0.connection, 0, &deviceDescription);
    HandleError(gD0, _T("lgLcdEnumerate"));

    // open it
    ZeroMemory(&openContextD0, sizeof(openContextD0));
    openContextD0.connection = connectContextD0.connection;
    openContextD0.index = 0;
    // we have no softbutton notification callback
    openContextD0.onSoftbuttonsChanged.softbuttonsChangedCallback = NULL;
    openContextD0.onSoftbuttonsChanged.softbuttonsChangedContext = NULL;
    // the "device" member will be returned upon return
    openContextD0.device = LGLCD_INVALID_DEVICE;
    gD0 = lgLcdOpen(&openContextD0);
    HandleError(gD0, _T("lgLcdEnumerate"));
	gD0used=1;

	// coming back from lgLcdOpen, we have a device handle (in openContextD0.device)
    // which we will be using from now on until the program exits
	}

	if (gused==0)
	{
		if (gA0used==1)
		{
			gA0 = lgLcdSetAsLCDForegroundApp(openContextA0.device, LGLCD_LCD_FOREGROUND_APP_YES);
		}
		else if (gB0used==1)
		{
			gB0 = lgLcdSetAsLCDForegroundApp(openContextB0.device, LGLCD_LCD_FOREGROUND_APP_YES);
		}
		else if (gC0used==1)
		{
			gC0 = lgLcdSetAsLCDForegroundApp(openContextC0.device, LGLCD_LCD_FOREGROUND_APP_YES);
		}
		else if (gD0used==1)
		{
			gD0 = lgLcdSetAsLCDForegroundApp(openContextD0.device, LGLCD_LCD_FOREGROUND_APP_YES);
		}
	}
	
	#endif

	return rv_ok;
}
s32 FASTCALL Init(void* data,u32 id,maple_init_params* params)
{
	if (id==1)
	{
		sync_counter=0;
		next_sync_counter=1;
		int test = Init_netplay()==0;
		verify(test);
	}
	return rv_ok;
}
void FASTCALL Term(void* data,u32 id)
{
}
void FASTCALL Destroy(void* data,u32 id)
{
	if (id==2)
	{
		VMU_info* dev=(VMU_info* )data;
		if (dev->lcd.handle)
		{
			DestroyWindow(dev->lcd.handle);

	#ifdef _HAS_LGLCD_

			lgLcdBitmap160x43x1 bmp;
			bmp.hdr.Format = LGLCD_BMP_FORMAT_160x43x1;
			
			if (((int)dev->lcd.handle)==gA0temp)
			{
				// clear the display
				ZeroMemory(&bmp.pixels, sizeof(bmp.pixels));
				gA0 = lgLcdUpdateBitmap(openContextA0.device, &bmp.hdr, LGLCD_ASYNC_UPDATE(LGLCD_PRIORITY_ALERT));
				HandleError(gA0, _T("lgLcdUpdateBitmap"));
				// close the device
				gA0 = lgLcdClose(openContextA0.device);
				HandleError(gA0, _T("lgLcdClose"));
				// take down the connection
				gA0 = lgLcdDisconnect(connectContextA0.connection);
				HandleError(gA0, _T("lgLcdDisconnect"));
				gA0used=0;
			}

			if (((int)dev->lcd.handle)==gB0temp)
			{
				// clear the display
				ZeroMemory(&bmp.pixels, sizeof(bmp.pixels));
				gB0 = lgLcdUpdateBitmap(openContextB0.device, &bmp.hdr, LGLCD_ASYNC_UPDATE(LGLCD_PRIORITY_ALERT));
				HandleError(gB0, _T("lgLcdUpdateBitmap"));
				// close the device
				gB0 = lgLcdClose(openContextB0.device);
				HandleError(gB0, _T("lgLcdClose"));
				// take down the connection
				gB0 = lgLcdDisconnect(connectContextB0.connection);
				HandleError(gB0, _T("lgLcdDisconnect"));
				gB0used=0;
			}

			if (((int)dev->lcd.handle)==gC0temp)
			{
				// clear the display
				ZeroMemory(&bmp.pixels, sizeof(bmp.pixels));
				gC0 = lgLcdUpdateBitmap(openContextC0.device, &bmp.hdr, LGLCD_ASYNC_UPDATE(LGLCD_PRIORITY_ALERT));
				HandleError(gC0, _T("lgLcdUpdateBitmap"));
				// close the device
				gC0 = lgLcdClose(openContextC0.device);
				HandleError(gC0, _T("lgLcdClose"));
				// take down the connection
				gC0 = lgLcdDisconnect(connectContextC0.connection);
				HandleError(gC0, _T("lgLcdDisconnect"));
				gC0used=0;
			}

			if (((int)dev->lcd.handle)==gD0temp)
			{
				// clear the display
				ZeroMemory(&bmp.pixels, sizeof(bmp.pixels));
				gD0 = lgLcdUpdateBitmap(openContextD0.device, &bmp.hdr, LGLCD_ASYNC_UPDATE(LGLCD_PRIORITY_ALERT));
				HandleError(gD0, _T("lgLcdUpdateBitmap"));
				// close the device
				gD0 = lgLcdClose(openContextD0.device);
				HandleError(gD0, _T("lgLcdClose"));
				// take down the connection
				gD0 = lgLcdDisconnect(connectContextD0.connection);
				HandleError(gD0, _T("lgLcdDisconnect"));
				gD0used=0;
			}

	#endif

		}
		free(data);
	}	
}


#define MMD(name,flags) \
	wcscpy(km.devices[mdi].Name,name);	\
	km.devices[mdi].Type=MDT_Main;	\
	km.devices[mdi].Flags= flags;	\
	mdi++;

#define MSD(name,flags)	\
	wcscpy(km.devices[mdi].Name,name);	\
	km.devices[mdi].Type=MDT_Sub;	\
	km.devices[mdi].Flags= flags;	\
	mdi++;

#define MDLE() km.devices[mdi].Type=MDT_EndOfList;
//Give a list of the devices to teh emu
#define __T(x) L##x
#define _T(x) __T(x)
void EXPORT_CALL dcGetInterface(plugin_interface* info)
{

#define km info->maple

#define c info->common
	
	info->InterfaceVersion=PLUGIN_I_F_VERSION;

	c.InterfaceVersion=MAPLE_PLUGIN_I_F_VERSION;

	c.Load=Load;
	c.Unload=Unload;
	c.Type=Plugin_Maple;
	
	wcscpy(c.Name,L"nullDC Maple Devices [" _T(__DATE__) L"]");

	km.CreateMain=CreateMain;
	km.CreateSub=CreateSub;
	km.Init=Init;
	km.Term=Term;
	km.Destroy=Destroy;

#ifdef BUILD_NAOMI
	u32 mdi=0;
	MMD(L"nullDC NAOMI JAMMA Controller[WinHook] (" _T(__DATE__) L")",0);
#else
	u32 mdi=0;
	//0
	MMD(L"nullDC Controller [WinHook] (" _T(__DATE__) L")",MDTF_Hotplug|MDTF_Sub0|MDTF_Sub1);

	//1
	MMD(L"nullDC Controller [WinHook,NET] (" _T(__DATE__) L")",MDTF_Hotplug|MDTF_Sub0|MDTF_Sub1);

	//2
	#ifdef _HAS_LGLCD_
		MSD(L"nullDC VMU Logitech friendly version (" _T(__DATE__) L")",MDTF_Hotplug);
	#else
		MSD(L"nullDC VMU (" _T(__DATE__) L")",MDTF_Hotplug);
	#endif

	//3
	MMD(L"nullDC Keyboard [WinHook] (" _T(__DATE__) L")",MDTF_Hotplug);

	//4
	MMD(L"nullDC Controller [no input] (" _T(__DATE__) L")",MDTF_Hotplug|MDTF_Sub0|MDTF_Sub1);

	//5
	MMD(L"nullDC Mouse [WinHook] (" _T(__DATE__) L")",MDTF_Hotplug);
#endif

	MDLE();
}
void LoadSettings()
{
	for (int port=0;port<4;port++)
	{
		for (int i=0;joypad_settings_K[i].name;i++)
		{
			wchar temp[512];
			swprintf_s(temp,L"Port%c_%s",port+'A',&joypad_settings_K[i].name[4]);
			joypad_settings[port][i].KC=host.ConfigLoadInt(L"ndc_hookjoy",temp,joypad_settings_K[i].KC);
		}
	}
	local_port=host.ConfigLoadInt(L"ndc_hookjoy",L"local_port",0);
	wchar temp[512];
	host.ConfigLoadStr(L"ndc_hookjoy",L"server_addr",temp,L"192.168.1.33");
	wcstombs(server_addr,temp,sizeof(temp));
	host.ConfigLoadStr(L"ndc_hookjoy",L"server_port",temp,L"11122");
	wcstombs(server_port,temp,sizeof(temp));

	g_ShowVMU = host.ConfigLoadInt(L"drkMaple",L"VMU.Show",1);
	mouseSensitivity = (float)host.ConfigLoadInt(L"drkMaple",L"Mouse.Sensitivity",100)/100.0f;
}

void SaveSettings()
{
	for (int port=0;port<4;port++)
	{
		for (int i=0;joypad_settings_K[i].name;i++)
		{
			wchar temp[512];
			swprintf_s(temp,L"Port%c_%s",port+'A',&joypad_settings_K[i].name[4]);
			host.ConfigSaveInt(L"ndc_hookjoy",temp,joypad_settings[port][i].KC);
		}
	}
	wchar temp[512];
	host.ConfigSaveInt(L"ndc_hookjoy",L"local_port",0);
	mbstowcs(temp,server_addr,sizeof(temp));
	host.ConfigSaveStr(L"ndc_hookjoy",L"server_addr",temp);
	mbstowcs(temp,server_port,sizeof(temp));
	host.ConfigSaveStr(L"ndc_hookjoy",L"server_port",temp);

	host.ConfigSaveInt(L"drkMaple",L"ShowVMU",g_ShowVMU);
}