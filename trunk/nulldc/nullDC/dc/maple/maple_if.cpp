/*
	Maple host controller:
		Makes frames, routes them, etc
	
	--It's not very nicely implemented, i really love much more the (newer) ndce implementation :)
	--This code almost dates back to 2004
*/
#include "types.h"
#include <string.h>

#include "maple_if.h"

#include "config/config.h"

#include "dc/sh4/intc.h"
#include "dc/mem/sb.h"
#include "dc/mem/sh4_mem.h"
#include "plugins/plugin_manager.h"
#include "dc/sh4/rec_v1/blockmanager.h"
#include "dc/asic/asic.h"

eMDevInf MapleDevices_dd[4][6];
maple_device_instance MapleDevices[4];

void DoMapleDma();


void testJoy_GotData(u32 header1,u32 header2,u32*data,u32 datalen);

//realy hackish
//misses delay , and stop/start implementation
bool maple_ddt_pending_reset=false;
void maple_vblank()
{
	if (SB_MDEN &1)
	{
		if (SB_MDTSEL&1)
		{
			if (maple_ddt_pending_reset)
			{
				//log("DDT vblank ; reset pending\n");
			}
			else
			{
				//log("DDT vblank\n");
				DoMapleDma();
				//the periodial callback handlers raising the interrupt and stuff
				if ((SB_MSYS>>12)&1)
				{
					maple_ddt_pending_reset=true;
				}
			}
		}
		else
		{
			maple_ddt_pending_reset=false;
		}
	}
}
void maple_SB_MSHTCL_Write(u32 data)
{
	if (data&1)
		maple_ddt_pending_reset=false;
}

s32 maple_pending_dma = 0;

void maple_periodical(u32 cycl)
{
	if (maple_pending_dma > 0)
	{
		verify(SB_MDST==1);

		cycl = (maple_pending_dma <= 0) ? 0 : cycl;
		maple_pending_dma-=cycl;

		if (maple_pending_dma <= 0)
		{
			//log("%u %d\n",cycl,(s32)maple_pending_dma);
			asic_RaiseInterrupt(holly_MAPLE_DMA);
			maple_pending_dma = 0;
			SB_MDST=0;
		}
	}
}
void maple_SB_MDST_Write(u32 data)
{
	if (data & 0x1)
	{
		if (SB_MDEN &1)
		{
			DoMapleDma();
		}
	}
	//SB_MDST = 0;	//No dma in progress :)
}

