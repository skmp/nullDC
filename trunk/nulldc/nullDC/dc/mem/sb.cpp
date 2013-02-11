/*
	System bus registers
	This doesn't implement any functionality, only routing
*/

#include "types.h"
#include "sb.h"
#include "dc/dc.h"
#include "dc/asic/asic.h"
#include "dc/pvr/pvr_sb_regs.h"
#include "dc/gdrom/gdrom_if.h"
#include "dc/maple/maple_if.h"
#include "dc/aica/aica_if.h"

#if defined(BUILD_NAOMI	) || defined(BUILD_ATOMISWAVE)
#include "../../naomi/naomi.h"
#endif

__declspec(align(32)) RegisterStruct sb_regs[0x540];


//(addr>= 0x005F6800) && (addr<=0x005F7CFF) -> 0x1500 bytes -> 0x540 possible registers , 125 actualy exist olny
//	System Control Reg.		//0x100 bytes
//	Maple i/f Control Reg.	//0x100 bytes
//	GD-ROM					//0x100 bytes
//	G1 i/f Control Reg.		//0x100 bytes
//	G2 i/f Control Reg.		//0x100 bytes
//	PVR i/f Control Reg.	//0x100 bytes
//	much empty space



//0x005F6800	SB_C2DSTAT	RW	ch2-DMA destination address 
 u32 SB_C2DSTAT = 0;
//0x005F6804	SB_C2DLEN	RW	ch2-DMA length 
 u32 SB_C2DLEN = 0;
//0x005F6808	SB_C2DST	RW	ch2-DMA start 
 u32 SB_C2DST = 0;
			
//0x005F6810	SB_SDSTAW	RW	Sort-DMA start link table address 
 u32 SB_SDSTAW = 0;
//0x005F6814	SB_SDBAAW	RW	Sort-DMA link base address 
 u32 SB_SDBAAW = 0;
//0x005F6818	SB_SDWLT	RW	Sort-DMA link address bit width 
 u32 SB_SDWLT = 0;
//0x005F681C	SB_SDLAS	RW	Sort-DMA link address shift control 
 u32 SB_SDLAS = 0;
//0x005F6820	SB_SDST	RW	Sort-DMA start 
 u32 SB_SDST = 0;
		
//0x005F6860 SB_SDDIV R(?) Sort-DMA LAT index (guess)
u32 SB_SDDIV = 0;

//0x005F6840	SB_DBREQM	RW	DBREQ# signal mask control 
 u32 SB_DBREQM = 0;
//0x005F6844	SB_BAVLWC	RW	BAVL# signal wait count 
 u32 SB_BAVLWC = 0;
//0x005F6848	SB_C2DPRYC	RW	DMA (TA/Root Bus) priority count 
 u32 SB_C2DPRYC = 0;
//0x005F684C	SB_C2DMAXL	RW	ch2-DMA maximum burst length 
 u32 SB_C2DMAXL = 0;
			
//0x005F6880	SB_TFREM	R	TA FIFO remaining amount 
 u32 SB_TFREM = 0;
//0x005F6884	SB_LMMODE0	RW	Via TA texture memory bus select 0 
 u32 SB_LMMODE0 = 0;
//0x005F6888	SB_LMMODE1	RW	Via TA texture memory bus select 1 
 u32 SB_LMMODE1 = 0;
//0x005F688C	SB_FFST	R	FIFO status 
 u32 SB_FFST;
//0x005F6890	SB_SFRES	W	System reset 
 u32 SB_SFRES = 0;
			
//0x005F689C	SB_SBREV	R	System bus revision number 
 u32 SB_SBREV = 0;
//0x005F68A0	SB_RBSPLT	RW	SH4 Root Bus split enable 
 u32 SB_RBSPLT = 0;
			
//0x005F6900	SB_ISTNRM	RW	Normal interrupt status 
 u32 SB_ISTNRM = 0;
//0x005F6904	SB_ISTEXT	R	al interrupt status 
 u32 SB_ISTEXT = 0;
//0x005F6908	SB_ISTERR	RW	Error interrupt status 
 u32 SB_ISTERR = 0;
			
//0x005F6910	SB_IML2NRM	RW	Level 2 normal interrupt mask 
 u32 SB_IML2NRM = 0;
//0x005F6914	SB_IML2EXT	RW	Level 2 al interrupt mask 
 u32 SB_IML2EXT = 0;
//0x005F6918	SB_IML2ERR	RW	Level 2 error interrupt mask 
 u32 SB_IML2ERR = 0;
			
//0x005F6920	SB_IML4NRM	RW	Level 4 normal interrupt mask 
 u32 SB_IML4NRM = 0;
//0x005F6924	SB_IML4EXT	RW	Level 4 al interrupt mask 
 u32 SB_IML4EXT = 0;
//0x005F6928	SB_IML4ERR	RW	Level 4 error interrupt mask 
 u32 SB_IML4ERR = 0;
			
//0x005F6930	SB_IML6NRM	RW	Level 6 normal interrupt mask 
 u32 SB_IML6NRM = 0;
//0x005F6934	SB_IML6EXT	RW	Level 6 al interrupt mask 
 u32 SB_IML6EXT = 0;
//0x005F6938	SB_IML6ERR	RW	Level 6 error interrupt mask 
 u32 SB_IML6ERR = 0;
			
//0x005F6940	SB_PDTNRM	RW	Normal interrupt PVR-DMA startup mask 
 u32 SB_PDTNRM = 0;
//0x005F6944	SB_PDTEXT	RW	al interrupt PVR-DMA startup mask 
 u32 SB_PDTEXT = 0;
			
//0x005F6950	SB_G2DTNRM	RW	Normal interrupt G2-DMA startup mask 
 u32 SB_G2DTNRM = 0;
//0x005F6954	SB_G2DTEXT	RW	al interrupt G2-DMA startup mask 
 u32 SB_G2DTEXT = 0;
			
//0x005F6C04	SB_MDSTAR	RW	Maple-DMA command table address 
 u32 SB_MDSTAR = 0;
			
//0x005F6C10	SB_MDTSEL	RW	Maple-DMA trigger select 
 u32 SB_MDTSEL = 0;
//0x005F6C14	SB_MDEN	RW	Maple-DMA enable 
 u32 SB_MDEN = 0;
//0x005F6C18	SB_MDST	RW	Maple-DMA start 
 u32 SB_MDST = 0;
			
//0x005F6C80	SB_MSYS	RW	Maple system control 
 u32 SB_MSYS = 0;
//0x005F6C84	SB_MST	R	Maple status 
 u32 SB_MST = 0;
//0x005F6C88	SB_MSHTCL	W	Maple-DMA hard trigger clear 
 u32 SB_MSHTCL = 0;
//0x005F6C8C	SB_MDAPRO	W	Maple-DMA address range 
 u32 SB_MDAPRO = 0;
			
//0x005F6CE8	SB_MMSEL	RW	Maple MSB selection 
 u32 SB_MMSEL = 0;
			
//0x005F6CF4	SB_MTXDAD	R	Maple Txd address counter 
 u32 SB_MTXDAD = 0;
//0x005F6CF8	SB_MRXDAD	R	Maple Rxd address counter 
 u32 SB_MRXDAD = 0;
//0x005F6CFC	SB_MRXDBD	R	Maple Rxd base address 
 u32 SB_MRXDBD = 0;





//0x005F7404	SB_GDSTAR	RW	GD-DMA start address 
 u32 SB_GDSTAR = 0;
//0x005F7408	SB_GDLEN	RW	GD-DMA length 
 u32 SB_GDLEN = 0;
