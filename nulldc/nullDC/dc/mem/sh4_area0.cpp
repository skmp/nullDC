/*
	Dreamcast 'area 0' emulation
	Pretty much all peripheral registers are mapped here
	
	Routing is mostly handled here, as well as flash/SRAM emulation
*/
#include "types.h"
#include "sh4_mem.h"
#include "sb.h"
#include "dc/pvr/pvr_if.h"
#include "dc/gdrom/gdrom_if.h"
#include "dc/aica/aica_if.h"
#include "naomi/naomi.h"

#include "plugins/plugin_manager.h"

u32 patchRB=0;

void PatchRandB(u8* addr,int sz)
{
	if (settings.dreamcast.region==0)
	{
	u8* address=addr;

	u8* patchaddr=&address[0x1A002];
	memcpy(patchaddr,"0",1);
	patchaddr=&address[0x1A0A2];
	memcpy(patchaddr,"0",1);
	}

	else if (settings.dreamcast.region==1)
	{
	u8* address=addr;

	u8* patchaddr=&address[0x1A002];
	memcpy(patchaddr,"1",1);
	patchaddr=&address[0x1A0A2];
	memcpy(patchaddr,"1",1);
	}

	else if (settings.dreamcast.region==2)
	{
	u8* address=addr;

	u8* patchaddr=&address[0x1A002];
	memcpy(patchaddr,"2",1);
	patchaddr=&address[0x1A0A2];
	memcpy(patchaddr,"2",1);
	}

	if (settings.dreamcast.broadcast==0)
	{
	u8* address=addr;

	u8* patchaddr=&address[0x1A004];
	memcpy(patchaddr,"0",1);
	patchaddr=&address[0x1A0A4];
	memcpy(patchaddr,"0",1);
	}

	else if (settings.dreamcast.broadcast==1)
	{
	u8* address=addr;

	u8* patchaddr=&address[0x1A004];
	memcpy(patchaddr,"1",1);
	patchaddr=&address[0x1A0A4];
	memcpy(patchaddr,"1",1);
	}

	else if (settings.dreamcast.broadcast==2)
	{
	u8* address=addr;

	u8* patchaddr=&address[0x1A004];
	memcpy(patchaddr,"2",1);
	patchaddr=&address[0x1A0A4];
	memcpy(patchaddr,"2",1);
	}

	else if (settings.dreamcast.broadcast==3)
	{
	u8* address=addr;

	u8* patchaddr=&address[0x1A004];
	memcpy(patchaddr,"3",1);
	patchaddr=&address[0x1A0A4];
	memcpy(patchaddr,"3",1);
	}
}

struct MemChip
{
	u8* data;
	u32 size;
	u32 mask;

	MemChip(u32 size)
	{
		this->data=new u8[size];
		this->size=size;
		this->mask=size-1;//must be power of 2
	}
	~MemChip() { delete[] data; }

	u32 Read(u32 addr,u32 sz) 
	{ 

		if ((size==0x20000) & (patchRB==0) & (DC_PLATFORM==DC_PLATFORM_NORMAL))
		{
		PatchRandB(data,sz);
		patchRB=1;
		}

		addr&=mask;
		switch (sz)
		{
		case 1:
			return data[addr];
		case 2:
			return *(u16*)&data[addr];
		case 4:
			return *(u32*)&data[addr];
		}

		die("invalid access size");
		return 0;
	}

	bool Load(wchar* file)
	{
		FILE* f=_wfopen(file,L"rb");
		if (f)
		{
			bool rv=fread(data,1,size,f)==size;
			fclose(f);
			return rv;
		}
		return false;
	}

	void Save(wchar* file)
	{
		FILE* f=_wfopen(file,L"wb");
		if (f)
		{
			fwrite(data,1,size,f);
			fclose(f);
		}
	}

