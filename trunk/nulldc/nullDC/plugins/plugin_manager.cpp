/*
			--nullDC plugin managment code--
	Does plugin enumeration and handles dynamic lib loading.
	Also has code for plugin init/reset/term calls :).
*/
#include "types.h"
#include "plugin_manager.h"
#include "dc/pvr/pvr_if.h"
#include "dc/pvr/pvrLock.h"
#include "dc/aica/aica_if.h"
#include "dc/asic/asic.h"
#include "dc/gdrom/gdrom_if.h"
#include "gui/base.h"
#include "dc/maple/maple_if.h"
#include "config/config.h"
#include "gui/base.h"
#include "naomi/naomi.h"

#include <string.h>
#include "dc\mem\sb.h"

//to avoid including windows.h
#define EXCEPTION_EXECUTE_HANDLER       1
#define EXCEPTION_CONTINUE_SEARCH       0
#define EXCEPTION_CONTINUE_EXECUTION    -1

wchar* lcp_name;
s32 lcp_error=0;

//Currently Loaded plugins
nullDC_PowerVR_plugin		libPvr;
nullDC_GDRom_plugin			libGDR;
nullDC_AICA_plugin			libAICA;
nullDC_ARM_plugin			libARM;
vector<nullDC_Maple_plugin*>	libMaple;
nullDC_ExtDevice_plugin		libExtDevice;
gui_plugin_info				libgui;

sh4_if*						sh4_cpu;
emu_info					eminf;
//more to come

bool plugins_inited=false;
wchar plugins_path[1000]=_T("");
bool plugins_enumerated=false;
bool plugins_first_load=false;

vector<PluginLoadInfo>			PluginList_cached;
vector<MapleDeviceDefinition>		MapleDeviceList_cached;
vector<cDllHandler*>				LoadedTempDlls;

const wchar* pluginDefaults[]=
{
	L"drkPvr_Win32.dll",
	L"ImgReader_Win32.dll",
	L"nullAica_Win32.dll",
	L"vbaARM_Win32.dll",
	L"nullExtDev_Win32.dll",


	L"drkMapleDevices_Win32.dll:2",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"drkMapleDevices_Win32.dll:0",

	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",

	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",

	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
};

//Plugin Enumeration/Validation
enum PluginValidationErrors
{
	pve_invalid_pointer=-200,
	pve_invalid_cif_ver=-201,
	pve_invalid_sif_ver=-202,
	pve_missing_pointers=-203,
	pve_invalid_plugin_type=-204,
};
void EXPORT_CALL plgmng_nileh(u32 nid,void* p)
{
}
void EXPORT_CALL BroadcastEvent(u32 target,u32 eid,void* pdata,u32 ldata)
{
	if (libPvr.Loaded)
		libPvr.EventHandler(eid,pdata);

	if (libGDR.Loaded)
		libGDR.EventHandler(eid,pdata);

	if (libAICA.Loaded)
		libAICA.EventHandler(eid,pdata);

	if (libARM.Loaded)
		libARM.EventHandler(eid,pdata);

	for (u32 i=0;i<libMaple.size();i++)
		if (libMaple[i]->Loaded)
			libMaple[i]->EventHandler(eid,pdata);

	if (libExtDevice.Loaded)
		libExtDevice.EventHandler(eid,pdata);

	
	libgui.EventHandler(eid,pdata);
}
s32 ValidatePlugin(plugin_interface* plugin)
{
	if (plugin==0)
		return  pve_invalid_pointer;

	if (plugin->InterfaceVersion!=PLUGIN_I_F_VERSION)
	{
		log("%X != %X\n",plugin->InterfaceVersion,PLUGIN_I_F_VERSION);
		return pve_invalid_cif_ver;
	}


	switch(plugin->common.Type)
	{
	case Plugin_PowerVR:
		{
			if (plugin->common.InterfaceVersion!=PVR_PLUGIN_I_F_VERSION)
				return pve_invalid_sif_ver;
		}
		break;

	case Plugin_GDRom:
		{
			if (plugin->common.InterfaceVersion!=GDR_PLUGIN_I_F_VERSION)
				return pve_invalid_sif_ver;
		}
		break;

	case Plugin_AICA:
		{
			if (plugin->common.InterfaceVersion!=AICA_PLUGIN_I_F_VERSION)
				return pve_invalid_sif_ver;
		}
		break;

	case Plugin_ARM:
		{
			if (plugin->common.InterfaceVersion!=ARM_PLUGIN_I_F_VERSION)
				return pve_invalid_sif_ver;
		}
		break;

	case Plugin_Maple:
		{
			if (plugin->common.InterfaceVersion!=MAPLE_PLUGIN_I_F_VERSION)
				return pve_invalid_sif_ver;
		}
		break;

	case Plugin_ExtDevice:
		{
			if (plugin->common.InterfaceVersion!=EXTDEVICE_PLUGIN_I_F_VERSION)
				return pve_invalid_sif_ver;
		}
		break;
	default:
		return pve_invalid_plugin_type;
	}

	return rv_ok;
}
bool AddToPluginList(wchar* dll)
{
	cDllHandler* lib=new cDllHandler();
	LoadedTempDlls.push_back(lib);

	//load dll
	if (!lib->Load(dll))
		return false;
	
	//Get functions
	dcGetInterfaceFP* dcGetInterface=(dcGetInterfaceFP*)lib->GetProcAddress("dcGetInterface");

	if (dcGetInterface==0)
		return false;

	
	//Get plugin list
	plugin_interface info;
	memset(&info,0,sizeof(info));

	dcGetInterface(&info);

	//Make sure the plugin is valid :)
	if (s32 rv = ValidatePlugin(&info))
	{
		return false;
	}
	
	PluginLoadInfo load_info;

	load_info.Type=(PluginType)info.common.Type;

	//load_info.subdev_info=info.maple.subdev_info;
	wcscpy(load_info.Name,info.common.Name);
	GetFileNameFromPath(dll,load_info.dll);

	//if correct ver , add em to plugins list :)
	PluginList_cached.push_back(load_info);

	if (info.common.Type==Plugin_Maple)
	{
		MapleDeviceDefinition t;
		wcscpy(t.dll_file,load_info.dll);
		for (int i=0;i<16;i++)
		{
			if (info.maple.devices[i].Type==MDT_EndOfList)
				break;
			t.id=i;
			swprintf(t.dll,_T("%s:%d"),load_info.dll,i);
			maple_device_definition* mdd=&t;
			memcpy(mdd,&info.maple.devices[i],sizeof(maple_device_definition));

			MapleDeviceList_cached.push_back(t);
		}
	}

	//lib.Unload();
	return true;
}

