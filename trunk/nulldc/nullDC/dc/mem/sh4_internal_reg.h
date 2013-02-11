#pragma once
#include "types.h"

#define OnChipRAM_SIZE (0x2000)
#define OnChipRAM_MASK (OnChipRAM_SIZE-1)

extern __declspec(align(64)) u8 sq_both[64];

extern __declspec(align(32)) RegisterStruct CCN[20];
extern __declspec(align(32)) RegisterStruct UBC[20];
extern __declspec(align(32)) RegisterStruct BSC[20];
extern __declspec(align(32)) RegisterStruct DMAC[20];
extern __declspec(align(32)) RegisterStruct CPG[20];
extern __declspec(align(32)) RegisterStruct RTC[20];
extern __declspec(align(32)) RegisterStruct INTC[20];
extern __declspec(align(32)) RegisterStruct TMU[20];
extern __declspec(align(32)) RegisterStruct SCI[20];
extern __declspec(align(32)) RegisterStruct SCIF[20];

/*
//Region P4
u32 ReadMem_P4(u32 addr,u32 sz);
void WriteMem_P4(u32 addr,u32 data,u32 sz);

//Area7
u32 ReadMem_area7(u32 addr,u32 sz);
void WriteMem_area7(u32 addr,u32 data,u32 sz);
void __fastcall WriteMem_sq_32(u32 address,u32 data);*/

//Init/Res/Term
void sh4_internal_reg_Init();
void sh4_internal_reg_Reset(bool Manual);
void sh4_internal_reg_Term();

#define A7_REG_HASH(addr) ((addr>>16)&0x1FFF)

//CCN module registers base
#define CCN_BASE_addr 0x1F000000

//CCN PTEH 0xFF000000 0x1F000000 32 Undefined Undefined Held Held Iclk
#define CCN_PTEH_addr 0x1F000000

//CCN PTEL 0xFF000004 0x1F000004 32 Undefined Undefined Held Held Iclk
#define CCN_PTEL_addr 0x1F000004

//CCN TTB 0xFF000008 0x1F000008 32 Undefined Undefined Held Held Iclk
#define CCN_TTB_addr 0x1F000008

//CCN TEA 0xFF00000C 0x1F00000C 32 Undefined Held Held Held Iclk
#define CCN_TEA_addr 0x1F00000C

//CCN MMUCR 0xFF000010 0x1F000010 32 0x00000000 0x00000000 Held Held Iclk
#define CCN_MMUCR_addr 0x1F000010

//CCN BASRA 0xFF000014 0x1F000014 8 Undefined Held Held Held Iclk
#define CCN_BASRA_addr 0x1F000014

//CCN BASRB 0xFF000018 0x1F000018 8 Undefined Held Held Held Iclk
#define CCN_BASRB_addr 0x1F000018

//CCN CCR 0xFF00001C 0x1F00001C 32 0x00000000 0x00000000 Held Held Iclk
#define CCN_CCR_addr 0x1F00001C

//CCN TRA 0xFF000020 0x1F000020 32 Undefined Undefined Held Held Iclk
#define CCN_TRA_addr 0x1F000020

//CCN EXPEVT 0xFF000024 0x1F000024 32 0x00000000 0x00000020 Held Held Iclk
#define CCN_EXPEVT_addr 0x1F000024

//CCN INTEVT 0xFF000028 0x1F000028 32 Undefined Undefined Held Held Iclk
#define CCN_INTEVT_addr 0x1F000028

//CCN PTEA 0xFF000034 0x1F000034 32 Undefined Undefined Held Held Iclk
#define CCN_PTEA_addr 0x1F000034

//CCN QACR0 0xFF000038 0x1F000038 32 Undefined Undefined Held Held Iclk
#define CCN_QACR0_addr 0x1F000038

//CCN QACR1 0xFF00003C 0x1F00003C 32 Undefined Undefined Held Held Iclk
#define CCN_QACR1_addr 0x1F00003C

//UBC module registers base
#define UBC_BASE_addr 0x1F200000

//UBC BARA 0xFF200000 0x1F200000 32 Undefined Held Held Held Iclk
#define UBC_BARA_addr 0x1F200000

//UBC BAMRA 0xFF200004 0x1F200004 8 Undefined Held Held Held Iclk
#define UBC_BAMRA_addr 0x1F200004

//UBC BBRA 0xFF200008 0x1F200008 16 0x0000 Held Held Held Iclk
#define UBC_BBRA_addr 0x1F200008