	bool Load(wchar* root,wchar* prefix,const wchar* names_ro,const wchar* title)
	{
		wchar base[512];
		wchar temp[512];
		wchar names[512];
		wcscpy(names,names_ro);
		swprintf(base,512,L"%s",root);

		wchar* curr=names;
		wchar* next;
		do
		{
			next=wcsstr(curr,L";");
			if(next) *next=0;
			if (curr[0]=='%')
			{
				swprintf(temp,512,L"%s%s%s",base,prefix,curr+1);
			}
			else
			{
				swprintf(temp,512,L"%s%s",base,curr);
			}
			
			curr=next+1;

			if (Load(temp))
			{
				wprintf(L"Loaded %s as %s\n\n",temp,title);
				return true;
			}
		} while(next);


		return false;
	}
	void Save(wchar* root,wchar* prefix,const wchar* name_ro,const wchar* title)
	{
		wchar path[512];

		swprintf(path,512,L"%s%s%s",root,prefix,name_ro);
		Save(path);

		wprintf(L"Saved %s as %s\n\n",path,title);
	}
};
struct RomChip : MemChip
{
	RomChip(u32 sz) : MemChip(sz) {}
	void Reset()
	{
		//nothing, its permanent read only ;p
	}
	void Write(u32 addr,u32 data,u32 sz)
	{
		die("Write to RomChip is not possible, addr=%x,data=%x,size=%d");
	}
};
struct SRamChip : MemChip
{
	SRamChip(u32 sz) : MemChip(sz) {}

	void Reset()
	{
		//nothing, its battery backed up storage
	}
	void Write(u32 addr,u32 val,u32 sz)
	{
		addr&=mask;
		switch (sz)
		{
		case 1:
			data[addr]=(u8)val;
			return;
		case 2:
			*(u16*)&data[addr]=(u16)val;
			return;
		case 4:
			*(u32*)&data[addr]=val;
			return;
		}

		die("invalid access size");
	}
};
struct DCFlashChip : MemChip	//i think its micronix :p
{
	DCFlashChip(u32 sz): MemChip(sz) { }

	enum FlashState
	{
		FS_CMD_AA,		//Waiting AA
		FS_CMD_55,		//Waiting 55
		FS_CMD,			//Waiting command

		FS_Erase_AA,
		FS_Erase_55,
		FS_Erase,

		FS_Write,
		

	};

	FlashState state;
	void Reset()
	{
		//reset the flash chip state
		state=FS_CMD_AA;
	}
	
	void Write(u32 addr,u32 val,u32 sz)
	{
		if (sz!=1) die("invalid access size");

		addr&=mask;
		
		switch(state)
		{
		case FS_Erase_AA:
		case FS_CMD_AA:
			{
				verify(addr==0x5555 && val==0xAA);
				state=(FlashState)(state+1);
			}
			break;

		case FS_Erase_55:
		case FS_CMD_55:
			{
				verify((addr==0xAAAA || addr==0x2AAA) && val==0x55);
				state=(FlashState)(state+1);
			}
			break;

		case FS_CMD:
			{
				switch(val)
				{
				case 0xA0:
					state=FS_Write;
					break;
				case 0x80:
					state=FS_Erase_AA;
					break;
				default:
					log("Flash write %06X %08X %d\n",addr,val,sz);
					state=FS_CMD_AA;
					die("lolwhut");
				}
			}
			break;

		case FS_Erase:
			{
				switch(val)
				{
				case 0x30:
					log("Erase Sector %08X! (%08X)\n",addr,addr&(~0x3FFF));
					memset(&data[addr&(~0x3FFF)],0xFF,0x4000);
					break;
				default:
					log("Flash write %06X %08X %d\n",addr,val,sz);
					die("erase .. what ?");
				}
				state=FS_CMD_AA;
			}
			break;

		case FS_Write:
			{
				//log("flash write\n");
				data[addr]&=val;
				state=FS_CMD_AA;
			}
			break;
		}
	}	
};

RomChip sys_rom(BIOS_SIZE);

#ifdef FLASH_SIZE
DCFlashChip sys_nvmem(FLASH_SIZE);
#endif

#ifdef BBSRAM_SIZE
SRamChip sys_nvmem(BBSRAM_SIZE);
#endif

