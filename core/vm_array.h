#pragma once

#include "vm.h"

#define VM_CAST_ARRAY(var) MMID_TO_PTR((var)->data.m, vm_array_t*)

typedef struct {
	uint32_t rcnt;
	uint32_t size;
	uint32_t used;
	int32_t offset;
	vm_variable_t data[0];
} vm_array_t;

vm_exception_t vm_array_set(vm_array_t* array, int32_t pos, vm_variable_data_t value, vm_type_t type);

vm_exception_t vm_array_get(const vm_array_t* id, int32_t pos, vm_variable_t* value);

vm_mmid_t vm_array_create(uint32_t size);

void vm_array_free(vm_array_t* array);

vm_mmid_t vm_array_slice(const vm_array_t* array, int32_t start, int32_t stop);

vm_array_t* vm_array_resize(vm_array_t* array, uint32_t size);

vm_array_t* vm_array_push(vm_array_t* array, vm_variable_data_t value, vm_type_t type);

vm_exception_t vm_array_pop(vm_array_t* array, vm_variable_t* value);

vm_array_t* vm_array_unshift(vm_array_t* array, vm_variable_data_t value, vm_type_t type);

vm_exception_t vm_array_shift(vm_array_t* array, vm_variable_t* value);

vm_exception_t vm_array_write(vm_array_t* dst, const vm_array_t* src, int32_t offset, int32_t len);

vm_exception_t vm_array_fill(vm_array_t* array, vm_variable_data_t value, vm_type_t type, int32_t offset, int32_t len);

void vm_array_reverse(vm_array_t* array);

int32_t vm_array_find(vm_array_t* array, vm_variable_data_t value, vm_type_t type, int32_t offset);

vm_variable_t* vm_array_apply(const vm_array_t* array, vm_variable_t* top);
