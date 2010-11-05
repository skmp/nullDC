#include "iso9660.h"

Disc* load_gdi(wchar* file_)
{
	char file[512];
	int iso_tc;
	Disc* disc = new Disc();
	
	wcstombs(file,file_,512);
	//memset(&gdi_toc,0xFFFFFFFF,sizeof(gdi_toc));
	//memset(&gdi_ses,0xFFFFFFFF,sizeof(gdi_ses));
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
		fscanf(t,"%d %d %d %d",&TRACK,&FADS,&CTRL,&SSIZE);
		//%s %d\r\n,temp,&OFFSET);
		//disc->tracks.push_back(
		while(iswspace(fgetc(t))) ;
		fseek(t,-1,SEEK_CUR);
		if (fgetc(t)=='"')
		{
			char c;
			int i=0;
			while((c=fgetc(t))!='"')
				temp[i++]=c;
			temp[i]=0;
		}
		else
		{
			fseek(t,-1,SEEK_CUR);
			fscanf(t,"%s",temp);
		}

		fscanf(t,"%d\r\n",&OFFSET);
		printf("file[%d] \"%s\": FAD:%d, CTRL:%d, SSIZE:%d, OFFSET:%d\n",TRACK,temp,FADS,CTRL,SSIZE,OFFSET);
		
		

		Track t;
		t.ADDR=0;
		t.CTRL=CTRL;
		t.StartFAD=FADS+150;
		t.EndFAD=0;		//fill it in
		t.file=0;

		if (SSIZE!=0)
		{
			strcpy(pathptr,temp);
			t.file = new RawTrackFile(fopen(path,"rb"),OFFSET,t.StartFAD,SSIZE);	
		}
		disc->tracks.push_back(t);
	}

	

	Session ses;

	//session 1 : start @ track 1, and its fad
	ses.FirstTrack=1;
	ses.StartFAD=disc->tracks[0].StartFAD;
	disc->sessions.push_back(ses);

	//session 2 : start @ track 3, and its fad
	ses.FirstTrack=3;
	ses.StartFAD=disc->tracks[2].StartFAD;
	disc->sessions.push_back(ses);

	//this isn't always true for gdroms, depens on area look @ the get-toc code
	disc->type=GdRom;
	disc->LeadOut.ADDR=0;
	disc->LeadOut.CTRL=0;
	disc->LeadOut.StartFAD=549300;

	disc->EndFAD=549300;

	return disc;
}


Disc* gdi_parse(wchar* file)
{
	size_t len=wcslen(file);
	if (len>4)
	{
		if (_tcsicmp( &file[len-4],L".gdi")==0)
		{
			return load_gdi(file);
		}
	}
	return 0;
}

void iso_term()
{
}