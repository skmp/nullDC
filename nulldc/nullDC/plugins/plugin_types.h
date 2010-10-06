#pragma once

#ifndef _PLUGIN_HEADER_
#error beef
#endif

struct VersionNumber
{
	union
	{
		struct
		{
			u8 major:8;
			u8 minnor:8;
			u8 build:8;
			u8 flags:8;
		};
		u32 full;
	};
};

#define EXPORT extern "C" __declspec(dllexport)

#define EXPORT_CALL __stdcall
#define FASTCALL __fastcall
#define C_CALL __cdecl

#define DC_PLATFORM_MASK		7
#define DC_PLATFORM_NORMAL		0   /* Works, for the most part */
#define DC_PLATFORM_DEV_UNIT	1	/* This is missing hardware */
#define DC_PLATFORM_NAOMI		2   /* Works, for the most part */ 
#define DC_PLATFORM_NAOMI2		3   /* Needs to be done, 2xsh4 + 2xpvr + custom TNL */
#define DC_PLATFORM_ATOMISWAVE	4   /* Needs to be done, DC-like hardware with possibly more ram */
#define DC_PLATFORM_HIKARU		5   /* Needs to be done, 2xsh4, 2x aica , custom vpu */
#define DC_PLATFORM_AURORA		6   /* Needs to be done, Uses newer 300 mhz sh4 + 150 mhz pvr mbx SoC */
 

#define DC_PLATFORM DC_PLATFORM_NORMAL


#if (DC_PLATFORM==DC_PLATFORM_NORMAL)

	#define BUILD_DREAMCAST 1
	
	//DC : 16 mb ram, 8 mb vram, 2 mb aram, 2 mb bios, 128k flash
	#define RAM_SIZE (16*1024*1024)
	#define VRAM_SIZE (8*1024*1024)
	#define ARAM_SIZE (2*1024*1024)
	#define BIOS_SIZE (2*1024*1024)
	#define FLASH_SIZE (128*1024)

	#define ROM_PREFIX L"dc_"
	#define ROM_NAMES
	#define NVR_OPTIONAL 0

#elif  (DC_PLATFORM==DC_PLATFORM_DEV_UNIT)
	
	#define BUILD_DEV_UNIT 1

	//Devkit : 32 mb ram, 8? mb vram, 2? mb aram, 2? mb bios, ? flash
	#define RAM_SIZE (32*1024*1024)
	#define VRAM_SIZE (8*1024*1024)
	#define ARAM_SIZE (2*1024*1024)
	#define BIOS_SIZE (2*1024*1024)
	#define FLASH_SIZE (128*1024)

	#define ROM_PREFIX L"hkt_"
	#define ROM_NAMES
	#define NVR_OPTIONAL 0

#elif  (DC_PLATFORM==DC_PLATFORM_NAOMI)
	
	#define BUILD_NAOMI 1
	#define BUILD_NAOMI1 1

	//Naomi : 32 mb ram, 16 mb vram, 8 mb aram, 2 mb bios, ? flash
	#define RAM_SIZE (32*1024*1024)
	#define VRAM_SIZE (16*1024*1024)
	#define ARAM_SIZE (8*1024*1024)
	#define BIOS_SIZE (2*1024*1024)
	#define BBSRAM_SIZE (8*1024)

	#define ROM_PREFIX L"naomi_"
	#define ROM_NAMES L";epr-21576d.bin"
	#define NVR_OPTIONAL 1

#elif  (DC_PLATFORM==DC_PLATFORM_NAOMI2)
	
	#define BUILD_NAOMI 1
	#define BUILD_NAOMI2 1

	//Naomi2 : 32 mb ram, 16 mb vram, 8 mb aram, 2 mb bios, ? flash
	#define RAM_SIZE (32*1024*1024)
	#define VRAM_SIZE (16*1024*1024)
	#define ARAM_SIZE (8*1024*1024)
	#define BIOS_SIZE (2*1024*1024)	
	#define BBSRAM_SIZE (8*1024)

	#define ROM_PREFIX L"n2_"
	#define ROM_NAMES
	#define NVR_OPTIONAL 1