bool LoadRomFiles(wchar* root)
{
	//char* bios_path[512];
	
	if (!sys_rom.Load(root,ROM_PREFIX,L"%boot.bin;%boot.bin.bin;%bios.bin;%bios.bin.bin" ROM_NAMES,L"bootrom"))
	{
		msgboxf(_T("Unable to find bios in \n%s\nExiting .."),MBX_ICONERROR,root);
		return false;
	}
	if (!sys_nvmem.Load(root,ROM_PREFIX,L"%nvmem.bin;%flash_wb.bin;%flash.bin;%flash.bin.bin",L"nvram"))
	{
#if NVR_OPTIONAL

			log("flash/nvmem is missing, will create new file ..");
#else
			msgboxf(_T("Unable to find flash/nvmem in \n%s\nExiting .."),MBX_ICONERROR,root);
			return false;
#endif
	}
	
	return true;
}

void SaveRomFiles(wchar* root)
{
	sys_nvmem.Save(root,ROM_PREFIX,L"nvmem.bin",L"nvmem");
}

#if (DC_PLATFORM == DC_PLATFORM_NORMAL) || (DC_PLATFORM == DC_PLATFORM_DEV_UNIT) || (DC_PLATFORM == DC_PLATFORM_NAOMI) || (DC_PLATFORM == DC_PLATFORM_NAOMI2)

u32 ReadBios(u32 addr,u32 sz) { return sys_rom.Read(addr,sz); }
void WriteBios(u32 addr,u32 data,u32 sz) { EMUERROR4("Write to  [Boot ROM] is not possible, addr=%x,data=%x,size=%d",addr,data,sz); }

u32 ReadFlash(u32 addr,u32 sz) { return sys_nvmem.Read(addr,sz); }
void WriteFlash(u32 addr,u32 data,u32 sz) { sys_nvmem.Write(addr,data,sz); }

#elif (DC_PLATFORM == DC_PLATFORM_ATOMISWAVE)
	u32 ReadFlash(u32 addr,u32 sz) { EMUERROR3("Read from [Flash ROM] is not possible, addr=%x,size=%d",addr,sz); return 0; }
	void WriteFlash(u32 addr,u32 data,u32 sz) { EMUERROR4("Write to  [Flash ROM] is not possible, addr=%x,data=%x,size=%d",addr,data,sz); }

	u32 ReadBios(u32 addr,u32 sz) 
	{ 
		if (!(addr&0x10000))	//uper 64 kb are flashrom
		{
			return sys_rom.Read(addr,sz);
		}
		else
		{
			return sys_nvmem.Read(addr,sz);
		}
	}
	void WriteBios(u32 addr,u32 data,u32 sz) 
	{ 
		if (!(addr&0x10000))	//uper 64 kb are flashrom
		{
			EMUERROR4("Write to  [Boot ROM] is not possible, addr=%x,data=%x,size=%d",addr,data,sz); 
		}
		else
		{
			sys_nvmem.Write(addr,data,sz);
		}
	}

#else
#error unknown flash
#endif

#pragma warning( disable : 4127 /*4244*/)
//Area 0 mem map
//0x00000000- 0x001FFFFF	:MPX	System/Boot ROM
//0x00200000- 0x0021FFFF	:Flash Memory
//0x00400000- 0x005F67FF	:Unassigned
//0x005F6800- 0x005F69FF	:System Control Reg.
//0x005F6C00- 0x005F6CFF	:Maple i/f Control Reg.
//0x005F7000- 0x005F70FF	:GD-ROM / NAOMI BD Reg.
//0x005F7400- 0x005F74FF	:G1 i/f Control Reg.
//0x005F7800- 0x005F78FF	:G2 i/f Control Reg.
//0x005F7C00- 0x005F7CFF	:PVR i/f Control Reg.
//0x005F8000- 0x005F9FFF	:TA / PVR Core Reg.
//0x00600000- 0x006007FF	:MODEM
//0x00600800- 0x006FFFFF	:G2 (Reserved)
//0x00700000- 0x00707FFF	:AICA- Sound Cntr. Reg.
//0x00710000- 0x0071000B	:AICA- RTC Cntr. Reg.
//0x00800000- 0x00FFFFFF	:AICA- Wave Memory
//0x01000000- 0x01FFFFFF	:Ext. Device
//0x02000000- 0x03FFFFFF*	:Image Area*	2MB

