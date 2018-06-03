#pragma once
#include "vm.h"

#define VM_CAST_STRING(var) MMID_TO_PTR((var)->data.m,vm_string_t*)

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

vm_mmid_t vm_string_concat(vm_string_t* a, vm_string_t* b);

uint32_t vm_string_cmp(vm_string_t* a, vm_string_t* b);

vm_mmid_t vm_string_intern(vm_string_t* str);

int32_t vm_string_find(vm_string_t* str, vm_string_t* needle, int32_t offset);

void vm_string_free(vm_string_t* str);

vm_mmid_t vm_string_cstr(const char* cstr, uint32_t len);

static inline wstring_t* vm_string_to_wstr(vm_string_t* str) {
	return (wstring_t*)(&str->size);
}
