#include "cdi.h"
#define BYTE u8
#define WORD u16
#define DWORD u32
#include "pfctoc.h"

PfcFreeTocFP* PfcFreeToc;
PfcGetTocFP*  PfcGetToc;

// int main(int, char*)
// {
//   SPfcToc* pstToc;
//   DWORD dwSize;
// 
//   DWORD dwErr = PfcGetToc(_T("C:\\TEMP\\IMAGE1.CDI"), pstToc, dwSize);
//   if (dwErr == PFCTOC_OK) {
//     assert(IsBadReadPtr(pstToc, dwSize) == FALSE);
// 
//     //
//     // Do something with the TOC
//     //
// 
//     dwErr = PfcFreeToc(pstToc);
//   }
// 
//   return ((int)dwErr);

SPfcToc* pstToc;
SessionInfo cdi_ses;
TocInfo cdi_toc;
DiscType cdi_Disctype;

struct file_TrackInfo
{
	u32 FAD;
	u32 Control;
	u32 Offset;
	u32 SectorSize;
};

file_TrackInfo Track[101];

u32 TrackCount;

u8 SecTemp[2448];
FILE* fp_cdi;
void cdi_ReadSSect(u8* p_out,u32 sector,u32 secsz)
{
	for (u32 i=0;i<TrackCount;i++)
	{
		if (Track[i+1].FAD>sector)
		{
			verify(sizeof(SecTemp)>=Track[i].SectorSize);
			u32 fad_off=sector-Track[i].FAD;
			fseek(fp_cdi,Track[i].Offset+fad_off*Track[i].SectorSize,SEEK_SET);
			fread(SecTemp,Track[i].SectorSize,1,fp_cdi);

			ConvertSector(SecTemp,p_out,Track[i].SectorSize,secsz,sector);
			break;
		}
	}
}
void FASTCALL cdi_DriveReadSector(u8 * buff,u32 StartSector,u32 SectorCount,u32 secsz)
{
	//printf("GDR->Read : Sector %d , size %d , mode %d \n",StartSector,SectorCount,secsz);
	while(SectorCount--)
	{
		cdi_ReadSSect(buff,StartSector,secsz);
		buff+=secsz;
		StartSector++;
	}
}

