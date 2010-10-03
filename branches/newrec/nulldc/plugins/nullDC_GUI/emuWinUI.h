#pragma once
#include "nullDC_GUI.h"

u32 EXPORT_CALL AddMenuItem(u32 parent,s32 pos,const wchar* text,MenuItemSelectedFP* handler ,u32 checked);
void EXPORT_CALL DeleteAllMenuItemChilds(u32 id);
void EXPORT_CALL SetMenuItemStyle(u32 id,u32 style,u32 mask);
void EXPORT_CALL GetMenuItem(u32 id,MenuItem* info,u32 mask);
void EXPORT_CALL SetMenuItem(u32 id,MenuItem* info,u32 mask);
void EXPORT_CALL DeleteMenuItem(u32 id);
//These still exist , but are no longer given to plugins
void EXPORT_CALL SetMenuItemHandler(u32 id,MenuItemSelectedFP* h);
void EXPORT_CALL SetMenuItemBitmap(u32 id,void* hbmp);
u32 EXPORT_CALL GetMenuItemStyle(u32 id);
void* EXPORT_CALL GetMenuItemBitmap(u32 id);

bool EXPORT_CALL SelectPluginsGui();

bool uiInit();
void uiTerm();
void uiMain();

extern u32 PowerVR_menu;
extern u32 GDRom_menu;
extern u32 Aica_menu;
extern u32 Arm_menu;
extern u32 Maple_menu;
extern u32 Maple_menu_ports[4][6];
extern u32 ExtDev_menu;
extern u32 Debug_menu;