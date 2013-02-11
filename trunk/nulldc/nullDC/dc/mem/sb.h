/*
	Nice helper #defines
*/

#pragma once
#include "types.h"



u32 sb_ReadMem(u32 addr,u32 sz);
void sb_WriteMem(u32 addr,u32 data,u32 sz);
void sb_Init();
void sb_Reset(bool Manual);
void sb_Term();

extern __declspec(align(32)) RegisterStruct sb_regs[0x540];	


#define SB_BASE 0x005F6800

//0x005F6800	SB_C2DSTAT	RW	ch2-DMA destination address 
#define SB_C2DSTAT_addr 0x005F6800
//0x005F6804	SB_C2DLEN	RW	ch2-DMA length 
#define SB_C2DLEN_addr 0x005F6804
//0x005F6808	SB_C2DST	RW	ch2-DMA start 
#define SB_C2DST_addr 0x005F6808
			
//0x005F6810	SB_SDSTAW	RW	Sort-DMA start link table address 
#define SB_SDSTAW_addr 0x005F6810
//0x005F6814	SB_SDBAAW	RW	Sort-DMA link base address 
#define SB_SDBAAW_addr 0x005F6814
//0x005F6818	SB_SDWLT	RW	Sort-DMA link address bit width 
#define SB_SDWLT_addr 0x005F6818
//0x005F681C	SB_SDLAS	RW	Sort-DMA link address shift control 
#define SB_SDLAS_addr 0x005F681C
//0x005F6820	SB_SDST	RW	Sort-DMA start 
#define SB_SDST_addr 0x005F6820
//0x005F6860 SB_SDDIV R(?) Sort-DMA LAT index (guess)
#define SB_SDDIV_addr 0x005F6860
			
//0x005F6840	SB_DBREQM	RW	DBREQ# signal mask control 
#define SB_DBREQM_addr 0x005F6840
//0x005F6844	SB_BAVLWC	RW	BAVL# signal wait count 
#define SB_BAVLWC_addr 0x005F6844
//0x005F6848	SB_C2DPRYC	RW	DMA (TA/Root Bus) priority count 
#define SB_C2DPRYC_addr 0x005F6848
//0x005F684C	SB_C2DMAXL	RW	ch2-DMA maximum burst length 
#define SB_C2DMAXL_addr 0x005F684C
			
//0x005F6880	SB_TFREM	R	TA FIFO remaining amount 
#define SB_TFREM_addr 0x005F6880
//0x005F6884	SB_LMMODE0	RW	Via TA texture memory bus select 0 
#define SB_LMMODE0_addr 0x005F6884
//0x005F6888	SB_LMMODE1	RW	Via TA texture memory bus select 1 
#define SB_LMMODE1_addr 0x005F6888
//0x005F688C	SB_FFST	R	FIFO status 
#define SB_FFST_addr 0x005F688C
//0x005F6890	SB_SFRES	W	System reset 
#define SB_SFRES_addr 0x005F6890
			
//0x005F689C	SB_SBREV	R	System bus revision number 
#define SB_SBREV_addr 0x005F689C
//0x005F68A0	SB_RBSPLT	RW	SH4 Root Bus split enable 
#define SB_RBSPLT_addr 0x005F68A0
			
//0x005F6900	SB_ISTNRM	RW	Normal interrupt status 
#define SB_ISTNRM_addr 0x005F6900
//0x005F6904	SB_ISTEXT	R	External interrupt status 
#define SB_ISTEXT_addr 0x005F6904
//0x005F6908	SB_ISTERR	RW	Error interrupt status 
#define SB_ISTERR_addr 0x005F6908
			
//0x005F6910	SB_IML2NRM	RW	Level 2 normal interrupt mask 
#define SB_IML2NRM_addr 0x005F6910
//0x005F6914	SB_IML2EXT	RW	Level 2 external interrupt mask 
#define SB_IML2EXT_addr 0x005F6914
//0x005F6918	SB_IML2ERR	RW	Level 2 error interrupt mask 
#define SB_IML2ERR_addr 0x005F6918
			
//0x005F6920	SB_IML4NRM	RW	Level 4 normal interrupt mask 
#define SB_IML4NRM_addr 0x005F6920
//0x005F6924	SB_IML4EXT	RW	Level 4 external interrupt mask 
#define SB_IML4EXT_addr 0x005F6924
//0x005F6928	SB_IML4ERR	RW	Level 4 error interrupt mask 
#define SB_IML4ERR_addr 0x005F6928
			
//0x005F6930	SB_IML6NRM	RW	Level 6 normal interrupt mask 
#define SB_IML6NRM_addr 0x005F6930
//0x005F6934	SB_IML6EXT	RW	Level 6 external interrupt mask 
#define SB_IML6EXT_addr 0x005F6934
//0x005F6938	SB_IML6ERR	RW	Level 6 error interrupt mask 
#define SB_IML6ERR_addr 0x005F6938
			
//0x005F6940	SB_PDTNRM	RW	Normal interrupt PVR-DMA startup mask 
#define SB_PDTNRM_addr 0x005F6940
//0x005F6944	SB_PDTEXT	RW	External interrupt PVR-DMA startup mask 
#define SB_PDTEXT_addr 0x005F6944
			
//0x005F6950	SB_G2DTNRM	RW	Normal interrupt G2-DMA startup mask 
#define SB_G2DTNRM_addr 0x005F6950
//0x005F6954	SB_G2DTEXT	RW	External interrupt G2-DMA startup mask 
#define SB_G2DTEXT_addr 0x005F6954
			
