#pragma once
#include "vm.h"

#define VM_ARRAY_SIZE_MIN 8

typedef struct {
	uint32_t rcnt;
	uint32_t size;
	uint32_t used;
	int32_t offset;
	vm_variable_t data[0];
} vm_array_t;

uint32_t vm_array_set(vm_array_t* array, int32_t pos, vm_variable_t value);

vm_variable_t vm_array_get(vm_array_t* id, int32_t pos);

vm_mmid_t vm_array_create(uint32_t size);

void vm_array_free(vm_array_t* array);

vm_mmid_t vm_array_slice(vm_array_t* array, int32_t start, int32_t stop);

void vm_array_grow(vm_array_t* array);

void vm_array_push(vm_array_t* array, vm_variable_t value);

vm_variable_t vm_array_pop(vm_array_t* array);

void vm_array_unshift(vm_array_t* array, vm_variable_t value);

vm_variable_t vm_array_shift(vm_array_t* array);

vm_mmid_t vm_array_concat_m(vm_mmid_t a, vm_mmid_t b);

static inline vm_variable_t vm_array_get_m(vm_mmid_t id, int32_t pos){
	return vm_array_get(MMID_TO_PTR(id,vm_array_t*),pos);
}

static inline uint32_t vm_array_set_m(vm_mmid_t id, int32_t pos, vm_variable_t value){
	return vm_array_set(MMID_TO_PTR(id,vm_array_t*),pos,value);
}

static inline void vm_array_free_m(vm_mmid_t id){
	vm_array_free(MMID_TO_PTR(id,vm_array_t*));
}

static inline vm_mmid_t vm_array_slice_m(vm_mmid_t id, int32_t start, int32_t stop){
	return vm_array_slice(MMID_TO_PTR(id,vm_array_t*),start,stop);
}

static inline void vm_array_grow_m(vm_mmid_t id){
	vm_array_grow(MMID_TO_PTR(id,vm_array_t*));
}

static inline vm_mmid_t vm_array_concat(vm_array_t* a, vm_array_t* b){
	return vm_array_concat_m(PTR_TO_MMID(a),PTR_TO_MMID(b));
}

static inline void vm_array_push_m(vm_mmid_t id,vm_variable_t value){
	vm_array_push(MMID_TO_PTR(id,vm_array_t*),value);
}

static inline vm_variable_t vm_array_pop_m(vm_mmid_t id){
	return vm_array_pop(MMID_TO_PTR(id,vm_array_t*));
}

static inline void vm_array_unshift_m(vm_mmid_t id,vm_variable_t value){
	vm_array_unshift(MMID_TO_PTR(id,vm_array_t*),value);
}

static inline void vm_array_shift_m(vm_mmid_t id){
	vm_array_shift(MMID_TO_PTR(id,vm_array_t*));
}