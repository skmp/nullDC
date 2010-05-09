#pragma once
#include "types.h"
#include "dc/sh4/sh4_if.h"
#include "plugins/plugin_manager.h"

bool CreateGUI();
void DestroyGUI();
void GuiLoop();

void* EXPORT_CALL GetRenderTargetHandle();
bool SelectPluginsGui();

//void EXPORT_CALL DeleteAllMenuItemChilds(u32 id);
void SetMenuItemHandler(u32 id,MenuItemSelectedFP* h);
/*
u32 EXPORT_CALL AddMenuItem(u32 parent,s32 pos,char* text,MenuItemSelectedFP* handler ,u32 checked);
void EXPORT_CALL SetMenuItemStyle(u32 id,u32 style,u32 mask);
void EXPORT_CALL GetMenuItem(u32 id,MenuItem* info,u32 mask);
void EXPORT_CALL SetMenuItem(u32 id,MenuItem* info,u32 mask);
void EXPORT_CALL DeleteMenuItem(u32 id);
//These still exist , but are no longer given to plugins

void EXPORT_CALL SetMenuItemBitmap(u32 id,void* hbmp);
u32 EXPORT_CALL GetMenuItemStyle(u32 id);
void* EXPORT_CALL GetMenuItemBitmap(u32 id);
*/
extern MenuIDList MenuIDs;
