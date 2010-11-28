#pragma once
#include "log.hpp"

//Macros
#undef DISPLAY_FULL_SOURCE_FILENAME

//Temp hack
static inline const char* getFilename(const char* path)
{
	register u32 ptr;
	
	if(*(path + 0) != '\\' && *(path + 0) != '/')
		return path;

	ptr = strlen(path) - 1;

	while(ptr)
	{
		if(*(path + ptr) == '\\' || *(path + ptr) == '/')
			return (path + (ptr + 1));

		--ptr;
	}

	return path; //not good
}

#ifdef DISPLAY_FULL_SOURCE_FILENAME
	#define logSetMode(_newMode_) getActiveLogger()->setMode(_newMode_,0)
	#define logSetModeEx(_newMode_,_arg0_) getActiveLogger()->setMode(_newMode_,_arg0_)
	#define logWrite(_fmt_,...) getActiveLogger()->dump(0,__FUNCTION__,__FILE__,__LINE__,_fmt_,__VA_ARGS__)
	#define logWriteEx(_field_,_fmt_,...) getActiveLogger()->dump(_field_,__FUNCTION__,__FILE__,__LINE__,_fmt_,__VA_ARGS__)
#else
	#define logSetMode(_newMode_) getActiveLogger()->setMode(_newMode_,0)
	#define logSetModeEx(_newMode_,_arg0_) getActiveLogger()->setMode(_newMode_,_arg0_)
	#define logWrite(_fmt_,...) getActiveLogger()->dump(0,__FUNCTION__,getFilename(__FILE__),__LINE__,_fmt_,__VA_ARGS__)
	#define logWriteEx(_field_,_fmt_,...) getActiveLogger()->dump(_field_,__FUNCTION__,getFilename(__FILE__),__LINE__,_fmt_,__VA_ARGS__)
#endif
