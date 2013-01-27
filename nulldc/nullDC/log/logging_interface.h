#pragma once
#define DEFAULT_LOGGING

#ifdef DEFAULT_LOGGING
	#define logSetMode(_newMode_)
	#define logSetModeEx(_newMode_,_arg0_)
	#define log(_fmt_,...) //printf(_fmt_,__VA_ARGS__)
	#define logEx(_field_,_fmt_,...)
#else
	#include "logging.h"
	#define logSetMode(_newMode_) getActiveLogger()->setMode(_newMode_,0)
	#define logSetModeEx(_newMode_,_arg0_) getActiveLogger()->setMode(_newMode_,_arg0_)
	#define log(_fmt_,...) getActiveLogger()->dump(0,__FUNCTION__,__FILE__,__LINE__,_fmt_,__VA_ARGS__)
	#define logEx(_field_,_fmt_,...) getActiveLogger()->dump(_field_,__FUNCTION__,__FILE__,__LINE__,_fmt_,__VA_ARGS__)
#endif
