#include "vm_exception.h"

static vm_exception_data_t data;

const vm_exception_data_t* vm_exception_data_get() {
	return &data;
}

void vm_exception_oob(int32_t index, uint32_t size) {
	data.f1 = (uint32_t)index;
	data.f2 = size;
}

void vm_exception_type(vm_type_t actual, vm_type_t expected) {
	data.f1 = (uint32_t)actual;
	data.f2 = (uint32_t)expected;
}

void vm_exception_user(wstring_t* message) {
	data.f1 = (uint32_t)message;
}

void vm_exception_arity(uint32_t actual, uint32_t expected) {
	data.f1 = actual;
	data.f2 = expected;
}
