#include "log.hpp"
#include <iostream>

static CLogger gActiveLogger;

CLogger::CLogger() 
		:mMode(LOG_MODE_SILENT),
		 mFirstRun(true),
		 mBuf0(0),
		 mBuf1(0),
		 mFile(0)
{
}

CLogger::~CLogger()
{
	if(mFile)
	{
		mFile->close();
		delete mFile;
	}
	
	delete mBuf0;
	delete mBuf1;
}

void CLogger::invalidateStream()
{
	if(!mFile)
		return;
	
	mFile->close();
	delete mFile;
	mFile = 0;
}

void CLogger::setMode(const ELogMode mode,const char* arg0)
{
	mMode = mode;
	
	switch(mMode)
	{
		case LOG_MODE_SILENT:
		{
			delete mBuf0;
			delete mBuf1;
			
			mBuf0 = mBuf1 = 0;
			invalidateStream();
		}
		return;
		
		case LOG_MODE_COMBINE:
		case LOG_MODE_STREAM:
		{
			invalidateStream();
				
			if(arg0 != 0)
			{
				mFile = new std::ofstream(arg0,std::ios::out | ( (mFirstRun) ?  std::ios::trunc : std::ios::ate));
				mFirstRun = false;
			}
		}
		return;

		default:
			invalidateStream();
		return;
	}
}

void CLogger::dump(const char* field,const char* function,const char* sourceFile,const u32 line,const char* format,...)
{
	if(mMode == LOG_MODE_SILENT)
		return;
			
	//Allocate buffers here and don't flood static bss relocs or .ctor calls
	if(!mBuf0)
	{
		mBuf0 = new char[1024];
		
		if(!mBuf0)
			return;
	}
	
	if(!mBuf1)
	{
		mBuf1 = new char[2048];
		
		if(!mBuf1)
			return;
	}

	mBuf0[0] = mBuf1[0] = '\0';
	
	va_list ap;
	va_start(ap,format);
	vsprintf(mBuf0,format,ap);
	va_end(ap);
	
	switch(mMode)
	{
		case LOG_MODE_DEFAULT:
		{
			if(field)
				std::cout << field << "::[" << sourceFile << ":" << line << "](@" << function << ")" << mBuf0;
			else
				std::cout << "[" << sourceFile << ":" << line << "](@" << function << ")" << mBuf0;
		}
		break;
			
		case LOG_MODE_STREAM:
		{
			if(!mFile)
				return;
				
			if(mFile->is_open())
			{
				if(field)
					sprintf(mBuf1,"%s::[%s:%u](@%s)%s",field,sourceFile,line,function,mBuf0);
				else
					sprintf(mBuf1,"[%s:%u](@%s)%s",sourceFile,line,function,mBuf0);

				(*mFile) << mBuf1 << std::endl;
			}
		}
		break;
			
		case LOG_MODE_COMBINE:
		{
			if(!mFile)
				return;

			if(field)
				sprintf(mBuf1,"%s::[%s:%u](@%s)%s",field,sourceFile,line,function,mBuf0);
			else
				sprintf(mBuf1,"[%s:%u](@%s)%s",sourceFile,line,function,mBuf0);
			
			if(mFile->is_open())
			{
				(*mFile) << mBuf1 << std::endl;
				std::cout << mBuf1;// << std::endl;
			}
			else
				std::cout << mBuf1 << "(<!>Failed to dump to log file<!>)" << std::endl;
		}
		break;
	}
}

CLogger* getActiveLogger()
{
	return &gActiveLogger;
}
