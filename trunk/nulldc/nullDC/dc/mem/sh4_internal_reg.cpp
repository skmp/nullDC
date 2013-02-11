/*
	Sh4 internal register routing (P4 & 'area 7')
*/
#include "types.h"
#include "sh4_internal_reg.h"

#include "dc/sh4/bsc.h"
#include "dc/sh4/ccn.h"
#include "dc/sh4/cpg.h"
#include "dc/sh4/dmac.h"
#include "dc/sh4/intc.h"
#include "dc/sh4/rtc.h"
#include "dc/sh4/sci.h"
#include "dc/sh4/scif.h"
#include "dc/sh4/tmu.h"
#include "dc/sh4/ubc.h"
#include "_vmem.h"
#include "mmu.h"

//64bytes of sq
__declspec(align(64)) u8 sq_both[64];

//i know , its because of templates :)
#pragma warning( disable : 4127 /*4244*/)

__declspec(align(32)) u8 OnChipRAM[OnChipRAM_SIZE];
//All registers are 4 byte alligned

__declspec(align(32)) RegisterStruct CCN[20];
__declspec(align(32)) RegisterStruct UBC[20];
__declspec(align(32)) RegisterStruct BSC[20];
__declspec(align(32)) RegisterStruct DMAC[20];
__declspec(align(32)) RegisterStruct CPG[20];
__declspec(align(32)) RegisterStruct RTC[20];
__declspec(align(32)) RegisterStruct INTC[20];
__declspec(align(32)) RegisterStruct TMU[20];
__declspec(align(32)) RegisterStruct SCI[20];
__declspec(align(32)) RegisterStruct SCIF[20];



//helper functions
template <u32 size>
INLINE u32 RegSRead(RegisterStruct* reg,u32 offset)
{
#ifdef TRACE
	if (offset & 3/*(size-1)*/) //4 is min allign size
	{
		EMUERROR("unallinged register read");
	}
#endif

	offset>>=2;

#ifdef TRACE
	if (reg[offset].flags& size)
	{
#endif
		if (reg[offset].flags & REG_READ_DATA )
		{
			if (size==4)
				return  *reg[offset].data32;
			else if (size==2)
				return  *reg[offset].data16;
			else 
				return  *reg[offset].data8;
		}
		else
		{
			if (reg[offset].readFunction)
				return reg[offset].readFunction();
			else
			{
				if (!(reg[offset].flags& REG_NOT_IMPL))
					EMUERROR("ERROR [readed write olny register]");
			}
		}
#ifdef TRACE
	}
	else
	{
		if (!(reg[offset].flags& REG_NOT_IMPL))
			EMUERROR("ERROR [wrong size read on register]");
	}
#endif
	if (reg[offset].flags& REG_NOT_IMPL)
		EMUERROR2("Read from internal Regs , not  implemented , offset=%x",offset);
	return 0;
}
template <u32 size>
INLINE void RegSWrite(RegisterStruct* reg,u32 offset,u32 data)
{
#ifdef TRACE
	if (offset & 3/*(size-1)*/) //4 is min allign size
	{
		EMUERROR("unallinged register write");
	}
#endif
	offset>>=2;
#ifdef TRACE
	if (reg[offset].flags & size)
	{
#endif
		if (reg[offset].flags & REG_WRITE_DATA)
		{
			if (size==4)
				*reg[offset].data32=data;
			else if (size==2)
				*reg[offset].data16=(u16)data;
			else
				*reg[offset].data8=(u8)data;

			return;
		}
		else
		{
			if (reg[offset].flags & REG_CONST)
				EMUERROR("Error [Write to read olny register , const]");
			else
			{
				if (reg[offset].writeFunction)
				{
					reg[offset].writeFunction(data);
					return;
				}
				else
				{
					if (!(reg[offset].flags& REG_NOT_IMPL))
						EMUERROR("ERROR [Write to read olny register]");
				}
			}
		}
#ifdef TRACE
	}
	else
	{
		if (!(reg[offset].flags& REG_NOT_IMPL))
			EMUERROR4("ERROR :wrong size write on register ; offset=%x , data=%x,sz=%d",offset,data,size);
	}
#endif
	if ((reg[offset].flags& REG_NOT_IMPL))
		EMUERROR3("Write to internal Regs , not  implemented , offset=%x,data=%x",offset,data);
}

