#pragma once
#include "vm.h"

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
		const vm_opcode_t* address;
		vm_native_t native;
		void* raw;
	} code;
	vm_hashmap_pair_t data[0];
} vm_hashmap_t;


vm_mmid_t vm_hashmap_create(uint32_t size, vm_type_t type, vm_mmid_t name, vm_mmid_t parent, void* code);

void vm_hashmap_set(vm_hashmap_t* map, vm_mmid_t key, vm_variable_t value);

vm_variable_t vm_hashmap_get(vm_hashmap_t* map, vm_mmid_t key);

static inline void vm_hashmap_set_m(vm_mmid_t id, vm_mmid_t key, vm_variable_t value){
	vm_hashmap_set(MMID_TO_PTR(id,vm_hashmap_t*),key,value);	
}

static inline vm_variable_t vm_hashmap_get_m(vm_mmid_t id, vm_mmid_t key){
	return vm_hashmap_get(MMID_TO_PTR(id,vm_hashmap_t*),key);	
}