//use unified size handler for registers
//it realy makes no sense to use different size handlers on em -> especialy when we can use templates :p
template<u32 sz, class T, u32 b_start,u32 b_end>
T __fastcall ReadMem_area0(u32 addr)
{
	addr &= 0x01FFFFFF;//to get rid of non needed bits
	const u32 base_start=b_start & 0x01FF;
	const u32 base_end=b_end & 0x01FF;
	//map 0x0000 to 0x01FF to Default handler
	//mirror 0x0200 to 0x03FF , from 0x0000 to 0x03FFF
	//map 0x0000 to 0x001F
	if ((base_start<=0x001F))//	:MPX	System/Boot ROM
	{
		return (T)ReadBios(addr,sz);
	}
	//map 0x0020 to 0x0021
	else if ((base_start>= 0x0020) && (base_end<= 0x0021))		//	:Flash Memory
	{
		return (T)ReadFlash(addr&0x1FFFF,sz);
	}
	//map 0x005F to 0x005F
	else if ((base_start >=0x005F) && (base_end <=0x005F) /*&& (addr>= 0x00400000)*/ && (addr<= 0x005F67FF))		//	:Unassigned
	{
		EMUERROR2("Read from area0_32 not implemented [Unassigned], addr=%x",addr);
	}
	else if ((base_start >=0x005F) && (base_end <=0x005F) && (addr>= 0x005F7000) && (addr<= 0x005F70FF)) //	:GD-ROM
	{
		//EMUERROR3("Read from area0_32 not implemented [GD-ROM], addr=%x,size=%d",addr,sz);
#if defined(BUILD_NAOMI	) || defined(BUILD_ATOMISWAVE)
		return (T)ReadMem_naomi(addr,sz);
#else
		return (T)ReadMem_gdrom(addr,sz);
#endif
	}
	else if ((base_start >=0x005F) && (base_end <=0x005F) && (addr>= 0x005F6800) && (addr<=0x005F7CFF)) //	/*:PVR i/f Control Reg.*/ -> ALL SB registers now
	{
		//EMUERROR2("Read from area0_32 not implemented [PVR i/f Control Reg], addr=%x",addr);
		return (T)sb_ReadMem(addr,sz);
	}
	else if ((base_start >=0x005F) && (base_end <=0x005F) && (addr>= 0x005F8000) && (addr<=0x005F9FFF)) //	:TA / PVR Core Reg.
	{
		//EMUERROR2("Read from area0_32 not implemented [TA / PVR Core Reg], addr=%x",addr);
		return (T)pvr_readreg_TA(addr,sz);
	}
	//map 0x0060 to 0x0060
	else if ((base_start >=0x0060) && (base_end <=0x0060) /*&& (addr>= 0x00600000)*/ && (addr<= 0x006007FF)) //	:MODEM
	{
		return (T)libExtDevice.ReadMem_A0_006(addr,sz);
		//EMUERROR2("Read from area0_32 not implemented [MODEM], addr=%x",addr);
	}
	//map 0x0060 to 0x006F
	else if ((base_start >=0x0060) && (base_end <=0x006F) && (addr>= 0x00600800) && (addr<= 0x006FFFFF)) //	:G2 (Reserved)
	{
		EMUERROR2("Read from area0_32 not implemented [G2 (Reserved)], addr=%x",addr);
	}
	//map 0x0070 to 0x0070
	else if ((base_start >=0x0070) && (base_end <=0x0070) /*&& (addr>= 0x00700000)*/ && (addr<=0x00707FFF)) //	:AICA- Sound Cntr. Reg.
	{
		//EMUERROR2("Read from area0_32 not implemented [AICA- Sound Cntr. Reg], addr=%x",addr);
		return (T) ReadMem_aica_reg(addr,sz);//libAICA.ReadMem_aica_reg(addr,sz);
	}
	//map 0x0071 to 0x0071
	else if ((base_start >=0x0071) && (base_end <=0x0071) /*&& (addr>= 0x00710000)*/ && (addr<= 0x0071000B)) //	:AICA- RTC Cntr. Reg.
	{
		//EMUERROR2("Read from area0_32 not implemented [AICA- RTC Cntr. Reg], addr=%x",addr);
		return (T)ReadMem_aica_rtc(addr,sz);
	}
	//map 0x0080 to 0x00FF
	else if ((base_start >=0x0080) && (base_end <=0x00FF) /*&& (addr>= 0x00800000) && (addr<=0x00FFFFFF)*/) //	:AICA- Wave Memory
	{
		EMUERROR2("Read from area0_32 not implemented [AICA- Wave Memory], addr=%x",addr);
		//return (T)libAICA.ReadMem_aica_ram(addr,sz);
	}
	//map 0x0100 to 0x01FF
	else if ((base_start >=0x0100) && (base_end <=0x01FF) /*&& (addr>= 0x01000000) && (addr<= 0x01FFFFFF)*/) //	:Ext. Device
	{
	//	EMUERROR2("Read from area0_32 not implemented [Ext. Device], addr=%x",addr);
		return (T)libExtDevice.ReadMem_A0_010(addr,sz);
	}
	//rest of it ;P
	/*else 
	{
		EMUERROR2("Read from area0_32 not mapped!!! , addr=%x",addr);
	}*/
	return 0;
}