//Region P4
//Read P4
template <u32 sz,class T>
T __fastcall ReadMem_P4(u32 addr)
{
	/*if (((addr>>26)&0x7)==7)
	{
	return ReadMem_area7(addr,sz);	
	}*/

	switch((addr>>24)&0xFF)
	{

	case 0xE0:
	case 0xE1:
	case 0xE2:
	case 0xE3:
		log("Unhandled p4 read [Store queue] 0x%x\n",addr);
		return (T)0;

	case 0xF0:
		//log("Unhandled p4 read [Instruction cache address array] 0x%x\n",addr);
		return (T)0;

	case 0xF1:
		//log("Unhandled p4 read [Instruction cache data array] 0x%x\n",addr);
		return (T)0;

	case 0xF2:
		//log("Unhandled p4 read [Instruction TLB address array] 0x%x\n",addr);
		{
			u32 entry=(addr>>8)&3;
			return (T)(ITLB[entry].Address.reg_data | (ITLB[entry].Data.V<<8));
		}


	case 0xF3:
		//log("Unhandled p4 read [Instruction TLB data arrays 1 and 2] 0x%x\n",addr);
		{
			u32 entry=(addr>>8)&3;
			return (T)ITLB[entry].Data.reg_data;
		}


	case 0xF4:
		{
			//int W,Set,A;
			//W=(addr>>14)&1;
			//A=(addr>>3)&1;
			//Set=(addr>>5)&0xFF;
			//log("Unhandled p4 read [Operand cache address array] %d:%d,%d  0x%x\n",Set,W,A,addr);
			return (T)0;
		}


	case 0xF5:
		//log("Unhandled p4 read [Operand cache data array] 0x%x",addr);
		return (T)0;


	case 0xF6:
		//log("Unhandled p4 read [Unified TLB address array] 0x%x\n",addr);
		{
			u32 entry=(addr>>8)&63;
			u32 rv=UTLB[entry].Address.reg_data;
			rv|=UTLB[entry].Data.D<<9;
			rv|=UTLB[entry].Data.V<<8;
			return (T)rv;
		}
		

	case 0xF7:
		//log("Unhandled p4 read [Unified TLB data arrays 1 and 2] 0x%x\n",addr);
		{
			u32 entry=(addr>>8)&63;
			return (T)UTLB[entry].Data.reg_data;
		}
	

	case 0xFF:
		log("Unhandled p4 read [area7] 0x%x\n",addr);
		break;

	default:
		log("Unhandled p4 read [Reserved] 0x%x\n",addr);
		break;
	}

	EMUERROR2("Read from P4 not implemented , addr=%x",addr);
	return (T)0;

}
bool mmu_match(u32 va,CCN_PTEH_type Address,CCN_PTEL_type Data);
//Write P4
template <u32 sz,class T>
void __fastcall WriteMem_P4(u32 addr,T data)
{
	/*if (((addr>>26)&0x7)==7)
	{
	WriteMem_area7(addr,data,sz);	
	return;
	}*/

	switch((addr>>24)&0xFF)
	{

	case 0xE0:
	case 0xE1:
	case 0xE2:
	case 0xE3:
		log("Unhandled p4 Write [Store queue] 0x%x",addr);
		break;

	case 0xF0:
		//log("Unhandled p4 Write [Instruction cache address array] 0x%x = %x\n",addr,data);
		return;
		break;

	case 0xF1:
		//log("Unhandled p4 Write [Instruction cache data array] 0x%x = %x\n",addr,data);
		return;
		break;

	case 0xF2:
		//log("Unhandled p4 Write [Instruction TLB address array] 0x%x = %x\n",addr,data);
		{
			u32 entry=(addr>>8)&3;
			ITLB[entry].Address.reg_data=data & 0xFFFFFCFF;
			ITLB[entry].Data.V=((u32)data>>8) & 1;
			ITLB_Sync(entry);
			return;
		}
		break;

	case 0xF3:
		if (addr&0x800000)
		{
			log("Unhandled p4 Write [Instruction TLB data array 2] 0x%x = %x\n",addr,data);
		}
		else
		{
			//log("Unhandled p4 Write [Instruction TLB data array 1] 0x%x = %x\n",addr,data);
			u32 entry=(addr>>8)&3;
			ITLB[entry].Data.reg_data=data;
			ITLB_Sync(entry);
			return;
		}
		break;

	case 0xF4:
		{
			//int W,Set,A;
			//W=(addr>>14)&1;
			//A=(addr>>3)&1;
			//Set=(addr>>5)&0xFF;
			//log("Unhandled p4 Write [Operand cache address array] %d:%d,%d  0x%x = %x\n",Set,W,A,addr,data);
			return;
		}
		break;

	case 0xF5:
		//log("Unhandled p4 Write [Operand cache data array] 0x%x = %x\n",addr,data);
		return;
		break;

	case 0xF6:
		{
			if (addr&0x80)
			{
				//log("Unhandled p4 Write [Unified TLB address array , Associative Write] 0x%x = %x\n",addr,data);
				CCN_PTEH_type t;
				t.reg_data=data;

				u32 va=t.VPN<<10;

				for (int i=0;i<64;i++)
				{
					if (mmu_match(va,UTLB[i].Address,UTLB[i].Data))
					{
						UTLB_SyncUnmap(i);
						UTLB[i].Data.V=((u32)data>>8)&1;
						UTLB[i].Data.D=((u32)data>>9)&1;
						UTLB_SyncMap(i);
					}
				}

				for (int i=0;i<4;i++)
				{
					if (mmu_match(va,ITLB[i].Address,ITLB[i].Data))
					{
						ITLB[i].Data.V=((u32)data>>8)&1;
						ITLB[i].Data.D=((u32)data>>9)&1;
						ITLB_Sync(i);
					}
				}
			}
			else
			{
				u32 entry=(addr>>8)&63;
				UTLB_SyncUnmap(entry);
				UTLB[entry].Address.reg_data=data & 0xFFFFFCFF;
				UTLB[entry].Data.D=((u32)data>>9)&1;
				UTLB[entry].Data.V=((u32)data>>8)&1;
				UTLB_SyncMap(entry);
			}
			return;
		}
		break;

	case 0xF7:
		if (addr&0x800000)
		{
			log("Unhandled p4 Write [Unified TLB data array 2] 0x%x = %x\n",addr,data);
		}
		else
		{
			//log("Unhandled p4 Write [Unified TLB data array 1] 0x%x = %x\n",addr,data);
			u32 entry=(addr>>8)&63;
			UTLB_SyncUnmap(entry);
			UTLB[entry].Data.reg_data=data;
			UTLB_SyncMap(entry);
			return;
		}
		break;

	case 0xFF:
		log("Unhandled p4 Write [area7] 0x%x = %x\n",addr,data);
		break;

	default:
		log("Unhandled p4 Write [Reserved] 0x%x\n",addr);
		break;
	}

	EMUERROR3("Write to P4 not implemented , addr=%x,data=%x",addr,data);
}