//0x005F740C	SB_GDDIR	RW	GD-DMA direction 
 u32 SB_GDDIR = 0;
			
//0x005F7414	SB_GDEN	RW	GD-DMA enable 
 u32 SB_GDEN = 0;
//0x005F7418	SB_GDST	RW	GD-DMA start 
 u32 SB_GDST = 0;
			
//0x005F7480	SB_G1RRC	W	System ROM read access timing 
 u32 SB_G1RRC = 0;
//0x005F7484	SB_G1RWC	W	System ROM write access timing 
 u32 SB_G1RWC = 0;
//0x005F7488	SB_G1FRC	W	Flash ROM read access timing 
 u32 SB_G1FRC = 0;
//0x005F748C	SB_G1FWC	W	Flash ROM write access timing 
 u32 SB_G1FWC = 0;
//0x005F7490	SB_G1CRC	W	GD PIO read access timing 
 u32 SB_G1CRC = 0;
//0x005F7494	SB_G1CWC	W	GD PIO write access timing 
 u32 SB_G1CWC = 0;
			
//0x005F74A0	SB_G1GDRC	W	GD-DMA read access timing 
 u32 SB_G1GDRC = 0;
//0x005F74A4	SB_G1GDWC	W	GD-DMA write access timing 
 u32 SB_G1GDWC = 0;
			
//0x005F74B0	SB_G1SYSM	R	System mode 
 u32 SB_G1SYSM = 0;
//0x005F74B4	SB_G1CRDYC	W	G1IORDY signal control 
 u32 SB_G1CRDYC = 0;
//0x005F74B8	SB_GDAPRO	W	GD-DMA address range 
 u32 SB_GDAPRO = 0;
			
//0x005F74F4	SB_GDSTARD	R	GD-DMA address count (on Root Bus) 
 u32 SB_GDSTARD = 0;
//0x005F74F8	SB_GDLEND	R	GD-DMA transfer counter 
 u32 SB_GDLEND = 0;
			
//0x005F7800	SB_ADSTAG	RW	AICA:G2-DMA G2 start address 
 u32 SB_ADSTAG = 0;
//0x005F7804	SB_ADSTAR	RW	AICA:G2-DMA system memory start address 
 u32 SB_ADSTAR = 0;
//0x005F7808	SB_ADLEN	RW	AICA:G2-DMA length 
 u32 SB_ADLEN = 0;
//0x005F780C	SB_ADDIR	RW	AICA:G2-DMA direction 
 u32 SB_ADDIR = 0;
//0x005F7810	SB_ADTSEL	RW	AICA:G2-DMA trigger select 
 u32 SB_ADTSEL = 0;
//0x005F7814	SB_ADEN	RW	AICA:G2-DMA enable 
 u32 SB_ADEN = 0;

//0x005F7818	SB_ADST	RW	AICA:G2-DMA start 
 u32 SB_ADST = 0;
//0x005F781C	SB_ADSUSP	RW	AICA:G2-DMA suspend 
 u32 SB_ADSUSP = 0;
			
//0x005F7820	SB_E1STAG	RW	Ext1:G2-DMA G2 start address 
 u32 SB_E1STAG = 0;
//0x005F7824	SB_E1STAR	RW	Ext1:G2-DMA system memory start address 
 u32 SB_E1STAR = 0;
//0x005F7828	SB_E1LEN	RW	Ext1:G2-DMA length 
 u32 SB_E1LEN = 0;
//0x005F782C	SB_E1DIR	RW	Ext1:G2-DMA direction 
 u32 SB_E1DIR = 0;
//0x005F7830	SB_E1TSEL	RW	Ext1:G2-DMA trigger select 
 u32 SB_E1TSEL = 0;
//0x005F7834	SB_E1EN	RW	Ext1:G2-DMA enable 
 u32 SB_E1EN = 0;
//0x005F7838	SB_E1ST	RW	Ext1:G2-DMA start 
 u32 SB_E1ST = 0;
//0x005F783C	SB_E1SUSP	RW	Ext1: G2-DMA suspend 
 u32 SB_E1SUSP = 0;
			
//0x005F7840	SB_E2STAG	RW	Ext2:G2-DMA G2 start address 
 u32 SB_E2STAG = 0;
//0x005F7844	SB_E2STAR	RW	Ext2:G2-DMA system memory start address 
 u32 SB_E2STAR = 0;
//0x005F7848	SB_E2LEN	RW	Ext2:G2-DMA length 
 u32 SB_E2LEN = 0;
//0x005F784C	SB_E2DIR	RW	Ext2:G2-DMA direction 
 u32 SB_E2DIR = 0;
//0x005F7850	SB_E2TSEL	RW	Ext2:G2-DMA trigger select 
 u32 SB_E2TSEL = 0;
//0x005F7854	SB_E2EN	RW	Ext2:G2-DMA enable 
 u32 SB_E2EN = 0;
//0x005F7858	SB_E2ST	RW	Ext2:G2-DMA start 
 u32 SB_E2ST = 0;
//0x005F785C	SB_E2SUSP	RW	Ext2: G2-DMA suspend 
 u32 SB_E2SUSP = 0;
			
//0x005F7860	SB_DDSTAG	RW	Dev:G2-DMA G2 start address 
 u32 SB_DDSTAG = 0;
//0x005F7864	SB_DDSTAR	RW	Dev:G2-DMA system memory start address 
 u32 SB_DDSTAR = 0;
//0x005F7868	SB_DDLEN	RW	Dev:G2-DMA length 
 u32 SB_DDLEN = 0;
//0x005F786C	SB_DDDIR	RW	Dev:G2-DMA direction 
 u32 SB_DDDIR = 0;
//0x005F7870	SB_DDTSEL	RW	Dev:G2-DMA trigger select 
 u32 SB_DDTSEL = 0;
//0x005F7874	SB_DDEN	RW	Dev:G2-DMA enable 
 u32 SB_DDEN = 0;
//0x005F7878	SB_DDST	RW	Dev:G2-DMA start 
 u32 SB_DDST = 0;
//0x005F787C	SB_DDSUSP	RW	Dev: G2-DMA suspend 
 u32 SB_DDSUSP = 0;
			
//0x005F7880	SB_G2ID	R	G2 bus version 
 u32 SB_G2ID = 0;
			
//0x005F7890	SB_G2DSTO	RW	G2/DS timeout 
 u32 SB_G2DSTO = 0;
//0x005F7894	SB_G2TRTO	RW	G2/TR timeout 
 u32 SB_G2TRTO = 0;
//0x005F7898	SB_G2MDMTO	RW	Modem unit wait timeout 
 u32 SB_G2MDMTO = 0;
//0x005F789C	SB_G2MDMW	RW	Modem unit wait time 
 u32 SB_G2MDMW = 0;
			
//0x005F78BC	SB_G2APRO	W	G2-DMA address range 
 u32 SB_G2APRO = 0;
			
//0x005F78C0	SB_ADSTAGD	R	AICA-DMA address counter (on AICA) 
 u32 SB_ADSTAGD = 0;
//0x005F78C4	SB_ADSTARD	R	AICA-DMA address counter (on root bus) 
 u32 SB_ADSTARD = 0;
//0x005F78C8	SB_ADLEND	R	AICA-DMA transfer counter 
 u32 SB_ADLEND = 0;
			
//0x005F78D0	SB_E1STAGD	R	Ext-DMA1 address counter (on Ext) 
 u32 SB_E1STAGD = 0;
