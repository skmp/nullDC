#include "iso9660.h"

bool inbios=true;
FILE* f_1=0;
FILE* f_2=0;

u8 isotemshit[5000];
struct file_TrackInfo
{
	FILE* f;
	u32 FAD;
	u32 SectorSize;
	u32 ctrl;
	s32 offset;

	bool ReadSector(u8 * buff,u32 sector,u32 secsz)
	{
		if (sector>=FAD)
		{ 
			if (SectorSize==0)
				printf("Read from missing sector %d\n",sector);
			else
			{
				u8* ptr=isotemshit;
				s32 off2=(sector-FAD)*SectorSize + offset;
				if (off2>=0)
				{
					fseek(f,off2,SEEK_SET);
					fread(ptr,SectorSize,1,f);
				}
				else
				{
					fseek(f,0,SEEK_SET);
					fread(ptr,SectorSize-off2,1,f);
					ptr+=off2;
				}
			//	printf("readed %d bytes from file 0x%X , converting to %d [sec %d]\n",
			//		SectorSize,f,secsz,sector);
				ConvertSector(ptr,buff,SectorSize,secsz,sector);
				if (sector==45000)
					PatchRegion_0(buff,secsz);
				if (sector==45006)
					PatchRegion_6(buff,secsz);
			}
			return true;
		}
		return false;
	}
};

file_TrackInfo iso_tracks[101];
TocInfo gdi_toc;
SessionInfo gdi_ses;
u32 iso_tc=0;

void iso_ReadSSect(u8* p_out,u32 sector,u32 secsz)
{
	for (s32 i=(s32)iso_tc-1;i>=0;i--)
	{
		if (iso_tracks[i].ReadSector(p_out,sector,secsz))
			break;
	}
}


void rss(u8* buff,u32 ss,FILE* file)
{
	fseek(file,ss*2352+0x10,SEEK_SET);
	fread(buff,2048,1,file);
}
void FASTCALL iso_DriveReadSector(u8 * buff,u32 StartSector,u32 SectorCount,u32 secsz)
{
	//printf("GDR->Read : Sector %d , size %d , mode %d \n",StartSector,SectorCount,secsz);
	if (StartSector>150)
		StartSector-=150;
	while(SectorCount--)
	{
		iso_ReadSSect(buff,StartSector,secsz);
		buff+=secsz;
		StartSector++;
	}
	return;
	/*
	if (f_1==0)
	{
		f_1 = fopen("D:/Tekaman/Games/Dreamcast RAW/thps/45000to63139.bin","rb");
		f_2 = fopen("D:/Tekaman/Games/Dreamcast RAW/thps/69313to549150.bin","rb");
	}
	
	if (StartSector>=69463)
	{
		StartSector-=69463;
		for (u32 i = 0 ; i < SectorCount;i++)
		{
			rss(buff,StartSector+i,f_2);
			buff+=2048;
		}
	}
	else if (StartSector>=63438 )
	{
		StartSector-=63438 ;
		for (u32 i = 0 ; i < SectorCount;i++)
		{
			printf("Unable to read %d\n",StartSector+i); 
			buff+=2048;
		}
	}
	else if (StartSector>=45150 )
	{
		StartSector-=45150 ;
		for (u32 i = 0 ; i < SectorCount;i++)
		{
			rss(buff,StartSector+i,f_1);
			buff+=2048;
		}
	}
	else
	{
		for (u32 i = 0 ; i < SectorCount;i++)
		{
			printf("Unable to read %d\n",StartSector+i); 
			buff+=2048;
		}
	}*/
	/*
	if (StartSector>=45000)
	{
		if (inbios)
		{
			StartSector-=45000;
			if (StartSector==150+16)
				inbios=false;//bios dma'd pvd
		}
	}
	else
	{
		if (inbios)
			inbios=false;//prop not bios/bios after pvd
	}

	if (StartSector>=150)
		StartSector-=150;


	if (StartSector<16)
	{
		FILE* fip=fopen("c:\\ip.bin","rb");
		fseek(fip,StartSector*2048,SEEK_SET);
		size_t rd=fread(buff,1,SectorCount*2048,fip);
		fclose(fip);
		return;
	}*/
	/*
	StartSector-=45150;
	if (f_iso)
	{
		fseek(f_iso,StartSector*2048,SEEK_SET);
		size_t rd=fread(buff,1,SectorCount*2048,f_iso);
		if (rd!=SectorCount*2048)
		{
			printf("fread  failed ; managed to read %d sectors olny (%d bytes)\n",rd/2048,rd);
			getc(stdin);
		}
	}
	else
	{
		char fn[512]="";
		if(GetFile(fn,"FILE0003.DUP \0FILE0003.DUP\0\0")==false)
			return;
		f_iso=fopen(fn,"rb");
		
		fseek(f_iso,StartSector*2048,SEEK_SET);
		size_t rd=fread(buff,1,SectorCount*2048,f_iso);
		if (rd!=SectorCount*2048)
		{
			printf("fread  failed ; managed to read %d sectors olny (%d bytes)\n",rd/2048,rd);
			getc(stdin);
		}
	}*/

}
/*
void iso_GetSessionsInfo(SessionInfo* sessions)
{
	printf("iso_GetSessionsInfo\n");
}
void iso_DriveGetTocInfo(TocInfo* toc,DiskArea area)
{
	printf("GDROM toc\n");
	memset(toc,0,sizeof(TocInfo));
}
*/
void iso_DriveGetTocInfo(TocInfo* toc,DiskArea area)
{
	memcpy(toc,&gdi_toc,sizeof(TocInfo));
	if(area==SingleDensity)
	{
		toc->LeadOut.FAD=13085;
		toc->FistTrack=1;
		toc->LastTrack=2;

		for (int tr=2;tr<99;tr++)
		{
			toc->tracks[tr].FAD=0xFFFFFFF;
			toc->tracks[tr].Addr=0xFF;
			toc->tracks[tr].Control=0xFF;
		}
	}
	else
	{
		toc->LeadOut.FAD=549300;
		toc->FistTrack=3;
		toc->LastTrack=gdi_toc.LastTrack;
		
		for (int tr=0;tr<2;tr++)
		{
			toc->tracks[tr].FAD=0xFFFFFFF;
			toc->tracks[tr].Addr=0xFF;
			toc->tracks[tr].Control=0xFF;
		}
	}
}
void iso_GetSessionsInfo(SessionInfo* sessions)
{
	memcpy(sessions,&gdi_ses,sizeof(SessionInfo));
}
//TODO : fix up
u32 FASTCALL iso_DriveGetDiscType()
{
	if (iso_tc==0)
		return NoDisk;
	else
		return GdRom;
} 