//UBC BARB 0xFF20000C 0x1F20000C 32 Undefined Held Held Held Iclk
#define UBC_BARB_addr 0x1F20000C

//UBC BAMRB 0xFF200010 0x1F200010 8 Undefined Held Held Held Iclk
#define UBC_BAMRB_addr 0x1F200010

//UBC BBRB 0xFF200014 0x1F200014 16 0x0000 Held Held Held Iclk
#define UBC_BBRB_addr 0x1F200014

//UBC BDRB 0xFF200018 0x1F200018 32 Undefined Held Held Held Iclk
#define UBC_BDRB_addr 0x1F200018

//UBC BDMRB 0xFF20001C 0x1F20001C 32 Undefined Held Held Held Iclk
#define UBC_BDMRB_addr 0x1F20001C

//UBC BRCR 0xFF200020 0x1F200020 16 0x0000 Held Held Held Iclk
#define UBC_BRCR_addr 0x1F200020

//BSC module registers base
#define BSC_BASE_addr 0x1F800000

//BSC BCR1 0xFF800000 0x1F800000 32 0x00000000 Held Held Held Bclk
#define BSC_BCR1_addr 0x1F800000

//BSC BCR2 0xFF800004 0x1F800004 16 0x3FFC Held Held Held Bclk
#define BSC_BCR2_addr 0x1F800004

//BSC WCR1 0xFF800008 0x1F800008 32 0x77777777 Held Held Held Bclk
#define BSC_WCR1_addr 0x1F800008

//BSC WCR2 0xFF80000C 0x1F80000C 32 0xFFFEEFFF Held Held Held Bclk
#define BSC_WCR2_addr 0x1F80000C

//BSC WCR3 0xFF800010 0x1F800010 32 0x07777777 Held Held Held Bclk
#define BSC_WCR3_addr 0x1F800010

//BSC MCR 0xFF800014 0x1F800014 32 0x00000000 Held Held Held Bclk
#define BSC_MCR_addr 0x1F800014

//BSC PCR 0xFF800018 0x1F800018 16 0x0000 Held Held Held Bclk
#define BSC_PCR_addr 0x1F800018

//BSC RTCSR 0xFF80001C 0x1F80001C 16 0x0000 Held Held Held Bclk
#define BSC_RTCSR_addr 0x1F80001C

//BSC RTCNT 0xFF800020 0x1F800020 16 0x0000 Held Held Held Bclk
#define BSC_RTCNT_addr 0x1F800020

//BSC RTCOR 0xFF800024 0x1F800024 16 0x0000 Held Held Held Bclk
#define BSC_RTCOR_addr 0x1F800024

//BSC RFCR 0xFF800028 0x1F800028 16 0x0000 Held Held Held Bclk
#define BSC_RFCR_addr 0x1F800028

//BSC PCTRA 0xFF80002C 0x1F80002C 32 0x00000000 Held Held Held Bclk
#define BSC_PCTRA_addr 0x1F80002C

//BSC PDTRA 0xFF800030 0x1F800030 16 Undefined Held Held Held Bclk
#define BSC_PDTRA_addr 0x1F800030

//BSC PCTRB 0xFF800040 0x1F800040 32 0x00000000 Held Held Held Bclk
#define BSC_PCTRB_addr 0x1F800040

//BSC PDTRB 0xFF800044 0x1F800044 16 Undefined Held Held Held Bclk
#define BSC_PDTRB_addr 0x1F800044

//BSC GPIOIC 0xFF800048 0x1F800048 16 0x00000000 Held Held Held Bclk
#define BSC_GPIOIC_addr 0x1F800048

//BSC SDMR2 0xFF90xxxx 0x1F90xxxx 8 Write-only Bclk
#define BSC_SDMR2_addr 0x1F900000

//BSC SDMR3 0xFF94xxxx 0x1F94xxxx 8 Bclk
#define BSC_SDMR3_addr 0x1F940000

//DMAC module registers base
#define DMAC_BASE_addr 0x1FA00000

//DMAC SAR0 0xFFA00000 0x1FA00000 32 Undefined Undefined Held Held Bclk
#define DMAC_SAR0_addr 0x1FA00000

//DMAC DAR0 0xFFA00004 0x1FA00004 32 Undefined Undefined Held Held Bclk
#define DMAC_DAR0_addr 0x1FA00004

//DMAC DMATCR0 0xFFA00008 0x1FA00008 32 Undefined Undefined Held Held Bclk
#define DMAC_DMATCR0_addr 0x1FA00008

