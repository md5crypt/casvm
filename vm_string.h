#pragma once
#include "vm.h"

typedef struct {
	uint32_t rcnt;
	uint32_t size;
	uint16_t data[0];
} vm_string_t;

void vm_stringset_init(uint32_t size);

vm_mmid_t vm_string_create(uint32_t len, bool constant);

vm_mmid_t vm_string_copy(vm_mmid_t id, bool constant);

vm_mmid_t vm_string_concat(vm_mmid_t a, vm_mmid_t b);

uint32_t vm_string_cmp(vm_mmid_t a, vm_mmid_t b);

vm_mmid_t vm_string_intern(vm_mmid_t id);
