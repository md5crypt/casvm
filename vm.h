#pragma once
#include <inttypes.h>
#include <stdbool.h>
#include "vm_type.h"
#include "vm_exception.h"
#include "vm_memory.h"

#define VM_ISTYPE(who, what) (vm_type_matrix[VM_TYPE_COUNT*who + what])
#define VM_CONSTANT 0xFFFFFFFF

typedef struct {
	vm_type_t type;
	union {
		int32_t i;
		vm_mmid_t m;
		float f;
	} data;
} vm_variable_t;

typedef union {
	uint32_t o32;
	struct {
		uint32_t op:8;
		uint32_t value:24;
	} o24;
	struct {
		uint8_t op;
		uint8_t type;
		int16_t value;
	} o16;
} vm_opcode_t;

typedef struct {
	uint32_t arguments;
	vm_opcode_t* link;
	vm_variable_t* base;	
} vm_stackframe_t;

typedef vm_exception_t (*vm_native_t)(vm_variable_t*, vm_stackframe_t*);

extern vm_native_t vm_native_api[];

extern vm_memory_t vm_mem_level_0; //constants
extern vm_memory_t vm_mem_level_1; //hashmaps
extern vm_memory_t vm_mem_level_2; //arrays
extern vm_memory_t vm_mem_level_3; //strings

void vm_init(void);

void vm_reset(void);

void vm_push(vm_variable_t var);

vm_variable_t vm_pop();

void vm_call(vm_mmid_t func);