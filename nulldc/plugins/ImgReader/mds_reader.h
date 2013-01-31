#pragma once
#include "ImgReader.h"
#include <stdio.h>
#include <malloc.h>
#include <string.h>

typedef struct _track {
	int track;
	int mode;
	int flags;

	int pmin;
	int psec;
	int pfrm;

	int sectorsize;
	int sector;
	int sectors;
	int pregap;
	__int64 offset;
} strack;

typedef struct _session
{
	int session;
	int pregap;
	int sectors;
	int datablocks;
	int leadinblocks;
	int last_track;

	int something1;
	int something2;

	int datablocks_offset;
	int extrablocks_offset;

	strack tracks[256];
	int ntracks;
} session;

extern session sessions[256];
extern session* mds_sessions;
extern int nsessions;

bool parse_mds(wchar *mds_filename,bool verbose);
bool parse_nrg(wchar *nrg_filename,bool verbose);