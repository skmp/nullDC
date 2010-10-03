#pragma once
#include "EmptyAICA.h"
#include "aica_hax.h"

void ARM_Katana_Driver_Info();

struct _audio_drv_header 
{
	u8 sndVersion;
	u8 sndMinorVersion;
	u8 sndLocalVersion;
	u8 reserved;	/* 0x00 */
	u8 prodStage[4]; /* 'alpha','beta',' GM ' */
	u32 sndUniqueId1;
	u32 sndUniqueId2;
	/* 0x10 */
	u8 sndCopyrightInfo[32];
	/* 0x30 */
	u8 sndAuthorInfoDate[10];
	u8 sndAuthorInfoName[38];
	u8 sndModifierInfo[48];
	u8 sndModifierInfo2[16];
	/* 0xa0 */
	u32 sndToolInterfaceWork;	/* tool interface area top address */
	u32 hostSeqStatus;			/* sound status area top address */
	u32 sndSystemStatus;		/* system status area top address */
	u32 seqModuleMap;			/* map area top address */
	u32 cmdBuffer;				/* command buffer area top address */
	u32 cmdHistoryBuffer;		/* command history area top address */
	u32 sndManbowLibraryWork;
	u32 sndManbowLibraryWorkSize;
	/* 0xc0 */
	/* ALL set to zero unless msapi is being used */
	u32 DSoundPortStatus;		/* x 16 byte size */
	u32 DSoundPortInt;			
	u32 DSoundReadAddress;
	u32 null1;
	/* 0xd4 */
	/* ALL zero */
	u32 null2;
	u32 null3;
	u32 null4;
};

struct _audio_drv_strings
{
	u8 sndLocalVersion[2];
	u8 prodStage[5];
	u8 sndCopyrightInfo[33];
	u8 sndAuthorInfoDate[11];
	u8 sndAuthorInfoName[39];
	u8 sndModifierInfo[49];
	u8 sndModifierInfo2[17];
};

/*
sndVersionInfo
		DCB	sndVersion			;maijor version
		DCB	sndMinorVersion			;minor version
		DCB	sndLocalVersion			;local version
		DCB	0x00				;- reserved -
		DCB	" GM "				;alph/beta/GM
sndUniqueId
		DCD	0x00000000			;
		DCD	0x00000000			;
	;------- 0x30 -
sndCopyrightInfo
		DCB	"1998,(C)SEGA ENTERPRISES        "
	;------- 0x50 -
sndAuthorInfo
		DCB	"199",sndYear,"."
		DCB	sndMonth1,sndMonth2,'.'
		DCB	sndDate1,sndDate2
	[	msapi=on
		DCB	":Y.Kashima / K.Suyama / Tom Miley     "
	|
		DCB	":DigitalMedia :Y.Kashima / K.Suyama   "
	]
sndModifierInfo
		DCB	"Version 1.90h.2 Midi/DA Hybrid                  "	;48 bytes
		DCB	"                "	;16 bytes
	;------- 0xc0 -
sndToolIfAreaInfo
		DCD	sndToolInterfaceWork		;tool interface area top address
sndStatusAreaInfo
		DCD	hostSeqStatus			;sound status area top address
sndSystemStatAreaInfo
		DCD	sndSystemStatus			;system status area top address
sndMapAreaInfo
		DCD	seqModuleMap			;map area top address
sndCommandAreaInfo
		DCD	cmdBuffer			;command buffer area top address
sndCmdHistoryAreaInfo
		DCD	cmdHistoryBuffer		;command history area top address
sndManbowLibrayWorkAreaInfo
		DCD	sndManbowLibraryWork		;
		DCD	sndManbowLibraryWorkSize	;
	;------- 0xe0 -
	[	msapi=on	;------- ms api mode
DSoundPortStatusPtr
		DCD	DSoundPortStatus		;x 16 byte size
DSoundPortIntPtr
		DCD	DSoundPortInt
DSoundReadAddressPtr	
		DCD	DSoundReadAddress
sndMemoryFreeArea
		DCD	0x00000000
		DCD DMusicReadAddress
	|			;------- normal mode
		DCD	0x00000000
		DCD	0x00000000
		DCD	0x00000000
		DCD	0x00000000
		DCD	0x00000000
	]			;------
	;------- 0xf4 -
		DCD	0x00000000
		DCD	0x00000000
sndMidiIOFlag
		DCD	0x00000000
*/

extern _audio_drv_header *aud_drv;

void InitHLE();
void ResetHLE();
void TermHLE();
u32 GetKatanaDriverVersion();
bool HleEnabled();
void Hle_process();
void ARM_Katana_Driver_Info();
extern bool arm7_on;