//DMAC CHCR0 0xFFA0000C 0x1FA0000C 32 0x00000000 0x00000000 Held Held Bclk
#define DMAC_CHCR0_addr 0x1FA0000C

//DMAC SAR1 0xFFA00010 0x1FA00010 32 Undefined Undefined Held Held Bclk
#define DMAC_SAR1_addr 0x1FA00010

//DMAC DAR1 0xFFA00014 0x1FA00014 32 Undefined Undefined Held Held Bclk
#define DMAC_DAR1_addr 0x1FA00014

//DMAC DMATCR1 0xFFA00018 0x1FA00018 32 Undefined Undefined Held Held Bclk
#define DMAC_DMATCR1_addr 0x1FA00018

//DMAC CHCR1 0xFFA0001C 0x1FA0001C 32 0x00000000 0x00000000 Held Held Bclk
#define DMAC_CHCR1_addr 0x1FA0001C

//DMAC SAR2 0xFFA00020 0x1FA00020 32 Undefined Undefined Held Held Bclk
#define DMAC_SAR2_addr 0x1FA00020

//DMAC DAR2 0xFFA00024 0x1FA00024 32 Undefined Undefined Held Held Bclk
#define DMAC_DAR2_addr 0x1FA00024

//DMAC DMATCR2 0xFFA00028 0x1FA00028 32 Undefined Undefined Held Held Bclk
#define DMAC_DMATCR2_addr 0x1FA00028

//DMAC CHCR2 0xFFA0002C 0x1FA0002C 32 0x00000000 0x00000000 Held Held Bclk
#define DMAC_CHCR2_addr 0x1FA0002C

//DMAC SAR3 0xFFA00030 0x1FA00030 32 Undefined Undefined Held Held Bclk
#define DMAC_SAR3_addr 0x1FA00030

//DMAC DAR3 0xFFA00034 0x1FA00034 32 Undefined Undefined Held Held Bclk
#define DMAC_DAR3_addr 0x1FA00034

//DMAC DMATCR3 0xFFA00038 0x1FA00038 32 Undefined Undefined Held Held Bclk
#define DMAC_DMATCR3_addr 0x1FA00038

//DMAC CHCR3 0xFFA0003C 0x1FA0003C 32 0x00000000 0x00000000 Held Held Bclk
#define DMAC_CHCR3_addr 0x1FA0003C

//DMAC DMAOR 0xFFA00040 0x1FA00040 32 0x00000000 0x00000000 Held Held Bclk
#define DMAC_DMAOR_addr 0x1FA00040

//CPG module registers base
#define CPG_BASE_addr 0x1FC00000

//CPG FRQCR 0xFFC00000 0x1FC00000 16  Held Held Held Pclk
#define CPG_FRQCR_addr 0x1FC00000

//CPG STBCR 0xFFC00004 0x1FC00004 8 0x00 Held Held Held Pclk
#define CPG_STBCR_addr 0x1FC00004

//CPG WTCNT 0xFFC00008 0x1FC00008 8/16 0x00 Held Held Held Pclk
#define CPG_WTCNT_addr 0x1FC00008

//CPG WTCSR 0xFFC0000C 0x1FC0000C 8/16 0x00 Held Held Held Pclk
#define CPG_WTCSR_addr 0x1FC0000C

//CPG STBCR2 0xFFC00010 0x1FC00010 8 0x00 Held Held Held Pclk
#define CPG_STBCR2_addr 0x1FC00010

//RTC module registers base
#define RTC_BASE_addr 0x1FC80000

//RTC R64CNT 0xFFC80000 0x1FC80000 8 Held Held Held Held Pclk
#define RTC_R64CNT_addr 0x1FC80000

//RTC RSECCNT 0xFFC80004 0x1FC80004 8 Held Held Held Held Pclk
#define RTC_RSECCNT_addr 0x1FC80004

//RTC RMINCNT 0xFFC80008 0x1FC80008 8 Held Held Held Held Pclk
#define RTC_RMINCNT_addr 0x1FC80008

//RTC RHRCNT 0xFFC8000C 0x1FC8000C 8 Held Held Held Held Pclk
#define RTC_RHRCNT_addr 0x1FC8000C

//RTC RWKCNT 0xFFC80010 0x1FC80010 8 Held Held Held Held Pclk
#define RTC_RWKCNT_addr 0x1FC80010

