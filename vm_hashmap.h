#pragma once
#include "vm.h"

typedef struct {
	vm_mmid_t key;
	vm_variable_t data;
} vm_hashmap_pair_t;

typedef struct {
	uint32_t size;
	uint32_t used;
	vm_mmid_t name;
	vm_variable_t parent;
	vm_variable_t super;
	vm_opcode_t* address;
	vm_hashmap_pair_t data[0];
} vm_hashmap_t;


vm_mmid_t vm_hashmap_init(uint32_t size);

void vm_hashmap_set(vm_mmid_t mapid, vm_mmid_t key, vm_variable_t value);

vm_variable_t vm_hashmap_get(vm_mmid_t mapid, vm_mmid_t key);

void vm_hashmap_remove(vm_mmid_t mapid, vm_mmid_t key);