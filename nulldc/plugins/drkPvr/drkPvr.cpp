// drkPvr.cpp : Defines the entry point for the DLL application.
//

/*
	Plugin structure
	Interface
	SPG
	TA
	Renderer
*/

#include "drkPvr.h"

#include "ta.h"
#include "spg.h"
#include "regs.h"
#include "renderer_if.h"
#include <algorithm>

//RaiseInterruptFP* RaiseInterrupt;

//void* Hwnd;

emu_info emu;
wchar emu_name[512];

pvr_init_params params;
_settings_type settings;


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

class Optiongroup
{
public:
	Optiongroup()
	{
		root_menu=0;
		format=0;
	}
	u32 root_menu;
	wchar* format;
	struct itempair { u32 id;int value;wchar* ex_name;};
	vector<itempair> items;

	void (*callback) (int val) ;
	void Add(u32 id,int val,wchar* ex_name) { itempair t={id,val,ex_name}; items.push_back(t); }
	void Add(u32 root,wchar* name,int val,wchar* ex_name=0,int style=0) 
	{ 
		if (root_menu==0)
			root_menu=root;
		u32 ids=emu.AddMenuItem(root,-1,name,handler,0);
		emu.SetMenuItemStyle(ids,style,style);
		
		MenuItem t;
		t.PUser=this;
		emu.SetMenuItem(ids,&t,MIM_PUser);

		Add(ids,val,ex_name);
	}

	static void EXPORT_CALL handler(u32 id,void* win,void* puser)
	{
		Optiongroup* pthis=(Optiongroup*)puser;
		pthis->handle(id);
	}
	void SetValue(int val)
	{
		for (u32 i=0;i<items.size();i++)
		{
			if (val==items[i].value)
			{
				emu.SetMenuItemStyle(items[i].id,MIS_Checked,MIS_Checked);
				if (root_menu && format)
				{
					MenuItem t;
					emu.GetMenuItem(items[i].id,&t,MIM_Text);
					wchar temp[512];
					swprintf(temp,512,format,items[i].ex_name==0?t.Text:items[i].ex_name);
					t.Text=temp;
					emu.SetMenuItem(root_menu,&t,MIM_Text);
				}
			}
			else
				emu.SetMenuItemStyle(items[i].id,0,MIS_Checked);
		}
		callback(val);
	}
	void handle(u32 id)
	{
		int val=0;
		for (u32 i=0;i<items.size();i++)
		{
			if (id==items[i].id)
			{
				val=items[i].value;
			}
		}

		SetValue(val);
	}

};
void AddSeperator(u32 menuid)
{
	emu.AddMenuItem(menuid,-1,0,0,0);
}
Optiongroup menu_res;
Optiongroup menu_sortmode;
Optiongroup menu_modvolmode;
Optiongroup menu_palmode;
Optiongroup menu_zbuffer;
Optiongroup menu_TCM;
Optiongroup menu_widemode;
Optiongroup menu_resolution;
int oldmode=-1;
int osx=-1,osy=-1;


//dst[0] -> width of rect to be adjusted
//dst[1] -> height
//src: w,h of rect to be matched
void CalcRect(float* dst,float* src)
{
	if (settings.Enhancements.AspectRatioMode!=0)
	{
		//printf("New rect %d %d\n",sx,sy);
		float nw=((float)src[0]/(float)src[1])*dst[1];
		float nh=((float)src[1]/(float)src[0])*dst[0];
		if (nh<dst[1])
		{
			nh=dst[1];
		}
		else
		{
			nw=dst[0];
		}
		dst[0]=nw;
		dst[1]=nh;
	}
}
void UpdateRRect()
{
	float dc_wh[2]={640,480};
	float rect[4];
	WINDOWINFO winf;
	GetWindowInfo((HWND)emu.GetRenderTarget(),&winf);

	int sx=winf.rcClient.right-winf.rcClient.left;
	int sy=winf.rcClient.bottom-winf.rcClient.top;
	if (osx!=sx || osy!=sy)
	{
		osx=sx;
		osy=sy;
		oldmode=-1;
	}

	float win_wh[2]={(float)sx,(float)sy};

	CalcRect(dc_wh,win_wh);

	rect[0]=(dc_wh[0]-640)/2;
	rect[2]=dc_wh[0];
	rect[1]=(dc_wh[1]-480)/2;
	rect[3]=dc_wh[1];

	rend_set_render_rect(rect,oldmode!=settings.Enhancements.AspectRatioMode);
	oldmode=settings.Enhancements.AspectRatioMode;
}

