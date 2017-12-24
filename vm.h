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
		int32_t value:24;
		uint32_t op:8;
	} o24;
	struct {
		int16_t value;
		uint8_t type;
		uint8_t op;
	} o16;
} vm_opcode_t;

typedef vm_exception_t (*vm_native_t)(vm_variable_t*, uint32_t);
typedef void (*vm_destructor_t)(void*);

extern vm_memory_t vm_mem_const;
extern vm_memory_t vm_mem_hashmap;
extern vm_memory_t vm_mem_array;
extern vm_memory_t vm_mem_string;
extern vm_memory_t vm_mem_thread;

extern const vm_opcode_t* vm_progmem;

void vm_init(uint32_t mmid_offset);

bool vm_extern_resolve(vm_mmid_t hashmap, const char* name);

void vm_call(const vm_opcode_t* address);
vm_exception_t vm_run(void);

extern void vm_stdlib_init(void);

extern const vm_destructor_t vm_destructor_lut[VM_TYPE_COUNT];

inline static void vm_make_const(vm_mmid_t id){
	MMID_TO_PTR(id, uint32_t*)[0] = VM_CONSTANT;
}

inline static void vm_reference(void* ptr){
	if(((uint32_t*)ptr)[0] != VM_CONSTANT)
		((uint32_t*)ptr)[0] += 1;
}

inline static void vm_reference_m(vm_mmid_t id){
	uint32_t* cnt = MMID_TO_PTR(id, uint32_t*);
	if(cnt[0] != VM_CONSTANT)
		cnt[0]++;
}

inline static void vm_dereference(void* ptr, vm_type_t type){
	vm_destructor_t destructor = vm_destructor_lut[type];
	if(destructor){
		uint32_t cnt = ((uint32_t*)ptr)[0];
		if(cnt <= 1)
			destructor(ptr);
		else if(cnt != VM_CONSTANT)
			((uint32_t*)ptr)[0] = cnt-1;
	}
}

inline static void vm_dereference_m(vm_mmid_t id, vm_type_t type){
	vm_destructor_t destructor = vm_destructor_lut[type];
	if(destructor){
		void* ptr = MMID_TO_PTR(id, void*);
		uint32_t cnt = ((uint32_t*)ptr)[0];
		if(cnt <= 1)
			destructor(ptr);
		else if(cnt != VM_CONSTANT)
			((uint32_t*)ptr)[0] = cnt-1;
	}
}

inline static void vm_variable_reference(vm_variable_t v){
	if(vm_destructor_lut[v.type])
		vm_reference_m(v.data.i);
}

inline static void vm_variable_dereference(vm_variable_t v){
	vm_dereference_m(v.data.m,v.type);
}