//RTC RDAYCNT 0xFFC80014 0x1FC80014 8 Held Held Held Held Pclk
#define RTC_RDAYCNT_addr 0x1FC80014

//RTC RMONCNT 0xFFC80018 0x1FC80018 8 Held Held Held Held Pclk
#define RTC_RMONCNT_addr 0x1FC80018

//RTC RYRCNT 0xFFC8001C 0x1FC8001C 16 Held Held Held Held Pclk
#define RTC_RYRCNT_addr 0x1FC8001C

//RTC RSECAR 0xFFC80020 0x1FC80020 8 Held  Held Held Held Pclk
#define RTC_RSECAR_addr 0x1FC80020

//RTC RMINAR 0xFFC80024 0x1FC80024 8 Held  Held Held Held Pclk
#define RTC_RMINAR_addr 0x1FC80024

//RTC RHRAR 0xFFC80028 0x1FC80028 8 Held  Held Held Held Pclk
#define RTC_RHRAR_addr 0x1FC80028

//RTC RWKAR 0xFFC8002C 0x1FC8002C 8 Held  Held Held Held Pclk
#define RTC_RWKAR_addr 0x1FC8002C

//RTC RDAYAR 0xFFC80030 0x1FC80030 8 Held  Held Held Held Pclk
#define RTC_RDAYAR_addr 0x1FC80030

//RTC RMONAR 0xFFC80034 0x1FC80034 8 Held  Held Held Held Pclk
#define RTC_RMONAR_addr 0x1FC80034

//RTC RCR1 0xFFC80038 0x1FC80038 8 0x00 0x00 Held Held Pclk
#define RTC_RCR1_addr 0x1FC80038

//RTC RCR2 0xFFC8003C 0x1FC8003C 8 0x09 0x00 Held Held Pclk
#define RTC_RCR2_addr 0x1FC8003C

//INTC module registers base
#define INTC_BASE_addr 0x1FD00000

//INTC ICR 0xFFD00000 0x1FD00000 16 0x0000 0x0000 Held Held Pclk
#define INTC_ICR_addr 0x1FD00000

//INTC IPRA 0xFFD00004 0x1FD00004 16 0x0000 0x0000 Held Held Pclk
#define INTC_IPRA_addr 0x1FD00004

//INTC IPRB 0xFFD00008 0x1FD00008 16 0x0000 0x0000 Held Held Pclk
#define INTC_IPRB_addr 0x1FD00008

//INTC IPRC 0xFFD0000C 0x1FD0000C 16 0x0000 0x0000 Held Held Pclk
#define INTC_IPRC_addr 0x1FD0000C

//TMU module registers base
#define TMU_BASE_addr 0x1FD80000

//TMU TOCR 0xFFD80000 0x1FD80000 8 0x00 0x00 Held Held Pclk
#define TMU_TOCR_addr 0x1FD80000

//TMU TSTR 0xFFD80004 0x1FD80004 8 0x00 0x00 Held 0x00 Pclk
#define TMU_TSTR_addr 0x1FD80004

//TMU TCOR0 0xFFD80008 0x1FD80008 32 0xFFFFFFFF 0xFFFFFFFF Held Held Pclk
#define TMU_TCOR0_addr 0x1FD80008

//TMU TCNT0 0xFFD8000C 0x1FD8000C 32 0xFFFFFFFF 0xFFFFFFFF Held Held Pclk
#define TMU_TCNT0_addr 0x1FD8000C

//TMU TCR0 0xFFD80010 0x1FD80010 16 0x0000 0x0000 Held Held Pclk
#define TMU_TCR0_addr 0x1FD80010

//TMU TCOR1 0xFFD80014 0x1FD80014 32 0xFFFFFFFF 0xFFFFFFFF Held Held Pclk
#define TMU_TCOR1_addr 0x1FD80014

//TMU TCNT1 0xFFD80018 0x1FD80018 32 0xFFFFFFFF 0xFFFFFFFF Held Held Pclk
#define TMU_TCNT1_addr 0x1FD80018

//TMU TCR1 0xFFD8001C 0x1FD8001C 16 0x0000 0x0000 Held Held Pclk
#define TMU_TCR1_addr 0x1FD8001C

//TMU TCOR2 0xFFD80020 0x1FD80020 32 0xFFFFFFFF 0xFFFFFFFF Held Held Pclk
#define TMU_TCOR2_addr 0x1FD80020

//TMU TCNT2 0xFFD80024 0x1FD80024 32 0xFFFFFFFF 0xFFFFFFFF Held Held Pclk
#define TMU_TCNT2_addr 0x1FD80024

