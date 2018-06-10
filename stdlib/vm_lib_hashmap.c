#include "vm_lib.h"

vm_exception_t vm_lib_hashmap_has(vm_variable_t* top, uint32_t arguments) {
	ASSERT_ARITY(2);
	ASSERT_TYPE_WEAK(1, VM_HASHMAP_T);
	ASSERT_TYPE(2, VM_STRING_T);
	top[1] = VM_VARIABLE_BOOL(vm_hashmap_has(
		VM_CAST_HASHMAP(top - 1),
		vm_string_intern(VM_CAST_STRING(top - 2))
	));
	return VM_NONE_E;
}

vm_exception_t vm_lib_hashmap_keys(vm_variable_t* top, uint32_t arguments) {
	ASSERT_ARITY(1);
	ASSERT_TYPE_WEAK(1, VM_HASHMAP_T);
	top[1] = VM_VARIABLE_MMID(VM_ARRAY_T, vm_hashmap_keys(VM_CAST_HASHMAP(top - 1)));
	return VM_NONE_E;
}

vm_exception_t vm_lib_hashmap_values(vm_variable_t* top, uint32_t arguments) {
	ASSERT_ARITY(1);
	ASSERT_TYPE_WEAK(1, VM_HASHMAP_T);
	top[1] = VM_VARIABLE_MMID(VM_ARRAY_T, vm_hashmap_values(VM_CAST_HASHMAP(top - 1)));
	return VM_NONE_E;
}