void FASTCALL vramLockCB (vram_block* block,u32 addr)
{
	rend_text_invl(block);
}
#include <vector>
using std::vector;

extern volatile bool render_restart;

void EXPORT_CALL handler_Vsync (u32 id,void* win,void* puser)
{
	if (settings.Video.VSync)
		settings.Video.VSync=0;
	else
		settings.Video.VSync=1;

	emu.SetMenuItemStyle(id,settings.Video.VSync?MIS_Checked:0,MIS_Checked);
	
	SaveSettings();
	render_restart=true;
}

void EXPORT_CALL handler_ShowStats(u32 id,void* win,void* puser)
{
	if (settings.OSD.ShowStats)
		settings.OSD.ShowStats=0;
	else
		settings.OSD.ShowStats=1;

	emu.SetMenuItemStyle(id,settings.OSD.ShowStats?MIS_Checked:0,MIS_Checked);
	
	SaveSettings();
}

void EXPORT_CALL handler_ShowFps(u32 id,void* win,void* puser)
{
	if (settings.OSD.ShowFPS)
		settings.OSD.ShowFPS=0;
	else
		settings.OSD.ShowFPS=1;

	emu.SetMenuItemStyle(id,settings.OSD.ShowFPS?MIS_Checked:0,MIS_Checked);
	
	SaveSettings();
}
void handler_widemode(int mode)
{
	settings.Enhancements.AspectRatioMode=mode;
		
	SaveSettings();
	UpdateRRect();
}

void handler_resmode(int mode)
{
	settings.Video.ResolutionMode=mode;
		
	SaveSettings();
	rend_handle_event(NDE_GUI_RESIZED,0);
}
void handler_PalMode(int  mode)
{
	settings.Emulation.PaletteMode=mode;
	SaveSettings();
}
void handler_ModVolMode(int  mode)
{
	settings.Emulation.ModVolMode=mode;
	SaveSettings();
}
void handler_TexCacheMode(int  mode)
{
	settings.Emulation.TexCacheMode=mode;
	SaveSettings();
}

void handler_ZBufferMode(int  mode)
{
	settings.Emulation.ZBufferMode=mode;
	SaveSettings();
	render_restart=true;
}
u32 AA_mid_menu;
u32 AA_mid_0;

struct resolution
{
	u32 w;
	u32 h;
	u32 rr;
};
bool operator<(const resolution &left, const resolution &right)
{
	/* put any condition you want to sort on here */
	if (left.h*(u64)left.w>right.h*(u64)right.w)
		return true;
	else if (left.h*(u64)left.w==right.h*(u64)right.w)
		return left.rr>right.rr;
	else
		return false;
}

void EXPORT_CALL handle_About(u32 id,void* w,void* p)
{
	MessageBox((HWND)w,L"Made by the nullDC Team",L"About nullDC PVR...",MB_ICONINFORMATION);
}
u32 sort_menu;
u32 sort_sm[3];


u32 ssm(u32 nm)
{
	nm=min(nm,2);

	return nm;
}

