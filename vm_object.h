#pragma once
#include "vm.h"
#include "vm_hashmap.h"

typedef struct {
	vm_mmid_t name;
	vm_mmid_t parent;
	vm_mmid_t super;
	vm_hashmap_t hashmap;
} vm_object_t;