//0x005F78D4	SB_E1STARD	R	Ext-DMA1 address counter (on root bus) 
 u32 SB_E1STARD = 0;
//0x005F78D8	SB_E1LEND	R	Ext-DMA1 transfer counter 
 u32 SB_E1LEND = 0;
			
//0x005F78E0	SB_E2STAGD	R	Ext-DMA2 address counter (on Ext) 
 u32 SB_E2STAGD = 0;
//0x005F78E4	SB_E2STARD	R	Ext-DMA2 address counter (on root bus) 
 u32 SB_E2STARD = 0;
//0x005F78E8	SB_E2LEND	R	Ext-DMA2 transfer counter 
 u32 SB_E2LEND = 0;
			
//0x005F78F0	SB_DDSTAGD	R	Dev-DMA address counter (on Ext) 
 u32 SB_DDSTAGD = 0;
//0x005F78F4	SB_DDSTARD	R	Dev-DMA address counter (on root bus) 
 u32 SB_DDSTARD = 0;
//0x005F78F8	SB_DDLEND	R	Dev-DMA transfer counter 
 u32 SB_DDLEND = 0;
			
//0x005F7C00	SB_PDSTAP	RW	PVR-DMA PVR start address 
 u32 SB_PDSTAP = 0;
//0x005F7C04	SB_PDSTAR	RW	PVR-DMA system memory start address 
 u32 SB_PDSTAR = 0;
//0x005F7C08	SB_PDLEN	RW	PVR-DMA length 
 u32 SB_PDLEN = 0;
//0x005F7C0C	SB_PDDIR	RW	PVR-DMA direction 
 u32 SB_PDDIR = 0;
//0x005F7C10	SB_PDTSEL	RW	PVR-DMA trigger select 
 u32 SB_PDTSEL = 0;
//0x005F7C14	SB_PDEN	RW	PVR-DMA enable 
 u32 SB_PDEN = 0;
//0x005F7C18	SB_PDST	RW	PVR-DMA start 
 u32 SB_PDST = 0;
			
//0x005F7C80	SB_PDAPRO	W	PVR-DMA address range 
 u32 SB_PDAPRO = 0;
			
//0x005F7CF0	SB_PDSTAPD	R	PVR-DMA address counter (on Ext) 
 u32 SB_PDSTAPD = 0;
//0x005F7CF4	SB_PDSTARD	R	PVR-DMA address counter (on root bus) 
 u32 SB_PDSTARD = 0;
//0x005F7CF8	SB_PDLEND	R	PVR-DMA transfer counter 
 u32 SB_PDLEND = 0;

u32 sb_ReadMem(u32 addr,u32 sz)
{	
	u32 offset = addr-SB_BASE;
	register u32 rflags;
	RegisterStruct* reg;
#ifdef TRACE
	if (offset & 3/*(size-1)*/) //4 is min allign size
	{
		EMUERROR("unallinged System Bus register read");
	}
#endif

	offset>>=2;
	reg = &sb_regs[offset];
	rflags = reg->flags;

#ifdef TRACE
	if (rflags & sz)
	{
#endif
		#ifdef SB_MAP_UNKNOWN_REGS
		if(reg->unk)
			log("Read from unk-mapped SB reg : addr=%x\n",addr);
		#endif

		if (rflags & REG_READ_DATA )
		{
			switch(sz)
			{
				case 2:
					return *reg->data16;

				case 4:
					return *reg->data32;

				default:
					return *reg->data8;
			}
		}
		else
		{
			if (reg->readFunction)
				return reg->readFunction();
			else
			{
				if (!(rflags& REG_NOT_IMPL))
					EMUERROR("ERROR [readed write olny register]\n");
			}
		}
#ifdef TRACE
	}
	else
	{
		if (!(rflags& REG_NOT_IMPL))
			EMUERROR("ERROR [wrong size read on register]");
	}
#endif
	if ((rflags& REG_NOT_IMPL))
		EMUERROR2("Read from System Control Regs , not  implemented , addr=%x\n",addr);

	return 0;
}

void sb_WriteMem(u32 addr,u32 data,u32 sz)
{
	u32 offset = addr-SB_BASE;
	register u32 rflags;
	RegisterStruct* reg;
#ifdef TRACE
	if (offset & 3/*(size-1)*/) //4 is min allign size
	{
		EMUERROR("unallinged System bus register write");
	}
#endif

	offset>>=2;
	reg = &sb_regs[offset];
	rflags = reg->flags;

#ifdef TRACE
	if (rflags & sz)
	{
#endif
		#ifdef SB_MAP_UNKNOWN_REGS
		if(reg->unk)
			log("Write to unk-mapped SB reg : addr=%x,data=%x\n",addr,data);
		#endif

		if (rflags & REG_WRITE_DATA)
		{
			switch(sz)
			{
				case 2:
					*reg->data16=(u16)data;
				return;

				case 4:
					*reg->data32=data;
				return;

				default:
					*reg->data8=(u8)data;
				return;
			}

			return;
		}
		else
		{
			if (rflags & REG_CONST)
				EMUERROR("Error [Write to read olny register , const]\n");
			else
			{
				if (reg->writeFunction)
				{
					reg->writeFunction(data);
					return;
				}
				else
				{
					if (!(rflags & REG_NOT_IMPL))
						EMUERROR("ERROR [Write to read olny register]");
				}
			}
		}
#ifdef TRACE
	}
	else
	{
		if (!(rflags & REG_NOT_IMPL))
			EMUERROR4("ERROR :wrong size write on register ; offset=%x , data=%x,sz=%d",offset,data,sz);
	}
#endif
	if ((rflags & REG_NOT_IMPL))
		EMUERROR3("Write to System Control Regs , not  implemented , addr=%x,data=%x\n",addr,data);
}

u32 SB_FFST_rc;
u32 RegRead_SB_FFST()
{
	SB_FFST_rc++;
	if (SB_FFST_rc & 0x8)
	{
		SB_FFST^=31;
	}
	return SB_FFST;
}
void SB_SFRES_write32(u32 data)
{
	if ((u16)data==0x7611)
	{
		log("SB/HOLLY : System reset requested\n");
		if (!SoftReset_DC())
			log("SOFT RESET REQUEST FAILED\n");
	}
}

#ifdef SB_MAP_UNKNOWN_REGS
static u32 sb_unk[256] = {0};
static u32 sb_unk_ptr = 0;

void sb_implement_unk(u32 loc,u32 base,const u32 size = 4)
{
	base = (loc - base) >> 2;

	if(base >= 0x540)
	{
		log("sb_implement_unk : base(%u) out of range\n",base);
		return;
	}

	switch(size)
	{
		case 2:
			sb_regs[base].flags=REG_16BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
			sb_regs[base].data16=(u16*)(sb_unk + (sb_unk_ptr++));
		break;

		case 4:
			sb_regs[base].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
			sb_regs[base].data32=(u32*)(sb_unk + (sb_unk_ptr++));
		break;

		default:
			sb_regs[base].flags=REG_8BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
			sb_regs[base].data8=(u8*)(sb_unk + (sb_unk_ptr++));
		break;
	}

	sb_regs[base].readFunction=0;
	sb_regs[base].writeFunction=0;
	sb_regs[base].unk = 1;
}
#endif

