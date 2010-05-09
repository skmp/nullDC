#pragma once
#include "gd_driver.h"
extern u32 NullDriveDiscType;
struct TocTrackInfo
{
	u32 FAD;	//fad , intel format
	u8 Control;	//cotnrol info
	u8 Addr;	//addr info
	u8 Session; //Session where teh track belongs
};
struct TocInfo
{
	//0-98 ->1-99
	TocTrackInfo tracks[99];

	u8 FistTrack;
	u8 LastTrack;

	TocTrackInfo LeadOut;	//session set to 0 on that one
};

struct SessionInfo
{
	u32 SessionsEndFAD;	//end of Disc (?)
	u8 SessionCount;	//must be at least 1
	u32 SessionStart[99];//start track for session
	u32 SessionFAD[99];	//for sessions 1-99 ;)
};



typedef bool IsCompatableFileFP();
typedef bool InitFP(wchar* file);
typedef void TermFP();
typedef void DriveGetTocInfoModFP(TocInfo* toc,DiskArea area);
typedef void DriveGetSessionInfoModFP(SessionInfo* ses);

enum gd_drivers
{
	none=-1,
	Iso=0,
	cdi=1,
	mds=2,
};

struct DriveIF
{
	DriveReadSectorFP*  ReadSector;
	DriveGetTocInfoModFP*  GetToc;
	DriveGetDiscTypeFP* GetDiscType;
	DriveGetSessionInfoModFP*GetSessionInfo;
	InitFP*				Init;
	TermFP*				Term;
	
	char name[128];
	bool Inited;
};


extern DriveIF* CurrDrive;
extern DriveNotifyEventFP* DriveNotifyEvent;

bool ConvertSector(u8* in_buff , u8* out_buff , int from , int to,int sector);

bool InitDrive(u32 fileflags=0);
void TermDrive();

void PatchRegion_0(u8* sector,int size);
void PatchRegion_6(u8* sector,int size);
void ConvToc(u32* to,TocInfo* from);
void GetDriveToc(u32* to,DiskArea area);
void GetDriveSessionInfo(u8* to,u8 session);
int GetFile(TCHAR *szFileName, TCHAR *szParse=0,u32 flags=0);
int msgboxf(wchar* text,unsigned int type,...);
void printtoc(TocInfo* toc,SessionInfo* ses);
extern u8 q_subchannel[96];