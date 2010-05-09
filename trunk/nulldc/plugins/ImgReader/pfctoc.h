// -----------------------------------------------------------------------------------------------------------------------------
//
// Public file / main header file
// Copyright (C) 1996-2002 Padus, Inc.
// All rights reserved.
//
// Written by: Mirco Caramori
//
// -----------------------------------------------------------------------------------------------------------------------------

#pragma once

// Includes and Macros ---------------------------------------------------------------------------------------------------------

#ifdef _PFCTOCDLL
#define PFCTOCAPI               __declspec(dllexport)   
#else
#define PFCTOCAPI               __declspec(dllimport)   
#endif // _PFCTOCDLL

// Error codes

#define PFCTOC_OK               0
#define PFCTOC_NOTAFILE         1
#define PFCTOC_OPENERROR        2
#define PFCTOC_INVALIDFILESIZE  3
#define PFCTOC_UNKNOWNIMAGE     4
#define PFCTOC_INVALIDFILEHDR   5
#define PFCTOC_READERROR        6
#define PFCTOC_ILLEGALPARAMETER 7
#define PFCTOC_ILLEGALTOC       8
#define PFCTOC_OUTOFMEMORY      9

// Declarations ----------------------------------------------------------------------------------------------------------------

struct SPfcTrack {                                              // ---> 30h bytes size
  BYTE abReserved[14];                                          // [00] Padding
  BYTE bCtrl;                                                   // [0E] Track control nibble
  BYTE bMode;                                                   // [0F] 0) Audio, 1) Mode1/DVD, 2) Mode2
  BYTE abISRC[12];                                              // [10] ISRC (International Standard Recording Code)
  BOOL fISRCValid;                                              // [1C] TRUE if the ISRC is valid
  DWORD dwBlockSize;                                            // [20] Block size in bytes (2048, 2336, 2352, 2368, 2448)
  DWORD dwIndexCount;                                           // [24] Total number of index points (min. 2)
  DWORD* pdwIndex;                                              // [28] Pointer to an array of indexes. Each element contains
                                                                //      the size in blocks of the corresponding index 
                                                                //      (adwIndex[0] can be zero)
  #ifndef _WIN64
  DWORD dwReserved;                                             // [2C] Padding
  #endif
};                                                              
                                                                
struct SPfcSession {                                            // ---> 10h bytes size
  BYTE abReserved[3];                                           // [00] Padding
  BYTE bType;                                                   // [03] 0) CD-DA, 1) CD-ROM/DVD, 2) CD-XA, 3) CD-I, 4) Open
  DWORD dwTrackCount;                                           // [04] Number of tracks in the session (can be 0 if session is open)
  SPfcTrack* pstTrack;                                          // [08] Pointer to an array of tracks belonging to the session
  #ifndef _WIN64                                                 
  DWORD dwReserved;                                             // [0C] Padding
  #endif
};

struct SPfcToc {                                                // ---> 30h bytes size
  BYTE bVersion;                                                // [00] Higher nibble) Major version, Lower nibble) Minor version
  BYTE bFirstSessionNum;                                        // [01] Number of the first session on the disc
  BYTE bFirstTrackNum;                                          // [02] Number of the first track on the disc
  BYTE abUPC[13];                                               // [03] UPC (Universal Product Code)

  DWORD fUPCValid              :1;                              // [10] If set abUPC[] content is valid
  DWORD fDVD                   :1;                              // [10] If set this is a DVD
  DWORD fReserved              :30;                             // [10] Reserved for future use

  DWORD dwTextSize;                                             // [14] CD-TEXT info size in bytes (multiple of 18, can be 0)
  BYTE* pbText;                                                 // [18] Pointer to valid CD-TEXT info if !NULL
  #ifndef _WIN64                                                 
  DWORD dwReserved;                                             // [1C] Padding
  #endif
  DWORD dwOuterLeadOut;                                         // [20] Highest possible lead-out starting address in blocks
  DWORD dwSessionCount;                                         // [24] Total number of session in the TOC (cannot be 0)
  SPfcSession* pstSession;                                      // [28] Pointer to an array of session belonging to the TOC
  #ifndef _WIN64                                                 
  DWORD dwReserved1;                                            // [2C] Padding
  #endif
};

extern "C" {

typedef DWORD PFCTOCAPI PfcGetTocFP(LPCSTR pszImageName, SPfcToc*& pstToc, DWORD& dwSize);
//
// LPCTSTR pszImageName:        pointer to .CDI full pathname
// SPfcToc*& pstToc:            on exit it will point to a buffer containing .CDI TOC info
// DWORD& dwSize:               size in bytes of the buffer pointed to by 'pstToc'
//
// Return value:                error code or PFCTOC_OK if operation completed succefully
//

typedef  DWORD PFCTOCAPI PfcFreeTocFP(SPfcToc* pstToc);

}

//
// Sample:
// =======
//
// #include "stdafx.h"
// #include "..\pfctoc.h"
// 
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
// }
//

//
// Notes:
// ======
//
// - .CDI image doesn't contain lead-in and lead-out areas but it contains tracks gaps (index 0) for all the tracks
//   including the first one. Therefore the beginning of the file is 00:00:00 MSF or 150 LBA. For example:
//
//   SetFilePointer(hFile, pstToc->pstSession[0].pstTrack[0].dwBlockSize * 166, NULL, FILE_BEGIN)
//
//   will seek to the PVD assuming the first track is data and contains a valid ISO-9660 file-system
//
// - Open discs are represented by an open and empty session (type 4) at the end of the TOC
//
// - In case of block size 2368 and 2448 Q subcode data is in BCD format
//
//

// EOF -------------------------------------------------------------------------------------------------------------------------