void sb_Init()
{
	for (u32 i=0;i<0x540;i++)
	{
		sb_regs[i].flags=REG_NOT_IMPL;

		#ifdef SB_MAP_UNKNOWN_REGS
		sb_regs[i].unk = 0;
		#endif
	}
	
	#ifdef SB_MAP_UNKNOWN_REGS
	{
		//Unknown regs
		sb_unk_ptr = 0;

		sb_implement_unk(0x005f68a4,SB_BASE);
		sb_implement_unk(0x005f68ac,SB_BASE);
		sb_implement_unk(0x005f68a0,SB_BASE);
		sb_implement_unk(0x005f74a0,SB_BASE);
		sb_implement_unk(0x005f74a4,SB_BASE);
		sb_implement_unk(0x005f74a8,SB_BASE);
		sb_implement_unk(0x005f74ac,SB_BASE);
		sb_implement_unk(0x005f74b0,SB_BASE);
		sb_implement_unk(0x005f74b4,SB_BASE);
		sb_implement_unk(0x005f74e4,SB_BASE);
		sb_implement_unk(0x005f78a0,SB_BASE);
		sb_implement_unk(0x005f78a4,SB_BASE);
		sb_implement_unk(0x005f78a8,SB_BASE);
		sb_implement_unk(0x005f78ac,SB_BASE);
		sb_implement_unk(0x005f78b0,SB_BASE);
		sb_implement_unk(0x005f78b4,SB_BASE);
		sb_implement_unk(0x005f78b8,SB_BASE);

		sb_implement_unk(0x005f6b80,SB_BASE);
		sb_implement_unk(0x005f6b8c,SB_BASE);
	}
	#endif

	sb_regs[((SB_C2DSTAT_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_C2DSTAT_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_C2DSTAT_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_C2DSTAT_addr-SB_BASE))>>2].data32=&SB_C2DSTAT;


	//0x005F6804	SB_C2DLEN	RW	ch2-DMA length
	sb_regs[((SB_C2DLEN_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_C2DLEN_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_C2DLEN_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_C2DLEN_addr-SB_BASE))>>2].data32=&SB_C2DLEN;


	//0x005F6808	SB_C2DST	RW	ch2-DMA start
	sb_regs[((SB_C2DST_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_C2DST_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_C2DST_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_C2DST_addr-SB_BASE))>>2].data32=&SB_C2DST;


				
	//0x005F6810	SB_SDSTAW	RW	Sort-DMA start link table address
	sb_regs[((SB_SDSTAW_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_SDSTAW_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_SDSTAW_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_SDSTAW_addr-SB_BASE))>>2].data32=&SB_SDSTAW;


	//0x005F6814	SB_SDBAAW	RW	Sort-DMA link base address
	sb_regs[((SB_SDBAAW_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_SDBAAW_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_SDBAAW_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_SDBAAW_addr-SB_BASE))>>2].data32=&SB_SDBAAW;


	//0x005F6818	SB_SDWLT	RW	Sort-DMA link address bit width
	sb_regs[((SB_SDWLT_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_SDWLT_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_SDWLT_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_SDWLT_addr-SB_BASE))>>2].data32=&SB_SDWLT;


	//0x005F681C	SB_SDLAS	RW	Sort-DMA link address shift control
	sb_regs[((SB_SDLAS_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_SDLAS_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_SDLAS_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_SDLAS_addr-SB_BASE))>>2].data32=&SB_SDLAS;


	//0x005F6820	SB_SDST	RW	Sort-DMA start
	sb_regs[((SB_SDST_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_SDST_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_SDST_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_SDST_addr-SB_BASE))>>2].data32=&SB_SDST;

	//0x005F6820	SB_SDST	RW	Sort-DMA start
	sb_regs[((SB_SDDIV_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA;
	sb_regs[((SB_SDDIV_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_SDDIV_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_SDDIV_addr-SB_BASE))>>2].data32=&SB_SDDIV;


				
	//0x005F6840	SB_DBREQM	RW	DBREQ# signal mask control
	sb_regs[((SB_DBREQM_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_DBREQM_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_DBREQM_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_DBREQM_addr-SB_BASE))>>2].data32=&SB_DBREQM;


	//0x005F6844	SB_BAVLWC	RW	BAVL# signal wait count
	sb_regs[((SB_BAVLWC_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_BAVLWC_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_BAVLWC_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_BAVLWC_addr-SB_BASE))>>2].data32=&SB_BAVLWC;


	//0x005F6848	SB_C2DPRYC	RW	DMA (TA/Root Bus) priority count
	sb_regs[((SB_C2DPRYC_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_C2DPRYC_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_C2DPRYC_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_C2DPRYC_addr-SB_BASE))>>2].data32=&SB_C2DPRYC;


	//0x005F684C	SB_C2DMAXL	RW	ch2-DMA maximum burst length
	sb_regs[((SB_C2DMAXL_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_C2DMAXL_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_C2DMAXL_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_C2DMAXL_addr-SB_BASE))>>2].data32=&SB_C2DMAXL;


				
	//0x005F6880	SB_TFREM	R	TA FIFO remaining amount
	sb_regs[((SB_TFREM_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA ;
	sb_regs[((SB_TFREM_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_TFREM_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_TFREM_addr-SB_BASE))>>2].data32=&SB_TFREM;


	//0x005F6884	SB_LMMODE0	RW	Via TA texture memory bus select 0
	sb_regs[((SB_LMMODE0_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_LMMODE0_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_LMMODE0_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_LMMODE0_addr-SB_BASE))>>2].data32=&SB_LMMODE0;


	//0x005F6888	SB_LMMODE1	RW	Via TA texture memory bus select 1
	sb_regs[((SB_LMMODE1_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_LMMODE1_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_LMMODE1_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_LMMODE1_addr-SB_BASE))>>2].data32=&SB_LMMODE1;


	//0x005F688C	SB_FFST	R	FIFO status
	sb_regs[((SB_FFST_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE ;
	sb_regs[((SB_FFST_addr-SB_BASE))>>2].readFunction=RegRead_SB_FFST;
	sb_regs[((SB_FFST_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_FFST_addr-SB_BASE))>>2].data32=0;


	//0x005F6890	SB_SFRES	W	System reset
	sb_regs[((SB_SFRES_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE ;
	sb_regs[((SB_SFRES_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_SFRES_addr-SB_BASE))>>2].writeFunction=SB_SFRES_write32;
	sb_regs[((SB_SFRES_addr-SB_BASE))>>2].data32=&SB_SFRES;


				
	//0x005F689C	SB_SBREV	R	System bus revision number
	sb_regs[((SB_SBREV_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_CONST ;
	sb_regs[((SB_SBREV_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_SBREV_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_SBREV_addr-SB_BASE))>>2].data32=&SB_SBREV;


	//0x005F68A0	SB_RBSPLT	RW	SH4 Root Bus split enable
	sb_regs[((SB_RBSPLT_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_RBSPLT_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_RBSPLT_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_RBSPLT_addr-SB_BASE))>>2].data32=&SB_RBSPLT;


				
	//0x005F6900	SB_ISTNRM	RW	Normal interrupt status
	sb_regs[((SB_ISTNRM_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_ISTNRM_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_ISTNRM_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_ISTNRM_addr-SB_BASE))>>2].data32=&SB_ISTNRM;


	//0x005F6904	SB_ISTEXT	R	External interrupt status
	sb_regs[((SB_ISTEXT_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA ;
	sb_regs[((SB_ISTEXT_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_ISTEXT_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_ISTEXT_addr-SB_BASE))>>2].data32=&SB_ISTEXT;


	//0x005F6908	SB_ISTERR	RW	Error interrupt status
	sb_regs[((SB_ISTERR_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_ISTERR_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_ISTERR_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_ISTERR_addr-SB_BASE))>>2].data32=&SB_ISTERR;


				
	//0x005F6910	SB_IML2NRM	RW	Level 2 normal interrupt mask
	sb_regs[((SB_IML2NRM_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_IML2NRM_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_IML2NRM_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_IML2NRM_addr-SB_BASE))>>2].data32=&SB_IML2NRM;


	//0x005F6914	SB_IML2EXT	RW	Level 2 external interrupt mask
	sb_regs[((SB_IML2EXT_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_IML2EXT_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_IML2EXT_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_IML2EXT_addr-SB_BASE))>>2].data32=&SB_IML2EXT;


	//0x005F6918	SB_IML2ERR	RW	Level 2 error interrupt mask
	sb_regs[((SB_IML2ERR_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_IML2ERR_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_IML2ERR_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_IML2ERR_addr-SB_BASE))>>2].data32=&SB_IML2ERR;


				
	//0x005F6920	SB_IML4NRM	RW	Level 4 normal interrupt mask
	sb_regs[((SB_IML4NRM_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_IML4NRM_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_IML4NRM_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_IML4NRM_addr-SB_BASE))>>2].data32=&SB_IML4NRM;


	//0x005F6924	SB_IML4EXT	RW	Level 4 external interrupt mask
	sb_regs[((SB_IML4EXT_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_IML4EXT_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_IML4EXT_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_IML4EXT_addr-SB_BASE))>>2].data32=&SB_IML4EXT;


	//0x005F6928	SB_IML4ERR	RW	Level 4 error interrupt mask
	sb_regs[((SB_IML4ERR_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_IML4ERR_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_IML4ERR_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_IML4ERR_addr-SB_BASE))>>2].data32=&SB_IML4ERR;


				
	//0x005F6930	SB_IML6NRM	RW	Level 6 normal interrupt mask
	sb_regs[((SB_IML6NRM_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_IML6NRM_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_IML6NRM_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_IML6NRM_addr-SB_BASE))>>2].data32=&SB_IML6NRM;


	//0x005F6934	SB_IML6EXT	RW	Level 6 external interrupt mask
	sb_regs[((SB_IML6EXT_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_IML6EXT_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_IML6EXT_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_IML6EXT_addr-SB_BASE))>>2].data32=&SB_IML6EXT;


	//0x005F6938	SB_IML6ERR	RW	Level 6 error interrupt mask
	sb_regs[((SB_IML6ERR_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_IML6ERR_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_IML6ERR_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_IML6ERR_addr-SB_BASE))>>2].data32=&SB_IML6ERR;


				
	//0x005F6940	SB_PDTNRM	RW	Normal interrupt PVR-DMA startup mask
	sb_regs[((SB_PDTNRM_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_PDTNRM_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_PDTNRM_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_PDTNRM_addr-SB_BASE))>>2].data32=&SB_PDTNRM;


	//0x005F6944	SB_PDTEXT	RW	External interrupt PVR-DMA startup mask
	sb_regs[((SB_PDTEXT_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_PDTEXT_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_PDTEXT_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_PDTEXT_addr-SB_BASE))>>2].data32=&SB_PDTEXT;


				
	//0x005F6950	SB_G2DTNRM	RW	Normal interrupt G2-DMA startup mask
	sb_regs[((SB_G2DTNRM_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_G2DTNRM_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_G2DTNRM_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_G2DTNRM_addr-SB_BASE))>>2].data32=&SB_G2DTNRM;


	//0x005F6954	SB_G2DTEXT	RW	External interrupt G2-DMA startup mask
	sb_regs[((SB_G2DTEXT_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_G2DTEXT_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_G2DTEXT_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_G2DTEXT_addr-SB_BASE))>>2].data32=&SB_G2DTEXT;



	//0x005F6C04	SB_MDSTAR	RW	Maple-DMA command table address
	sb_regs[((SB_MDSTAR_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_MDSTAR_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_MDSTAR_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_MDSTAR_addr-SB_BASE))>>2].data32=&SB_MDSTAR;


				
	//0x005F6C10	SB_MDTSEL	RW	Maple-DMA trigger select
	sb_regs[((SB_MDTSEL_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_MDTSEL_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_MDTSEL_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_MDTSEL_addr-SB_BASE))>>2].data32=&SB_MDTSEL;


	//0x005F6C14	SB_MDEN	RW	Maple-DMA enable
	sb_regs[((SB_MDEN_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_MDEN_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_MDEN_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_MDEN_addr-SB_BASE))>>2].data32=&SB_MDEN;


	//0x005F6C18	SB_MDST	RW	Maple-DMA start
	sb_regs[((SB_MDST_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_MDST_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_MDST_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_MDST_addr-SB_BASE))>>2].data32=&SB_MDST;


				
	//0x005F6C80	SB_MSYS	RW	Maple system control
	sb_regs[((SB_MSYS_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_MSYS_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_MSYS_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_MSYS_addr-SB_BASE))>>2].data32=&SB_MSYS;


	//0x005F6C84	SB_MST	R	Maple status
	sb_regs[((SB_MST_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA ;
	sb_regs[((SB_MST_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_MST_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_MST_addr-SB_BASE))>>2].data32=&SB_MST;


	//0x005F6C88	SB_MSHTCL	W	Maple-DMA hard trigger clear
	sb_regs[((SB_MSHTCL_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_WRITE_DATA ;
	sb_regs[((SB_MSHTCL_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_MSHTCL_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_MSHTCL_addr-SB_BASE))>>2].data32=&SB_MSHTCL;


	//0x005F6C8C	SB_MDAPRO	W	Maple-DMA address range
	sb_regs[((SB_MDAPRO_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_WRITE_DATA ;
	sb_regs[((SB_MDAPRO_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_MDAPRO_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_MDAPRO_addr-SB_BASE))>>2].data32=&SB_MDAPRO;


				
	//0x005F6CE8	SB_MMSEL	RW	Maple MSB selection
	sb_regs[((SB_MMSEL_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_MMSEL_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_MMSEL_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_MMSEL_addr-SB_BASE))>>2].data32=&SB_MMSEL;


				
	//0x005F6CF4	SB_MTXDAD	R	Maple Txd address counter
	sb_regs[((SB_MTXDAD_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA ;
	sb_regs[((SB_MTXDAD_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_MTXDAD_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_MTXDAD_addr-SB_BASE))>>2].data32=&SB_MTXDAD;


	//0x005F6CF8	SB_MRXDAD	R	Maple Rxd address counter
	sb_regs[((SB_MRXDAD_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA ;
	sb_regs[((SB_MRXDAD_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_MRXDAD_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_MRXDAD_addr-SB_BASE))>>2].data32=&SB_MRXDAD;


	//0x005F6CFC	SB_MRXDBD	R	Maple Rxd base address
	sb_regs[((SB_MRXDBD_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA ;
	sb_regs[((SB_MRXDBD_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_MRXDBD_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_MRXDBD_addr-SB_BASE))>>2].data32=&SB_MRXDBD;
				
	//0x005F7404	SB_GDSTAR	RW	GD-DMA start address
	sb_regs[((SB_GDSTAR_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_GDSTAR_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_GDSTAR_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_GDSTAR_addr-SB_BASE))>>2].data32=&SB_GDSTAR;


	//0x005F7408	SB_GDLEN	RW	GD-DMA length
	sb_regs[((SB_GDLEN_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_GDLEN_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_GDLEN_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_GDLEN_addr-SB_BASE))>>2].data32=&SB_GDLEN;


	//0x005F740C	SB_GDDIR	RW	GD-DMA direction
	sb_regs[((SB_GDDIR_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_GDDIR_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_GDDIR_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_GDDIR_addr-SB_BASE))>>2].data32=&SB_GDDIR;


				
	//0x005F7414	SB_GDEN	RW	GD-DMA enable
	sb_regs[((SB_GDEN_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_GDEN_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_GDEN_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_GDEN_addr-SB_BASE))>>2].data32=&SB_GDEN;


	//0x005F7418	SB_GDST	RW	GD-DMA start
	sb_regs[((SB_GDST_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_GDST_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_GDST_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_GDST_addr-SB_BASE))>>2].data32=&SB_GDST;


				
	//0x005F7480	SB_G1RRC	W	System ROM read access timing
	sb_regs[((SB_G1RRC_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_WRITE_DATA ;
	sb_regs[((SB_G1RRC_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_G1RRC_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_G1RRC_addr-SB_BASE))>>2].data32=&SB_G1RRC;


	//0x005F7484	SB_G1RWC	W	System ROM write access timing
	sb_regs[((SB_G1RWC_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_WRITE_DATA ;
	sb_regs[((SB_G1RWC_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_G1RWC_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_G1RWC_addr-SB_BASE))>>2].data32=&SB_G1RWC;


	//0x005F7488	SB_G1FRC	W	Flash ROM read access timing
	sb_regs[((SB_G1FRC_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_WRITE_DATA ;
	sb_regs[((SB_G1FRC_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_G1FRC_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_G1FRC_addr-SB_BASE))>>2].data32=&SB_G1FRC;


	//0x005F748C	SB_G1FWC	W	Flash ROM write access timing
	sb_regs[((SB_G1FWC_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_WRITE_DATA ;
	sb_regs[((SB_G1FWC_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_G1FWC_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_G1FWC_addr-SB_BASE))>>2].data32=&SB_G1FWC;


	//0x005F7490	SB_G1CRC	W	GD PIO read access timing
	sb_regs[((SB_G1CRC_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_WRITE_DATA ;
	sb_regs[((SB_G1CRC_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_G1CRC_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_G1CRC_addr-SB_BASE))>>2].data32=&SB_G1CRC;


	//0x005F7494	SB_G1CWC	W	GD PIO write access timing
	sb_regs[((SB_G1CWC_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_WRITE_DATA ;
	sb_regs[((SB_G1CWC_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_G1CWC_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_G1CWC_addr-SB_BASE))>>2].data32=&SB_G1CWC;


				
	//0x005F74A0	SB_G1GDRC	W	GD-DMA read access timing
	sb_regs[((SB_G1GDRC_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_WRITE_DATA ;
	sb_regs[((SB_G1GDRC_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_G1GDRC_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_G1GDRC_addr-SB_BASE))>>2].data32=&SB_G1GDRC;


	//0x005F74A4	SB_G1GDWC	W	GD-DMA write access timing
	sb_regs[((SB_G1GDWC_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_WRITE_DATA ;
	sb_regs[((SB_G1GDWC_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_G1GDWC_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_G1GDWC_addr-SB_BASE))>>2].data32=&SB_G1GDWC;


				
	//0x005F74B0	SB_G1SYSM	R	System mode
	sb_regs[((SB_G1SYSM_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA ;
	sb_regs[((SB_G1SYSM_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_G1SYSM_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_G1SYSM_addr-SB_BASE))>>2].data32=&SB_G1SYSM;


	//0x005F74B4	SB_G1CRDYC	W	G1IORDY signal control
	sb_regs[((SB_G1CRDYC_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_WRITE_DATA ;
	sb_regs[((SB_G1CRDYC_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_G1CRDYC_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_G1CRDYC_addr-SB_BASE))>>2].data32=&SB_G1CRDYC;


	//0x005F74B8	SB_GDAPRO	W	GD-DMA address range
	sb_regs[((SB_GDAPRO_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_WRITE_DATA ;
	sb_regs[((SB_GDAPRO_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_GDAPRO_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_GDAPRO_addr-SB_BASE))>>2].data32=&SB_GDAPRO;


				
	//0x005F74F4	SB_GDSTARD	R	GD-DMA address count (on Root Bus)
	sb_regs[((SB_GDSTARD_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA ;
	sb_regs[((SB_GDSTARD_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_GDSTARD_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_GDSTARD_addr-SB_BASE))>>2].data32=&SB_GDSTARD;


	//0x005F74F8	SB_GDLEND	R	GD-DMA transfer counter
	sb_regs[((SB_GDLEND_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA ;
	sb_regs[((SB_GDLEND_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_GDLEND_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_GDLEND_addr-SB_BASE))>>2].data32=&SB_GDLEND;


				
	//0x005F7800	SB_ADSTAG	RW	AICA:G2-DMA G2 start address
	sb_regs[((SB_ADSTAG_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_ADSTAG_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_ADSTAG_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_ADSTAG_addr-SB_BASE))>>2].data32=&SB_ADSTAG;


	//0x005F7804	SB_ADSTAR	RW	AICA:G2-DMA system memory start address
	sb_regs[((SB_ADSTAR_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_ADSTAR_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_ADSTAR_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_ADSTAR_addr-SB_BASE))>>2].data32=&SB_ADSTAR;


	//0x005F7808	SB_ADLEN	RW	AICA:G2-DMA length
	sb_regs[((SB_ADLEN_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_ADLEN_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_ADLEN_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_ADLEN_addr-SB_BASE))>>2].data32=&SB_ADLEN;


	//0x005F780C	SB_ADDIR	RW	AICA:G2-DMA direction
	sb_regs[((SB_ADDIR_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_ADDIR_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_ADDIR_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_ADDIR_addr-SB_BASE))>>2].data32=&SB_ADDIR;


	//0x005F7810	SB_ADTSEL	RW	AICA:G2-DMA trigger select
	sb_regs[((SB_ADTSEL_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_ADTSEL_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_ADTSEL_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_ADTSEL_addr-SB_BASE))>>2].data32=&SB_ADTSEL;


	//0x005F7814	SB_ADEN	RW	AICA:G2-DMA enable
	sb_regs[((SB_ADEN_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_ADEN_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_ADEN_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_ADEN_addr-SB_BASE))>>2].data32=&SB_ADEN;



	//0x005F7818	SB_ADST	RW	AICA:G2-DMA start
	sb_regs[((SB_ADST_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_ADST_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_ADST_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_ADST_addr-SB_BASE))>>2].data32=&SB_ADST;


	//0x005F781C	SB_ADSUSP	RW	AICA:G2-DMA suspend
	sb_regs[((SB_ADSUSP_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_ADSUSP_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_ADSUSP_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_ADSUSP_addr-SB_BASE))>>2].data32=&SB_ADSUSP;


				
	//0x005F7820	SB_E1STAG	RW	Ext1:G2-DMA G2 start address
	sb_regs[((SB_E1STAG_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_E1STAG_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_E1STAG_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_E1STAG_addr-SB_BASE))>>2].data32=&SB_E1STAG;


	//0x005F7824	SB_E1STAR	RW	Ext1:G2-DMA system memory start address
	sb_regs[((SB_E1STAR_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_E1STAR_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_E1STAR_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_E1STAR_addr-SB_BASE))>>2].data32=&SB_E1STAR;


	//0x005F7828	SB_E1LEN	RW	Ext1:G2-DMA length
	sb_regs[((SB_E1LEN_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_E1LEN_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_E1LEN_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_E1LEN_addr-SB_BASE))>>2].data32=&SB_E1LEN;


	//0x005F782C	SB_E1DIR	RW	Ext1:G2-DMA direction
	sb_regs[((SB_E1DIR_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_E1DIR_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_E1DIR_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_E1DIR_addr-SB_BASE))>>2].data32=&SB_E1DIR;


	//0x005F7830	SB_E1TSEL	RW	Ext1:G2-DMA trigger select
	sb_regs[((SB_E1TSEL_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_E1TSEL_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_E1TSEL_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_E1TSEL_addr-SB_BASE))>>2].data32=&SB_E1TSEL;


	//0x005F7834	SB_E1EN	RW	Ext1:G2-DMA enable
	sb_regs[((SB_E1EN_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_E1EN_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_E1EN_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_E1EN_addr-SB_BASE))>>2].data32=&SB_E1EN;


	//0x005F7838	SB_E1ST	RW	Ext1:G2-DMA start
	sb_regs[((SB_E1ST_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_E1ST_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_E1ST_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_E1ST_addr-SB_BASE))>>2].data32=&SB_E1ST;


	//0x005F783C	SB_E1SUSP	RW	Ext1: G2-DMA suspend
	sb_regs[((SB_E1SUSP_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_E1SUSP_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_E1SUSP_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_E1SUSP_addr-SB_BASE))>>2].data32=&SB_E1SUSP;


				
	//0x005F7840	SB_E2STAG	RW	Ext2:G2-DMA G2 start address
	sb_regs[((SB_E2STAG_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_E2STAG_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_E2STAG_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_E2STAG_addr-SB_BASE))>>2].data32=&SB_E2STAG;


	//0x005F7844	SB_E2STAR	RW	Ext2:G2-DMA system memory start address
	sb_regs[((SB_E2STAR_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_E2STAR_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_E2STAR_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_E2STAR_addr-SB_BASE))>>2].data32=&SB_E2STAR;


	//0x005F7848	SB_E2LEN	RW	Ext2:G2-DMA length
	sb_regs[((SB_E2LEN_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_E2LEN_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_E2LEN_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_E2LEN_addr-SB_BASE))>>2].data32=&SB_E2LEN;


	//0x005F784C	SB_E2DIR	RW	Ext2:G2-DMA direction
	sb_regs[((SB_E2DIR_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_E2DIR_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_E2DIR_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_E2DIR_addr-SB_BASE))>>2].data32=&SB_E2DIR;


	//0x005F7850	SB_E2TSEL	RW	Ext2:G2-DMA trigger select
	sb_regs[((SB_E2TSEL_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_E2TSEL_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_E2TSEL_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_E2TSEL_addr-SB_BASE))>>2].data32=&SB_E2TSEL;


	//0x005F7854	SB_E2EN	RW	Ext2:G2-DMA enable
	sb_regs[((SB_E2EN_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_E2EN_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_E2EN_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_E2EN_addr-SB_BASE))>>2].data32=&SB_E2EN;


	//0x005F7858	SB_E2ST	RW	Ext2:G2-DMA start
	sb_regs[((SB_E2ST_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_E2ST_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_E2ST_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_E2ST_addr-SB_BASE))>>2].data32=&SB_E2ST;


	//0x005F785C	SB_E2SUSP	RW	Ext2: G2-DMA suspend
	sb_regs[((SB_E2SUSP_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_E2SUSP_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_E2SUSP_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_E2SUSP_addr-SB_BASE))>>2].data32=&SB_E2SUSP;


				
	//0x005F7860	SB_DDSTAG	RW	Dev:G2-DMA G2 start address
	sb_regs[((SB_DDSTAG_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_DDSTAG_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_DDSTAG_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_DDSTAG_addr-SB_BASE))>>2].data32=&SB_DDSTAG;


	//0x005F7864	SB_DDSTAR	RW	Dev:G2-DMA system memory start address
	sb_regs[((SB_DDSTAR_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_DDSTAR_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_DDSTAR_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_DDSTAR_addr-SB_BASE))>>2].data32=&SB_DDSTAR;


	//0x005F7868	SB_DDLEN	RW	Dev:G2-DMA length
	sb_regs[((SB_DDLEN_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_DDLEN_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_DDLEN_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_DDLEN_addr-SB_BASE))>>2].data32=&SB_DDLEN;


	//0x005F786C	SB_DDDIR	RW	Dev:G2-DMA direction
	sb_regs[((SB_DDDIR_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_DDDIR_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_DDDIR_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_DDDIR_addr-SB_BASE))>>2].data32=&SB_DDDIR;


	//0x005F7870	SB_DDTSEL	RW	Dev:G2-DMA trigger select
	sb_regs[((SB_DDTSEL_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_DDTSEL_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_DDTSEL_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_DDTSEL_addr-SB_BASE))>>2].data32=&SB_DDTSEL;


	//0x005F7874	SB_DDEN	RW	Dev:G2-DMA enable
	sb_regs[((SB_DDEN_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_DDEN_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_DDEN_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_DDEN_addr-SB_BASE))>>2].data32=&SB_DDEN;


	//0x005F7878	SB_DDST	RW	Dev:G2-DMA start
	sb_regs[((SB_DDST_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_DDST_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_DDST_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_DDST_addr-SB_BASE))>>2].data32=&SB_DDST;


	//0x005F787C	SB_DDSUSP	RW	Dev: G2-DMA suspend
	sb_regs[((SB_DDSUSP_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_DDSUSP_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_DDSUSP_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_DDSUSP_addr-SB_BASE))>>2].data32=&SB_DDSUSP;


				
	//0x005F7880	SB_G2ID	R	G2 bus version
	sb_regs[((SB_G2ID_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_CONST;
	sb_regs[((SB_G2ID_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_G2ID_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_G2ID_addr-SB_BASE))>>2].data32=&SB_G2ID;


				
	//0x005F7890	SB_G2DSTO	RW	G2/DS timeout
	sb_regs[((SB_G2DSTO_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_G2DSTO_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_G2DSTO_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_G2DSTO_addr-SB_BASE))>>2].data32=&SB_G2DSTO;


	//0x005F7894	SB_G2TRTO	RW	G2/TR timeout
	sb_regs[((SB_G2TRTO_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_G2TRTO_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_G2TRTO_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_G2TRTO_addr-SB_BASE))>>2].data32=&SB_G2TRTO;


	//0x005F7898	SB_G2MDMTO	RW	Modem unit wait timeout
	sb_regs[((SB_G2MDMTO_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_G2MDMTO_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_G2MDMTO_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_G2MDMTO_addr-SB_BASE))>>2].data32=&SB_G2MDMTO;


	//0x005F789C	SB_G2MDMW	RW	Modem unit wait time
	sb_regs[((SB_G2MDMW_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_G2MDMW_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_G2MDMW_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_G2MDMW_addr-SB_BASE))>>2].data32=&SB_G2MDMW;


				
	//0x005F78BC	SB_G2APRO	W	G2-DMA address range
	sb_regs[((SB_G2APRO_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_WRITE_DATA ;
	sb_regs[((SB_G2APRO_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_G2APRO_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_G2APRO_addr-SB_BASE))>>2].data32=&SB_G2APRO;


				
	//0x005F78C0	SB_ADSTAGD	R	AICA-DMA address counter (on AICA)
	sb_regs[((SB_ADSTAGD_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA ;
	sb_regs[((SB_ADSTAGD_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_ADSTAGD_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_ADSTAGD_addr-SB_BASE))>>2].data32=&SB_ADSTAGD;


	//0x005F78C4	SB_ADSTARD	R	AICA-DMA address counter (on root bus)
	sb_regs[((SB_ADSTARD_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA ;
	sb_regs[((SB_ADSTARD_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_ADSTARD_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_ADSTARD_addr-SB_BASE))>>2].data32=&SB_ADSTARD;


	//0x005F78C8	SB_ADLEND	R	AICA-DMA transfer counter
	sb_regs[((SB_ADLEND_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA ;
	sb_regs[((SB_ADLEND_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_ADLEND_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_ADLEND_addr-SB_BASE))>>2].data32=&SB_ADLEND;


				
	//0x005F78D0	SB_E1STAGD	R	Ext-DMA1 address counter (on Ext)
	sb_regs[((SB_E1STAGD_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA ;
	sb_regs[((SB_E1STAGD_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_E1STAGD_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_E1STAGD_addr-SB_BASE))>>2].data32=&SB_E1STAGD;


	//0x005F78D4	SB_E1STARD	R	Ext-DMA1 address counter (on root bus)
	sb_regs[((SB_E1STARD_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA ;
	sb_regs[((SB_E1STARD_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_E1STARD_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_E1STARD_addr-SB_BASE))>>2].data32=&SB_E1STARD;


	//0x005F78D8	SB_E1LEND	R	Ext-DMA1 transfer counter
	sb_regs[((SB_E1LEND_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA ;
	sb_regs[((SB_E1LEND_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_E1LEND_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_E1LEND_addr-SB_BASE))>>2].data32=&SB_E1LEND;


				
	//0x005F78E0	SB_E2STAGD	R	Ext-DMA2 address counter (on Ext)
	sb_regs[((SB_E2STAGD_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA ;
	sb_regs[((SB_E2STAGD_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_E2STAGD_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_E2STAGD_addr-SB_BASE))>>2].data32=&SB_E2STAGD;


	//0x005F78E4	SB_E2STARD	R	Ext-DMA2 address counter (on root bus)
	sb_regs[((SB_E2STARD_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA ;
	sb_regs[((SB_E2STARD_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_E2STARD_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_E2STARD_addr-SB_BASE))>>2].data32=&SB_E2STARD;


	//0x005F78E8	SB_E2LEND	R	Ext-DMA2 transfer counter
	sb_regs[((SB_E2LEND_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA ;
	sb_regs[((SB_E2LEND_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_E2LEND_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_E2LEND_addr-SB_BASE))>>2].data32=&SB_E2LEND;


				
	//0x005F78F0	SB_DDSTAGD	R	Dev-DMA address counter (on Ext)
	sb_regs[((SB_DDSTAGD_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA ;
	sb_regs[((SB_DDSTAGD_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_DDSTAGD_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_DDSTAGD_addr-SB_BASE))>>2].data32=&SB_DDSTAGD;


	//0x005F78F4	SB_DDSTARD	R	Dev-DMA address counter (on root bus)
	sb_regs[((SB_DDSTARD_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA ;
	sb_regs[((SB_DDSTARD_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_DDSTARD_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_DDSTARD_addr-SB_BASE))>>2].data32=&SB_DDSTARD;


	//0x005F78F8	SB_DDLEND	R	Dev-DMA transfer counter
	sb_regs[((SB_DDLEND_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA ;
	sb_regs[((SB_DDLEND_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_DDLEND_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_DDLEND_addr-SB_BASE))>>2].data32=&SB_DDLEND;


				
	//0x005F7C00	SB_PDSTAP	RW	PVR-DMA PVR start address
	sb_regs[((SB_PDSTAP_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_PDSTAP_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_PDSTAP_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_PDSTAP_addr-SB_BASE))>>2].data32=&SB_PDSTAP;


	//0x005F7C04	SB_PDSTAR	RW	PVR-DMA system memory start address
	sb_regs[((SB_PDSTAR_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_PDSTAR_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_PDSTAR_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_PDSTAR_addr-SB_BASE))>>2].data32=&SB_PDSTAR;


	//0x005F7C08	SB_PDLEN	RW	PVR-DMA length
	sb_regs[((SB_PDLEN_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_PDLEN_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_PDLEN_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_PDLEN_addr-SB_BASE))>>2].data32=&SB_PDLEN;


	//0x005F7C0C	SB_PDDIR	RW	PVR-DMA direction
	sb_regs[((SB_PDDIR_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_PDDIR_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_PDDIR_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_PDDIR_addr-SB_BASE))>>2].data32=&SB_PDDIR;


	//0x005F7C10	SB_PDTSEL	RW	PVR-DMA trigger select
	sb_regs[((SB_PDTSEL_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_PDTSEL_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_PDTSEL_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_PDTSEL_addr-SB_BASE))>>2].data32=&SB_PDTSEL;


	//0x005F7C14	SB_PDEN	RW	PVR-DMA enable
	sb_regs[((SB_PDEN_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_PDEN_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_PDEN_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_PDEN_addr-SB_BASE))>>2].data32=&SB_PDEN;


	//0x005F7C18	SB_PDST	RW	PVR-DMA start
	sb_regs[((SB_PDST_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((SB_PDST_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_PDST_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_PDST_addr-SB_BASE))>>2].data32=&SB_PDST;


				
	//0x005F7C80	SB_PDAPRO	W	PVR-DMA address range
	sb_regs[((SB_PDAPRO_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_WRITE_DATA ;
	sb_regs[((SB_PDAPRO_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_PDAPRO_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_PDAPRO_addr-SB_BASE))>>2].data32=&SB_PDAPRO;


				
	//0x005F7CF0	SB_PDSTAPD	R	PVR-DMA address counter (on Ext)
	sb_regs[((SB_PDSTAPD_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA ;
	sb_regs[((SB_PDSTAPD_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_PDSTAPD_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_PDSTAPD_addr-SB_BASE))>>2].data32=&SB_PDSTAPD;


	//0x005F7CF4	SB_PDSTARD	R	PVR-DMA address counter (on root bus)
	sb_regs[((SB_PDSTARD_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA ;
	sb_regs[((SB_PDSTARD_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_PDSTARD_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_PDSTARD_addr-SB_BASE))>>2].data32=&SB_PDSTARD;


	//0x005F7CF8	SB_PDLEND	R	PVR-DMA transfer counter
	sb_regs[((SB_PDLEND_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA ;
	sb_regs[((SB_PDLEND_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_PDLEND_addr-SB_BASE))>>2].writeFunction=0;
	sb_regs[((SB_PDLEND_addr-SB_BASE))>>2].data32=&SB_PDLEND;

	
	SB_SBREV=0xB;
	SB_G2ID=0x12;
	SB_G1SYSM=((0x0<<4) | (0x1));

	asic_reg_Init();

#if DC_PLATFORM!=DC_PLATFORM_NAOMI
	gdrom_reg_Init();
#else
	naomi_reg_Init();
#endif

	pvr_sb_Init();
	maple_Init();
	aica_sb_Init();
}

void sb_Reset(bool Manual)
{
	asic_reg_Reset(Manual);
#if DC_PLATFORM!=DC_PLATFORM_NAOMI
	gdrom_reg_Reset(Manual);
#else
	naomi_reg_Reset(Manual);
#endif
	pvr_sb_Reset(Manual);
	maple_Reset(Manual);
	aica_sb_Reset(Manual);
}

void sb_Term()
{
	aica_sb_Term();
	maple_Term();
	pvr_sb_Term();
#if DC_PLATFORM!=DC_PLATFORM_NAOMI
	gdrom_reg_Term();
#else
	naomi_reg_Term();
#endif
	asic_reg_Term();
}