#elif  (DC_PLATFORM==DC_PLATFORM_ATOMISWAVE)
	
	#define BUILD_ATOMISWAVE 1

	//Atomiswave : 16(?) mb ram, 16 mb vram, 8 mb aram, 64kb bios, 64k flash
	#define RAM_SIZE (16*1024*1024)
	#define VRAM_SIZE (16*1024*1024)
	#define ARAM_SIZE (8*1024*1024)
	#define BIOS_SIZE (64*1024)
	#define FLASH_SIZE (64*1024)

	#define ROM_PREFIX L"aw_"
	#define ROM_NAMES L";bios.ic23_l"
	#define NVR_OPTIONAL 1

#else
	#error invalid build config
#endif

#define RAM_MASK	(RAM_SIZE-1)
#define VRAM_MASK	(VRAM_SIZE-1)
#define ARAM_MASK	(ARAM_SIZE-1)
#define BIOS_MASK	(BIOS_SIZE-1)

#ifdef FLASH_SIZE
#define FLASH_MASK	(FLASH_SIZE-1)
#endif

#ifdef BBSRAM_SIZE
#define BBSRAM_MASK	(BBSRAM_SIZE-1)
#endif

#define GD_CLOCK 33868800				//GDROM XTAL -- 768fs

#define AICA_CORE_CLOCK (GD_CLOCK*4/3)		//[45158400]  GD->PLL 3:4 -> AICA CORE	 -- 1024fs
#define ADAC_CLOCK (AICA_CORE_CLOCK/2)		//[11289600]  44100*256, AICA CORE -> PLL 4:1 -> ADAC -- 256fs
#define AICA_ARM_CLOCK (AICA_CORE_CLOCK/2)	//[22579200]  AICA CORE -> PLL 2:1 -> ARM
#define AICA_SDRAM_CLOCK (GD_CLOCK*2)		//[67737600]  GD-> PLL 2 -> SDRAM
#define SH4_MAIN_CLOCK (200*1000*1000)		//[200000000] XTal(13.5) -> PLL (33.3) -> PLL 1:6 (200)
#define SH4_RAM_CLOCK (100*1000*1000)		//[100000000] XTal(13.5) -> PLL (33.3) -> PLL 1:6 (100)	, also suplied to HOLLY chip
#define G2_BUS_CLOCK (25*1000*1000)			//[25000000]  from Holly, from SH4_RAM_CLOCK w/ 2 2:1 plls

#define DC_MakeVersion(major,minor,build) (((DC_PLATFORM)<<24)|((build)<<16)|((minor)<<8)|(major))

enum PluginType
{
	Plugin_PowerVR=1,		//3D ;)
	Plugin_GDRom=2,			//guess it
	Plugin_AICA=3,			//Sound :p
	Plugin_Maple=4,			//controler ,mouse , ect
	Plugin_ExtDevice=5,		//BBA , Lan adapter , other 
	Plugin_ARM=6,			//Sound Cpu
};

enum ndc_error_codes
{
	rv_ok = 0,		//no error

	rv_error=-2,	//error
	rv_serror=-1,	//silent error , it has been reported to the user
};

//Simple struct to store window rect  ;)
//Top is 0,0 & numbers are in pixels.
//Only client size
struct NDC_WINDOW_RECT
{
	u32 width;
	u32 height;
};

enum ndc_events
{
	//gui -> *
	NDE_GUI_RESIZED=0,			//gui was resized, p points to a NDC_WINDOW_RECT with the new size.This event is not guaratneed to have any thread anfinity.The plugin
								//must handle sync. betwen threads to ensure proper operation.Borderless fullscreen use this, not NDC_GUI_REQESTFULLSCREEN
	
	NDE_GUI_REQESTFULLSCREEN,	//if (l) -> goto fullscreen, else goto window.This event can be safely ingored
	
	NDE_GUI_WINDOWCHANGE,		//if (l) old window handle is still valid, else it has been replaced with a new one.This event is sent with l!=0 before destructing the window, and then with l==0
								//after creating a new one.It is not sent for the initial or final window creation/destruction.

	//pvr -> *
	NDE_PVR_FULLSCREEN,			//if (l) -> new mode is fullscreen, else its window

	//Misc
	NDE_CUSTOM=0xFF000000,		//Base for custom events.be carefull with how you use these, as all plugins get em ;)
};
#define PLUGIN_I_F_VERSION DC_MakeVersion(1,0,2)

//These are provided by the emu

//Config
typedef void EXPORT_CALL ConfigLoadStrFP(const wchar * Section, const wchar * Key, wchar * Return,const wchar* Default);
typedef void EXPORT_CALL ConfigSaveStrFP(const wchar * Section, const wchar * Key, const wchar * Value);