template<u32 sz, class T, u32 b_start,u32 b_end>
void  __fastcall WriteMem_area0(u32 addr,T data)
{
	addr &= 0x01FFFFFF;//to get rid of non needed bits

	const u32 base_start=b_start & 0x01FF;
	const u32 base_end=b_end & 0x01FF;

	//map 0x0000 to 0x001F
	if ((base_start >=0x0000) && (base_end <=0x001F) /*&& (addr<=0x001FFFFF)*/)//	:MPX	System/Boot ROM
	{
		//EMUERROR4("Write to  [MPX	System/Boot ROM] is not possible, addr=%x,data=%x,size=%d",addr,data,sz);
		WriteBios(addr,data,sz);		
	}
	//map 0x0020 to 0x0021
	else if ((base_start >=0x0020) && (base_end <=0x0021) /*&& (addr>= 0x00200000) && (addr<= 0x0021FFFF)*/)		//	:Flash Memory
	{
		//EMUERROR4("Write to [Flash Memory] , sz?!, addr=%x,data=%x,size=%d",addr,data,sz);
		WriteFlash(addr,data,sz);
	}
	//map 0x0040 to 0x005F -> actualy , i'l olny map 0x005F to 0x005F , b/c the rest of it is unpammed (left to defalt handler)
	//map 0x005F to 0x005F
	else if ((base_start >=0x005F) && (base_end <=0x005F) /*&& (addr>= 0x00400000) */&& (addr<= 0x005F67FF))		//	:Unassigned
	{
		EMUERROR4("Write to area0_32 not implemented [Unassigned], addr=%x,data=%x,size=%d",addr,data,sz);
	}
	else if ((base_start >=0x005F) && (base_end <=0x005F) && (addr>= 0x005F7000) && (addr<= 0x005F70FF)) //	:GD-ROM
	{
		//EMUERROR4("Write to area0_32 not implemented [GD-ROM], addr=%x,data=%x,size=%d",addr,data,sz);
#if defined(BUILD_NAOMI	) || defined(BUILD_ATOMISWAVE)
		WriteMem_naomi(addr,data,sz);
#else
		WriteMem_gdrom(addr,data,sz);
#endif
	}
	else if ((base_start >=0x005F) && (base_end <=0x005F) && (addr>= 0x005F6800) && (addr<=0x005F7CFF)) //	/*:PVR i/f Control Reg.*/ -> ALL SB registers
	{
		//EMUERROR4("Write to area0_32 not implemented [PVR i/f Control Reg], addr=%x,data=%x,size=%d",addr,data,sz);
		sb_WriteMem(addr,data,sz);
	}
	else if ((base_start >=0x005F) && (base_end <=0x005F) && (addr>= 0x005F8000) && (addr<=0x005F9FFF)) //	:TA / PVR Core Reg.
	{
		//EMUERROR4("Write to area0_32 not implemented [TA / PVR Core Reg], addr=%x,data=%x,size=%d",addr,data,sz);
		pvr_writereg_TA(addr,data,sz);
		return;
	}
	//map 0x0060 to 0x0060
	else if ((base_start >=0x0060) && (base_end <=0x0060) /*&& (addr>= 0x00600000)*/ && (addr<= 0x006007FF)) //	:MODEM
	{
		//EMUERROR4("Write to area0_32 not implemented [MODEM], addr=%x,data=%x,size=%d",addr,data,sz);
		libExtDevice.WriteMem_A0_006(addr,data,sz);
	}
	//map 0x0060 to 0x006F
	else if ((base_start >=0x0060) && (base_end <=0x006F) && (addr>= 0x00600800) && (addr<= 0x006FFFFF)) //	:G2 (Reserved)
	{
		EMUERROR4("Write to area0_32 not implemented [G2 (Reserved)], addr=%x,data=%x,size=%d",addr,data,sz);
	}
	//map 0x0070 to 0x0070
	else if ((base_start >=0x0070) && (base_end <=0x0070) /*&& (addr>= 0x00700000)*/ && (addr<=0x00707FFF)) //	:AICA- Sound Cntr. Reg.
	{
		//EMUERROR4("Write to area0_32 not implemented [AICA- Sound Cntr. Reg], addr=%x,data=%x,size=%d",addr,data,sz);
		//aica_writereg(addr,data,sz);
		//libAICA.WriteMem_aica_reg(addr,data,sz);
		WriteMem_aica_reg(addr,data,sz);
		return;
	}
	//map 0x0071 to 0x0071
	else if ((base_start >=0x0071) && (base_end <=0x0071) /*&& (addr>= 0x00710000)*/ && (addr<= 0x0071000B)) //	:AICA- RTC Cntr. Reg.
	{
		//EMUERROR4("Write to area0_32 not implemented [AICA- RTC Cntr. Reg], addr=%x,data=%x,size=%d",addr,data,sz);
		WriteMem_aica_rtc(addr,data,sz);
		return;
	}
	//map 0x0080 to 0x00FF
	else if ((base_start >=0x0080) && (base_end <=0x00FF) /*&& (addr>= 0x00800000) && (addr<=0x00FFFFFF)*/) //	:AICA- Wave Memory
	{
		EMUERROR4("Write to area0_32 not implemented [AICA- Wave Memory], addr=%x,data=%x,size=%d",addr,data,sz);
		//aica_writeram(addr,data,sz);
		//libAICA.WriteMem_aica_ram(addr,data,sz);
		return;
	}
	//map 0x0100 to 0x01FF
	else if ((base_start >=0x0100) && (base_end <=0x01FF) /*&& (addr>= 0x01000000) && (addr<= 0x01FFFFFF)*/) //	:Ext. Device
	{
		//EMUERROR4("Write to area0_32 not implemented [Ext. Device], addr=%x,data=%x,size=%d",addr,data,sz);
		libExtDevice.WriteMem_A0_010(addr,data,sz);
	}
	/*else
	{
		EMUERROR4("Write to area0_32 not mapped!!! , addr=%x,data=%x,size=%d",addr,data,sz);
	}*/
	return;
}

