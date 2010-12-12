/*
	Mostly buggy, old, glue code that somehow still works
	Most of the work is now delegated on vtlb and only helpers are here
*/
#include "types.h"
#include <memory.h>

#include "memutil.h"
#include "sh4_mem.h"
#include "sh4_area0.h"
#include "sh4_internal_reg.h"
#include "dc/pvr/pvr_if.h"
#include "dc/sh4/sh4_registers.h"
#include "dc/dc.h"
#include "dc/sh4/rec_v1/blockmanager.h"
#include "_vmem.h"
#include "mmu.h"



//main system mem
VArray2 mem_b;

u8 MEMCALL ReadMem8_i(u32 addr);
u16 MEMCALL ReadMem16_i(u32 addr);
u32 MEMCALL ReadMem32_i(u32 addr);

void MEMCALL WriteMem8_i(u32 addr,u8 data);
void MEMCALL WriteMem16_i(u32 addr,u16 data);
void MEMCALL WriteMem32_i(u32 addr,u32 data);

void _vmem_init();
void _vmem_reset();
void _vmem_term();

//MEM MAPPINNGG

//AREA 1
_vmem_handler area1_32b;
void map_area1_init()
{
	area1_32b = _vmem_register_handler(pvr_read_area1_8,pvr_read_area1_16,pvr_read_area1_32,
									pvr_write_area1_8,pvr_write_area1_16,pvr_write_area1_32);
}

void map_area1(u32 base)
{
	//map vram
	
	//Lower 32 mb map
	//64b interface
	_vmem_map_block_mirror(vram.data,0x0400 | base,0x04FF | base,VRAM_SIZE);
	//32b interface
	_vmem_map_handler(area1_32b,0x0500 | base,0x05FF | base);
	
	//Upper 32 mb mirror
	//0x0600 to 0x07FF
	_vmem_mirror_mapping(0x0600|base,0x0400|base,0x0200);
}

//AREA 2
void map_area2_init()
{
	//nothing to map :p
}

void map_area2(u32 base)
{
	//nothing to map :p
}


//AREA 3
void map_area3_init()
{
}

void map_area3(u32 base)
{
	//32x2 or 16x4
	_vmem_map_block_mirror(mem_b.data,0x0C00 | base,0x0FFF | base,RAM_SIZE);
}

//AREA 4
void map_area4_init()
{
	
}

void map_area4(u32 base)
{
	//TODO : map later

	//upper 32mb mirror lower 32 mb
	_vmem_mirror_mapping(0x1200|base,0x1000|base,0x0200);
}


//AREA 5	--	Ext. Device
//Read Ext.Device
template <u32 sz,class T>
T __fastcall ReadMem_extdev_T(u32 addr)
{
	return (T)libExtDevice.ReadMem_A5(addr,sz);
}

//Write Ext.Device
template <u32 sz,class T>
void __fastcall WriteMem_extdev_T(u32 addr,T data)
{
	libExtDevice.WriteMem_A5(addr,data,sz);
}

_vmem_handler area5_handler;
void map_area5_init()
{
	area5_handler = _vmem_register_handler_Template(ReadMem_extdev_T,WriteMem_extdev_T);
}

void map_area5(u32 base)
{
	//map whole region to plugin handler :)
	_vmem_map_handler(area5_handler,base|0x1400,base|0x17FF);
}

//AREA 6	--	Unassigned 
void map_area6_init()
{
	//nothing to map :p
}
void map_area6(u32 base)
{
	//nothing to map :p
}