void plugin_FileIsFound(wchar* file,void* param)
{
	wchar dllfile[1024]=_T("");
	wcscat(dllfile,plugins_path);
	wcscat(dllfile,file);
	AddToPluginList(dllfile);
}
//Get a list of all plugins that exist on plugin directory and can be loaded
vector<PluginLoadInfo>* GetPluginList(PluginType type)
{
	vector<PluginLoadInfo>* rv=new vector<PluginLoadInfo>();

	for (u32 i=0;i<PluginList_cached.size();i++)
	{
		PluginLoadInfo* t=&PluginList_cached[i];
		if (t->Type == type) 
			rv->push_back(*t);
	}

	return rv;
}
vector<MapleDeviceDefinition>* GetMapleDeviceList(MapleDeviceType type)
{
	vector<MapleDeviceDefinition>* rv = new vector<MapleDeviceDefinition>();
	for (size_t i=0;i<MapleDeviceList_cached.size();i++)
	{
		if ((type==MDT_EndOfList) || MapleDeviceList_cached[i].Type==(u32)type)
		{
			rv->push_back(MapleDeviceList_cached[i]);
		}
	}
	return rv;
}
void EnumeratePlugins()
{
	if (plugins_enumerated)
		return;
	plugins_enumerated=true;

	cfgLoadStr(_T("emu"),_T("PluginPath"),plugins_path,0);

	PluginList_cached.clear();
	MapleDeviceList_cached.clear();

	
	wchar dllfile[1024]=_T("");
	wcscat(dllfile,plugins_path);
	wcscat(dllfile,_T("*.dll"));
	//Look & load all dll's :)
	FindAllFiles(plugin_FileIsFound,dllfile,0);

	//We unload all dll's at once :) this gives a HUGE speedup , as it does not reload common
	//dll's while enumerating ...
	for (size_t i=0;i<LoadedTempDlls.size();i++)
	{
		if (LoadedTempDlls[i]->IsLoaded())
			LoadedTempDlls[i]->Unload();
		delete LoadedTempDlls[i];
	}
	LoadedTempDlls.clear();
}
//
//
//Implementation of the plugin classes
//
//
//-101 -> null Plugin
//-102 path too long
//-103 -> invalid name (split failed)
//-104 dll load failed
//-105 some of the exports are missing
//-106 wrong i/f version
//-107 LoadI (plugin specialised interface) failed
//-108 Invalid id (id >= count)
//0 (rv_ok) -> ok :)
s32 nullDC_plugin::Open(wchar* plugin)
{
	Inited=false;
	Loaded=false;

	if (!wcscmp(plugin,_T("NULL")))
	{
		return -101;
	}

	if (wcslen(plugin)>480)
	{
		msgboxf(_T("Plugin dll path is way too long"),MBX_ICONERROR | MBX_OK);
		return -102;
	}
	
	wchar ttt[512];
	wcscpy(dll_file,plugin);
	size_t dllfl=wcslen(dll_file);
	size_t pathl=wcslen(plugins_path);
	if (dllfl<=pathl || (memcmp(dll_file,plugins_path,pathl)!=0))
	{
		wcscpy(ttt,plugins_path);
		wcscat(ttt,dll_file);
		wcscpy(dll_file,plugin);
	}
	else
	{
		wcscpy(ttt,dll_file);
		wcscpy(dll_file,&dll_file[pathl]);
	}
	if (!dll.Load(ttt))
		return -104;
	dcGetInterfaceFP* getplugin=(dcGetInterfaceFP*)dll.GetProcAddress("dcGetInterface");

	
	if (getplugin==0)
	{
		dll.Unload();
		return -105;
	}

	plugin_interface t;
	memset(&t,0,sizeof(t));
	getplugin(&t);

	if (s32 rv=ValidatePlugin(&t))
	{
		dll.Unload();
		return rv;
	}

	if (t.common.EventHandler==0)
		t.common.EventHandler=plgmng_nileh;

	LoadI(&t);
	
	return rv_ok;
}
bool nullDC_plugin::IsOpened()
{
	return dll.IsLoaded();
}
void nullDC_plugin::Close()
{
	if (IsOpened())
		dll.Unload();
}
//*::LoadI
void nullDC_PowerVR_plugin::LoadI(plugin_interface* t)
{
	common_info* p1= this;
	pvr_plugin_if* p2= this;
	memcpy(p1,&t->common,sizeof(*p1));
	memcpy(p2,&t->pvr,sizeof(*p2));
}
void nullDC_GDRom_plugin::LoadI(plugin_interface* t)
{
	common_info* p1= this;
	gdr_plugin_if* p2= this;
	memcpy(p1,&t->common,sizeof(*p1));
	memcpy(p2,&t->gdr,sizeof(*p2));
}
void nullDC_AICA_plugin::LoadI(plugin_interface* t)
{
	common_info* p1= this;
	aica_plugin_if* p2= this;
	memcpy(p1,&t->common,sizeof(*p1));
	memcpy(p2,&t->aica,sizeof(*p2));
}
void nullDC_ARM_plugin::LoadI(plugin_interface* t)
{
	common_info* p1= this;
	arm_plugin_if* p2= this;
	memcpy(p1,&t->common,sizeof(*p1));
	memcpy(p2,&t->arm,sizeof(*p2));
}
void nullDC_Maple_plugin::LoadI(plugin_interface* t)
{
	common_info* p1= this;
	maple_plugin_if* p2= this;
	memcpy(p1,&t->common,sizeof(*p1));
	memcpy(p2,&t->maple,sizeof(*p2));

	ReferenceCount=0;
}
void nullDC_ExtDevice_plugin::LoadI(plugin_interface* t)
{
	common_info* p1= this;
	ext_device_plugin_if* p2= this;
	memcpy(p1,&t->common,sizeof(*p1));
	memcpy(p2,&t->ext_dev,sizeof(*p2));
}
//
//Maple Plugins :
/*
	when nullDC is started , ALL maple plugins are loaded.Thats why its good to not allocate much things on
	Load().Then , right after Load() , the devices are created(using CreateMain/Sub()).As the user may edit the 
	config , devices can be deleted (using DestroyMain/Sub) and recrated.On CreateMain/Sub the plugin should
	add the menus (if any) and allocate resources as needed.

	When emulation starts , InitMain/Sub is called.Befor Term* is called before Destroy*.If a device is
	hotplugable , it can go into many Create->Init->Term->Destroy cycles while the emulation is running.

	Its quite simple :
	
	Load

	a:
	Create* ->Destroy* -> a or b
			
	        ->Init* -> Term* ->Destroy*-> if(hotplug) (a or b) else b

	b:
	Unload
*/
void maple_cfg_name(int i,int j,wchar * out)
{
	swprintf(out,_T("Current_maple%d_%d"),i,j);
}
void maple_cfg_plug(int i,int j,wchar * out)
{
	wchar temp[512];
	maple_cfg_name(i,j,temp);
	cfgLoadStr(_T("nullDC_plugins"),temp,out,pluginDefaults[NDCS_PLUGIN_MAPLE_0_0-NDCS_PLUGIN_PVR + i*6+j]);
}
u8 GetMaplePort(u32 port,u32 device)
{
	u32 rv=port<<6;

	rv|=1<<device;
	return (u8)rv;
}

