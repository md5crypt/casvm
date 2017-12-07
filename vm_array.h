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

uint32_t vm_array_set(vm_mmid_t id, int32_t pos, vm_variable_t value);

vm_variable_t vm_array_get(vm_mmid_t id, int32_t pos);

vm_mmid_t vm_array_create(uint32_t size);

void vm_array_free(vm_mmid_t id);

vm_mmid_t vm_array_copy(vm_mmid_t id, uint32_t size);

void vm_array_resize(vm_mmid_t id, uint32_t size);

void vm_array_push(vm_mmid_t id, vm_variable_t value);

vm_variable_t vm_array_pop(vm_mmid_t id);

void vm_array_unshift(vm_mmid_t id, vm_variable_t value);

vm_variable_t vm_array_shift(vm_mmid_t id);