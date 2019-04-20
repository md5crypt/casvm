#include <stdlib.h>
#include "vm_lib.h"

vm_mmid_t lib_type2str(vm_type_t value) {
	static vm_mmid_t type_strings[VM_TYPE_COUNT] = {0};
	if (type_strings[0] == 0) {
		for (uint32_t i = 0; i < VM_TYPE_COUNT; i++) {
			vm_mmid_t id = vm_string_cstr(vm_type_names[i], strlen(vm_type_names[i]));
			type_strings[i] = vm_string_intern(MMID_TO_PTR(id, vm_string_t*));
		}
	}
	return type_strings[value];
}

vm_exception_t vm_lib_itos(vm_variable_t* top, uint32_t arguments) {
	ASSERT_ARITY(1);
	ASSERT_TYPE(1, VM_INTEGER_T);
	top[1] = VM_VARIABLE_MMID(VM_STRING_T, lib_int2str(top[-1].data.i));
	return VM_NONE_E;
}

vm_exception_t vm_lib_dtos(vm_variable_t* top, uint32_t arguments) {
	ASSERT_ARITY(1);
	ASSERT_TYPE(1, VM_FLOAT_T);
	top[1] = VM_VARIABLE_MMID(VM_STRING_T, lib_float2str(top[-1].data.f));
	return VM_NONE_E;
}

vm_exception_t vm_lib_typeof(vm_variable_t* top, uint32_t arguments) {
	ASSERT_ARITY(1);
	top[1] = VM_VARIABLE_MMID(VM_STRING_T, lib_type2str(top[-1].type));
	return VM_NONE_E;
}

vm_exception_t vm_lib_nameof(vm_variable_t* top, uint32_t arguments) {
	ASSERT_ARITY(1);
	ASSERT_TYPE_WEAK(1, VM_HASHMAP_T);
	top[1] = VM_VARIABLE_MMID(VM_STRING_T, MMID_TO_PTR(top[-1].data.m, vm_hashmap_t*)->name);
	return VM_NONE_E;
}

vm_exception_t vm_lib_length(vm_variable_t* top, uint32_t arguments) {
	ASSERT_ARITY(1);
	vm_type_t type = top[-1].type;
	if (type == VM_ARRAY_T) {
		top[1] = VM_VARIABLE_INTEGER(VM_CAST_ARRAY(top - 1)->used);
	} else if (type == VM_STRING_T) {
		top[1] = VM_VARIABLE_INTEGER(VM_CAST_STRING(top - 1)->size);
	} else if (VM_ISTYPE(type, VM_HASHMAP_T)) {
		vm_hashmap_t* map = VM_CAST_HASHMAP(top - 1);
		top[1] = VM_VARIABLE_INTEGER(map->used - map->deleted);
	} else {
		vm_exception_type(top[-1].type, VM_INDEXABLE_T);
		return VM_TYPE_E;
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
		vm_array_set(array, i, stat[i], VM_INTEGER_T);
	}
	if (arguments == 0) {
		top[1] = VM_VARIABLE_MMID(VM_ARRAY_T, id);
	}
	return VM_NONE_E;
}