nullDC_Maple_plugin* FindMaplePlugin(MapleDeviceDefinition* mdd)
{
	for (u32 i=0;i<libMaple.size();i++)
	{
		if (wcscmp(libMaple[i]->dll_file,mdd->dll_file)==0)
			return libMaple[i];
	}
	msgboxf(_T("Fatal error :nullDC_Maple_plugin* FindMaplePlugin(MapleDeviceDefinition* mdd) failed"),MBX_ICONERROR);
	return 0;
}
MapleDeviceDefinition* FindMapleDevice(wchar* mfdn)
{
	for (u32 i=0;i<MapleDeviceList_cached.size();i++)
	{
		if (wcscmp(MapleDeviceList_cached[i].dll,mfdn)==0)
			return &MapleDeviceList_cached[i];
	}
	msgboxf(_T("Fatal error :MapleDeviceDefinition* FindMapleDevice(char* mfdn=%s) failed"),MBX_ICONERROR,mfdn);
	return 0;
}

s32 CreateMapleDevice(u32 pos,wchar* device,bool hotplug);
s32 CreateMapleSubDevice(u32 pos,u32 subport,wchar* device,bool hotplug);
s32 DestroyMapleDevice(u32 pos);
s32 DestroyMapleSubDevice(u32 pos,u32 subport);
u32 GetMaplePort(u32 addr);
void EXPORT_CALL menu_handle_attach_sub(u32 id,void* win,void* p)
{
	size_t offs=(u8*)p-(u8*)0;
	u32 plid=(u32)offs&0xFFFFFF;
	
	u32 prt=((u32)offs>>24);

	u32 port=prt&3;
	u32 subport=prt>>2;

	verify(MapleDevices_dd[port][subport].Created==false && MapleDevices[port].subdevices[subport].connected==false);

	int rv=CreateMapleSubDevice(port,subport,MapleDeviceList_cached[plid].dll,true);
	
	if (rv<0)
		return;

	MapleDevices_dd[port][subport].Created=true;

	if (plugins_inited)
	{	
		MapleDeviceDefinition* mdd=MapleDevices_dd[port][subport].mdd;

		FindMaplePlugin(mdd)->Init(MapleDevices[port].subdevices[subport].data,mdd->id,0);
		MapleDevices_dd[port][subport].Inited=true;
	}
}
void EXPORT_CALL menu_handle_detach_sub(u32 id,void* win,void* p)
{
	size_t offs=(u8*)p-(u8*)0;

	u32 prt=((u32)offs>>24);

	u32 port=prt&3;
	u32 subport=prt>>2;

	verify(MapleDevices_dd[port][subport].Created==true);

	if (MapleDevices_dd[port][subport].Inited)
	{
		MapleDeviceDefinition* mdd=MapleDevices_dd[port][subport].mdd;

		FindMaplePlugin(mdd)->Term(MapleDevices[port].subdevices[subport].data,mdd->id);
		MapleDevices_dd[port][subport].Inited=false;
	}

	if (MapleDevices_dd[port][subport].Created)
	{
		DestroyMapleSubDevice(port,subport);
		MapleDevices_dd[port][subport].Created=false;
	}
}
void cm_MampleSubEmpty(u32 root,u32 port,u32 subport)
{
	libgui.DeleteAllMenuItemChilds(root);
	//l8r
	for (size_t i=0;i<MapleDeviceList_cached.size();i++)
	{
		if (!(MapleDeviceList_cached[i].Flags & MDTF_Hotplug))
			continue;
		if (MapleDeviceList_cached[i].Type!=MDT_Sub)
			continue;
		wchar text[512];
		swprintf(text,_T("Attach %s"),MapleDeviceList_cached[i].Name);
		//Attach NAME
		u32 menu=libgui.AddMenuItem(root,-1,text,menu_handle_attach_sub,0);
		MenuItem mi;
		u8* t=0;
		t+=i;
		t+=(port<<24)|(subport<<26);
		mi.PUser=t;
		libgui.SetMenuItem(menu,&mi,MIM_PUser);
	}
}
void cm_MampleSubUsed(u32 root,u32 port,u32 subport)
{
	libgui.DeleteAllMenuItemChilds(root);
	//l8r
	u32 menu=libgui.AddMenuItem(root,-1,_T("Unplug"),menu_handle_detach_sub,0);
	MenuItem mi;
	u8* t=0;
	t+=(port<<24);
	t+=(subport<<26);
	mi.PUser=t;
	libgui.SetMenuItem(menu,&mi,MIM_PUser);

	libgui.AddMenuItem(root,-1,0,0,0);
}
void EXPORT_CALL menu_handle_attach_main(u32 id,void* win,void* p)
{
	size_t offs=(u8*)p-(u8*)0;
	u32 plid=(u32)offs&0xFFFFFF;
	u32 port=((u32)offs>>24);

	verify(MapleDevices_dd[port][5].Created==false);

	int rv = CreateMapleDevice(port,MapleDeviceList_cached[plid].dll,true);
	if (rv<0)
		return;

	MapleDevices_dd[port][5].Created=true;

	if (plugins_inited)
	{	
		MapleDeviceDefinition* mdd=MapleDevices_dd[port][5].mdd;

		FindMaplePlugin(mdd)->Init(MapleDevices[port].data,mdd->id,0);
		MapleDevices_dd[port][5].Inited=true;
	}
}
void EXPORT_CALL menu_handle_detach_main(u32 id,void* win,void* p)
{
	size_t offs=(u8*)p-(u8*)0;

	u32 port=((u32)offs>>24);

	verify(MapleDevices_dd[port][5].Created==true);

	if (MapleDevices_dd[port][5].Inited)
	{
		MapleDeviceDefinition* mdd=MapleDevices_dd[port][5].mdd;

		FindMaplePlugin(mdd)->Term(MapleDevices[port].data,mdd->id);
		MapleDevices_dd[port][5].Inited=false;
	}

	if (MapleDevices_dd[port][5].Created)
	{
		DestroyMapleDevice(port);
		MapleDevices_dd[port][5].Created=false;
	}
}
void cm_MampleMainEmpty(u32 root,u32 port)
{
	libgui.DeleteAllMenuItemChilds(root);
	for (int i=0;i<5;i++)
		MenuIDs.Maple_port[port][i]=0;
	//Add the
	
	for (size_t i=0;i<MapleDeviceList_cached.size();i++)
	{
		if (!(MapleDeviceList_cached[i].Flags & MDTF_Hotplug))
			continue;
		if (MapleDeviceList_cached[i].Type!=MDT_Main)
			continue;
		wchar text[512];
		swprintf(text,_T("Attach %s"),MapleDeviceList_cached[i].Name);
		//Attach NAME
		u32 menu=libgui.AddMenuItem(root,-1,text,menu_handle_attach_main,0);
		MenuItem mi;
		u8* t=0;
		t+=i;
		t+=(port<<24);
		mi.PUser=t;
		libgui.SetMenuItem(menu,&mi,MIM_PUser);
	}
}
void cm_MampleMainUsed(u32 root,u32 port,u32 flags)
{
	libgui.DeleteAllMenuItemChilds(root);
	
	if (flags&0x1f)
	{
		//add the 
		//Subdevice X ->[default empty]
		for (u32 i=0;i<5;i++)
		{
			if (flags & (1<<i))
			{
				wchar temp[512];
				swprintf(temp,_T("Subdevice %d"),i+1);
				u32 sdr=libgui.AddMenuItem(root,-1,temp,0,0);
				MenuIDs.Maple_port[port][i]=sdr;
				cm_MampleSubEmpty(sdr,port,i);
			}
			else
			{
				MenuIDs.Maple_port[port][i]=0;
			}
		}


		//-
		libgui.AddMenuItem(root,-1,0,0,0);
	}
	//Unplug
	u32 menu=libgui.AddMenuItem(root,-1,_T("Unplug"),menu_handle_detach_main,0);
	MenuItem mi;
	u8* t=0;
	t+=(port<<24);
	mi.PUser=t;
	libgui.SetMenuItem(menu,&mi,MIM_PUser);
}
//These are the 'raw' functions , they handle creation/destruction of a device *only*
enum pmd_errors
{
	pmde_device_state=-250,		//device allready connected/disconected
	pmde_invalid_pos=-251,		//invalid device pos (0~3 , 0~4 are valid only)
	pmde_failed_create=-252,	//failed to create device
	pmde_failed_create_s=-253,	//failed to create device , silent error
	pmde_failed_find_mdd_s=-254,//failed to find maple device definition
	pmde_failed_find_maple_plugin_s=-255,//failed to find maple plugin

};
s32 CreateMapleDevice(u32 pos,wchar* device,bool hotplug)
{
	if (pos>3)
		return pmde_invalid_pos;

	if (MapleDevices[pos].connected)
		return pmde_device_state;

	MapleDeviceDefinition* mdd=FindMapleDevice(device);
	if (!mdd)
		return pmde_failed_find_mdd_s;
	nullDC_Maple_plugin* plg =FindMaplePlugin(mdd);
	if (!mdd)
		return pmde_failed_find_maple_plugin_s;
	cm_MampleMainUsed(MenuIDs.Maple_port[pos][5],pos,mdd->Flags);
	MapleDevices[pos].port=GetMaplePort(pos,5);
	
	if (s32 rv= plg->CreateMain(&MapleDevices[pos],mdd->id,hotplug?MDCF_Hotplug:MDCF_None,MenuIDs.Maple_port[pos][5]))
	{
		cm_MampleMainEmpty(MenuIDs.Maple_port[pos][5],pos);
		if (rv==rv_error)
			return pmde_failed_create;
		else
			return pmde_failed_create_s;
	}
	MapleDevices_dd[pos][5].mdd=mdd;
	MapleDevices[pos].connected=true;

	return rv_ok;
}
s32 DestroyMapleDevice(u32 pos)
{
	if (pos>3)
		return pmde_invalid_pos;

	if (!MapleDevices[pos].connected)
		return pmde_device_state;

	if (!MapleDevices_dd[pos][5].Created)
		return pmde_device_state;
	

	//MapleDevices_dd[pos][5].Inited

	MapleDeviceDefinition* mdd=MapleDevices_dd[pos][5].mdd;
	MapleDevices_dd[pos][5].mdd=0;
	nullDC_Maple_plugin* plg =FindMaplePlugin(mdd);

	plg->Destroy(MapleDevices[pos].data,mdd->id);
	cm_MampleMainEmpty(MenuIDs.Maple_port[pos][5],pos);

	MapleDevices[pos].connected=false;

	return rv_ok;
}
s32 CreateMapleSubDevice(u32 pos,u32 subport,wchar* device,bool hotplug)
{
	if (pos>3)
		return pmde_invalid_pos;
	if (subport>4)
		return pmde_invalid_pos;

	if (!MapleDevices[pos].connected)
		return pmde_device_state;

	if (MapleDevices[pos].subdevices[subport].connected)
		return pmde_device_state;

	MapleDeviceDefinition* mdd=FindMapleDevice(device);
	if (!mdd)
		return pmde_failed_create_s;

	nullDC_Maple_plugin* plg =FindMaplePlugin(mdd);
	
	cm_MampleSubUsed(MenuIDs.Maple_port[pos][subport],pos,subport);
	MapleDevices[pos].subdevices[subport].port=GetMaplePort(pos,subport);
	
	if (s32 rv= plg->CreateSub(&MapleDevices[pos].subdevices[subport],mdd->id,hotplug?MDCF_Hotplug:MDCF_None,MenuIDs.Maple_port[pos][subport]))
	{
		cm_MampleSubEmpty(MenuIDs.Maple_port[pos][subport],pos,subport);
		if (rv==rv_error)
			return pmde_failed_create;
		else
			return pmde_failed_create_s;
	}
	MapleDevices_dd[pos][subport].mdd=mdd;
	MapleDevices[pos].subdevices[subport].connected=true;

	return rv_ok;
}
s32 DestroyMapleSubDevice(u32 pos,u32 subport)
{
	if (pos>3)
		return pmde_invalid_pos;

	if (!MapleDevices[pos].connected)
		return pmde_device_state;
	
	if (!MapleDevices[pos].subdevices[subport].connected)
		return pmde_device_state;

	if (!MapleDevices_dd[pos][subport].Created)
		return pmde_device_state;
	
	MapleDeviceDefinition* mdd=MapleDevices_dd[pos][subport].mdd;
	MapleDevices_dd[pos][subport].mdd=0;
	nullDC_Maple_plugin* plg =FindMaplePlugin(mdd);

	plg->Destroy(MapleDevices[pos].subdevices[subport].data,mdd->id);
	cm_MampleSubEmpty(MenuIDs.Maple_port[pos][subport],pos,subport);
	MapleDevices[pos].subdevices[subport].connected=false;
	return rv_ok;
}