//set vmem to defualt values
void mem_map_defualt()
{
	//vmem - init/reset :)
	_vmem_init();

	
	//*TEMP*
	//setup a fallback handler , that calls old code :)
	//_vmem_handler def_handler =
	//	_vmem_register_handler(ReadMem8_i,ReadMem16_i,ReadMem32_i,WriteMem8_i,WriteMem16_i,WriteMem32_i);
	//_vmem_map_handler(def_handler,0,0xFFFF);

	//U0/P0
	//0x0xxx xxxx	-> normal memmap
	//0x2xxx xxxx	-> normal memmap
	//0x4xxx xxxx	-> normal memmap
	//0x6xxx xxxx	-> normal memmap
	//-----------
	//P1
	//0x8xxx xxxx	-> normal memmap
	//-----------
	//P2
	//0xAxxx xxxx	-> normal memmap
	//-----------
	//P3
	//0xCxxx xxxx	-> normal memmap
	//-----------
	//P4
	//0xExxx xxxx	-> internal area

	//Init Memmaps (register handlers)
	map_area0_init();
	map_area1_init();
	map_area2_init();
	map_area3_init();
	map_area4_init();
	map_area5_init();
	map_area6_init();
	map_area7_init();

	//0x0-0xD : 7 times the normal memmap mirrors :)
	//some areas can be customised :)
	for (int i=0x0;i<0xE;i+=0x2)
	{
		map_area0(i<<12);	//Bios,Flahsrom,i/f regs,Ext. Device,Sound Ram
		map_area1(i<<12);	//Vram
		map_area2(i<<12);	//Unassigned
		map_area3(i<<12);	//Ram
		map_area4(i<<12);	//TA
		map_area5(i<<12);	//Ext. Device
		map_area6(i<<12);	//Unassigned
		map_area7(i<<12);	//Sh4 Regs
	}

	//map p4 region :)
	map_p4();
}
void mem_Init()
{
	//Allocate mem for memory/bios/flash
	//mem_b.Init(&sh4_reserved_mem[0x0C000000],RAM_SIZE);

	sh4_area0_Init();
	sh4_internal_reg_Init();
	MMU_Init();
}

//Reset Sysmem/Regs -- Pvr is not changed , bios/flash are not zero'd out
void mem_Reset(bool Manual)
{
	//mem is reseted on hard restart(power on) , not manual...
	if (!Manual)
	{
		//fill mem w/ 0's
		mem_b.Zero();

		wchar* temp_path=GetEmuPath(L"data\\");
		u32 pl=(u32)wcslen(temp_path);

		#ifdef BUILD_DREAMCAST
			wcscat(temp_path,L"syscalls.bin");
			LoadFileToSh4Mem(0x00000, temp_path);
			temp_path[pl]=0;

			wcscat(temp_path,L"IP.bin");
			LoadFileToSh4Mem(0x08000, temp_path);
			temp_path[pl]=0;
		#endif

		free(temp_path);

		LoadSyscallHooks();
	}

	//Reset registers
	sh4_area0_Reset(Manual);
	sh4_internal_reg_Reset(Manual);
	MMU_Reset(Manual);
}

void mem_Term()
{
	MMU_Term();
	sh4_internal_reg_Term();
	sh4_area0_Term();

	//write back flash/sram
	wchar* temp_path=GetEmuPath(L"data\\");
	SaveRomFiles(temp_path);
	free(temp_path);
	
	//mem_b.Term(); // handled by vmem

	//vmem
	_vmem_term();
}

void MEMCALL WriteMemBlock_nommu_dma(u32 dst,u32 src,u32 size)
{
	for (u32 i=0;i<size;i+=4)
	{
		WriteMem32_nommu(dst+i,ReadMem32_nommu(src+i));
	}
}
void MEMCALL WriteMemBlock_nommu_ptr(u32 dst,u32* src,u32 size)
{
	for (u32 i=0;i<size;i+=4)
	{
		WriteMem32_nommu(dst+i,src[i>>2]);
	}
}

void MEMCALL WriteMemBlock_ptr(u32 addr,u32* data,u32 size)
{
	for (u32 i=0;i<size;i+=4)
	{
		WriteMem32(addr+i,data[i>>2]);
	}
}

//for debugger - needs to be done
bool ReadMem_DB(u32 addr,u32& data,u32 size )
{
	return false;
}
bool WriteMem_DB(u32 addr,u32 data,u32 size )
{
	return false;
}

//Get pointer to ram area , 0 if error
//For debugger(gdb) - dynarec
u8* GetMemPtr(u32 Addr,u32 size)
{
	verify((((Addr>>29) &0x7)!=7));
	switch ((Addr>>26)&0x7)
	{
		case 3:
		return &mem_b[Addr & RAM_MASK];
		
		case 0:
		case 1:
		case 2:
		case 4:
		case 5:
		case 6:
		case 7:
		default:
			log("Get MemPtr not suported area ; addr=0x%X",Addr);
			return 0;
	}
}

//Get infomation about an area , eg ram /size /anything
//For dynarec - needs to be done
void GetMemInfo(u32 addr,u32 size)
{
	//needs to be done
}

bool IsOnRam(u32 addr)
{
	if (((addr>>26)&0x7)==3)
	{
		if ((((addr>>29) &0x7)!=7) && (((addr>>29) &0x7)!=3))
		{
			return true;
		}
	}

	return false;
}

u32 __fastcall GetRamPageFromAddress(u32 RamAddress)
{
	verify(IsOnRam(RamAddress));
	return (RamAddress & RAM_MASK)/PAGE_SIZE;
}