//***********
//Store Queue
//***********
//TODO : replace w/ mem mapped array
//Read SQ
template <u32 sz,class T>
T __fastcall ReadMem_sq(u32 addr)
{
	if (sz!=4)
	{
		log("Store Queue Error , olny 4 byte read are possible[x%X]\n",addr);
		return 0xDE;
	}

	u32 united_offset=addr & 0x3C;

	return (T)*(u32*)&sq_both[united_offset];
}


//Write SQ
template <u32 sz,class T>
void __fastcall WriteMem_sq(u32 addr,T data)
{
	if (sz!=4)
		log("Store Queue Error , olny 4 byte writes are possible[x%X=0x%X]\n",addr,data);
	//u32 offset = (addr >> 2) & 7; // 3 bits
	u32 united_offset=addr & 0x3C;

	/*if ((addr & 0x20)) // 0: SQ0, 1: SQ1
	{
	sq1_dw[offset] = data;
	}
	else
	{
	sq0_dw[offset] = data;
	}
	return;*/

	*(u32*)&sq_both[united_offset]=data;
}


//***********
//**Area  7**
//***********
//Read Area7
template <u32 sz,class T,u32 map_base>
T __fastcall ReadMem_area7(u32 addr)
{
	addr&=0x1FFFFFFF;
	switch (map_base & 0x1FFF)
	{
	case A7_REG_HASH(CCN_BASE_addr):
		if (addr<=0x1F00003C)
		{
			return (T)RegSRead<sz>(CCN,addr & 0xFF);
		}
		else
		{
			EMUERROR2("Out of range on register index . %x",addr);
		}
		break;

	case A7_REG_HASH(UBC_BASE_addr):
		if (addr<=0x1F200020)
		{
			return (T)RegSRead<sz>(UBC,addr & 0xFF);
		}
		else
		{
			EMUERROR2("Out of range on register index . %x",addr);
		}
		break;

	case A7_REG_HASH(BSC_BASE_addr):
		if (addr<=0x1F800048)
		{
			return (T)RegSRead<sz>(BSC,addr & 0xFF);
		}
		else if ((addr>=BSC_SDMR2_addr) && (addr<= 0x1F90FFFF))
		{
			//dram settings 2 / write olny
			EMUERROR("Read from write-olny registers [dram settings 2]");
		}
		else if ((addr>=BSC_SDMR3_addr) && (addr<= 0x1F94FFFF))
		{
			//dram settings 3 / write olny
			EMUERROR("Read from write-olny registers [dram settings 3]");
		}
		else
		{
			EMUERROR2("Out of range on register index . %x",addr);
		}
		break;



	case A7_REG_HASH(DMAC_BASE_addr):
		if (addr<=0x1FA00040)
		{
			return (T)RegSRead<sz>(DMAC,addr & 0xFF);
		}
		else
		{
			EMUERROR2("Out of range on register index . %x",addr);
		}
		break;

	case A7_REG_HASH(CPG_BASE_addr):
		if (addr<=0x1FC00010)
		{
			return (T)RegSRead<sz>(CPG,addr & 0xFF);
		}
		else
		{
			EMUERROR2("Out of range on register index . %x",addr);
		}
		break;

	case A7_REG_HASH(RTC_BASE_addr):
		if (addr<=0x1FC8003C)
		{
			return (T)RegSRead<sz>(RTC,addr & 0xFF);
		}
		else
		{
			EMUERROR2("Out of range on register index . %x",addr);
		}
		break;

	case A7_REG_HASH(INTC_BASE_addr):
		if (addr<=0x1FD0000C)
		{
			return (T)RegSRead<sz>(INTC,addr & 0xFF);
		}
		else
		{
			EMUERROR2("Out of range on register index . %x",addr);
		}
		break;

	case A7_REG_HASH(TMU_BASE_addr):
		if (addr<=0x1FD8002C)
		{
			return (T)RegSRead<sz>(TMU,addr & 0xFF);
		}
		else
		{
			EMUERROR2("Out of range on register index . %x",addr);
		}
		break;

	case A7_REG_HASH(SCI_BASE_addr):
		if (addr<=0x1FE0001C)
		{
			return (T)RegSRead<sz>(SCI,addr & 0xFF);
		}
		else
		{
			EMUERROR2("Out of range on register index . %x",addr);
		}
		break;

	case A7_REG_HASH(SCIF_BASE_addr):
		if (addr<=0x1FE80024)
		{
			return (T)RegSRead<sz>(SCIF,addr & 0xFF);
		}
		else
		{
			EMUERROR2("Out of range on register index . %x",addr);
		}
		break;

		//who realy cares about ht-udi ? it's not existant on dc iirc ..
	case A7_REG_HASH(UDI_BASE_addr):
		switch(addr)
		{
			//UDI SDIR 0x1FF00000 0x1FF00000 16 0xFFFF Held Held Held Pclk
		case UDI_SDIR_addr :
			break;


			//UDI SDDR 0x1FF00008 0x1FF00008 32 Held Held Held Held Pclk
		case UDI_SDDR_addr :
			break;
		}
		break;
	}


	//EMUERROR2("unknown Read from Area7 , addr=%x",addr);
	return 0;
}