//Misc handling code ~
/************************************/
/*********Plugin Load/Unload*********/
/************************************/
template<typename T>
s32 load_plugin(wchar* dll,T* plug,u32 rootmenu)
{
	lcp_error=-100;
	lcp_name=dll;
	if (!plug->Loaded)
	{
		SetMenuItemHandler(rootmenu,0);
		eminf.RootMenu=rootmenu;
		if ((lcp_error=plug->Open(dll))!=rv_ok)
		{
			return rv_error;
		}
		if (s32 rv = plug->Load(&eminf))
		{
			return rv;
		}
		plug->Loaded=true;
		wprintf(L"Loaded %s[%s]\n",plug->Name,lcp_name);
	}

	return rv_ok;
}
template<typename T>
void unload_plugin(T* plug,u32 rootmenu)
{
	lcp_name=plug->Name;
	libgui.DeleteAllMenuItemChilds(rootmenu);
	SetMenuItemHandler(rootmenu,0);
	if (plug->IsOpened()) 
	{
		if (plug->Loaded) 
		{
			plug->Unload(); 
			plug->Loaded=false;
			wprintf(L"Unloaded %s[%s]\n",plug->Name,plug->dll_file);
		}
		plug->Close(); 
	}
}

#define load_plugin_(cfg_name,to,menu,def) \
{ \
	wchar dllf[512]; \
	dllf[0]=0; \
	cfgLoadStr(_T("nullDC_plugins"),cfg_name,dllf,def); \
	if (s32 rv=load_plugin(dllf,to,menu)) \
		return rv; \
}