//0x005F6C04	SB_MDSTAR	RW	Maple-DMA command table address 
#define SB_MDSTAR_addr 0x005F6C04
			
//0x005F6C10	SB_MDTSEL	RW	Maple-DMA trigger select 
#define SB_MDTSEL_addr 0x005F6C10
//0x005F6C14	SB_MDEN	RW	Maple-DMA enable 
#define SB_MDEN_addr 0x005F6C14
//0x005F6C18	SB_MDST	RW	Maple-DMA start 
#define SB_MDST_addr 0x005F6C18
			
//0x005F6C80	SB_MSYS	RW	Maple system control 
#define SB_MSYS_addr 0x005F6C80
//0x005F6C84	SB_MST	R	Maple status 
#define SB_MST_addr 0x005F6C84
//0x005F6C88	SB_MSHTCL	W	Maple-DMA hard trigger clear 
#define SB_MSHTCL_addr 0x005F6C88
//0x005F6C8C	SB_MDAPRO	W	Maple-DMA address range 
#define SB_MDAPRO_addr 0x005F6C8C
			
//0x005F6CE8	SB_MMSEL	RW	Maple MSB selection 
#define SB_MMSEL_addr 0x005F6CE8
			
//0x005F6CF4	SB_MTXDAD	R	Maple Txd address counter 
#define SB_MTXDAD_addr 0x005F6CF4
//0x005F6CF8	SB_MRXDAD	R	Maple Rxd address counter 
#define SB_MRXDAD_addr 0x005F6CF8
//0x005F6CFC	SB_MRXDBD	R	Maple Rxd base address 
#define SB_MRXDBD_addr 0x005F6CFC
			





//0x005F7404	SB_GDSTAR	RW	GD-DMA start address 
#define SB_GDSTAR_addr 0x005F7404
//0x005F7408	SB_GDLEN	RW	GD-DMA length 
#define SB_GDLEN_addr 0x005F7408
//0x005F740C	SB_GDDIR	RW	GD-DMA direction 
#define SB_GDDIR_addr 0x005F740C
			
//0x005F7414	SB_GDEN	RW	GD-DMA enable 
#define SB_GDEN_addr 0x005F7414
//0x005F7418	SB_GDST	RW	GD-DMA start 
#define SB_GDST_addr 0x005F7418
			
//0x005F7480	SB_G1RRC	W	System ROM read access timing 
#define SB_G1RRC_addr 0x005F7480
//0x005F7484	SB_G1RWC	W	System ROM write access timing 
#define SB_G1RWC_addr 0x005F7484
//0x005F7488	SB_G1FRC	W	Flash ROM read access timing 
#define SB_G1FRC_addr 0x005F7488
//0x005F748C	SB_G1FWC	W	Flash ROM write access timing 
#define SB_G1FWC_addr 0x005F748C
//0x005F7490	SB_G1CRC	W	GD PIO read access timing 
#define SB_G1CRC_addr 0x005F7490
//0x005F7494	SB_G1CWC	W	GD PIO write access timing 
#define SB_G1CWC_addr 0x005F7494
			
//0x005F74A0	SB_G1GDRC	W	GD-DMA read access timing 
#define SB_G1GDRC_addr 0x005F74A0
//0x005F74A4	SB_G1GDWC	W	GD-DMA write access timing 
#define SB_G1GDWC_addr 0x005F74A4
			
//0x005F74B0	SB_G1SYSM	R	System mode 
#define SB_G1SYSM_addr 0x005F74B0
//0x005F74B4	SB_G1CRDYC	W	G1IORDY signal control 
#define SB_G1CRDYC_addr 0x005F74B4
//0x005F74B8	SB_GDAPRO	W	GD-DMA address range 
#define SB_GDAPRO_addr 0x005F74B8
			
//0x005F74F4	SB_GDSTARD	R	GD-DMA address count (on Root Bus) 
#define SB_GDSTARD_addr 0x005F74F4
//0x005F74F8	SB_GDLEND	R	GD-DMA transfer counter 
#define SB_GDLEND_addr 0x005F74F8
			
//0x005F7800	SB_ADSTAG	RW	AICA:G2-DMA G2 start address 
#define SB_ADSTAG_addr 0x005F7800
//0x005F7804	SB_ADSTAR	RW	AICA:G2-DMA system memory start address 
#define SB_ADSTAR_addr 0x005F7804
//0x005F7808	SB_ADLEN	RW	AICA:G2-DMA length 
#define SB_ADLEN_addr 0x005F7808
//0x005F780C	SB_ADDIR	RW	AICA:G2-DMA direction 
#define SB_ADDIR_addr 0x005F780C
//0x005F7810	SB_ADTSEL	RW	AICA:G2-DMA trigger select 
#define SB_ADTSEL_addr 0x005F7810
//0x005F7814	SB_ADEN	RW	AICA:G2-DMA enable 
#define SB_ADEN_addr 0x005F7814

//0x005F7818	SB_ADST	RW	AICA:G2-DMA start 
#define SB_ADST_addr 0x005F7818
//0x005F781C	SB_ADSUSP	RW	AICA:G2-DMA suspend 
#define SB_ADSUSP_addr 0x005F781C
			
