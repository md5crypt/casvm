#pragma once
#include <inttypes.h>
#include <stdbool.h>
#include "vm_type.h"
#include "vm_exception.h"
#include "vm_memory.h"

typedef struct {
	vm_type_t type;
	union {
		int32_t i;
		vm_mmid_t m;
		float f;
	} data;
} vm_variable_t;

#define VM_ISTYPE(who, what) (vm_type_matrix[VM_TYPE_COUNT*who + what])
#define VM_CONSTANT 0xFFFFFFFF

extern vm_memory_t vm_mem_level_0; //constants
extern vm_memory_t vm_mem_level_1; //hashmaps
extern vm_memory_t vm_mem_level_2; //arrays
extern vm_memory_t vm_mem_level_3; //strings

void vm_init(void);
