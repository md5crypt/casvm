#pragma once

#include <stdint.h>
#include "vm_type.h"
#include "vm_util.h"

typedef enum {
	VM_NONE_E,
	VM_YIELD_E,
	VM_USER_E,
	VM_OOB_E,
	VM_TYPE_E,
	VM_ARITY_E,
	VM_IMMUTABLE_E,
	VM_DIV0_E,
	VM_INTERNAL_E
} vm_exception_t;

typedef struct {
	uint32_t f1;
	uint32_t f2;
} vm_exception_data_t;

void vm_exception_oob(int32_t index, uint32_t size);

void vm_exception_type(vm_type_t actual, vm_type_t expected);

void vm_exception_user(wstring_t* message);

void vm_exception_arity(uint32_t actual, uint32_t expected);

const vm_exception_data_t* vm_exception_data_get();