typedef s32 EXPORT_CALL ConfigLoadIntFP(const wchar * Section, const wchar * Key,const s32 Default);
typedef void EXPORT_CALL ConfigSaveIntFP(const wchar * Section, const wchar * Key, const s32 Value);
typedef s32 EXPORT_CALL ConfigExistsFP(const wchar * Section, const wchar * Key);

//Menus
enum MenuItemStyles
{
	MIS_Seperator	=1,
	MIS_Radiocheck	=2,
	MIS_Bitmap		=4,

	MIS_Grayed		=0x40000000,
	MIS_Checked		=0x80000000,
};

typedef void EXPORT_CALL MenuItemSelectedFP(u32 id,void* WindowHandle,void* user);

enum MenuItemMask
{
	MIM_Text=1,
	MIM_Handler=2,
	MIM_Bitmap=4,
	MIM_Style=8,
	MIM_PUser=16,
	MIM_All=0xFFFFFFFF,
};
struct MenuItem
{
	wchar* Text;			//Text of the menu item
	MenuItemSelectedFP* Handler;	//called when the menu is clicked
	void* Bitmap;		//bitmap handle
	u32 Style;			//MIS_* combination
	void* PUser;		//User defined pointer :)
};
//
enum MsgTarget
{
	MT_Core=1<<0,
	MT_Gui =1<<1,
	MT_PowerVR=1<<2,
	MT_GDRom=1<<3,
	MT_Maple=1<<4,			//controler ,mouse , vmu (both main and subdevs)
	MT_ExtDevice=1<<5,		//BBA , Lan adapter , other 
	MT_All=0xFFFFFFFF,
};
enum SyncSourceFlags
{
	SSF_NeedsSync=1,	//The provider needs perfect sync, like audio stream out or vsync'd video
};

typedef u32 EXPORT_CALL AddMenuItemFP(u32 parent,s32 pos,const wchar* text,MenuItemSelectedFP* handler , u32 checked);
typedef void EXPORT_CALL SetMenuItemStyleFP(u32 id,u32 style,u32 mask);
typedef void EXPORT_CALL GetMenuItemFP(u32 id,MenuItem* info,u32 mask);
typedef void EXPORT_CALL SetMenuItemFP(u32 id,MenuItem* info,u32 mask);
typedef void EXPORT_CALL DeleteMenuItemFP(u32 id);
typedef void* EXPORT_CALL GetRenderTargetFP();
typedef void EXPORT_CALL SendMsgFP(u32 target,u32 eid,void* pdata,u32 ldata);
typedef void EXPORT_CALL RegisterSyncSourceFP(wchar* name,u32 id,u32 freq,u32 flags);

struct emu_info
{
	GetRenderTargetFP*	GetRenderTarget;		//Handle of the window that rendering is done

	ConfigLoadStrFP*	ConfigLoadStr;	//Can be used to Read/Write settings :)
	ConfigSaveStrFP*	ConfigSaveStr;
	ConfigLoadIntFP*	ConfigLoadInt;
	ConfigSaveIntFP*	ConfigSaveInt;
	ConfigExistsFP*		ConfigExists;

	AddMenuItemFP*		AddMenuItem;
	SetMenuItemStyleFP*	SetMenuItemStyle;
	SetMenuItemFP*		SetMenuItem;
	GetMenuItemFP*		GetMenuItem;
	DeleteMenuItemFP*	DeleteMenuItem;

	SendMsgFP* BroardcastEvent;
	u32 RootMenu;
	u32 DebugMenu;

	RegisterSyncSourceFP* RegisterSyncSource;
};

//common plugin functions
//called when plugin is used by emu (you should do first time init here)
typedef s32 FASTCALL PluginInitFP(emu_info* param);

//called when plugin is unloaded by emu , olny if dcInit is called (eg , not called to enumerate plugins)
typedef void FASTCALL PluginTermFP();

//Unhandled Write Exeption handler
typedef bool FASTCALL ExeptionHanlderFP(void* addr);

//Some other commonly used function typesdef's ;)
typedef u32 FASTCALL ReadMemFP(u32 addr,u32 size);
typedef void FASTCALL WriteMemFP(u32 addr,u32 data,u32 size);
typedef void FASTCALL UpdateFP(u32 cycles);
typedef void FASTCALL PluginResetFP(bool Manual);
typedef void EXPORT_CALL EventHandlerFP(u32 nid,void* p);