void cdi_CreateToc()
{
	//clear structs to 0xFF :)
	memset(Track,0xFF,sizeof(Track));
	memset(&cdi_ses,0xFF,sizeof(cdi_ses));
	memset(&cdi_toc,0xFF,sizeof(cdi_toc));

	printf("\n--GD toc info start--\n");
	int track=0;
	bool CD_DA=false;
	bool CD_M1=false;
	bool CD_M2=false;



	printf("Last Sector : %d\n",pstToc->dwOuterLeadOut);
	printf("Session count : %d\n",pstToc->dwSessionCount);

	cdi_toc.FistTrack=1;
	u32 last_FAD=0;
	u32 TrackOffset=0;

	u32 ses_count=0;
	for (u32 s=0;s<pstToc->dwSessionCount;s++)
	{
		printf("Session %d:\n",s);
		SPfcSession* ses=&pstToc->pstSession[s];
		if (ses->bType==4 && ses->dwTrackCount==0)
		{
			printf("Detected open disc\n");
			continue;
		}
		ses_count++;

		printf("  Track Count: %d\n",ses->dwTrackCount);
		for (u32 t=0;t< ses->dwTrackCount ;t++)
		{
			SPfcTrack* cdi_track=&ses->pstTrack[t];

			//pre gap
			last_FAD	+=cdi_track->pdwIndex[0];
			TrackOffset	+=cdi_track->pdwIndex[0]*cdi_track->dwBlockSize;

			if (t==0)
			{
				cdi_ses.SessionFAD[s]=last_FAD;
				cdi_ses.SessionStart[s]=track+1;
				printf("  Session start FAD: %d\n",last_FAD);
			}

			verify(cdi_track->dwIndexCount==2);
			printf("  track %d:\n",t);
			printf("    Type : %d\n",cdi_track->bMode);

			if (cdi_track->bMode==2)
				CD_M2=true;
			if (cdi_track->bMode==1)
				CD_M1=true;
			if (cdi_track->bMode==0)
				CD_DA=true;
			
			

			cdi_toc.tracks[track].Addr=1;//hmm is that ok ?
			cdi_toc.tracks[track].Session=s;
			cdi_toc.tracks[track].Control=cdi_track->bCtrl;
			cdi_toc.tracks[track].FAD=last_FAD;


			Track[track].FAD=cdi_toc.tracks[track].FAD;
			Track[track].Control=cdi_toc.tracks[track].Control;
			Track[track].SectorSize=cdi_track->dwBlockSize;
			Track[track].Offset=TrackOffset;
			printf("    Start FAD : %d\n",Track[track].FAD);
			printf("    SectorSize : %d\n",Track[track].SectorSize);
			printf("    File Offset : %d\n",Track[track].Offset);

			printf("    %d indexes \n",cdi_track->dwIndexCount);
			for (u32 i=0;i<cdi_track->dwIndexCount;i++)
			{
				printf("     index %d : %d\n",i,cdi_track->pdwIndex[i]);
			}
			//main track data
			TrackOffset+=(cdi_track->pdwIndex[1])*cdi_track->dwBlockSize;
			last_FAD+=cdi_track->pdwIndex[1];
			track++;
		}
		last_FAD+=11400-150;///next session
	}

	if ((CD_M1==true) && (CD_DA==false) && (CD_M2==false))
		cdi_Disctype = CdRom;
	else if (CD_M2)
		cdi_Disctype = CdRom_XA;
	else if (CD_DA && CD_M1) 
		cdi_Disctype = CdRom_Extra;
	else
		cdi_Disctype=CdRom;//hmm?

	cdi_ses.SessionCount=ses_count;
	cdi_ses.SessionsEndFAD=pstToc->dwOuterLeadOut;
	cdi_toc.LeadOut.FAD=pstToc->dwOuterLeadOut;
	cdi_toc.LeadOut.Addr=0;
	cdi_toc.LeadOut.Control=0;
	cdi_toc.LeadOut.Session=0;

	printf("Disc Type = %d\n",cdi_Disctype);
	TrackCount=track;
	cdi_toc.LastTrack=track;
	printf("--GD toc info end--\n\n");
}

HMODULE pfctoc_mod=NULL;
bool cdi_init(wchar* file_)
{
	char file[512];
	wcstombs(file,file_,512);
	pfctoc_mod=LoadLibrary(L"plugins\\pfctoc.dll");
	if (pfctoc_mod==NULL)
		pfctoc_mod=LoadLibrary(L"pfctoc.dll");
	if(pfctoc_mod==NULL)
		return false;

	PfcFreeToc=(PfcFreeTocFP*)GetProcAddress(pfctoc_mod,"PfcFreeToc");
	PfcGetToc=(PfcGetTocFP*)GetProcAddress(pfctoc_mod,"PfcGetToc");
	verify(PfcFreeToc!=NULL && PfcFreeToc!=NULL);

	//char fn[512]="";
	//GetFile(fn,"cdi images (*.cdi) \0*.cdi\0\0");
	DWORD dwSize;//
	DWORD dwErr = PfcGetToc(file, pstToc, dwSize);
    if (dwErr == PFCTOC_OK) 
	{
		cdi_CreateToc();
    }
	else
	{
		return false;
		//printf("Failed to open file , %d",dwErr);
	}
	fp_cdi=fopen(file,"rb");

	return true;
}

void cdi_term()
{
	if (pstToc)
		PfcFreeToc(pstToc);
	if (pfctoc_mod)
		FreeLibrary(pfctoc_mod);
	pstToc=0;
	pfctoc_mod=0;
}

u32 FASTCALL cdi_DriveGetDiscType()
{
	return cdi_Disctype;
}
void cdi_DriveGetTocInfo(TocInfo* toc,DiskArea area)
{
	verify(area==SingleDensity);
	memcpy(toc,&cdi_toc,sizeof(TocInfo));
}
void cdi_GetSessionsInfo(SessionInfo* sessions)
{
	memcpy(sessions,&cdi_ses,sizeof(SessionInfo));
}