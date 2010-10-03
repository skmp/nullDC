#include "cdi.h"
#define BYTE u8
#define WORD u16
#define DWORD u32
#include "pfctoc.h"

struct CDIDisc : Disc
{
	CDIDisc(SPfcToc* pstToc,const wchar* file)
	{
		printf("\n--CDI toc info start--\n");

		printf("Last Sector : %d\n",pstToc->dwOuterLeadOut);
		printf("Session count : %d\n",pstToc->dwSessionCount);

		u32 last_FAD=0;
		u32 TrackOffset=0;
		bool CD_M1=false,CD_M2=false,CD_DA=false;

		u32 ses_count=0;
		u32 track=0;
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
					Session s;
					s.StartFAD=last_FAD;
					s.FirstTrack=track+1;
					sessions.push_back(s);
					printf("  Session start FAD: %d\n",last_FAD);
				}

				verify(cdi_track->dwIndexCount==2);
				printf("  track %d:\n",t);
				printf("    Type : %d\n",cdi_track->bMode);

				Track trk;
				
				if (cdi_track->bMode==2)
					CD_M2=true;
				if (cdi_track->bMode==1)
					CD_M1=true;
				if (cdi_track->bMode==0)
					CD_DA=true;



				trk.ADDR=1;//hmm is that ok ?
				trk.CTRL=cdi_track->bCtrl;
				trk.StartFAD=last_FAD;
				trk.EndFAD=0;
				trk.file = new RawTrackFile(_wfopen(file,L"rb"),TrackOffset,last_FAD,cdi_track->dwBlockSize);
				
				tracks.push_back(trk);
				printf("    Start FAD : %d\n",trk.StartFAD);
				//printf("    SectorSize : %d\n",Track[track].SectorSize);
				//printf("    File Offset : %d\n",Track[track].Offset);

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

		type=GuessDiscType(CD_M1,CD_M2,CD_DA);

		
		this->EndFAD=pstToc->dwOuterLeadOut;

		LeadOut.StartFAD=pstToc->dwOuterLeadOut;
		LeadOut.ADDR=0;
		LeadOut.CTRL=0;

		printf("Disc Type = %d\n",type);
		
		printf("--CDI toc info end--\n\n");
	}
};

Disc* cdi_parse(wchar* file_)
{
	Disc* rv=0;
	char file[512];
	wcstombs(file,file_,512);
	SPfcToc* pstToc;
	HMODULE pfctoc_mod=NULL;
	pfctoc_mod=LoadLibrary(L"plugins\\pfctoc.dll");
	if (pfctoc_mod==NULL)
		pfctoc_mod=LoadLibrary(L"pfctoc.dll");
	if(pfctoc_mod==NULL)
		return false;

	PfcFreeTocFP* PfcFreeToc;
	PfcGetTocFP*  PfcGetToc;

	PfcFreeToc=(PfcFreeTocFP*)GetProcAddress(pfctoc_mod,"PfcFreeToc");
	PfcGetToc=(PfcGetTocFP*)GetProcAddress(pfctoc_mod,"PfcGetToc");
	verify(PfcFreeToc!=NULL && PfcGetToc!=NULL);

	DWORD dwSize;//
	DWORD dwErr = PfcGetToc(file, pstToc, dwSize);
    if (dwErr == PFCTOC_OK) 
	{
		rv= new CDIDisc(pstToc,file_);
    }

	if (pstToc)
		PfcFreeToc(pstToc);
	if (pfctoc_mod)
		FreeLibrary(pfctoc_mod);
	pstToc=0;
	pfctoc_mod=0;

	return rv;
}