//Write Area7
template <u32 sz,class T,u32 map_base>
void __fastcall WriteMem_area7(u32 addr,T data)
{
	addr&=0x1FFFFFFF;
	switch (map_base & 0x1FFF)
	{

	case A7_REG_HASH(CCN_BASE_addr):
		if (addr<=0x1F00003C)
		{
			RegSWrite<sz>(CCN,addr & 0xFF,data);
			return;
		}
		else
		{
			EMUERROR2("Out of range on register index . %x",addr);
		}
		break;

	case A7_REG_HASH(UBC_BASE_addr):
		if (addr<=0x1F200020)
		{
			RegSWrite<sz>(UBC,addr & 0xFF,data);
			return;
		}
		else
		{
			EMUERROR2("Out of range on register index . %x",addr);
		}
		break;

	case A7_REG_HASH(BSC_BASE_addr):
		if (addr<=0x1F800048)
		{
			RegSWrite<sz>(BSC,addr & 0xFF,data);
			return;
		}
		else if ((addr>=BSC_SDMR2_addr) && (addr<= 0x1F90FFFF))
		{
			//dram settings 2 / write olny
			return;//no need ?
		}
		else if ((addr>=BSC_SDMR3_addr) && (addr<= 0x1F94FFFF))
		{
			//dram settings 3 / write olny
			return;//no need ?
		}
		else
		{
			EMUERROR2("Out of range on register index . %x",addr);
		}
		break;



	case A7_REG_HASH(DMAC_BASE_addr):
		if (addr<=0x1FA00040)
		{
			RegSWrite<sz>(DMAC,addr & 0xFF,data);
			return;
		}
		else
		{
			EMUERROR2("Out of range on register index . %x",addr);
		}
		break;

	case A7_REG_HASH(CPG_BASE_addr):
		if (addr<=0x1FC00010)
		{
			RegSWrite<sz>(CPG,addr & 0xFF,data);
			return;
		}
		else
		{
			EMUERROR2("Out of range on register index . %x",addr);
		}
		break;

	case A7_REG_HASH(RTC_BASE_addr):
		if (addr<=0x1FC8003C)
		{
			RegSWrite<sz>(RTC,addr & 0xFF,data);
			return;
		}
		else
		{
			EMUERROR2("Out of range on register index . %x",addr);
		}
		break;

	case A7_REG_HASH(INTC_BASE_addr):
		if (addr<=0x1FD0000C)
		{
			RegSWrite<sz>(INTC,addr & 0xFF,data);
			return;
		}
		else
		{
			EMUERROR2("Out of range on register index . %x",addr);
		}
		break;

	case A7_REG_HASH(TMU_BASE_addr):
		if (addr<=0x1FD8002C)
		{
			RegSWrite<sz>(TMU,addr & 0xFF,data);
			return;
		}
		else
		{
			EMUERROR2("Out of range on register index . %x",addr);
		}
		break;

	case A7_REG_HASH(SCI_BASE_addr):
		if (addr<=0x1FE0001C)
		{
			RegSWrite<sz>(SCI,addr & 0xFF,data);
			return;
		}
		else
		{
			EMUERROR2("Out of range on register index . %x",addr);
		}
		break;

	case A7_REG_HASH(SCIF_BASE_addr):
		if (addr<=0x1FE80024)
		{
			RegSWrite<sz>(SCIF,addr & 0xFF,data);
			return;
		}
		else
		{
			EMUERROR2("Out of range on register index . %x",addr);
		}
		break;

		//who realy cares about ht-udi ? it's not existant on dc iirc ..
	case A7_REG_HASH(UDI_BASE_addr):
		switch(addr)
		{
			//UDI SDIR 0xFFF00000 0x1FF00000 16 0xFFFF Held Held Held Pclk
		case UDI_SDIR_addr :
			break;


			//UDI SDDR 0xFFF00008 0x1FF00008 32 Held Held Held Held Pclk
		case UDI_SDDR_addr :
			break;
		}
		break;
	}

	//EMUERROR3("Write to Area7 not implemented , addr=%x,data=%x",addr,data);
}


