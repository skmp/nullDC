#include "types.h"

//If the MEM_ALLOC_CHECK define is on , we check for allocation errors 
//and report/break

//Notice for future:
//We can add a check witch mallocs were not freed ect
//What bout calloc and other shit ?

#ifdef malloc
#undef malloc
#endif

#ifdef realloc
#undef realloc
#endif

#ifdef free
#undef free
#endif

#ifdef MEM_ALLOC_CHECK
	void * debug_malloc(size_t size)
	{
		if (size==0)
		{
			log("debug_malloc warning : malloc with size=0");
			MEM_ERROR_BREAK;
		}
		void *rv= malloc(size);
		if (rv==0)
		{
			log("debug_malloc warning : malloc (%d) failed",size);
			MEM_ERROR_BREAK;
		}
		//if (size==1024*128)
		//	memset(rv,0,size);
		//log("malloc %X\n",*(u8*)rv);
		
		return rv;
	}

	void * debug_realloc(void* mem,size_t size)
	{
		if (size==0)
		{
			log("debug_realloc warning : malloc with size=0 called \n");
			MEM_ERROR_BREAK;
		}

		if (mem==0)
			log("debug_realloc warning : malloc with ptr=0 called \n");
		
		void *rv= realloc(mem,size);
		if (rv==0)
		{
			log("debug_realloc warning : realloc (0x%p,%d) failed",mem,size);
			MEM_ERROR_BREAK;
		}
		return rv;
	}

	void debug_free(void* ptr)
	{
		if (ptr==0)
		{
			log("debug_free warning : malloc with ptr==0 called \n");
			MEM_ERROR_BREAK;
		}

		free(ptr);
	}
#endif