//0x005F7820	SB_E1STAG	RW	Ext1:G2-DMA G2 start address 
#define SB_E1STAG_addr 0x005F7820
//0x005F7824	SB_E1STAR	RW	Ext1:G2-DMA system memory start address 
#define SB_E1STAR_addr 0x005F7824
//0x005F7828	SB_E1LEN	RW	Ext1:G2-DMA length 
#define SB_E1LEN_addr 0x005F7828
//0x005F782C	SB_E1DIR	RW	Ext1:G2-DMA direction 
#define SB_E1DIR_addr 0x005F782C
//0x005F7830	SB_E1TSEL	RW	Ext1:G2-DMA trigger select 
#define SB_E1TSEL_addr 0x005F7830
//0x005F7834	SB_E1EN	RW	Ext1:G2-DMA enable 
#define SB_E1EN_addr 0x005F7834
//0x005F7838	SB_E1ST	RW	Ext1:G2-DMA start 
#define SB_E1ST_addr 0x005F7838
//0x005F783C	SB_E1SUSP	RW	Ext1: G2-DMA suspend 
#define SB_E1SUSP_addr 0x005F783C
			
//0x005F7840	SB_E2STAG	RW	Ext2:G2-DMA G2 start address 
#define SB_E2STAG_addr 0x005F7840
//0x005F7844	SB_E2STAR	RW	Ext2:G2-DMA system memory start address 
#define SB_E2STAR_addr 0x005F7844
//0x005F7848	SB_E2LEN	RW	Ext2:G2-DMA length 
#define SB_E2LEN_addr 0x005F7848
//0x005F784C	SB_E2DIR	RW	Ext2:G2-DMA direction 
#define SB_E2DIR_addr 0x005F784C
//0x005F7850	SB_E2TSEL	RW	Ext2:G2-DMA trigger select 
#define SB_E2TSEL_addr 0x005F7850
//0x005F7854	SB_E2EN	RW	Ext2:G2-DMA enable 
#define SB_E2EN_addr 0x005F7854
//0x005F7858	SB_E2ST	RW	Ext2:G2-DMA start 
#define SB_E2ST_addr 0x005F7858
//0x005F785C	SB_E2SUSP	RW	Ext2: G2-DMA suspend 
#define SB_E2SUSP_addr 0x005F785C
			
//0x005F7860	SB_DDSTAG	RW	Dev:G2-DMA G2 start address 
#define SB_DDSTAG_addr 0x005F7860
//0x005F7864	SB_DDSTAR	RW	Dev:G2-DMA system memory start address 
#define SB_DDSTAR_addr 0x005F7864
//0x005F7868	SB_DDLEN	RW	Dev:G2-DMA length 
#define SB_DDLEN_addr 0x005F7868
//0x005F786C	SB_DDDIR	RW	Dev:G2-DMA direction 
#define SB_DDDIR_addr 0x005F786C
//0x005F7870	SB_DDTSEL	RW	Dev:G2-DMA trigger select 
#define SB_DDTSEL_addr 0x005F7870
//0x005F7874	SB_DDEN	RW	Dev:G2-DMA enable 
#define SB_DDEN_addr 0x005F7874
//0x005F7878	SB_DDST	RW	Dev:G2-DMA start 
#define SB_DDST_addr 0x005F7878
//0x005F787C	SB_DDSUSP	RW	Dev: G2-DMA suspend 
#define SB_DDSUSP_addr 0x005F787C
			
//0x005F7880	SB_G2ID	R	G2 bus version 
#define SB_G2ID_addr 0x005F7880
			
//0x005F7890	SB_G2DSTO	RW	G2/DS timeout 
#define SB_G2DSTO_addr 0x005F7890
//0x005F7894	SB_G2TRTO	RW	G2/TR timeout 
#define SB_G2TRTO_addr 0x005F7894
//0x005F7898	SB_G2MDMTO	RW	Modem unit wait timeout 
#define SB_G2MDMTO_addr 0x005F7898
//0x005F789C	SB_G2MDMW	RW	Modem unit wait time 
#define SB_G2MDMW_addr 0x005F789C
			
//0x005F78BC	SB_G2APRO	W	G2-DMA address range 
#define SB_G2APRO_addr 0x005F78BC
			
//0x005F78C0	SB_ADSTAGD	R	AICA-DMA address counter (on AICA) 
#define SB_ADSTAGD_addr 0x005F78C0
//0x005F78C4	SB_ADSTARD	R	AICA-DMA address counter (on root bus) 
#define SB_ADSTARD_addr 0x005F78C4
//0x005F78C8	SB_ADLEND	R	AICA-DMA transfer counter 
#define SB_ADLEND_addr 0x005F78C8
			
//0x005F78D0	SB_E1STAGD	R	Ext-DMA1 address counter (on Ext) 
#define SB_E1STAGD_addr 0x005F78D0
//0x005F78D4	SB_E1STARD	R	Ext-DMA1 address counter (on root bus) 
#define SB_E1STARD_addr 0x005F78D4
//0x005F78D8	SB_E1LEND	R	Ext-DMA1 transfer counter 
#define SB_E1LEND_addr 0x005F78D8
			
//0x005F78E0	SB_E2STAGD	R	Ext-DMA2 address counter (on Ext) 
#define SB_E2STAGD_addr 0x005F78E0
//0x005F78E4	SB_E2STARD	R	Ext-DMA2 address counter (on root bus) 
#define SB_E2STARD_addr 0x005F78E4
//0x005F78E8	SB_E2LEND	R	Ext-DMA2 transfer counter 
#define SB_E2LEND_addr 0x005F78E8
			
//0x005F78F0	SB_DDSTAGD	R	Dev-DMA address counter (on Ext) 
#define SB_DDSTAGD_addr 0x005F78F0
//0x005F78F4	SB_DDSTARD	R	Dev-DMA address counter (on root bus) 
#define SB_DDSTARD_addr 0x005F78F4
//0x005F78F8	SB_DDLEND	R	Dev-DMA transfer counter 
#define SB_DDLEND_addr 0x005F78F8
			