//Internal function for load/unload w/ error checking :)
s32 plugins_Load_()
{
	if (!plugins_first_load)
	{
		//if first time load , init the maple menus
		plugins_first_load=true;
		for (u32 i=0;i<4;i++)
			cm_MampleMainEmpty(MenuIDs.Maple_port[i][5],i);
	}
	eminf.ConfigLoadStr=cfgLoadStr;
	eminf.ConfigSaveStr=cfgSaveStr;
	eminf.ConfigLoadInt=cfgLoadInt;
	eminf.ConfigSaveInt=cfgSaveInt;
	eminf.ConfigExists=cfgExists;

	eminf.AddMenuItem=libgui.AddMenuItem;
	eminf.SetMenuItemStyle=libgui.SetMenuItemStyle;
	eminf.GetMenuItem=libgui.GetMenuItem;
	eminf.SetMenuItem=libgui.SetMenuItem;
	eminf.DeleteMenuItem=libgui.DeleteMenuItem;

	eminf.GetRenderTarget=GetRenderTargetHandle;
	eminf.DebugMenu=MenuIDs.Debug;
	eminf.BroardcastEvent=BroadcastEvent;

	load_plugin_(_T("Current_PVR"),&libPvr,MenuIDs.PowerVR,pluginDefaults[NDCS_PLUGIN_PVR-NDCS_PLUGIN_PVR]);
	load_plugin_(_T("Current_GDR"),&libGDR,MenuIDs.GDRom,pluginDefaults[NDCS_PLUGIN_GDR-NDCS_PLUGIN_PVR]);
	load_plugin_(_T("Current_AICA"),&libAICA,MenuIDs.Aica,pluginDefaults[NDCS_PLUGIN_AICA-NDCS_PLUGIN_PVR]);
	load_plugin_(_T("Current_ARM"),&libARM,MenuIDs.Arm,pluginDefaults[NDCS_PLUGIN_ARM-NDCS_PLUGIN_PVR]);
 	load_plugin_(_T("Current_ExtDevice"),&libExtDevice,MenuIDs.ExtDev,pluginDefaults[NDCS_PLUGIN_EXTDEV-NDCS_PLUGIN_PVR]);

	vector<PluginLoadInfo>* mpl= GetPluginList(Plugin_Maple);

	if (libMaple.size()!=mpl->size())
	{
		for (size_t i=0;i<mpl->size();i++)
		{
			wchar fname[512];
			nullDC_Maple_plugin* cmp= new nullDC_Maple_plugin();

			GetFileNameFromPath((*mpl)[i].dll,fname);

			if (s32 rv=load_plugin(fname,cmp,MenuIDs.Maple))
			{
				delete cmp;
				return rv;
			}
			libMaple.push_back(cmp);
		}
	}
	delete mpl;

	//Create Maple Devices

	for (int port=0;port<4;port++)
	{
		MapleDevices[port].port=GetMaplePort(port,5);
		for (int sub=0;sub<5;sub++)
		{
			MapleDevices[port].subdevices[sub].port=GetMaplePort(port,sub);
		}
	}

	wchar plug_name[512];
	for (int port=0;port<4;port++)
	{
		maple_cfg_plug(port,5,plug_name);
		lcp_name=plug_name;
		if (wcscmp(plug_name,L"NULL")!=0)
		{
			if (!MapleDevices_dd[port][5].Created)
			{
				s32 rv=CreateMapleDevice(port,plug_name,false);
				if (rv!=rv_ok)
				{
					return rv;
				}
				MapleDevices_dd[port][5].Created=true;
			}
			u32 flags=MapleDevices_dd[port][5].mdd->Flags;

			for (int subport=0;subport<5;subport++)
			{
				if (!(flags & (1<<subport)))
				{
					MapleDevices_dd[port][subport].Created=false;
					continue;
				}

				maple_cfg_plug(port,subport,plug_name);
				lcp_name=plug_name;
				if (wcscmp(plug_name,L"NULL")!=0)
				{
					if (!MapleDevices_dd[port][subport].Created)
					{
						//Create it
						s32 rv=CreateMapleSubDevice(port,subport,plug_name,false);
						if (rv!=rv_ok)
						{
							return rv;
						}

						MapleDevices_dd[port][subport].Created=true;
					}
				}
			}
		}
		else
		{
			MapleDevices_dd[port][5].Created=false;
		}
	}

	return rv_ok;
}

