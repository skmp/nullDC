#include "types.h"
#include "emitter.h"
#include "windows.h"

x86_features x86_caps;

#ifndef EXCEPTION_EXECUTE_HANDLER
#define EXCEPTION_EXECUTE_HANDLER       1
#endif

void x86_features::detect()
{
	sse_1=true;
	sse_2=true;
	sse_3=true;
	ssse_3=true;
	mmx=true;

	__try
	{
		__asm addps xmm0,xmm0
	}
	__except(EXCEPTION_EXECUTE_HANDLER) 
	{
		sse_1=false;
	}

	__try
	{
		__asm addpd xmm0,xmm0
	}
	__except(EXCEPTION_EXECUTE_HANDLER) 
	{
		sse_2=false;
	}

	__try
	{
		__asm addsubpd xmm0,xmm0
	}
	__except(EXCEPTION_EXECUTE_HANDLER) 
	{
		sse_3=false;
	}

	__try
	{
		__asm phaddw xmm0,xmm0
	}
	__except(EXCEPTION_EXECUTE_HANDLER) 
	{
		ssse_3=false;
	}

	
	__try
	{
		__asm paddd mm0,mm1
	}
	__except(EXCEPTION_EXECUTE_HANDLER) 
	{
		mmx=false;
	}




	wchar* command_line = GetCommandLine();

	if (ssse_3 && wcsstr(command_line,L"-nossse3"))
	{
		log("sSSE3 detected but disabled[-nossse3]\n");
		ssse_3=false;
	}
	if (sse_3 && wcsstr(command_line,L"-nosse3"))
	{
		log("SSE3 detected but disabled[-nosse3]\n");
		sse_3=false;
	}

	if (sse_2 && wcsstr(command_line,L"-nosse2"))
	{
		log("SSE2 detected but disabled[-nosse2]\n");
		sse_2=false;
	}

	if (sse_1 && wcsstr(command_line,L"-nosse1"))
	{
		log("SSE1 detected but disabled[-nosse1]\n");
		sse_1=false;
	}

	if (mmx && wcsstr(command_line,L"-nommx"))
	{
		log("MMX detected but disabled[-nommx]\n");
		mmx=false;
	}
	

	log("Detected cpu features : ");
	if (mmx)
		log("MMX ");
	if (sse_1)
		log("SSE1 ");
	if (sse_2)
		log("SSE2 ");
	if (sse_3)
		log("SSE3 ");
	if (ssse_3)
		log("sSSE3[ohh god , is that a name?] ");

	log("\n");
	log("\n");

	__try
	{
		__asm sfence;
	}
	__except(EXCEPTION_EXECUTE_HANDLER) 
	{
		//Blah
	}	

	__try
	{
		__asm emms;
	}
	__except(EXCEPTION_EXECUTE_HANDLER) 
	{
		//Blah
	}	
}