#pragma once

#include "vm.h"
#include "vm_array.h"

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

inline static void vm_reference(vm_mmid_t id){
	MMID_TO_PTR(id, uint32_t*)[0]++;
}

inline static void vm_dereference(vm_mmid_t id, vm_type_t type){
	uint32_t cnt = MMID_TO_PTR(id, uint32_t*)[0];
	if(cnt <= 1){
		if(type==VM_ARRAY_T)
			vm_array_free(id);
		else
			vm_memory_free(&vm_mem_level_3, id);
	} else if(cnt != VM_CONSTANT){
		MMID_TO_PTR(id, uint32_t*)[0] = cnt-1;
	}
}

inline static void vm_variable_reference(vm_variable_t v){
	if(v.type == VM_ARRAY_T || v.type == VM_STRING_T)
		vm_reference(v.data.i);
}

inline static void vm_variable_dereference(vm_variable_t v){
	if(v.type == VM_ARRAY_T || v.type == VM_STRING_T)
		vm_dereference(v.data.i,v.type);
}
