#pragma once

#include "vm.h"

typedef struct {
	const char* name;
	vm_native_t function;
} vm_extern_t;

extern const vm_extern_t vm_externs[];