bool load_gdi(wchar* file_)
{
	char file[512];
	wcstombs(file,file_,512);
	memset(&gdi_toc,0xFFFFFFFF,sizeof(gdi_toc));
	memset(&gdi_ses,0xFFFFFFFF,sizeof(gdi_ses));
	FILE* t=fopen(file,"rb");
	fscanf(t,"%d\r\n",&iso_tc);
	printf("\nGDI : %d tracks\n",iso_tc);

	char temp[512];
	char path[512];
	strcpy(path,file);
	size_t len=strlen(file);
	while (len>2)
	{
		if (path[len]=='\\')
			break;
		len--;
	}
	len++;
	char* pathptr=&path[len];
	u32 TRACK=0,FADS=0,CTRL=0,SSIZE=0;
	s32 OFFSET=0;
	for (u32 i=0;i<iso_tc;i++)
	{
		
		//TRACK FADS CTRL SSIZE file OFFSET
		
		fscanf(t,"%d %d %d %d %s %d\r\n",&TRACK,&FADS,&CTRL,&SSIZE,temp,&OFFSET);
		printf("file %s[%d] : FAD:%d,CTRL : %d, SSIZE :%d,OFFSET:%d\n",temp,TRACK,FADS,CTRL,SSIZE,OFFSET);
		
		if (SSIZE!=0)
		{
			strcpy(pathptr,temp);
			iso_tracks[i].f=fopen(path,"rb");
		}

		iso_tracks[i].FAD=FADS;
		iso_tracks[i].offset=OFFSET;
		iso_tracks[i].SectorSize=SSIZE;
		iso_tracks[i].ctrl=CTRL;

		gdi_toc.tracks[i].Addr=0;
		gdi_toc.tracks[i].Control=CTRL;
		gdi_toc.tracks[i].FAD=FADS+150;
	}

	
	gdi_toc.LastTrack=iso_tc;

	gdi_ses.SessionCount=2;
	//session 1 : start @ track 1, and its fad
	gdi_ses.SessionStart[0]=1;
	gdi_ses.SessionFAD[0]=gdi_toc.tracks[0].FAD;
	

	//session 2 : start @ track 3, and its fad
	gdi_ses.SessionStart[0]=3;
	gdi_ses.SessionFAD[1]=gdi_toc.tracks[2].FAD;
	gdi_ses.SessionsEndFAD=549300;

	return true;
}
bool iso_init(wchar* file)
{
	size_t len=wcslen(file);
	if (len>4)
	{
		if (_tcsicmp( &file[len-4],L".gdi")==0)
		{
			return load_gdi(file);
		}
	}
	return false;
}

void iso_term()
{
}