//TMU TCR2 0xFFD80028 0x1FD80028 16 0x0000 0x0000 Held Held Pclk
#define TMU_TCR2_addr 0x1FD80028

//TMU TCPR2 0xFFD8002C 0x1FD8002C 32 Held Held Held Held Pclk
#define TMU_TCPR2_addr 0x1FD8002C

//SCI module registers base
#define SCI_BASE_addr 0x1FE00000

//SCI SCSMR1 0xFFE00000 0x1FE00000 8 0x00 0x00 Held 0x00 Pclk
#define SCI_SCSMR1_addr 0x1FE00000

//SCI SCBRR1 0xFFE00004 0x1FE00004 8 0xFF 0xFF Held 0xFF Pclk
#define SCI_SCBRR1_addr 0x1FE00004

//SCI SCSCR1 0xFFE00008 0x1FE00008 8 0x00 0x00 Held 0x00 Pclk
#define SCI_SCSCR1_addr 0x1FE00008

//SCI SCTDR1 0xFFE0000C 0x1FE0000C 8 0xFF 0xFF Held 0xFF Pclk
#define SCI_SCTDR1_addr 0x1FE0000C

//SCI SCSSR1 0xFFE00010 0x1FE00010 8 0x84 0x84 Held 0x84 Pclk
#define SCI_SCSSR1_addr 0x1FE00010

//SCI SCRDR1 0xFFE00014 0x1FE00014 8 0x00 0x00 Held 0x00 Pclk
#define SCI_SCRDR1_addr 0x1FE00014

//SCI SCSCMR1 0xFFE00018 0x1FE00018 8 0x00 0x00 Held 0x00 Pclk
#define SCI_SCSCMR1_addr 0x1FE00018

//SCI SCSPTR1 0xFFE0001C 0x1FE0001C 8 0x00 0x00 Held 0x00*2Pclk
#define SCI_SCSPTR1_addr 0x1FE0001C

//SCIF module registers base
#define SCIF_BASE_addr 0x1FE80000

//SCIF SCSMR2 0xFFE80000 0x1FE80000 16 0x0000 0x0000 Held Held Pclk
#define SCIF_SCSMR2_addr 0x1FE80000

//SCIF SCBRR2 0xFFE80004 0x1FE80004 8 0xFF 0xFF Held Held Pclk
#define SCIF_SCBRR2_addr 0x1FE80004

//SCIF SCSCR2 0xFFE80008 0x1FE80008 16 0x0000 0x0000 Held Held Pclk
#define SCIF_SCSCR2_addr 0x1FE80008

//SCIF SCFTDR2 0xFFE8000C 0x1FE8000C 8 Undefined Undefined Held Held Pclk
#define SCIF_SCFTDR2_addr 0x1FE8000C

//SCIF SCFSR2 0xFFE80010 0x1FE80010 16 0x0060 0x0060 Held Held Pclk
#define SCIF_SCFSR2_addr 0x1FE80010

//SCIF SCFRDR2 0xFFE80014 0x1FE80014 8 Undefined Undefined Held Held Pclk
#define SCIF_SCFRDR2_addr 0x1FE80014

//SCIF SCFCR2 0xFFE80018 0x1FE80018 16 0x0000 0x0000 Held Held Pclk
#define SCIF_SCFCR2_addr 0x1FE80018

//SCIF SCFDR2 0xFFE8001C 0x1FE8001C 16 0x0000 0x0000 Held Held Pclk
#define SCIF_SCFDR2_addr 0x1FE8001C

//SCIF SCSPTR2 0xFFE80020 0x1FE80020 16 0x0000 0x0000 Held Held Pclk
#define SCIF_SCSPTR2_addr 0x1FE80020

//SCIF SCLSR2 0xFFE80024 0x1FE80024 16 0x0000 0x0000 Held Held Pclk
#define SCIF_SCLSR2_addr 0x1FE80024

//UDI module registers base
#define UDI_BASE_addr 0x1FF00000

//UDI SDIR 0xFFF00000 0x1FF00000 16 0xFFFF Held Held Held Pclk
#define UDI_SDIR_addr 0x1FF00000

//UDI SDDR 0xFFF00008 0x1FF00008 32 Held Held Held Held Pclk
#define UDI_SDDR_addr 0x1FF00008

//For mem mapping
void map_area7_init();
void map_area7(u32 base);
void map_p4();