void handler_SSM(int val)
{
	settings.Emulation.AlphaSortMode=val;

	SaveSettings();
}
void CreateSortMenu()
{
	sort_menu=emu.AddMenuItem(emu.RootMenu,-1,L"Sort : %s",0,0);
	
	menu_sortmode.format=L"Sort : %s";

	menu_sortmode.callback=handler_SSM;
	menu_sortmode.Add(sort_menu,L"Off (Fastest, lowest accuracy)",0,L"Off");
	menu_sortmode.Add(sort_menu,L"Per Strip",1,L"Strip");
	menu_sortmode.Add(sort_menu,L"Per Triangle (Slowest, highest accuracy)",2,L"Triangle");
	menu_sortmode.SetValue(settings.Emulation.AlphaSortMode);
}
//called when plugin is used by emu (you should do first time init here)
s32 FASTCALL Load(emu_info* emu_inf)
{
	// wchar temp[512]; // Unreferenced
	memcpy(&emu,emu_inf,sizeof(emu));
	emu.ConfigLoadStr(L"emu",L"shortname",emu_name,0);
	
	LoadSettings();

	u32 RSM=emu.AddMenuItem(emu.RootMenu,-1,L"Render Resolution: %s",0,0);
	menu_resolution.format=L"Resolution: %s";
	menu_resolution.callback=handler_resmode;

	menu_resolution.Add(RSM,L"Maximum Supported (Highest quality)",0);
	menu_resolution.Add(RSM,L"Maximum, but up to 1280x800",1);
	menu_resolution.Add(RSM,L"Native (640x480)",2);
	menu_resolution.Add(RSM,L"Half of maximum pixels",3);
	menu_resolution.Add(RSM,L"Quarter of maximum pixels (Lowest quality)",4);
	menu_resolution.SetValue(settings.Video.ResolutionMode);


	u32 WSM=emu.AddMenuItem(emu.RootMenu,-1,L"Aspect Ratio: %s",0,0);
	
	menu_widemode.format=L"Aspect Ratio: %s";
	menu_widemode.callback=handler_widemode;

	menu_widemode.Add(WSM,L"Stretch",0);
	menu_widemode.Add(WSM,L"Borders",1);
	menu_widemode.Add(WSM,L"Extra Geom",2);
	menu_widemode.SetValue(settings.Enhancements.AspectRatioMode);

	u32 PMT=emu.AddMenuItem(emu.RootMenu,-1,L"Palette Handling",0,0);
	
	menu_palmode.callback=handler_PalMode;

	menu_palmode.format=L"Paletted Textures: %s";
	menu_palmode.Add(PMT,L"Static",0);
	menu_palmode.Add(PMT,L"Versioned",1);
	AddSeperator(PMT);
	menu_palmode.Add(PMT,L"Dynamic, Point",2);
	menu_palmode.Add(PMT,L"Dynamic, Full",3);

	menu_palmode.SetValue(settings.Emulation.PaletteMode);

	CreateSortMenu();

	u32 MVM=emu.AddMenuItem(emu.RootMenu,-1,L"Modifier Volumes: %s",0,0);

	menu_modvolmode.format=L"Modifier Volumes: %s";
	menu_modvolmode.callback=handler_ModVolMode;

	menu_modvolmode.Add(MVM,L"Normal And Clip (Slowest, highest accuracy)",MVM_NormalAndClip);
	menu_modvolmode.Add(MVM,L"Normal (Good speed, good accuracy)",MVM_Normal);
	menu_modvolmode.Add(MVM,L"Off  (Fastest, no shadows)",MVM_Off);
	menu_modvolmode.Add(MVM,L"Volumes (For debuging)",MVM_Volume);
	menu_modvolmode.SetValue(settings.Emulation.ModVolMode);

	u32 ZBM=emu.AddMenuItem(emu.RootMenu,-1,L"Z Buffer Mode: %s",0,0);

	menu_zbuffer.format=L"Z Buffer Mode: %s";
	menu_zbuffer.callback=handler_ZBufferMode;

	menu_zbuffer.Add(ZBM,L"D24S8 Adaptive Linear (New, Buggy still)",4);
	menu_zbuffer.Add(ZBM,L"D24FS8 (Fast when avaiable, Best Precision)",0);
	menu_zbuffer.Add(ZBM,L"D24S8+FPE (Slow, Good Precision)",1);
	menu_zbuffer.Add(ZBM,L"D24S8 Mode 1 (Lower Precision)",2);
	menu_zbuffer.Add(ZBM,L"D24S8 Mode 2 (Lower Precision)",3);

	menu_zbuffer.SetValue(settings.Emulation.ZBufferMode);
	
	u32 TCM=emu.AddMenuItem(emu.RootMenu,-1,L"Texture Cache Mode: %s",0,0);

	menu_TCM.format=L"Texture Cache Mode: %s";
	menu_TCM.callback=handler_TexCacheMode;

	menu_TCM.Add(TCM,L"Delete old",0);
	menu_TCM.Add(TCM,L"Delete invalidated",1);

	menu_TCM.SetValue(settings.Emulation.TexCacheMode);

	AddSeperator(emu.RootMenu);
	
	emu.AddMenuItem(emu.RootMenu,-1,L"Vsync",handler_Vsync,settings.Video.VSync);
	emu.AddMenuItem(emu.RootMenu,-1,L"Show Fps",handler_ShowFps,settings.OSD.ShowFPS);
	emu.AddMenuItem(emu.RootMenu,-1,L"Show Stats",handler_ShowStats,settings.OSD.ShowStats);

	AddSeperator(emu.RootMenu);
	
	emu.AddMenuItem(emu.RootMenu,-1,L"About",handle_About,0);

	return rv_ok;
}

