#pragma once
#include "types.h"
#include "dc/sh4/ccn.h"

struct TLB_Entry
{
	CCN_PTEH_type Address;
	CCN_PTEL_type Data;
};

extern TLB_Entry UTLB[64];
extern TLB_Entry ITLB[4];

//sync mem mapping to mmu , suspend compiled blocks if needed.entry is a UTLB entry # , -1 is for full sync
bool UTLB_SyncMap(u32 entry);
void UTLB_SyncUnmap(u32 entry);
//sync mem mapping to mmu , suspend compiled blocks if needed.entry is a ITLB entry # , -1 is for full sync
void ITLB_Sync(u32 entry);


//Do a full lookup on the UTLB entry's
//Return Values
//Translation was sucessfull , rv contains return
#define MMU_ERROR_NONE	   0
//TLB miss
#define MMU_ERROR_TLB_MISS 1
//TLB Multyhit
#define MMU_ERROR_TLB_MHIT 2
//Mem is read/write protected (depends on translation type)
#define MMU_ERROR_PROTECTED 3
//Mem is write protected , firstwrite
#define MMU_ERROR_FIRSTWRITE 4
//data-Opcode read/write missasligned
#define MMU_ERROR_BADADDR 5
//Can't Execute
#define MMU_ERROR_EXECPROT 6

//Translation Types
//Opcode read
#define MMU_TT_IREAD 0
//Data write
#define MMU_TT_DWRITE 1
//Data write
#define MMU_TT_DREAD 2
//Do an mmu lookup for va , returns translation status , if MMU_ERROR_NONE , rv is set to translated index

extern u32 mmu_error_TT;

void MMU_Init();
void MMU_Reset(bool Manual);
void MMU_Term();

u8 __fastcall mmu_ReadMem8(u32 addr);
u16 __fastcall mmu_ReadMem16(u32 addr);
u16 __fastcall mmu_IReadMem16(u32 addr);
u32 __fastcall mmu_ReadMem32(u32 addr);
u64 __fastcall mmu_ReadMem64(u32 addr);

void __fastcall mmu_WriteMem8(u32 addr,u8 data);
void __fastcall mmu_WriteMem16(u32 addr,u16 data);
void __fastcall mmu_WriteMem32(u32 addr,u32 data);
void __fastcall mmu_WriteMem64(u32 addr,u64 data);
bool __fastcall mmu_TranslateSQW(u32& addr);