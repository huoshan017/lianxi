#pragma once

#include <cstddef>

void jmy_mem_init();
void jmy_mem_close();
void* jmy_mem_malloc(std::size_t s);
void jmy_mem_free(void* p);

template <typename T>
T* jmy_mem_malloc() {
	return (T*)jmy_mem_malloc(sizeof(T));
}

template <typename T>
void jmy_mem_free(T* p) {
	jmy_mem_free((void*)p);
}

