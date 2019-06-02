#pragma once
#include <stdbool.h>
#include "vm.h"

#define VM_CAST_HASHMAP(var) MMID_TO_PTR((var)->data.m, vm_hashmap_t*)

typedef struct {
	vm_mmid_t key;
	vm_variable_t data;
} vm_hashmap_pair_t;

typedef struct {
	uint32_t size;
	uint32_t used;
	uint32_t deleted;
	vm_mmid_t name;
	vm_mmid_t parent;
	vm_type_t type;
	union {
		uint32_t address;
		vm_native_t native;
	} code;
	uint32_t dirty;
	vm_hashmap_pair_t data[0];
} vm_hashmap_t;


vm_mmid_t vm_hashmap_create(uint32_t size, vm_type_t type, vm_mmid_t name, vm_mmid_t parent, void* code);

void vm_hashmap_set(vm_hashmap_t* map, vm_mmid_t key, vm_variable_data_t value, vm_type_t type);

void vm_hashmap_get(const vm_hashmap_t* map, vm_mmid_t key, vm_variable_t* value);

bool vm_hashmap_has(const vm_hashmap_t* map, vm_mmid_t key);

vm_mmid_t vm_hashmap_values(const vm_hashmap_t* map);

vm_mmid_t vm_hashmap_keys(const vm_hashmap_t* map);
