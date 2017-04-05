#pragma once

#include <cstddef>
#include <new>
#if defined(__cplusplus)
#include <stdarg.h>
#include <stdio.h>
#endif

void jmy_mem_init();
void jmy_mem_close();
void* jmy_mem_malloc(std::size_t s);
void jmy_mem_free(void* p);

template <typename T, typename... Args>
T* jmy_mem_malloc(Args&&... args) {
	void* p = jmy_mem_malloc(sizeof(T));
	T* n = new(p)T(args ...);
	return n;
}

template <typename T>
void jmy_mem_free(T* p) {
	p->~T();
	jmy_mem_free((void*)p);
}