bool IsOnSh4Ram(u32 addr)
{
	if (((addr>>26)&0x7)==3)
	{
		if ((((addr>>29) &0x7)!=7))
		{
			return true;
		}
	}

	return false;
}
u32 GetMaplePort(u32 addr)
{
	for (int i=0;i<6;i++)
	{
		if ((1<<i)&addr)
			return i;
	}
	return 0;
}
u32 GetConnectedDevices(u32 Port)
{
	verify(MapleDevices[Port].connected);

	u32 rv=0;
	
	if(MapleDevices[Port].subdevices[0].connected)
		rv|=0x01;
	if(MapleDevices[Port].subdevices[1].connected)
		rv|=0x02;
	if(MapleDevices[Port].subdevices[2].connected)
		rv|=0x04;
	if(MapleDevices[Port].subdevices[3].connected)
		rv|=0x08;
	if(MapleDevices[Port].subdevices[4].connected)
		rv|=0x10;

	return rv;
}
u32 dmacount=0;
void DoMapleDma()
{
	verify(SB_MDEN &1);
	verify(SB_MDST==0);
	u32 total_bytes=0;
#if debug_maple
	log("Maple :DoMapleDma\n");
#endif
	u32 addr = SB_MDSTAR & 0x1FFFFFE0;
	bool last = false;
	while (last != true)
	{
		dmacount++;
		u32 header_1 = ReadMem32_nommu(addr);
		u32 header_2 = ReadMem32_nommu(addr + 4) &0x1FFFFFE0;

		last = (header_1 >> 31) == 1;//is last transfer ?
		u32 plen = (header_1 & 0xFF )+1;//transfer lenght
		u32 device = (header_1 >> 16) & 0x3;
		u32 maple_op=(header_1>>8)&7;

		if (maple_op==0)
		{
			if (!IsOnSh4Ram(header_2))
			{
				log("MAPLE ERROR : DESTINATION NOT ON SH4 RAM 0x%X\n",header_2);
				header_2&=0xFFFFFF;
				header_2|=(3<<26);
				//goto dma_end;//a baaddd error
			}
			u32* p_out=(u32*)GetMemPtr(header_2,4);
			u32 outlen=0;

			u32* p_data =(u32*) GetMemPtr(addr + 8,(plen << 2)/**sizeof(u32)*/);
			//Command / Response code 
			//Recipient address 
			//Sender address 
			//Number of additional words in frame 
			u32 command=p_data[0] &0xFF;
			u32 reci=(p_data[0] >> 8) & 0xFF;//0-5;
			u32 subport=GetMaplePort(reci);
			//u32 wtfport=reci>>6;
			u32 send=(p_data[0] >> 16) & 0xFF;
			u32 inlen=(p_data[0]>>24) & 0xFF;
			u32 resp=0;
			inlen <<= 2;
			//device=wtfport;

			if (MapleDevices[device].connected && (subport==5 || MapleDevices[device].subdevices[subport].connected))
			{
				if(reci&0x20)							//do this here, so that the device can unplug itself after the dma
					reci|=GetConnectedDevices(device);

				//(maple_device_instance* device_instance,u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len,u32& responce);
				if (subport==5)
				{
					resp=MapleDevices[device].dma(
						MapleDevices[device].data,
						command,
						&p_data[1],
						inlen,
						&p_out[1],
						outlen);
				}
				else
				{
					resp=MapleDevices[device].subdevices[subport].dma(
						MapleDevices[device].subdevices[subport].data,
						command,
						&p_data[1],
						inlen,
						&p_out[1],
						outlen);
				}
				total_bytes+=inlen+outlen;

				#if debug_maple
					log("Maple :port%d_%d : 0x%02X -> done 0x%02X \n",device,subport,command,resp);
				#endif

				verify(resp==(u8)resp);
				verify(send==(u8)send);
				verify(reci==(u8)reci);
				verify((outlen>>2)==(u8)(outlen>>2));
				p_out[0]=(resp<<0)|(send<<8)|(reci<<16)|((outlen>>2)<<24);
				outlen+=4;
			}
			else
			{
				#if debug_maple
					log("Maple :port%d_%d : 0x%02X -> missing\n",device,subport,command);
				#endif

				outlen=4;
				p_out[0]=0xFFFFFFFF;
			}
			//NotifyMemWrite(header_2,outlen);

			//goto next command
			addr += (2 << 2) + (plen << 2);
		}
		else
		{
			addr += 1 * 4;
		}
	}
//dma_end:

	asic_RaiseInterrupt(holly_MAPLE_DMA);

	//maple_pending_dma= ((total_bytes*200000000)/262144)+1;
	//SB_MDST=1;
}

//device : 0 .. 4 -> subdevice , 5 -> main device :)
u32 GetMapleAddress(u32 port,u32 device)
{
	u32 rv=port<<6;
	rv|=1<<device;

	return rv;
}

//plugins are handled from plugin manager code from now :)
//Init registers :)
void maple_Init()
{
	sb_regs[(SB_MDST_addr-SB_BASE)>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA;
	sb_regs[(SB_MDST_addr-SB_BASE)>>2].writeFunction=maple_SB_MDST_Write;

	sb_regs[(SB_MSHTCL_addr-SB_BASE)>>2].flags=REG_32BIT_READWRITE;
	sb_regs[(SB_MSHTCL_addr-SB_BASE)>>2].writeFunction=maple_SB_MSHTCL_Write;
}

void maple_Reset(bool Manual)
{
	maple_ddt_pending_reset=false;
	SB_MDTSEL	= 0x00000000;
	SB_MDEN	= 0x00000000;
	SB_MDST	= 0x00000000;
	SB_MSYS	= 0x3A980000;
	SB_MSHTCL	= 0x00000000;
	SB_MDAPRO = 0x00007F00;
	SB_MMSEL	= 0x00000001;
}

void maple_Term()
{
	
}

