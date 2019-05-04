#pragma once
#include <inttypes.h>
#include <stdbool.h>
#include "vm_type.h"
#include "vm_exception.h"
#include "vm_memory.h"
#include "vm_symbols.h"

#define VM_ISTYPE(who, what)            (vm_type_matrix[(VM_TYPE_COUNT * who) + what])
#define VM_CONSTANT                     0xFFFFFFFF
#define VM_VARIABLE(_type, _data)       ((vm_variable_t) {.type = (_type), .data.i = (_data)})
#define VM_VARIABLE_OFTYPE(_type)       ((vm_variable_t) {.type = (_type), .data.i = 0})
#define VM_VARIABLE_INTEGER(value)      ((vm_variable_t) {.type = VM_INTEGER_T, .data.i = (value)})
#define VM_VARIABLE_BOOL(value)         ((vm_variable_t) {.type = VM_BOOLEAN_T, .data.i = (value)})
#define VM_VARIABLE_FLOAT(value)        ((vm_variable_t) {.type = VM_FLOAT_T, .data.f = (value)})
#define VM_VARIABLE_MMID(_type, value)  ((vm_variable_t) {.type = (_type), .data.m = (value)})

typedef uint32_t vm_variable_data_t;

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

extern vm_mmid_t vm_root;
extern const vm_opcode_t* vm_progmem;

void vm_init(void);

void vm_call(uint32_t address);
vm_exception_t vm_run(void);

extern const vm_destructor_t vm_destructor_lut[VM_TYPE_COUNT];

void vm_dereference(void* ptr, vm_type_t type);

void vm_dereference_m(vm_mmid_t id, vm_type_t type);

void vm_reference(void* ptr);

void vm_reference_m(vm_mmid_t id);


void vm_reference_type_ns(vm_variable_data_t data, vm_type_t type);

inline static void vm_make_const(vm_mmid_t id) {
	MMID_TO_PTR(id, uint32_t*)[0] = VM_CONSTANT;
}

inline static void vm_reference_unsafe(void* ptr) {
	if (((uint32_t*)ptr)[0] != VM_CONSTANT) {
		((uint32_t*)ptr)[0] += 1;
	}
}

inline static void vm_reference_m_inline(vm_mmid_t id) {
	uint32_t* cnt = MMID_TO_PTR(id, uint32_t*);
	if (cnt[0] != VM_CONSTANT) {
		cnt[0] += 1;
	}
}

inline static void vm_reference_inline(vm_variable_data_t data, vm_type_t type) {
	if (vm_destructor_lut[type]) {
		vm_reference_m_inline(data);
	}
}

inline static void vm_variable_reference(vm_variable_t v) {
	if (vm_destructor_lut[v.type]) {
		vm_reference_m_inline(v.data.m);
	}
}

inline static void vm_variable_dereference(vm_variable_t v) {
	vm_dereference_m(v.data.m, v.type);
}

bool vm_fault_trace(vm_symbols_location_t* loc);

vm_mmid_t vm_fault_get_thread(void);

void vm_fault_recover(void);

vm_mmid_t vm_get_current_thread(void);