struct common_info
{
	u32				InterfaceVersion;	//Note : this version is of the interface for this type of plugin :)
	wchar			Name[128];			//plugin name
	u32				Type;				//plugin type

	//Functions that are used for all plugins , these are SET by the plugin
	PluginInitFP*	Load;					//Init
	PluginTermFP*	Unload;					//Term
	EventHandlerFP* EventHandler;			//Event Handler
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////

//******************************************************
//*********************** PowerVR **********************
//******************************************************

#define PVR_PLUGIN_I_F_VERSION DC_MakeVersion(1,0,0)
 
typedef void FASTCALL vramlock_Unlock_blockFP  (vram_block* block);
typedef vram_block* FASTCALL vramlock_LockFP(u32 start_offset,u32 end_offset,void* userdata);
typedef void FASTCALL osdDrawFP();

struct pvr_init_params
{
	HollyRaiseInterruptFP*	RaiseInterrupt;

	//Vram is allocated by the emu.A pointer is given to the buffer here :)
	u8*					vram; 

	//This will work only when using the default exeption handler
	vramlock_LockFP*		 vram_lock_32;
	vramlock_LockFP*		 vram_lock_64;
	vramlock_Unlock_blockFP* vram_unlock;
	osdDrawFP*				 osdDraw;
};

typedef s32 FASTCALL PvrInitFP(pvr_init_params* param);
typedef void FASTCALL TaDMAFP(u32* data,u32 size);
typedef void FASTCALL TaSQFP(u32* data);

//OSD interface extensions

//can i fold that into a single line ?
typedef struct osdTexture_i* osdTexture;

//Texture formats
enum OSDFORMAT
{
	OSDFMT_A,		//1 bpp
	OSDFMT_RGB,		//3 bpp
	OSDFMT_RGBA,	//4 bpp
};

//Texture Functions
typedef s32 FASTCALL osdTexCreateFP(u32 w,u32 h,OSDFORMAT fmt,void* data);
typedef s32 FASTCALL osdTexBindFP(osdTexture tex);
typedef s32 FASTCALL osdTexDestroyFP(osdTexture tex);

//Vertices !
typedef s32 FASTCALL osdVtxFP(float x,float y,float z,float w,float r,float g,float b,float a,float u,float v);

struct pvr_plugin_if
{
	PvrInitFP*		Init;
	PluginResetFP*	Reset;
	PluginTermFP*	Term;

	ExeptionHanlderFP* ExeptionHanlder; //Called on unhandled write exception, set to 0 if not used.If not 0 , 
										//it disables the internal locking system

	UpdateFP*		UpdatePvr;			//called every ~ 1800 cycles , set to 0 if not used
	TaDMAFP*		TaDMA;				//size is 32 byte transfer counts
	TaSQFP*			TaSQ;				//size is 32 byte transfer counts
	ReadMemFP*		ReadReg;
	WriteMemFP*		WriteReg;

	//Will be called only when pvr locking is enabled
	vramLockCBFP*	LockedBlockWrite;	//set to 0 if not used

	//osd interface
	//I really need to decide if this will be optional or not
	struct 
	{
		osdVtxFP*				Vtx;
		//Texturing !
		osdTexCreateFP*			TexCreate;
		osdTexBindFP*			TexBind;
		osdTexDestroyFP*		TexDestroy;
	} osd;
};
//******************************************************
//************************ GDRom ***********************
//******************************************************
enum DiscType
{
	CdDA=0x00,
	CdRom=0x10,
	CdRom_XA=0x20,
	CdRom_Extra=0x30,
	CdRom_CDI=0x40,
	GdRom=0x80,		

