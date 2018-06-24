#include "vm_lib.h"

uint32_t test_lib_try = 0;

vm_exception_t vm_lib_try_start(vm_variable_t* top, uint32_t arguments) {
	ASSERT_ARITY(0);
	UNUSED(top);
	test_lib_try += 1;
	return VM_NONE_E;
}

vm_exception_t vm_lib_try_end(vm_variable_t* top, uint32_t arguments) {
	ASSERT_ARITY(0);
	UNUSED(top);
	if (test_lib_try > 0) {
		test_lib_try -= 1;
	}
	return VM_NONE_E;
}

vm_exception_t vm_lib_mem_stat(vm_variable_t* top, uint32_t arguments) {
	ASSERT_ARITY_LE(1);
	uint32_t stat[] = {
		vm_mem_const.used,
		vm_mem_hashmap.used,
		vm_mem_array.used,
		vm_mem_string.used,
		vm_mem_thread.used,
		vm_memmap.used - vm_memmap.stack.used
	};
	const uint32_t size = sizeof(stat) / sizeof(stat[0]);
	vm_mmid_t id = (arguments == 0) ? vm_array_create(size) : top[-1].data.m;
	vm_array_t* array = MMID_TO_PTR(id, vm_array_t*);
	if (array->used != size) {
		THROW("invalid array size");
	}
	for (uint32_t i = 0; i < size; i++) {
		vm_array_set(array, i, VM_VARIABLE_INTEGER(stat[i]));
	}
	if (arguments == 0) {
		top[1] = VM_VARIABLE_MMID(VM_ARRAY_T, id);
	}
	return VM_NONE_E;
}

vm_exception_t vm_lib_dbgbrk(vm_variable_t* top, uint32_t arguments) {
	UNUSED(top);
	UNUSED(arguments);
	return VM_NONE_E;
}