//Init/Res/Term
void sh4_area0_Init()
{
	sb_Init();
}

void sh4_area0_Reset(bool Manual)
{
	sb_Reset(Manual);
}

void sh4_area0_Term()
{
	sb_Term();
}


//AREA 0
_vmem_handler area0_handler_00_1F;
_vmem_handler area0_handler_20_21;
_vmem_handler area0_handler_5F_5F;
_vmem_handler area0_handler_60_60;
_vmem_handler area0_handler_61_6F;
_vmem_handler area0_handler_70_70;
_vmem_handler area0_handler_71_71;
//_vmem_handler area0_handler_80_FF;
_vmem_handler area0_handler_100_1FF;


//Different mem mapping regions for area0
//0x0000-0x001f
//0x0020-0x0021
//0x005F-0x005F
//0x0060-0x0060
//0x0061-0x006F
//0x0070-0x0070
//0x0071-0x0071
//0x0080-0x00FF
//0x0100-0x01FF
//Count : 9

void map_area0_init()
{
	//area0_handler =	_vmem_register_handler(ReadMem8_area0,ReadMem16_area0,ReadMem32_area0,
	//									  WriteMem8_area0,WriteMem16_area0,WriteMem32_area0);
	
	area0_handler_00_1F = _vmem_register_handler_Template2(ReadMem_area0,WriteMem_area0,0x0000,0x001F);
	area0_handler_20_21 = _vmem_register_handler_Template2(ReadMem_area0,WriteMem_area0,0x0020,0x0021);
	area0_handler_5F_5F = _vmem_register_handler_Template2(ReadMem_area0,WriteMem_area0,0x005F,0x005F);
	area0_handler_60_60 = _vmem_register_handler_Template2(ReadMem_area0,WriteMem_area0,0x0060,0x0060);
	area0_handler_61_6F = _vmem_register_handler_Template2(ReadMem_area0,WriteMem_area0,0x0061,0x006F);
	area0_handler_70_70 = _vmem_register_handler_Template2(ReadMem_area0,WriteMem_area0,0x0070,0x0070);
	area0_handler_71_71 = _vmem_register_handler_Template2(ReadMem_area0,WriteMem_area0,0x0071,0x0071);
	//area0_handler_80_FF = _vmem_register_handler_Template2(ReadMem_area0,WriteMem_area0,0x0080,0x00FF);
	area0_handler_100_1FF = _vmem_register_handler_Template2(ReadMem_area0,WriteMem_area0,0x0100,0x01FF);
}
void map_area0(u32 base)
{
	verify(base<0xE000);

	//Map 0x0000 to 0x01FF
	//u32 start=0x0000 | base;
	//u32 end  =start+0x01FF;
	
	//_vmem_map_handler(area0_handler,start,end);
	//0x0000-0x001f
	_vmem_map_handler(area0_handler_00_1F,0x0000|base,0x001F|base);
	//0x0020-0x0021
	_vmem_map_handler(area0_handler_20_21,0x0020|base,0x0021|base);
	//0x005F-0x005F
	_vmem_map_handler(area0_handler_5F_5F,0x005F|base,0x005F|base);
	//0x0060-0x0060
	_vmem_map_handler(area0_handler_60_60,0x0060|base,0x0060|base);
	//0x0061-0x006F
	_vmem_map_handler(area0_handler_61_6F,0x0061|base,0x006F|base);
	//0x0070-0x0070
	_vmem_map_handler(area0_handler_70_70,0x0070|base,0x0070|base);
	//0x0071-0x0071
	_vmem_map_handler(area0_handler_71_71,0x0071|base,0x0071|base);
	//0x0080-0x00FF
	//_vmem_map_handler(area0_handler_80_FF,0x0080|base,0x00FF|base);
	_vmem_map_block_mirror(aica_ram.data,0x0080|base,0x00FF|base,ARAM_SIZE);

	//0x0100-0x01FF
	_vmem_map_handler(area0_handler_100_1FF,0x0100|base,0x01FF|base);

	//0x0240 to 0x03FF mirrors 0x0040 to 0x01FF (no flashrom or bios)
	//0x0200 to 0x023F are unused
	_vmem_mirror_mapping(0x0240|base,0x0040|base,0x0200-0x40);
}