	NoDisk=0x1,			//These are a bit hacky .. but work for now ...
	Open=0x2,			//tray is open :)
	Busy=0x3			//busy -> needs to be automatically done by gdhost
};

enum DiskArea
{
	SingleDensity,
	DoubleDensity
};

enum DriveEvent
{
	DiskChange=1	//disk ejected/changed
};

//sends an event to the gd rom lle emulation code (from the gd rom input code)
typedef void FASTCALL DriveNotifyEventFP(u32 event,void* param);
//reads a sector xD
typedef void FASTCALL DriveReadSectorFP(u8 * buff,u32 StartSector,u32 SectorCount,u32 secsz);
//Gets toc for the specified area
typedef void FASTCALL DriveGetTocInfoFP(u32* toc,u32 area);
//Gets disk type
typedef u32 FASTCALL DriveGetDiscTypeFP();
//Get Session info for session "session" , put it to "pout" buffer (6 bytes)
typedef void FASTCALL DriveGetSessionInfoFP(u8* pout,u8 session);
//Get subchannel data
typedef void FASTCALL DriveReadSubChannelFP(u8 * buff, u32 format, u32 len);

#define GDR_PLUGIN_I_F_VERSION DC_MakeVersion(1,0,0)

//passed on GDRom init call
struct gdr_init_params
{
	wchar* source;
	DriveNotifyEventFP* DriveNotifyEvent;
};

typedef s32 FASTCALL GdrInitFP(gdr_init_params* param);

struct gdr_plugin_if
{
	GdrInitFP*		Init;
	PluginResetFP*	Reset;
	PluginTermFP*	Term;

	ExeptionHanlderFP* ExeptionHanlder; //Called on unhandled write exeption ;)

	//IO
	DriveReadSectorFP* ReadSector;
	DriveReadSubChannelFP *ReadSubChannel;
	DriveGetTocInfoFP* GetToc;
	DriveGetDiscTypeFP* GetDiscType;
	DriveGetSessionInfoFP* GetSessionInfo;
};
//******************************************************
//************************ AICA ************************
//******************************************************
typedef void FASTCALL ArmInterruptChangeFP(u32 bits,u32 L);
typedef void FASTCALL CDDA_SectorFP(s16* sector);

#define AICA_PLUGIN_I_F_VERSION DC_MakeVersion(1,0,1)

//passed on AICA init call
struct aica_init_params
{
	CDDA_SectorFP*	CDDA_Sector;		//For CDDA , returns a silent sector or cdda :)

	u8*				aica_ram;
	
	u32*					SB_ISTEXT;			//SB_ISTEXT register , so that aica can check interrupts =)
	HollyRaiseInterruptFP*	RaiseInterrupt;
	HollyCancelInterruptFP* CancelInterrupt;

	ArmInterruptChangeFP*	ArmInterruptChange;	//called when the arm interrupt vectors may have changed.Parameter is P&M
};

typedef s32 FASTCALL AicaInitFP(aica_init_params* param);

//Ram/Regs are managed by plugin , exept RTC regs (managed by main emu)
struct aica_plugin_if
{
	AicaInitFP*		Init;
	PluginResetFP*	Reset;
	PluginTermFP*	Term;

	ReadMemFP*  ReadMem_aica_reg;
	WriteMemFP* WriteMem_aica_reg;

	UpdateFP*	Update;				//called ~every sample, the parameter is how many samples to generate
};
//******************************************************
//******************** ARM Sound CPU *******************
//******************************************************
#define ARM_PLUGIN_I_F_VERSION DC_MakeVersion(1,0,0)
struct arm_init_params
{
	u8*			aica_ram;

	ReadMemFP*  ReadMem_aica_reg;
	WriteMemFP* WriteMem_aica_reg;
};

typedef s32 FASTCALL ArmInitFP(arm_init_params* param);


struct arm_plugin_if
{
	ArmInitFP*		Init;
	PluginResetFP*	Reset;
	PluginTermFP*	Term;

	UpdateFP* SetResetState;		//0 = Reset condition is removed, 1 = Reset is condition active.While the cpu is in reset mode no code runs on it.On 1 -> 0 transaction the cpu resets itself

	ExeptionHanlderFP* ExeptionHanlder; //Called on unhandled write exeption, set to 0 if not used

	UpdateFP*	Update;					//called ~every sample, the parameter is how many samples to generate
	
	ArmInterruptChangeFP*	ArmInterruptChange;	//callback from aica !
};
//******************************************************
//****************** Maple devices ******************
//******************************************************

#define MAPLE_PLUGIN_I_F_VERSION DC_MakeVersion(1,0,0)

enum MapleDeviceCreationFlags
{
	MDCF_None=0,
	MDCF_Hotplug=1
};

struct maple_subdevice_instance;
struct maple_device_instance;

//buffer_out_len and responce need to be filled w/ proper info by the plugin
//buffer_in must not be edited (its direct pointer on ram)
//output buffer must contain the frame data , the frame header is generated by the maple routing code
//typedef u32 FASTCALL MapleSubDeviceDMAFP(void* device_instance,u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len);
typedef u32 FASTCALL MapleDeviceDMAFP(void* device_instance,u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len);

struct maple_subdevice_instance
{
	//port
	u8 port;
	//user data
	void* data;
	//MapleDeviceDMA
	MapleDeviceDMAFP* dma;
	bool connected;
	u32 reserved;	//reserved for the emu , DO NOT EDIT
};
struct maple_device_instance
{
	//port
	u8 port;
	//user data
	void* data;
	//MapleDeviceDMA
	MapleDeviceDMAFP* dma;
	bool connected;

