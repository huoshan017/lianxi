#include "jmy_mem.h"
#if defined(__cplusplus)
extern "C" {
#endif
#include "../thirdparty/include/jemalloc/jemalloc.h"
#if defined(__cplusplus)
}
#endif
#include "jmy_log.h"

void jmy_mem_init()
{
#ifndef WIN32
	je_malloc_conf = "narenas:3";
	int narenas = 0;
	size_t sz = sizeof(narenas);
	je_mallctl("opt.narenas", (void *)&narenas, &sz, NULL, 0);
	if (narenas != 3) {
	    LibJmyLogError("Error: unexpected number of arenas: %d\n", narenas);
	    return;
	}
#endif
}


void jmy_mem_close()
{
}

void* jmy_mem_malloc(size_t s)
{
#ifdef WIN32
	return (void*)(new char[s]);
#else
	return je_malloc(s);
#endif
}

void jmy_mem_free(void* p)
{
#ifdef WIN32
	delete [] (char*)p;
#else
	je_free(p);
#endif
}
