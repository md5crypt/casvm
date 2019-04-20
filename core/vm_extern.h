#pragma once

#include "vm.h"
#include "vm_exception.h"

typedef struct {
	const char* name;
	vm_native_t function;
} vm_extern_t;

extern const vm_extern_t vm_extern_native[];

extern vm_exception_t vm_extern_call(uint32_t id, vm_variable_t* top, uint32_t arguments);

extern uint32_t vm_extern_resolve(const wstring_t* str);

vm_native_t vm_extern_native_resolve(const wstring_t* str);