	maple_subdevice_instance subdevices[5];
};

//Nothing for now ;)
struct maple_init_params;

typedef s32 FASTCALL MapleCreateInstanceFP(maple_device_instance* mdinst,u32 id,u32 flags,u32 rootmenu);
typedef s32 FASTCALL MapleSubCreateInstanceFP(maple_subdevice_instance* msdinst,u32 id,u32 flags,u32 rootmenu);

typedef s32 FASTCALL MapleInitInstanceFP(void* inst,u32 id,maple_init_params* params);
typedef void FASTCALL MapleTermInstanceFP(void* inst,u32 id);
typedef void FASTCALL MapleDestroyInstanceFP(void* inst,u32 id);

typedef s32 FASTCALL MapleInitFP(maple_init_params* param);

enum MapleDeviceType
{
	MDT_EndOfList=0,
	MDT_Main=1,
	MDT_Sub=2,
};
enum MapleDeviceTypeFlags
{
	MDTF_Sub0=1,		//these
	MDTF_Sub1=2,		//are
	MDTF_Sub2=4,		//ingored
	MDTF_Sub3=8,		//in
	MDTF_Sub4=16,		//subdevices :)

	MDTF_Hotplug=32,	//Can be added/removed at runtime.That means that it may never get the Init/Term calls
};
struct maple_device_definition
{
	wchar Name[128];
	u32 Type;
	u32 Flags;
};
struct maple_plugin_if
{
	//*Main functions are ignored if no main devices are exported
	//*Sub functions are ignored if no main devices are exported
	//Create Instance
	MapleCreateInstanceFP* CreateMain;
	MapleSubCreateInstanceFP* CreateSub;

	MapleDestroyInstanceFP* Destroy;
	MapleInitInstanceFP*	Init;
	MapleTermInstanceFP*	Term;
	
	maple_device_definition devices[32];	//Last one must be of type MDT_EndOfList , unless all 32 are used
};

//******************************************************
//********************* Ext.Device *********************
//******************************************************

#define EXTDEVICE_PLUGIN_I_F_VERSION DC_MakeVersion(1,0,0)

//passed on Ext.Device init call
struct ext_device_init_params
{
	HollyRaiseInterruptFP*	RaiseInterrupt;
	u32* SB_ISTEXT;
	HollyCancelInterruptFP* CancelInterrupt;
};

typedef s32 FASTCALL ExtInitFP(ext_device_init_params* param);

struct ext_device_plugin_if
{
	ExtInitFP*		Init;
	PluginResetFP*	Reset;
	PluginTermFP*	Term;

	ExeptionHanlderFP* ExeptionHanlder; //Called on unhandled write exeption, set to 0 if not used

	//Area 0 , 0x00600000- 0x006007FF	[MODEM]
	ReadMemFP*  ReadMem_A0_006;
	WriteMemFP* WriteMem_A0_006;

	//Area 0 , 0x01000000- 0x01FFFFFF	[Ext. Device]
	ReadMemFP*  ReadMem_A0_010;
	WriteMemFP* WriteMem_A0_010;
	
	//Area 5
	ReadMemFP*  ReadMem_A5;
	WriteMemFP* WriteMem_A5;

	UpdateFP*	UpdateExtDevice;//Called every ~1800 cycles, set to 0 if not used
};

//Plugin Exports
//These are the functions the plugin has to export :)

struct plugin_interface
{
	u32 InterfaceVersion;

	common_info common;
	union 
	{
		pvr_plugin_if			pvr;
		gdr_plugin_if			gdr;
		aica_plugin_if			aica;
		arm_plugin_if			arm;
		maple_plugin_if			maple;
		ext_device_plugin_if	ext_dev;

		u32 pad[4096];//padding & reserved space for future expantion :)
	};
};

//Dropped 
//exported as dcGetInterface
typedef void EXPORT_CALL dcGetInterfaceFP(plugin_interface* lst);