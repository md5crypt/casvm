#pragma once

#include "vm.h"

typedef struct {
	const char* name;
	vm_native_t function;
} vm_stdlib_t;

extern const vm_stdlib_t vm_stdlib[];

void vm_stdlib_init(void);