//Loads plugins , if allready loaded does nothing :)
bool plugins_Load()
{
	//__try 
	{
		if (s32 rv=plugins_Load_())
		{
			if (rv==rv_error)
			{
				msgboxf(_T("Unable to load %s plugin , errorlevel=%d"),MBX_ICONERROR,lcp_name,lcp_error);
			}

			plugins_Unload();
			return false;
		}
		else
			return true;
	}
	/*__except(EXCEPTION_EXECUTE_HANDLER)
	{
		msgboxf(_T("Unhandled exeption while loading %s plugin"),MBX_ICONERROR,lcp_name);
		plugins_Unload();
		return false;
	}*/
}

//Unloads plugins , if allready unloaded does nothing
void plugins_Unload()
{
	__try 
	{
		for (int port=3;port>=0;port--)
		{
			if (MapleDevices_dd[port][5].Created)
			{
				for (int subport=4;subport>=0;subport--)
				{
					if (MapleDevices_dd[port][subport].Created)
					{
						MapleDevices_dd[port][subport].Created=false;

						s32 rv=DestroyMapleSubDevice(port,subport);
						if (rv!=rv_ok)
						{
							log("DestroyMapleSubDevice(port,subport) failed: %d\n",rv);
						}
					}
				}
				MapleDevices_dd[port][5].Created=false;
				s32 rv=DestroyMapleDevice(port);
				if (rv!=rv_ok)
				{
					log("DestroyMapleDevice(port) failed: %d\n",rv);
				}
				/*
				for (int subport=0;subport<4;subport++)
				{
					if (MapleDevices_dd[port][subport].Created)
					{
					//destroy it
						MapleDevices_dd[port][subport].Created=false;
					}
				}
				*/
			}
		}

		for (size_t i=libMaple.size();i>0;i--)
		{
			unload_plugin(libMaple[i-1],MenuIDs.Maple);
		}
		libMaple.clear();

		unload_plugin(&libExtDevice,MenuIDs.ExtDev);
		unload_plugin(&libARM,MenuIDs.Arm);
		unload_plugin(&libAICA,MenuIDs.Aica);
		unload_plugin(&libGDR,MenuIDs.GDRom);
		unload_plugin(&libPvr,MenuIDs.PowerVR);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		msgboxf(_T("Unhandled exeption while unloading %s\n"),MBX_ICONERROR,lcp_name);
	}
}
/************************************/
/******Plugin selection-switch*******/
/************************************/
template<typename T>
void CheckPlugin(wchar*cfg_name, T* plug,u32 rootmenu)
{
	wchar dllf[512];
	dllf[0]=0;
	cfgLoadStr(_T("nullDC_plugins"),cfg_name,dllf,_T("NULL"));
	if (wcscmp(plug->dll_file,dllf)!=0)
	{
		unload_plugin(plug,rootmenu);
	}
}
void CheckMapleMD()
{

}
bool plugins_Select()
{
	if (plugins_inited)
	{
		if (msgboxf(_T("Emulation is started , plugins can't be changed now ..\r\nWant to Continue anyway ? (Changes will will take effect after retart)"),MBX_YESNO | MBX_ICONEXCLAMATION | MBX_TASKMODAL)==MBX_RV_NO)
			return false;
	}

	bool rv= SelectPluginsGui();

	if (rv && plugins_inited==false)
	{
		CheckPlugin(_T("Current_PVR"),&libPvr,MenuIDs.PowerVR);
		CheckPlugin(_T("Current_GDR"),&libGDR,MenuIDs.GDRom);
		CheckPlugin(_T("Current_AICA"),&libAICA,MenuIDs.Aica);
		CheckPlugin(_T("Current_ARM"),&libARM,MenuIDs.Arm);
 		CheckPlugin(_T("Current_ExtDevice"),&libExtDevice,MenuIDs.ExtDev);

		//Maple plugins
		for (int port =0;port<4;port++)
		{
			wchar dllf[512];
			maple_cfg_plug(port,5,dllf);
			if (MapleDevices_dd[port][5].Created)
			{
				//Check if main device is changed 
				if (wcscmp(dllf,MapleDevices_dd[port][5].mdd->dll)!=0)
				{
					//if it is destroy it & its childs
					for (int i=4;i>=0;i--)
					{
						DestroyMapleSubDevice(port,i);
						MapleDevices_dd[port][i].Created=false;
					}
					DestroyMapleDevice(port);
					MapleDevices_dd[port][5].Created=false;
				}
				else
				{
					for (int i=4;i>=0;i--)
					{
						if (!MapleDevices_dd[port][i].Created)
							continue;
						maple_cfg_plug(port,i,dllf);
						//else , check if a child is changed & destroy it
						if (wcscmp(dllf,MapleDevices_dd[port][i].mdd->dll)!=0)
						{
							DestroyMapleSubDevice(port,i);
							MapleDevices_dd[port][i].Created=false;
						}
					}
				}
			}
		}
		return plugins_Load();
	}
	return rv;
}
/************************************/
/*******Plugin Init/Term/Reset*******/
/************************************/
s32 plugins_Init_()
{
	if (plugins_inited)
		return rv_ok;
	plugins_inited=true;	

	gdr_init_params gdr_info;
	gdr_info.DriveNotifyEvent=NotifyEvent_gdrom;

	lcp_name=libGDR.Name;
	if (s32 rv = libGDR.Init(&gdr_info))
		return rv;
	libGDR.Inited=true;

	#ifdef BUILD_NAOMI
	if (!NaomiSelectFile(GetRenderTargetHandle()))
		return rv_serror;
	#endif
	

	aica_init_params aica_info;
	aica_info.RaiseInterrupt=asic_RaiseInterrupt;
	aica_info.SB_ISTEXT=&SB_ISTEXT;
	aica_info.CancelInterrupt=asic_CancelInterrupt;
	aica_info.CDDA_Sector=gdrom_get_cdda;
	aica_info.aica_ram=aica_ram.data;
	aica_info.ArmInterruptChange=libARM.ArmInterruptChange;

	lcp_name=libAICA.Name;
	if (s32 rv = libAICA.Init(&aica_info))
		return rv;
	libAICA.Inited=true;

	arm_init_params arm_info;
	arm_info.ReadMem_aica_reg=libAICA.ReadMem_aica_reg;
	arm_info.WriteMem_aica_reg=libAICA.WriteMem_aica_reg;
	arm_info.aica_ram=aica_ram.data;

	lcp_name=libARM.Name;
	if (s32 rv = libARM.Init(&arm_info))
		return rv;
	libARM.Inited=true;

	ext_device_init_params ext_device_info;
	ext_device_info.RaiseInterrupt=asic_RaiseInterrupt;
	ext_device_info.SB_ISTEXT=&SB_ISTEXT;
	ext_device_info.CancelInterrupt=asic_CancelInterrupt;

	lcp_name=libExtDevice.Name;
	if (s32 rv = libExtDevice.Init(&ext_device_info))
		return rv;
	libExtDevice.Inited=true;

	maple_init_params& mip=*(maple_init_params*)0;

	//Init Created maple devices
	for ( int i=0;i<4;i++)
	{
		if (MapleDevices_dd[i][5].Created)
		{
			lcp_name=_T("Made Device");
			verify(MapleDevices_dd[i][5].mdd!=0);
			//Init
			nullDC_Maple_plugin *nmp=FindMaplePlugin(MapleDevices_dd[i][5].mdd);
			lcp_name=MapleDevices_dd[i][5].mdd->Name;
			if (s32 rv=nmp->Init(MapleDevices[i].data,MapleDevices_dd[i][5].mdd->id,&mip))
				return rv;
			
			MapleDevices_dd[i][5].Inited=true;

			for (int j=0;j<5;j++)
			{
				if (MapleDevices_dd[i][j].Created)
				{
					lcp_name=_T("Made SubDevice");
					verify(MapleDevices_dd[i][j].mdd!=0);
					//Init
					nullDC_Maple_plugin *nmp=FindMaplePlugin(MapleDevices_dd[i][j].mdd);
					lcp_name=MapleDevices_dd[i][j].mdd->Name;
					if (s32 rv=nmp->Init(MapleDevices[i].subdevices[j].data,MapleDevices_dd[i][j].mdd->id,&mip))
						return rv;
				}
			}
		}
		else
			MapleDevices_dd[i][5].Inited=false;
	}

	//pvr
	pvr_init_params pvr_info;
	pvr_info.RaiseInterrupt=asic_RaiseInterrupt;
	pvr_info.vram=&vram[0];
	pvr_info.vram_lock_32=vramlock_Lock_32;
	pvr_info.vram_lock_64=vramlock_Lock_64;
	pvr_info.vram_unlock=vramlock_Unlock_block;
	pvr_info.osdDraw=0;

	lcp_name=libPvr.Name;
	if (s32 rv = libPvr.Init(&pvr_info))
		return rv;
	libPvr.Inited=true;

	return rv_ok;
}