//***********
//On Chip Ram
//***********
//Read OCR
template <u32 sz,class T>
T __fastcall ReadMem_area7_OCR_T(u32 addr)
{
	if (CCN_CCR.ORA)
	{
		if (sz==1)
			return (T)OnChipRAM[addr&OnChipRAM_MASK];
		else if (sz==2)
			return (T)*(u16*)&OnChipRAM[addr&OnChipRAM_MASK];
		else if (sz==4)
			return (T)*(u32*)&OnChipRAM[addr&OnChipRAM_MASK];
		else
		{
			log("ReadMem_area7_OCR_T: template SZ is wrong = %d\n",sz);
			return 0xDE;
		}
	}
	else
	{
		log("On Chip Ram Read ; but OCR is disabled\n");
		return 0xDE;
	}
}

//Write OCR
template <u32 sz,class T>
void __fastcall WriteMem_area7_OCR_T(u32 addr,T data)
{
	if (CCN_CCR.ORA)
	{
		if (sz==1)
			OnChipRAM[addr&OnChipRAM_MASK]=(u8)data;
		else if (sz==2)
			*(u16*)&OnChipRAM[addr&OnChipRAM_MASK]=(u16)data;
		else if (sz==4)
			*(u32*)&OnChipRAM[addr&OnChipRAM_MASK]=data;
		else
		{
			log("WriteMem_area7_OCR_T: template SZ is wrong = %d\n",sz);
		}
	}
	else
	{
		log("On Chip Ram Write ; but OCR is disabled\n");
	}
}

