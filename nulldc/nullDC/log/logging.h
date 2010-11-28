#pragma once

#include <stdarg.h>
#include <fstream>
#include "../types.h"

enum ELogMode
{
	LOG_MODE_DEFAULT = 0, //Dump to console
	LOG_MODE_STREAM,	  //Dump to file
	LOG_MODE_COMBINE,     //Dump to file + dump to console
	LOG_MODE_SILENT,	  //Do nothing
};

static const char* SLogMode[] =
{
	"LOG_MODE_DEFAULT",
	"LOG_MODE_STREAM",
	"LOG_MODE_COMBINE",
	"LOG_MODE_SILENT",
};

class CLogger
{
	private://Ptrs
		char* mBuf0;
		char* mBuf1;
		
	private://Consts/Enums
		ELogMode mMode;
		
	private://Classes
		std::ofstream* mFile;
		
	private://Flags
		bool mFirstRun;
		bool mDisplayFilenameOnly;
		
	private://Funcs
		void invalidateStream();
		
	public://C:D-tors
		CLogger();
		~CLogger();
	
	public://Setters
		void setMode(const ELogMode mode,const char* arg0 = 0);
		void setFilenameOnly(const bool filenameOnly);
		
	public://Utils
		void dump(const char* field,const char* function,const char* sourceFile,const u32 line,const char* format,...);
		
	public://Getters
		inline const ELogMode getMode() const { return mMode; }
		inline const char* getModeString() const { return *(SLogMode + mMode); }
};

CLogger* getActiveLogger();