//called when plugin is unloaded by emu , olny if dcInitPvr is called (eg , not called to enumerate plugins)
void FASTCALL Unload()
{
	
}

//It's suposed to reset anything but vram (vram is set to 0 by emu)
void FASTCALL ResetPvr(bool Manual)
{
	Regs_Reset(Manual);
	spg_Reset(Manual);
	rend_reset(Manual);
}

//called when entering sh4 thread , from the new thread context (for any thread speciacific init)
s32 FASTCALL InitPvr(pvr_init_params* param)
{
	memcpy(&params,param,sizeof(params));

	extern void BuildTwiddleTables();
	BuildTwiddleTables();

	if ((!Regs_Init()))
	{
		//failed
		return rv_error;
	}
	if (!spg_Init())
	{
		//failed
		return rv_error;
	}
	if (!rend_init())
	{
		//failed
		return rv_error;
	}
	UpdateRRect();
	//olny the renderer cares about thread speciacific shit ..
	if (!rend_thread_start())
	{
		return rv_error;
	}

	return rv_ok;
}

//called when exiting from sh4 thread , from the new thread context (for any thread speciacific de init) :P
void FASTCALL TermPvr()
{
	rend_thread_end();

	rend_term();
	spg_Term();
	Regs_Term();
}

//Helper functions
float GetSeconds()
{
	return timeGetTime()/1000.0f;
}