//Init/Res/Term
void sh4_internal_reg_Init()
{
	for (u32 i=0;i<20;i++)
	{
		CCN[i].flags=REG_NOT_IMPL;	//(16,true);	//CCN  : 14 registers
		UBC[i].flags=REG_NOT_IMPL;	//(9,true);		//UBC  : 9 registers
		BSC[i].flags=REG_NOT_IMPL;	//(19,true);	//BSC  : 18 registers
		DMAC[i].flags=REG_NOT_IMPL;	//(17,true);	//DMAC : 17 registers
		CPG[i].flags=REG_NOT_IMPL;	//(5,true);		//CPG  : 5 registers
		RTC[i].flags=REG_NOT_IMPL;	//(16,true);	//RTC  : 16 registers
		INTC[i].flags=REG_NOT_IMPL;	//(4,true);		//INTC : 4 registers
		TMU[i].flags=REG_NOT_IMPL;	//(12,true);	//TMU  : 12 registers
		SCI[i].flags=REG_NOT_IMPL;	//(8,true);		//SCI  : 8 registers
		SCIF[i].flags=REG_NOT_IMPL;	//(10,true);	//SCIF : 10 registers
	}

	//initialise Register structs
	bsc_Init();
	ccn_Init();
	cpg_Init();
	dmac_Init();
	intc_Init();
	rtc_Init();
	sci_Init();
	scif_Init();
	tmu_Init();
	ubc_Init();
}

void sh4_internal_reg_Reset(bool Manual)
{
	for (u32 i = 0;i < OnChipRAM_SIZE;++i) {
		OnChipRAM[i] = 0;
	}

	//Reset register values
	bsc_Reset(Manual);
	ccn_Reset(Manual);
	cpg_Reset(Manual);
	dmac_Reset(Manual);
	intc_Reset(Manual);
	rtc_Reset(Manual);
	sci_Reset(Manual);
	scif_Reset(Manual);
	tmu_Reset(Manual);
	ubc_Reset(Manual);
}

