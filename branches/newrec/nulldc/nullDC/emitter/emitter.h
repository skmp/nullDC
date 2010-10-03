#pragma once
#include "x86_emitter.h"
 
struct x86_features
{
	void detect();
	bool sse_1;
	bool sse_2;
	bool sse_3;
	bool ssse_3;
	bool mmx;
};

extern x86_features x86_caps;