//0x005F7C00	SB_PDSTAP	RW	PVR-DMA PVR start address 
#define SB_PDSTAP_addr 0x005F7C00
//0x005F7C04	SB_PDSTAR	RW	PVR-DMA system memory start address 
#define SB_PDSTAR_addr 0x005F7C04
//0x005F7C08	SB_PDLEN	RW	PVR-DMA length 
#define SB_PDLEN_addr 0x005F7C08
//0x005F7C0C	SB_PDDIR	RW	PVR-DMA direction 
#define SB_PDDIR_addr 0x005F7C0C
//0x005F7C10	SB_PDTSEL	RW	PVR-DMA trigger select 
#define SB_PDTSEL_addr 0x005F7C10
//0x005F7C14	SB_PDEN	RW	PVR-DMA enable 
#define SB_PDEN_addr 0x005F7C14
//0x005F7C18	SB_PDST	RW	PVR-DMA start 
#define SB_PDST_addr 0x005F7C18
			
//0x005F7C80	SB_PDAPRO	W	PVR-DMA address range 
#define SB_PDAPRO_addr 0x005F7C80
			
//0x005F7CF0	SB_PDSTAPD	R	PVR-DMA address counter (on Ext) 
#define SB_PDSTAPD_addr 0x005F7CF0
//0x005F7CF4	SB_PDSTARD	R	PVR-DMA address counter (on root bus) 
#define SB_PDSTARD_addr 0x005F7CF4
//0x005F7CF8	SB_PDLEND	R	PVR-DMA transfer counter 
#define SB_PDLEND_addr 0x005F7CF8

//0x005F6800	SB_C2DSTAT	RW	ch2-DMA destination address 
extern u32 SB_C2DSTAT;
//0x005F6804	SB_C2DLEN	RW	ch2-DMA length 
extern u32 SB_C2DLEN;
//0x005F6808	SB_C2DST	RW	ch2-DMA start 
extern u32 SB_C2DST;
			
//0x005F6810	SB_SDSTAW	RW	Sort-DMA start link table address 
extern u32 SB_SDSTAW;
//0x005F6814	SB_SDBAAW	RW	Sort-DMA link base address 
extern u32 SB_SDBAAW;
//0x005F6818	SB_SDWLT	RW	Sort-DMA link address bit width 
extern u32 SB_SDWLT;
//0x005F681C	SB_SDLAS	RW	Sort-DMA link address shift control 
extern u32 SB_SDLAS;
//0x005F6820	SB_SDST	RW	Sort-DMA start 
extern u32 SB_SDST;
//0x005F6860 SB_SDDIV R(?) Sort-DMA LAT index (guess)
extern u32 SB_SDDIV;
			
//0x005F6840	SB_DBREQM	RW	DBREQ# signal mask control 
extern u32 SB_DBREQM;
//0x005F6844	SB_BAVLWC	RW	BAVL# signal wait count 
extern u32 SB_BAVLWC;
//0x005F6848	SB_C2DPRYC	RW	DMA (TA/Root Bus) priority count 
extern u32 SB_C2DPRYC;
//0x005F684C	SB_C2DMAXL	RW	ch2-DMA maximum burst length 
extern u32 SB_C2DMAXL;
			
//0x005F6880	SB_TFREM	R	TA FIFO remaining amount 
extern u32 SB_TFREM;
//0x005F6884	SB_LMMODE0	RW	Via TA texture memory bus select 0 
extern u32 SB_LMMODE0;
//0x005F6888	SB_LMMODE1	RW	Via TA texture memory bus select 1 
extern u32 SB_LMMODE1;
//0x005F688C	SB_FFST	R	FIFO status 
extern u32 SB_FFST;
//0x005F6890	SB_SFRES	W	System reset 
extern u32 SB_SFRES;
			
//0x005F689C	SB_SBREV	R	System bus revision number 
extern u32 SB_SBREV;
//0x005F68A0	SB_RBSPLT	RW	SH4 Root Bus split enable 
extern u32 SB_RBSPLT;
			
//0x005F6900	SB_ISTNRM	RW	Normal interrupt status 
extern u32 SB_ISTNRM;
//0x005F6904	SB_ISTEXT	R	External interrupt status 
extern u32 SB_ISTEXT;
//0x005F6908	SB_ISTERR	RW	Error interrupt status 
extern u32 SB_ISTERR;
			
//0x005F6910	SB_IML2NRM	RW	Level 2 normal interrupt mask 
extern u32 SB_IML2NRM;
//0x005F6914	SB_IML2EXT	RW	Level 2 external interrupt mask 
extern u32 SB_IML2EXT;
//0x005F6918	SB_IML2ERR	RW	Level 2 error interrupt mask 
extern u32 SB_IML2ERR;
			
//0x005F6920	SB_IML4NRM	RW	Level 4 normal interrupt mask 
extern u32 SB_IML4NRM;
//0x005F6924	SB_IML4EXT	RW	Level 4 external interrupt mask 
extern u32 SB_IML4EXT;
//0x005F6928	SB_IML4ERR	RW	Level 4 error interrupt mask 
extern u32 SB_IML4ERR;
			
//0x005F6930	SB_IML6NRM	RW	Level 6 normal interrupt mask 
extern u32 SB_IML6NRM;
//0x005F6934	SB_IML6EXT	RW	Level 6 external interrupt mask 
extern u32 SB_IML6EXT;
//0x005F6938	SB_IML6ERR	RW	Level 6 error interrupt mask 
extern u32 SB_IML6ERR;
			
