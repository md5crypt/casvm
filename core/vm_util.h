#pragma once

#include <stdint.h>

#define UNUSED(x) (void)x

inline static uint32_t imin(uint32_t a, uint32_t b){
	return a > b? b : a;
}

inline static uint32_t imax(uint32_t a, uint32_t b){
	return a > b? a : b;
}

inline static uint32_t iabs(int32_t a){
	return a > 0? a : -a;
}

static inline uint32_t npot(uint32_t v){
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	return v+1;
}

typedef struct {
	uint32_t size;
	uint16_t data[];
} wstring_t;

static inline uint32_t wstrcmp8(wstring_t* a, const char* b){
	for(uint8_t i=0; i<a->size; i++){
		if(b[i] == 0 || a->data[i] != b[i])
			return 1;
	}
	return 0;
}
