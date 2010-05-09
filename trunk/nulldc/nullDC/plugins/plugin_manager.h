#pragma once
#include "types.h"
#include "gui_plugin_header.h"
#include "dc/sh4/sh4_if.h"

//Plugin handling functions :)
struct nullDC_plugin
{
	nullDC_plugin()
	{
		Inited=false;
		Loaded=false;
	}
	cDllHandler dll;
	int id;
	wchar dll_file[512];
	s32 Open(wchar* plugin);
	virtual void LoadI(plugin_interface* plugin)=0;
	bool IsOpened();
	void Close();
	bool Inited;
	bool Loaded;
};
struct nullDC_PowerVR_plugin:common_info,pvr_plugin_if,nullDC_plugin
{
	void LoadI(plugin_interface* plugin);
};

struct nullDC_GDRom_plugin:common_info,gdr_plugin_if,nullDC_plugin
{
	void LoadI(plugin_interface* plugin);
};

struct nullDC_AICA_plugin:common_info,aica_plugin_if,nullDC_plugin
{
	void LoadI(plugin_interface* plugin);
};
struct nullDC_ARM_plugin:common_info,arm_plugin_if,nullDC_plugin
{
	void LoadI(plugin_interface* plugin);
};

struct nullDC_Maple_plugin:common_info,maple_plugin_if,nullDC_plugin
{
	u32 ReferenceCount;
	void LoadI(plugin_interface* plugin);
};

struct nullDC_ExtDevice_plugin:common_info,ext_device_plugin_if,nullDC_plugin
{
	void LoadI(plugin_interface* plugin);
};
//Struct to hold plugin info
struct PluginLoadInfo
{
	wchar			Name[128];			//plugin name
	PluginType		Type;				//plugin type
	wchar			dll[512];
};
struct MapleDeviceDefinition:maple_device_definition
{
	wchar dll[512];		//xxxx.dll:id
	wchar dll_file[512];	//xxxx.dll
	u32 id;
};

void EnumeratePlugins();
vector<PluginLoadInfo>* GetPluginList(PluginType type);
vector<MapleDeviceDefinition>* GetMapleDeviceList(MapleDeviceType type);

//This is not used for maple

extern sh4_if*				  sh4_cpu;
//Currently used plugins
extern nullDC_PowerVR_plugin	libPvr;
extern nullDC_GDRom_plugin		libGDR;
extern nullDC_AICA_plugin		libAICA;
extern nullDC_ARM_plugin		libARM;
extern vector<nullDC_Maple_plugin*>libMaple;
extern nullDC_ExtDevice_plugin	libExtDevice;
extern gui_plugin_info			libgui;
//more to come

bool plugins_Load();
void plugins_Unload();
bool plugins_Select();
void plugins_Reset(bool Manual);

//sh4 thread
bool plugins_Init();
void plugins_Term();

void EXPORT_CALL BroadcastEvent(u32 target,u32 eid,void* pdata,u32 ldata);