//0x005F6940	SB_PDTNRM	RW	Normal interrupt PVR-DMA startup mask 
extern u32 SB_PDTNRM;
//0x005F6944	SB_PDTEXT	RW	External interrupt PVR-DMA startup mask 
extern u32 SB_PDTEXT;
			
//0x005F6950	SB_G2DTNRM	RW	Normal interrupt G2-DMA startup mask 
extern u32 SB_G2DTNRM;
//0x005F6954	SB_G2DTEXT	RW	External interrupt G2-DMA startup mask 
extern u32 SB_G2DTEXT;
			
//0x005F6C04	SB_MDSTAR	RW	Maple-DMA command table address 
extern u32 SB_MDSTAR;
			
//0x005F6C10	SB_MDTSEL	RW	Maple-DMA trigger select 
extern u32 SB_MDTSEL;
//0x005F6C14	SB_MDEN	RW	Maple-DMA enable 
extern u32 SB_MDEN;
//0x005F6C18	SB_MDST	RW	Maple-DMA start 
extern u32 SB_MDST;
			
//0x005F6C80	SB_MSYS	RW	Maple system control 
extern u32 SB_MSYS;
//0x005F6C84	SB_MST	R	Maple status 
extern u32 SB_MST;
//0x005F6C88	SB_MSHTCL	W	Maple-DMA hard trigger clear 
extern u32 SB_MSHTCL;
//0x005F6C8C	SB_MDAPRO	W	Maple-DMA address range 
extern u32 SB_MDAPRO;
			
//0x005F6CE8	SB_MMSEL	RW	Maple MSB selection 
extern u32 SB_MMSEL;
			
//0x005F6CF4	SB_MTXDAD	R	Maple Txd address counter 
extern u32 SB_MTXDAD;
//0x005F6CF8	SB_MRXDAD	R	Maple Rxd address counter 
extern u32 SB_MRXDAD;
//0x005F6CFC	SB_MRXDBD	R	Maple Rxd base address 
extern u32 SB_MRXDBD;




			
//0x005F7404	SB_GDSTAR	RW	GD-DMA start address 
extern u32 SB_GDSTAR;
//0x005F7408	SB_GDLEN	RW	GD-DMA length 
extern u32 SB_GDLEN;
//0x005F740C	SB_GDDIR	RW	GD-DMA direction 
extern u32 SB_GDDIR;
			
//0x005F7414	SB_GDEN	RW	GD-DMA enable 
extern u32 SB_GDEN;
//0x005F7418	SB_GDST	RW	GD-DMA start 
extern u32 SB_GDST;
			
//0x005F7480	SB_G1RRC	W	System ROM read access timing 
extern u32 SB_G1RRC;
//0x005F7484	SB_G1RWC	W	System ROM write access timing 
extern u32 SB_G1RWC;
//0x005F7488	SB_G1FRC	W	Flash ROM read access timing 
extern u32 SB_G1FRC;
//0x005F748C	SB_G1FWC	W	Flash ROM write access timing 
extern u32 SB_G1FWC;
//0x005F7490	SB_G1CRC	W	GD PIO read access timing 
extern u32 SB_G1CRC;
//0x005F7494	SB_G1CWC	W	GD PIO write access timing 
extern u32 SB_G1CWC;
			
//0x005F74A0	SB_G1GDRC	W	GD-DMA read access timing 
extern u32 SB_G1GDRC;
//0x005F74A4	SB_G1GDWC	W	GD-DMA write access timing 
extern u32 SB_G1GDWC;
			
//0x005F74B0	SB_G1SYSM	R	System mode 
extern u32 SB_G1SYSM;
//0x005F74B4	SB_G1CRDYC	W	G1IORDY signal control 
extern u32 SB_G1CRDYC;
//0x005F74B8	SB_GDAPRO	W	GD-DMA address range 
extern u32 SB_GDAPRO;
			
//0x005F74F4	SB_GDSTARD	R	GD-DMA address count (on Root Bus) 
extern u32 SB_GDSTARD;
//0x005F74F8	SB_GDLEND	R	GD-DMA transfer counter 
extern u32 SB_GDLEND;
			
//0x005F7800	SB_ADSTAG	RW	AICA:G2-DMA G2 start address 
extern u32 SB_ADSTAG;
//0x005F7804	SB_ADSTAR	RW	AICA:G2-DMA system memory start address 
extern u32 SB_ADSTAR;
//0x005F7808	SB_ADLEN	RW	AICA:G2-DMA length 
extern u32 SB_ADLEN;
//0x005F780C	SB_ADDIR	RW	AICA:G2-DMA direction 
extern u32 SB_ADDIR;
//0x005F7810	SB_ADTSEL	RW	AICA:G2-DMA trigger select 
extern u32 SB_ADTSEL;
//0x005F7814	SB_ADEN	RW	AICA:G2-DMA enable 
extern u32 SB_ADEN;

//0x005F7818	SB_ADST	RW	AICA:G2-DMA start 
extern u32 SB_ADST;
//0x005F781C	SB_ADSUSP	RW	AICA:G2-DMA suspend 
extern u32 SB_ADSUSP;
			
