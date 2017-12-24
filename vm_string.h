#pragma once
#include "vm.h"

typedef struct {
	uint32_t rcnt;
	uint32_t size;
	uint16_t data[0];
} vm_string_t;

void vm_stringset_init(uint32_t size);

vm_mmid_t vm_string_create(uint32_t len);

vm_mmid_t vm_string_insert(const uint16_t* data, uint32_t len);

vm_mmid_t vm_string_copy(vm_string_t* string, bool constant);

vm_mmid_t vm_string_slice(vm_string_t* string, int32_t start, int32_t stop);

vm_variable_t vm_string_get(vm_string_t* a, int32_t index);

vm_mmid_t vm_string_concat_m(vm_mmid_t a, vm_mmid_t b);

uint32_t vm_string_cmp(vm_string_t* a, vm_string_t* b);

uint32_t vm_string_cmp_m(vm_mmid_t a, vm_mmid_t b);

vm_mmid_t vm_string_intern_m(vm_mmid_t id);

void vm_string_free(vm_string_t* str);

static inline vm_mmid_t vm_string_copy_m(vm_mmid_t id, bool constant){
	return vm_string_copy(MMID_TO_PTR(id,vm_string_t*),constant);
}

static inline vm_mmid_t vm_string_slice_m(vm_mmid_t id, int32_t start, int32_t stop){
	return vm_string_slice(MMID_TO_PTR(id,vm_string_t*),start,stop);
}

static inline vm_variable_t vm_string_get_m(vm_mmid_t id, int32_t index){
	return vm_string_get(MMID_TO_PTR(id,vm_string_t*),index);
}

static inline vm_mmid_t vm_string_concat(vm_string_t* a, vm_string_t* b){
	return vm_string_concat_m(PTR_TO_MMID(a),PTR_TO_MMID(b));
}

static inline vm_mmid_t vm_string_intern(vm_string_t* str){
	return vm_string_intern_m(PTR_TO_MMID(str));	
}

static inline void vm_string_free_m(vm_mmid_t id){
	vm_string_free(MMID_TO_PTR(id,vm_string_t*));
}
