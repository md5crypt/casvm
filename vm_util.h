#pragma once

inline static uint32_t imin(uint32_t a, uint32_t b){
	return a > b? b : a;
}

inline static uint32_t imax(uint32_t a, uint32_t b){
	return a > b? a : b;
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

