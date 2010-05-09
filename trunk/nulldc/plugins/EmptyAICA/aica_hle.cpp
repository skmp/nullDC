//8/7/2006
//since i got no internet , and im bored , im starting to work on some aica hle code :)
//Many thanks to ZeZu & CyRus 64 , most of the code here is based/derived from Icarus hle :)

#include "aica_hle.h"
_audio_drv_header *aud_drv;
bool arm7_on;

void InitHLE()
{
	aud_drv=(struct _audio_drv_header *)(aica_ram + 0x20);
}
void ResetHLE()
{
}
void TermHLE()
{
}
u32 GetKatanaDriverVersion()
{
	return (aud_drv->sndVersion<<8) | (aud_drv->sndMinorVersion);
}
void ARM_Katana_Driver_Info()
{
	if (aud_drv->sndVersion>30 || GetKatanaDriverVersion()==0)
	{
		printf("<acHLE>:Invalid Katana Driver Version (%x)\n",GetKatanaDriverVersion());
		return;
	}

	_audio_drv_strings drv_str;
	
	memcpy((char *)&drv_str.sndLocalVersion,(char *)&aud_drv->sndLocalVersion,sizeof(drv_str.sndLocalVersion));
	drv_str.sndLocalVersion[1]=0x0;

	memcpy((char *)&drv_str.prodStage,(char *)&aud_drv->prodStage,sizeof(drv_str.prodStage));
	drv_str.prodStage[4]=0x0;

	memcpy((char *)&drv_str.sndCopyrightInfo,(char *)&aud_drv->sndCopyrightInfo,sizeof(drv_str.sndCopyrightInfo));
	drv_str.sndCopyrightInfo[32]=0x0;

	memcpy((char *)&drv_str.sndAuthorInfoDate,(char *)&aud_drv->sndAuthorInfoDate,sizeof(drv_str.sndAuthorInfoDate));
	drv_str.sndAuthorInfoDate[10]=0x0;

	memcpy((char *)&drv_str.sndAuthorInfoName,(char *)&aud_drv->sndAuthorInfoName,sizeof(drv_str.sndAuthorInfoName));
	drv_str.sndAuthorInfoName[38]=0x0;

	memcpy((char *)&drv_str.sndModifierInfo,(char *)&aud_drv->sndModifierInfo,sizeof(drv_str.sndModifierInfo));
	drv_str.sndModifierInfo[48]=0x0;

	memcpy((char *)&drv_str.sndModifierInfo2,(char *)&aud_drv->sndModifierInfo2,sizeof(drv_str.sndModifierInfo2));
	drv_str.sndModifierInfo2[16]=0x0;

	printf("*************AICA Audio driver (version: %d,%d local %s)************\n",aud_drv->sndVersion,aud_drv->sndMinorVersion,drv_str.sndLocalVersion);
	printf("Name: %s (%s)\n",drv_str.sndModifierInfo,drv_str.sndModifierInfo2);
	printf("Authors: %s\n",drv_str.sndAuthorInfoName);
	printf("Date: %s\n",drv_str.sndAuthorInfoDate);
	printf("Copyright: %s\n",drv_str.sndCopyrightInfo);
	printf("Stage: %s\n",drv_str.prodStage);
	printf("sndToolInterfaceWork: 0x%08x\n",aud_drv->sndToolInterfaceWork);
	printf("hostSeqStatus: 0x%08x\n",aud_drv->hostSeqStatus);
	printf("sndSystemStatus: 0x%08x\n",aud_drv->sndSystemStatus);
	printf("seqModuleMap: 0x%08x\n",aud_drv->seqModuleMap);
	printf("cmdBuffer: 0x%08x\n",aud_drv->cmdBuffer);
	printf("cmdHistoryBuffer: 0x%08x\n",aud_drv->cmdHistoryBuffer);
	printf("sndManbowLibraryWork: 0x%08x\n",aud_drv->sndManbowLibraryWork);
	printf("sndManbowLibraryWorkSize: 0x%08x\n",aud_drv->sndManbowLibraryWorkSize);
	printf("********************************END********************************\n");
}

bool HleEnabled()
{
	return  (aud_drv->sndVersion<30 && GetKatanaDriverVersion()!=0);
}
void Hle_process()
{
	return ;
	/*
	if (!HleEnabled() || arm7_on==false)
		return;

	u32* cmd_start= (u32*)&aica_ram[aud_drv->cmdBuffer & AICA_MEM_MASK];

	//u32 sz=aud_drv->
	while (*cmd_start!=0 && *cmd_start!=0xFFFFFFFF )
	//for (int i=0;i<64;i++)
	{
		char* data=(char*)cmd_start;

		//if (*cmd_start>10)
		*cmd_start=0xFFFFFFFF;
		cmd_start++;
	}*/
}