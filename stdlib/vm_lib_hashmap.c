#include "vm_lib.h"

vm_exception_t vm_lib_hashmap_keys(vm_variable_t* top, uint32_t arguments) {
	ASSERT_ARITY(1);
	ASSERT_TYPE_WEAK(1,VM_HASHMAP_T);
	top[1] = (vm_variable_t) {.type=VM_ARRAY_T, .data.m=vm_hashmap_keys(VM_CAST_HASHMAP(top-1))};
	return VM_NONE_E;
}

vm_exception_t vm_lib_hashmap_values(vm_variable_t* top, uint32_t arguments) {
	ASSERT_ARITY(1);
	ASSERT_TYPE_WEAK(1,VM_HASHMAP_T);
	top[1] = (vm_variable_t) {.type=VM_ARRAY_T, .data.m=vm_hashmap_values(VM_CAST_HASHMAP(top-1))};
	return VM_NONE_E;
}
