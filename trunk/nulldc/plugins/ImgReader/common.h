#pragma once
#include "gd_driver.h"
#include <vector>
using namespace std;

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


enum SectorFormat
{
	SECFMT_2352,				//full sector
	SECFMT_2048_MODE1,			//2048 user byte, form1 sector
	SECFMT_2048_MODE2_FORM1,	//2048 user bytes, form2m1 sector
};

enum SubcodeFormat
{
	SUBFMT_NONE,				//No subcode info
	SUBFMT_96					//raw 96-byte subcode info
};

extern DriveNotifyEventFP* DriveNotifyEvent;

bool ConvertSector(u8* in_buff , u8* out_buff , int from , int to,int sector);

bool InitDrive(u32 fileflags=0);
void TermDrive();

void PatchRegion_0(u8* sector,int size);
void PatchRegion_6(u8* sector,int size);
void ConvToc(u32* to,TocInfo* from);
void GetDriveToc(u32* to,DiskArea area);
void GetDriveSector(u8 * buff,u32 StartSector,u32 SectorCount,u32 secsz);

void GetDriveSessionInfo(u8* to,u8 session);
int GetFile(TCHAR *szFileName, TCHAR *szParse=0,u32 flags=0);
int msgboxf(wchar* text,unsigned int type,...);
void printtoc(TocInfo* toc,SessionInfo* ses);
extern u8 q_subchannel[96];


struct Session
{
	u32 StartFAD;			//session's start fad
	u8 FirstTrack;			//session's first track
};

struct TrackFile
{
	virtual void Read(u32 FAD,u8* dst,SectorFormat* sector_type,u8* subcode,SubcodeFormat* subcode_type)=0;
	virtual ~TrackFile() {};
};

struct Track
{
	TrackFile* file;	//handler for actual IO
	u32 StartFAD;		//Start FAD
	u32 EndFAD;			//End FAD
	u8 CTRL;
	u8 ADDR;

	bool Read(u32 FAD,u8* dst,SectorFormat* sector_type,u8* subcode,SubcodeFormat* subcode_type)
	{
		if (FAD>=StartFAD && (FAD<=EndFAD || EndFAD==0) && file)
		{
			file->Read(FAD,dst,sector_type,subcode,subcode_type);
			return true;
		}
		else
			return false;
	}
	void Destroy() { if (file) delete file; file=0; }
};

struct Disc
{
	wstring path;
	vector<Session> sessions;	//info for sessions
	vector<Track> tracks;		//info for tracks
	Track LeadOut;				//info for lead out track (can't read from here)
	u32 EndFAD;					//Last valid disc sector
	DiscType type;

	//functions !
	bool ReadSector(u32 FAD,u8* dst,SectorFormat* sector_type,u8* subcode,SubcodeFormat* subcode_type)
	{
		for (size_t i=tracks.size();i-->0;)
		{
			*subcode_type=SUBFMT_NONE;
			if (tracks[i].Read(FAD,dst,sector_type,subcode,subcode_type))
				return true;
		}

		return false;
	}

	void ReadSectors(u32 FAD,u32 count,u8* dst,u32 fmt)
	{
		u8 temp[2352];
		SectorFormat secfmt;
		SubcodeFormat subfmt;

		while(count)
		{
			if (!ReadSector(FAD,temp,&secfmt,q_subchannel,&subfmt))
			{
				verify(false);
			}

			//TODO: Proper sector convertions
			if (secfmt==SECFMT_2352)
			{
				ConvertSector(temp,dst,2352,fmt,FAD);
			}
			else if (fmt==2048 && (secfmt==SECFMT_2048_MODE1 || secfmt==SECFMT_2048_MODE2_FORM1 ))
			{
				memcpy(dst,temp,2048);
			}
			else
			{
				verify(false);
			}

			dst+=fmt;
			FAD++;
			count--;
		}
	}
	virtual ~Disc() 
	{
		for (size_t i=0;i<tracks.size();i++)
			tracks[i].Destroy();
	};
};

extern Disc* disc;

struct RawTrackFile : TrackFile
{
	FILE* file;
	s32 offset;
	u32 fmt;
	bool cleanup;

	RawTrackFile(FILE* file,u32 file_offs,u32 first_fad,u32 secfmt)
	{
		this->file=file;
		this->offset=file_offs-first_fad*secfmt;
		this->fmt=secfmt;
		this->cleanup=true;
	}

	virtual void Read(u32 FAD,u8* dst,SectorFormat* sector_type,u8* subcode,SubcodeFormat* subcode_type)
	{
		//for now hackish
		if (fmt==2352)
			*sector_type=SECFMT_2352;
		else if (fmt==2048)
			*sector_type=SECFMT_2048_MODE2_FORM1;
		else
		{
			verify(false);
		}

		fseek(file,offset+FAD*fmt,SEEK_SET);
		fread(dst,1,fmt,file);
	}
	virtual ~RawTrackFile()
	{
		if (cleanup && file)
			fclose(file);
	}
};