bool plugins_Init()
{
	__try 
	{
		if (s32 rv = plugins_Init_())
		{
			if (rv==rv_error)
			{
				msgboxf(_T("Failed to init %s"),MBX_ICONERROR,lcp_name);				
			}
			plugins_Term();
			return false;
		}
		return true;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		msgboxf(_T("Unhandled exeption while Initing %s plugin"),MBX_ICONERROR,lcp_name);
		plugins_Term();
		return false;
	}
}
template<typename T>
void term_plugin(T& pl)
{
	if (pl.Inited)
	{
		pl.Inited=false;
		pl.Term();
	}
}
void plugins_Term()
{
	if (!plugins_inited)
		return;
	plugins_inited=false;

	//Term inited maple devices
	for ( int i=3;i>=0;i--)
	{
		if (MapleDevices_dd[i][5].Inited)
		{ 
			for (int j=4;j>=0;j--)
			{
				if (MapleDevices_dd[i][j].Inited)
				{
					MapleDevices_dd[i][j].Inited=false;

					verify(MapleDevices_dd[i][j].mdd!=0);
					//term
					nullDC_Maple_plugin *nmp=FindMaplePlugin(MapleDevices_dd[i][j].mdd);
					nmp->Term(MapleDevices[i].subdevices[j].data,MapleDevices_dd[i][j].mdd->id);
				}
			}

			MapleDevices_dd[i][5].Inited=false;

			verify(MapleDevices_dd[i][5].mdd!=0);
			//term
			nullDC_Maple_plugin *nmp=FindMaplePlugin(MapleDevices_dd[i][5].mdd);
			nmp->Term(MapleDevices[i].data,MapleDevices_dd[i][5].mdd->id);
		}
	}
	
	//term all plugins
	term_plugin(libExtDevice);
	term_plugin(libARM);
	term_plugin(libAICA);
	term_plugin(libGDR);
	term_plugin(libPvr);
}

void plugins_Reset(bool Manual)
{
	libPvr.Reset(Manual);
	libGDR.Reset(Manual);
	libAICA.Reset(Manual);
	libARM.Reset(Manual);
	libExtDevice.Reset(Manual);
}