//0x005F7820	SB_E1STAG	RW	Ext1:G2-DMA G2 start address 
extern u32 SB_E1STAG;
//0x005F7824	SB_E1STAR	RW	Ext1:G2-DMA system memory start address 
extern u32 SB_E1STAR;
//0x005F7828	SB_E1LEN	RW	Ext1:G2-DMA length 
extern u32 SB_E1LEN;
//0x005F782C	SB_E1DIR	RW	Ext1:G2-DMA direction 
extern u32 SB_E1DIR;
//0x005F7830	SB_E1TSEL	RW	Ext1:G2-DMA trigger select 
extern u32 SB_E1TSEL;
//0x005F7834	SB_E1EN	RW	Ext1:G2-DMA enable 
extern u32 SB_E1EN;
//0x005F7838	SB_E1ST	RW	Ext1:G2-DMA start 
extern u32 SB_E1ST;
//0x005F783C	SB_E1SUSP	RW	Ext1: G2-DMA suspend 
extern u32 SB_E1SUSP;
			
//0x005F7840	SB_E2STAG	RW	Ext2:G2-DMA G2 start address 
extern u32 SB_E2STAG;
//0x005F7844	SB_E2STAR	RW	Ext2:G2-DMA system memory start address 
extern u32 SB_E2STAR;
//0x005F7848	SB_E2LEN	RW	Ext2:G2-DMA length 
extern u32 SB_E2LEN;
//0x005F784C	SB_E2DIR	RW	Ext2:G2-DMA direction 
extern u32 SB_E2DIR;
//0x005F7850	SB_E2TSEL	RW	Ext2:G2-DMA trigger select 
extern u32 SB_E2TSEL;
//0x005F7854	SB_E2EN	RW	Ext2:G2-DMA enable 
extern u32 SB_E2EN;
//0x005F7858	SB_E2ST	RW	Ext2:G2-DMA start 
extern u32 SB_E2ST;
//0x005F785C	SB_E2SUSP	RW	Ext2: G2-DMA suspend 
extern u32 SB_E2SUSP;
			
//0x005F7860	SB_DDSTAG	RW	Dev:G2-DMA G2 start address 
extern u32 SB_DDSTAG;
//0x005F7864	SB_DDSTAR	RW	Dev:G2-DMA system memory start address 
extern u32 SB_DDSTAR;
//0x005F7868	SB_DDLEN	RW	Dev:G2-DMA length 
extern u32 SB_DDLEN;
//0x005F786C	SB_DDDIR	RW	Dev:G2-DMA direction 
extern u32 SB_DDDIR;
//0x005F7870	SB_DDTSEL	RW	Dev:G2-DMA trigger select 
extern u32 SB_DDTSEL;
//0x005F7874	SB_DDEN	RW	Dev:G2-DMA enable 
extern u32 SB_DDEN;
//0x005F7878	SB_DDST	RW	Dev:G2-DMA start 
extern u32 SB_DDST;
//0x005F787C	SB_DDSUSP	RW	Dev: G2-DMA suspend 
extern u32 SB_DDSUSP;
			
//0x005F7880	SB_G2ID	R	G2 bus version 
extern u32 SB_G2ID;
			
//0x005F7890	SB_G2DSTO	RW	G2/DS timeout 
extern u32 SB_G2DSTO;
//0x005F7894	SB_G2TRTO	RW	G2/TR timeout 
extern u32 SB_G2TRTO;
//0x005F7898	SB_G2MDMTO	RW	Modem unit wait timeout 
extern u32 SB_G2MDMTO;
//0x005F789C	SB_G2MDMW	RW	Modem unit wait time 
extern u32 SB_G2MDMW;
			
//0x005F78BC	SB_G2APRO	W	G2-DMA address range 
extern u32 SB_G2APRO;
			
//0x005F78C0	SB_ADSTAGD	R	AICA-DMA address counter (on AICA) 
extern u32 SB_ADSTAGD;
//0x005F78C4	SB_ADSTARD	R	AICA-DMA address counter (on root bus) 
extern u32 SB_ADSTARD;
//0x005F78C8	SB_ADLEND	R	AICA-DMA transfer counter 
extern u32 SB_ADLEND;
			
//0x005F78D0	SB_E1STAGD	R	Ext-DMA1 address counter (on Ext) 
extern u32 SB_E1STAGD;
//0x005F78D4	SB_E1STARD	R	Ext-DMA1 address counter (on root bus) 
extern u32 SB_E1STARD;
//0x005F78D8	SB_E1LEND	R	Ext-DMA1 transfer counter 
extern u32 SB_E1LEND;
			
//0x005F78E0	SB_E2STAGD	R	Ext-DMA2 address counter (on Ext) 
extern u32 SB_E2STAGD;
//0x005F78E4	SB_E2STARD	R	Ext-DMA2 address counter (on root bus) 
extern u32 SB_E2STARD;
//0x005F78E8	SB_E2LEND	R	Ext-DMA2 transfer counter 
extern u32 SB_E2LEND;
			
//0x005F78F0	SB_DDSTAGD	R	Dev-DMA address counter (on Ext) 
extern u32 SB_DDSTAGD;
//0x005F78F4	SB_DDSTARD	R	Dev-DMA address counter (on root bus) 
extern u32 SB_DDSTARD;
//0x005F78F8	SB_DDLEND	R	Dev-DMA transfer counter 
extern u32 SB_DDLEND;
			
//0x005F7C00	SB_PDSTAP	RW	PVR-DMA PVR start address 
extern u32 SB_PDSTAP;
//0x005F7C04	SB_PDSTAR	RW	PVR-DMA system memory start address 
extern u32 SB_PDSTAR;
//0x005F7C08	SB_PDLEN	RW	PVR-DMA length 
extern u32 SB_PDLEN;
//0x005F7C0C	SB_PDDIR	RW	PVR-DMA direction 
extern u32 SB_PDDIR;
//0x005F7C10	SB_PDTSEL	RW	PVR-DMA trigger select 
extern u32 SB_PDTSEL;
//0x005F7C14	SB_PDEN	RW	PVR-DMA enable 
extern u32 SB_PDEN;
//0x005F7C18	SB_PDST	RW	PVR-DMA start 
extern u32 SB_PDST;
			