void sh4_internal_reg_Term()
{
	//free any alloc'd resources [if any]
	ubc_Term();
	tmu_Term();
	scif_Term();
	sci_Term();
	rtc_Term();
	intc_Term();
	dmac_Term();
	cpg_Term();
	ccn_Term();
	bsc_Term();
}
//Mem map :)

//AREA 7	--	Sh4 Regs
_vmem_handler area7_handler;
_vmem_handler area7_handler_1F00;
_vmem_handler area7_handler_1F20;
_vmem_handler area7_handler_1F80;
_vmem_handler area7_handler_1FA0;
_vmem_handler area7_handler_1FC0;
_vmem_handler area7_handler_1FC8;
_vmem_handler area7_handler_1FD0;
_vmem_handler area7_handler_1FD8;
_vmem_handler area7_handler_1FE0;
_vmem_handler area7_handler_1FE8;
_vmem_handler area7_handler_1FF0;

_vmem_handler area7_orc_handler;

void map_area7_init()
{
	//=_vmem_register_handler(ReadMem8_area7,ReadMem16_area7,ReadMem32_area7,
	//									WriteMem8_area7,WriteMem16_area7,WriteMem32_area7);

	//default area7 handler
	area7_handler= _vmem_register_handler_Template1(ReadMem_area7,WriteMem_area7,0);

#define Make_a7_handler(base) area7_handler_##base=_vmem_register_handler_Template1(ReadMem_area7,WriteMem_area7,0x##base);
	Make_a7_handler(1F00);
	Make_a7_handler(1F20);
	Make_a7_handler(1F80);
	Make_a7_handler(1FA0);
	Make_a7_handler(1FC0);
	Make_a7_handler(1FC8);
	Make_a7_handler(1FD0);
	Make_a7_handler(1FD8);
	Make_a7_handler(1FE0);
	Make_a7_handler(1FE8);
	Make_a7_handler(1FF0);

	area7_orc_handler= _vmem_register_handler_Template(ReadMem_area7_OCR_T,WriteMem_area7_OCR_T);
}
void map_area7(u32 base)
{
	//OCR @
	//((addr>=0x7C000000) && (addr<=0x7FFFFFFF))
	if (base==0x6000)
		_vmem_map_handler(area7_orc_handler,0x1C00 | base , 0x1FFF| base);
	else
	{
		_vmem_map_handler(area7_handler,0x1C00 | base , 0x1FFF| base);

#define mmap_a7_handler(mbase) _vmem_map_handler(area7_handler_##mbase,0x##mbase | base , 0x##mbase| base);

		mmap_a7_handler(1F00);
		mmap_a7_handler(1F20);
		mmap_a7_handler(1F80);
		mmap_a7_handler(1FA0);
		mmap_a7_handler(1FC0);
		mmap_a7_handler(1FC8);
		mmap_a7_handler(1FD0);
		mmap_a7_handler(1FD8);
		mmap_a7_handler(1FE0);
		mmap_a7_handler(1FE8);
		mmap_a7_handler(1FF0);
	}
}

//P4
void map_p4()
{
	//P4 Region :
	_vmem_handler p4_handler =	_vmem_register_handler_Template(ReadMem_P4,WriteMem_P4);

	//register this before area7 and SQ , so they overwrite it and handle em :)
	//Defualt P4 handler
	//0xE0000000-0xFFFFFFFF
	_vmem_map_handler(p4_handler,0xE000,0xFFFF);

	//Store Queues	--	Write olny 32bit (well , they can be readed too btw)
	_vmem_handler sq_handler =_vmem_register_handler_Template(ReadMem_sq,WriteMem_sq);
	_vmem_map_handler(sq_handler,0xE000,0xE3FF);
	map_area7(0xE000);
}