//Needed for EMUWARN/EMUERROR to work properly
//Misc function to get relative source directory for printf's
char temp[1000];
char* GetNullDCSoruceFileName(char* full)
{
	size_t len = strlen(full);
	while(len>18)
	{
		if (full[len]=='\\')
		{
			memcpy(&temp[0],&full[len-14],15*sizeof(char));
			temp[15*sizeof(char)]=0;
			if (strcmp(&temp[0],"\\nulldc\\nulldc\\")==0)
			{
				strcpy(temp,&full[len+1]);
				return temp;
			}
		}
		len--;
	}
	strcpy(temp,full);
	return &temp[0];
}
void EXPORT_CALL EventHandler(u32 id,void*p)
{
	rend_handle_event(id,p);
}
//Give to the emu pointers for the PowerVR interface
void EXPORT_CALL dcGetInterface(plugin_interface* info)
{
#define c  info->common
#define p info->pvr

	info->InterfaceVersion=PLUGIN_I_F_VERSION;
	
	c.Type=Plugin_PowerVR;
	c.InterfaceVersion=PVR_PLUGIN_I_F_VERSION;

	wcscpy(c.Name,L"nullDC PowerVR -- " REND_NAME L" [" _T(__DATE__) L"]");

	c.Load=Load;
	c.Unload=Unload;
	c.EventHandler=EventHandler;
	p.ExeptionHanlder=0;
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



//Windoze Code implementation of commong classes from here and after ..

//Thread class
cThread::cThread(ThreadEntryFP* function,void* prm)
{
	ended=true;
	Entry=function;
	param=prm;
	//hThread=CreateThread(NULL,NULL,(LPTHREAD_START_ROUTINE)function,prm,CREATE_SUSPENDED,NULL);
}
cThread::~cThread()
{
	//gota think of something !
}
	
void cThread::Start()
{
	if (ended)
		hThread=CreateThread(NULL,NULL,(LPTHREAD_START_ROUTINE)Entry,param,CREATE_SUSPENDED,NULL);
	ended=false;
	ResumeThread(hThread);
}
void cThread::Suspend()
{
	if (ended)
		hThread=CreateThread(NULL,NULL,(LPTHREAD_START_ROUTINE)Entry,param,CREATE_SUSPENDED,NULL);
	ended=false;
	SuspendThread(hThread);
}
void cThread::WaitToEnd(u32 msec)
{
	WaitForSingleObject(hThread,msec);
	ended=true;
}
//End thread class

//cResetEvent Calss
cResetEvent::cResetEvent(bool State,bool Auto)
{
		hEvent = CreateEvent( 
        NULL,             // default security attributes
		Auto?FALSE:TRUE,  // auto-reset event?
		State?TRUE:FALSE, // initial state is State
        NULL			  // unnamed object
        );
}
cResetEvent::~cResetEvent()
{
	//Destroy the event object ?
	 CloseHandle(hEvent);
}
void cResetEvent::Set()//Signal
{
	SetEvent(hEvent);
}
void cResetEvent::Reset()//reset
{
	ResetEvent(hEvent);
}
void cResetEvent::Wait(u32 msec)//Wait for signal , then reset
{
	WaitForSingleObject(hEvent,msec);
}
void cResetEvent::Wait()//Wait for signal , then reset
{
	WaitForSingleObject(hEvent,(u32)-1);
}
//End AutoResetEvent

int cfgGetInt(wchar* key,int def)
{
	return emu.ConfigLoadInt(L"drkpvr",key,def);
}
void cfgSetInt(wchar* key,int val)
{
	emu.ConfigSaveInt(L"drkpvr",key,val);
}

void LoadSettings()
{
	settings.Emulation.AlphaSortMode			=	cfgGetInt(L"Emulation.AlphaSortMode",1);
	settings.Emulation.PaletteMode				=	cfgGetInt(L"Emulation.PaletteMode",1);
	settings.Emulation.ModVolMode				= 	cfgGetInt(L"Emulation.ModVolMode",MVM_NormalAndClip);
	settings.Emulation.ZBufferMode				= 	cfgGetInt(L"Emulation.ZBufferMode",0);
	settings.Emulation.TexCacheMode				= 	cfgGetInt(L"Emulation.TexCacheMode",0);

	settings.OSD.ShowFPS						=	cfgGetInt(L"OSD.ShowFPS",0);
	settings.OSD.ShowStats						=	cfgGetInt(L"OSD.ShowStats",0);

	settings.Video.ResolutionMode				=	cfgGetInt(L"Video.ResolutionMode",0);
	settings.Video.VSync						=	cfgGetInt(L"Video.VSync",0);

	settings.Enhancements.MultiSampleCount		=	cfgGetInt(L"Enhancements.MultiSampleCount",0);
	settings.Enhancements.MultiSampleQuality	=	cfgGetInt(L"Enhancements.MultiSampleQuality",0);
	settings.Enhancements.AspectRatioMode		=	cfgGetInt(L"Enhancements.AspectRatioMode",1);
}


void SaveSettings()
{
	cfgSetInt(L"Emulation.AlphaSortMode",settings.Emulation.AlphaSortMode);
	cfgSetInt(L"Emulation.PaletteMode",settings.Emulation.PaletteMode);
	cfgSetInt(L"Emulation.ModVolMode",settings.Emulation.ModVolMode);
	cfgSetInt(L"Emulation.ZBufferMode",settings.Emulation.ZBufferMode);
	cfgSetInt(L"Emulation.TexCacheMode",settings.Emulation.TexCacheMode);

	cfgSetInt(L"OSD.ShowFPS",settings.OSD.ShowFPS);
	cfgSetInt(L"OSD.ShowStats",settings.OSD.ShowStats);

	cfgSetInt(L"Video.ResolutionMode",settings.Video.ResolutionMode);
	cfgSetInt(L"Video.VSync",settings.Video.VSync);

	cfgSetInt(L"Enhancements.MultiSampleCount",settings.Enhancements.MultiSampleCount);
	cfgSetInt(L"Enhancements.MultiSampleQuality",settings.Enhancements.MultiSampleQuality);
	cfgSetInt(L"Enhancements.AspectRatioMode",settings.Enhancements.AspectRatioMode);
}

int msgboxf(wchar* text,unsigned int type,...)
{
	va_list args;

	wchar temp[2048];
	va_start(args, type);
	vswprintf(temp,2048, text, args);
	va_end(args);


	return MessageBox(NULL,temp,emu_name,type | MB_TASKMODAL);
}