//0x005F7C80	SB_PDAPRO	W	PVR-DMA address range 
extern u32 SB_PDAPRO;
			
//0x005F7CF0	SB_PDSTAPD	R	PVR-DMA address counter (on Ext) 
extern u32 SB_PDSTAPD;
//0x005F7CF4	SB_PDSTARD	R	PVR-DMA address counter (on root bus) 
extern u32 SB_PDSTARD;
//0x005F7CF8	SB_PDLEND	R	PVR-DMA transfer counter 
extern u32 SB_PDLEND;

/*

0x005F6800	SB_C2DSTAT	RW	ch2-DMA destination address
0x005F6804	SB_C2DLEN	RW	ch2-DMA length
0x005F6808	SB_C2DST	RW	ch2-DMA start
			
0x005F6810	SB_SDSTAW	RW	Sort-DMA start link table address
0x005F6814	SB_SDBAAW	RW	Sort-DMA link base address
0x005F6818	SB_SDWLT	RW	Sort-DMA link address bit width
0x005F681C	SB_SDLAS	RW	Sort-DMA link address shift control
0x005F6820	SB_SDST	RW	Sort-DMA start
			
0x005F6840	SB_DBREQM	RW	DBREQ# signal mask control
0x005F6844	SB_BAVLWC	RW	BAVL# signal wait count
0x005F6848	SB_C2DPRYC	RW	DMA (TA/Root Bus) priority count
0x005F684C	SB_C2DMAXL	RW	ch2-DMA maximum burst length
			
0x005F6880	SB_TFREM	R	TA FIFO remaining amount
0x005F6884	SB_LMMODE0	RW	Via TA texture memory bus select 0
0x005F6888	SB_LMMODE1	RW	Via TA texture memory bus select 1
0x005F688C	SB_FFST	R	FIFO status
0x005F6890	SB_SFRES	W	System reset
			
0x005F689C	SB_SBREV	R	System bus revision number
0x005F68A0	SB_RBSPLT	RW	SH4 Root Bus split enable
			
0x005F6900	SB_ISTNRM	RW	Normal interrupt status
0x005F6904	SB_ISTEXT	R	External interrupt status
0x005F6908	SB_ISTERR	RW	Error interrupt status
			
0x005F6910	SB_IML2NRM	RW	Level 2 normal interrupt mask
0x005F6914	SB_IML2EXT	RW	Level 2 external interrupt mask
0x005F6918	SB_IML2ERR	RW	Level 2 error interrupt mask
			
0x005F6920	SB_IML4NRM	RW	Level 4 normal interrupt mask
0x005F6924	SB_IML4EXT	RW	Level 4 external interrupt mask
0x005F6928	SB_IML4ERR	RW	Level 4 error interrupt mask
			
0x005F6930	SB_IML6NRM	RW	Level 6 normal interrupt mask
0x005F6934	SB_IML6EXT	RW	Level 6 external interrupt mask
0x005F6938	SB_IML6ERR	RW	Level 6 error interrupt mask
			
0x005F6940	SB_PDTNRM	RW	Normal interrupt PVR-DMA startup mask
0x005F6944	SB_PDTEXT	RW	External interrupt PVR-DMA startup mask
			
0x005F6950	SB_G2DTNRM	RW	Normal interrupt G2-DMA startup mask
0x005F6954	SB_G2DTEXT	RW	External interrupt G2-DMA startup mask

0x005F6C04	SB_MDSTAR	RW	Maple-DMA command table address
			
0x005F6C10	SB_MDTSEL	RW	Maple-DMA trigger select
0x005F6C14	SB_MDEN	RW	Maple-DMA enable
0x005F6C18	SB_MDST	RW	Maple-DMA start
			
0x005F6C80	SB_MSYS	RW	Maple system control
0x005F6C84	SB_MST	R	Maple status
0x005F6C88	SB_MSHTCL	W	Maple-DMA hard trigger clear
0x005F6C8C	SB_MDAPRO	W	Maple-DMA address range
			
0x005F6CE8	SB_MMSEL	RW	Maple MSB selection
			
0x005F6CF4	SB_MTXDAD	R	Maple Txd address counter
0x005F6CF8	SB_MRXDAD	R	Maple Rxd address counter
0x005F6CFC	SB_MRXDBD	R	Maple Rxd base address
			
0x005F7404	SB_GDSTAR	RW	GD-DMA start address
0x005F7408	SB_GDLEN	RW	GD-DMA length
0x005F740C	SB_GDDIR	RW	GD-DMA direction
			
0x005F7414	SB_GDEN	RW	GD-DMA enable
0x005F7418	SB_GDST	RW	GD-DMA start
			
0x005F7480	SB_G1RRC	W	System ROM read access timing
0x005F7484	SB_G1RWC	W	System ROM write access timing
0x005F7488	SB_G1FRC	W	Flash ROM read access timing
0x005F748C	SB_G1FWC	W	Flash ROM write access timing
0x005F7490	SB_G1CRC	W	GD PIO read access timing
0x005F7494	SB_G1CWC	W	GD PIO write access timing
			
0x005F74A0	SB_G1GDRC	W	GD-DMA read access timing
0x005F74A4	SB_G1GDWC	W	GD-DMA write access timing
			
0x005F74B0	SB_G1SYSM	R	System mode
0x005F74B4	SB_G1CRDYC	W	G1IORDY signal control
0x005F74B8	SB_GDAPRO	W	GD-DMA address range
			
0x005F74F4	SB_GDSTARD	R	GD-DMA address count (on Root Bus)
0x005F74F8	SB_GDLEND	R	GD-DMA transfer counter
			
0x005F7800	SB_ADSTAG	RW	AICA:G2-DMA G2 start address
0x005F7804	SB_ADSTAR	RW	AICA:G2-DMA system memory start address
0x005F7808	SB_ADLEN	RW	AICA:G2-DMA length
0x005F780C	SB_ADDIR	RW	AICA:G2-DMA direction
0x005F7810	SB_ADTSEL	RW	AICA:G2-DMA trigger select
0x005F7814	SB_ADEN	RW	AICA:G2-DMA enable

0x005F7818	SB_ADST	RW	AICA:G2-DMA start
0x005F781C	SB_ADSUSP	RW	AICA:G2-DMA suspend
			
0x005F7820	SB_E1STAG	RW	Ext1:G2-DMA G2 start address
0x005F7824	SB_E1STAR	RW	Ext1:G2-DMA system memory start address
0x005F7828	SB_E1LEN	RW	Ext1:G2-DMA length
0x005F782C	SB_E1DIR	RW	Ext1:G2-DMA direction
0x005F7830	SB_E1TSEL	RW	Ext1:G2-DMA trigger select
0x005F7834	SB_E1EN	RW	Ext1:G2-DMA enable
0x005F7838	SB_E1ST	RW	Ext1:G2-DMA start
0x005F783C	SB_E1SUSP	RW	Ext1: G2-DMA suspend
			
0x005F7840	SB_E2STAG	RW	Ext2:G2-DMA G2 start address
0x005F7844	SB_E2STAR	RW	Ext2:G2-DMA system memory start address
0x005F7848	SB_E2LEN	RW	Ext2:G2-DMA length
0x005F784C	SB_E2DIR	RW	Ext2:G2-DMA direction
0x005F7850	SB_E2TSEL	RW	Ext2:G2-DMA trigger select
0x005F7854	SB_E2EN	RW	Ext2:G2-DMA enable
0x005F7858	SB_E2ST	RW	Ext2:G2-DMA start
0x005F785C	SB_E2SUSP	RW	Ext2: G2-DMA suspend
			
0x005F7860	SB_DDSTAG	RW	Dev:G2-DMA G2 start address
0x005F7864	SB_DDSTAR	RW	Dev:G2-DMA system memory start address
0x005F7868	SB_DDLEN	RW	Dev:G2-DMA length
0x005F786C	SB_DDDIR	RW	Dev:G2-DMA direction
0x005F7870	SB_DDTSEL	RW	Dev:G2-DMA trigger select
0x005F7874	SB_DDEN	RW	Dev:G2-DMA enable
0x005F7878	SB_DDST	RW	Dev:G2-DMA start
0x005F787C	SB_DDSUSP	RW	Dev: G2-DMA suspend
			
0x005F7880	SB_G2ID	R	G2 bus version
			
0x005F7890	SB_G2DSTO	RW	G2/DS timeout
0x005F7894	SB_G2TRTO	RW	G2/TR timeout
0x005F7898	SB_G2MDMTO	RW	Modem unit wait timeout
0x005F789C	SB_G2MDMW	RW	Modem unit wait time
			
0x005F78BC	SB_G2APRO	W	G2-DMA address range
			
0x005F78C0	SB_ADSTAGD	R	AICA-DMA address counter (on AICA)
0x005F78C4	SB_ADSTARD	R	AICA-DMA address counter (on root bus)
0x005F78C8	SB_ADLEND	R	AICA-DMA transfer counter
			
0x005F78D0	SB_E1STAGD	R	Ext-DMA1 address counter (on Ext)
0x005F78D4	SB_E1STARD	R	Ext-DMA1 address counter (on root bus)
0x005F78D8	SB_E1LEND	R	Ext-DMA1 transfer counter
			
0x005F78E0	SB_E2STAGD	R	Ext-DMA2 address counter (on Ext)
0x005F78E4	SB_E2STARD	R	Ext-DMA2 address counter (on root bus)
0x005F78E8	SB_E2LEND	R	Ext-DMA2 transfer counter
			
0x005F78F0	SB_DDSTAGD	R	Dev-DMA address counter (on Ext)
0x005F78F4	SB_DDSTARD	R	Dev-DMA address counter (on root bus)
0x005F78F8	SB_DDLEND	R	Dev-DMA transfer counter
			
0x005F7C00	SB_PDSTAP	RW	PVR-DMA PVR start address
0x005F7C04	SB_PDSTAR	RW	PVR-DMA system memory start address
0x005F7C08	SB_PDLEN	RW	PVR-DMA length
0x005F7C0C	SB_PDDIR	RW	PVR-DMA direction
0x005F7C10	SB_PDTSEL	RW	PVR-DMA trigger select
0x005F7C14	SB_PDEN	RW	PVR-DMA enable
0x005F7C18	SB_PDST	RW	PVR-DMA start
			
0x005F7C80	SB_PDAPRO	W	PVR-DMA address range
			
0x005F7CF0	SB_PDSTAPD	R	PVR-DMA address counter (on Ext)
0x005F7CF4	SB_PDSTARD	R	PVR-DMA address counter (on root bus)
0x005F7CF8	SB_PDLEND	R	PVR-DMA transfer counter

*/