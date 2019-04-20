#include "vm_lib.h"

vm_exception_t vm_lib_array_create(vm_variable_t* top, uint32_t arguments) {
	ASSERT_ARITY_RANGE(1, 2);
	ASSERT_TYPE(1, VM_INTEGER_T);
	int32_t size = top[-1].data.i;
	if (size < 0) {
		THROW("Negative array size");
	}
	vm_mmid_t id = vm_array_create(size);
	if (arguments > 1) {
		vm_variable_t* data = MMID_TO_PTR(id, vm_array_t*)->data;
		while (size--) {
			vm_variable_reference(top[-2]);
			*(data++) = top[-2];
		}
	}
	top[1] = VM_VARIABLE_MMID(VM_ARRAY_T, id);
	return VM_NONE_E;
}

vm_exception_t vm_lib_array_static(vm_variable_t* top, uint32_t arguments) {
	vm_mmid_t id = vm_array_create(arguments);
	if (arguments > 0) {
		vm_variable_t* data = MMID_TO_PTR(id, vm_array_t*)->data;
		for (uint32_t i = 1; i <= arguments; i++) {
			vm_variable_reference(top[-i]);
			*(data++) = top[-i];
		}
	}
	top[1] = VM_VARIABLE_MMID(VM_ARRAY_T, id);
	return VM_NONE_E;
}

vm_exception_t vm_lib_array_push(vm_variable_t* top, uint32_t arguments) {
	ASSERT_ARITY_GE(2);
	ASSERT_TYPE(1, VM_ARRAY_T);
	vm_array_t* array = VM_CAST_ARRAY(top - 1);
	for (uint32_t i = 2; i <= arguments; i++) {
		array = vm_array_push(array, top[-i].data.i, top[-i].type);
	}
	top[1] = top[-1];
	vm_reference_unsafe(array);
	return VM_NONE_E;
}

vm_exception_t vm_lib_array_pop(vm_variable_t* top, uint32_t arguments) {
	ASSERT_ARITY(1);
	ASSERT_TYPE(1, VM_ARRAY_T);
	return vm_array_pop(VM_CAST_ARRAY(top - 1), top + 1);
}

vm_exception_t vm_lib_array_unshift(vm_variable_t* top, uint32_t arguments) {
	ASSERT_ARITY_GE(2);
	ASSERT_TYPE(1, VM_ARRAY_T);
	vm_array_t* array = VM_CAST_ARRAY(top - 1);
	for (uint32_t i = arguments; i >= 2; i--) {
		array = vm_array_unshift(array, top[-i].data.i, top[-i].type);
	}
	top[1] = top[-1];
	vm_reference_unsafe(array);
	return VM_NONE_E;
}

vm_exception_t vm_lib_array_shift(vm_variable_t* top, uint32_t arguments) {
	ASSERT_ARITY(1);
	ASSERT_TYPE(1, VM_ARRAY_T);
	return vm_array_shift(VM_CAST_ARRAY(top - 1), top + 1);
}

vm_exception_t vm_lib_array_resize(vm_variable_t* top, uint32_t arguments) {
	ASSERT_ARITY_RANGE(2, 3);
	ASSERT_TYPE(1, VM_ARRAY_T);
	ASSERT_TYPE(2, VM_INTEGER_T);
	if (top[-2].data.i < 0) {
		THROW("Negative array size");
	}
	vm_array_t* array = VM_CAST_ARRAY(top - 1);
	uint32_t oldsize = array->used;
	array = vm_array_resize(array, (uint32_t)top[-2].data.i);
	if (arguments == 3) {
		vm_array_fill(array, top[-3].data.i, top[-3].type, oldsize, array->used - oldsize);
	}
	vm_reference_unsafe(array);
	top[1] = top[-1];
	return VM_NONE_E;
}

vm_exception_t vm_lib_array_slice(vm_variable_t* top, uint32_t arguments) {
	ASSERT_ARITY_RANGE(1, 3);
	ASSERT_TYPE(1, VM_ARRAY_T);
	vm_array_t* array = VM_CAST_ARRAY(top - 1);
	int32_t start = 0;
	int32_t stop = array->used;
	if (arguments > 1) {
		ASSERT_TYPE(2, VM_INTEGER_T);
		start = top[-2].data.i;
	}
	if (arguments > 2) {
		ASSERT_TYPE(3, VM_INTEGER_T);
		stop = top[-3].data.i;
	}
	vm_mmid_t id = vm_array_slice(array, start, stop);
	if (id == MMID_NULL) {
		top[1] = VM_VARIABLE_OFTYPE(VM_INVALID_T);
		return VM_OOB_E;
	}
	top[1] = VM_VARIABLE_MMID(VM_ARRAY_T, id);
	return VM_NONE_E;
}


vm_exception_t vm_lib_array_write(vm_variable_t* top, uint32_t arguments) {
	ASSERT_ARITY_RANGE(2, 4);
	ASSERT_TYPE(1, VM_ARRAY_T);
	ASSERT_TYPE(2, VM_ARRAY_T);
	vm_array_t* dst = VM_CAST_ARRAY(top - 1);
	vm_array_t* src = VM_CAST_ARRAY(top - 2);
	int32_t offset = 0;
	int32_t length = src->used;
	if (arguments > 2) {
		ASSERT_TYPE(3, VM_INTEGER_T);
		offset = top[-3].data.i;
	}
	if (arguments > 3) {
		ASSERT_TYPE(4, VM_INTEGER_T);
		length = top[-4].data.i;
	}
	return vm_array_write(dst, src, offset, length);
}

vm_exception_t vm_lib_array_fill(vm_variable_t* top, uint32_t arguments) {
	ASSERT_ARITY_RANGE(2, 4);
	ASSERT_TYPE(1, VM_ARRAY_T);
	vm_array_t* array = VM_CAST_ARRAY(top - 1);
	int32_t offset = 0;
	int32_t length;
	if (arguments > 2) {
		ASSERT_TYPE(3, VM_INTEGER_T);
		offset = top[-3].data.i;
	}
	if (arguments > 3) {
		ASSERT_TYPE(4, VM_INTEGER_T);
		length = top[-4].data.i;
	} else {
		length = offset >= 0 ? (int32_t)(array->used - offset) : -offset;
	}
	return vm_array_fill(array, top[-2].data.i, top[-2].type, offset, length);
}

vm_exception_t vm_lib_array_find(vm_variable_t* top, uint32_t arguments) {
	ASSERT_ARITY_RANGE(2, 3);
	ASSERT_TYPE(1, VM_ARRAY_T);
	vm_array_t* array = VM_CAST_ARRAY(top - 1);
	int32_t offset = 0;
	if (arguments == 3) {
		ASSERT_TYPE(3, VM_INTEGER_T);
		offset = top[-3].data.i;
	}
	int32_t result = vm_array_find(array, top[-2].data.i, top[-2].type, offset);
	if (result == -2) {
		return VM_OOB_E;
	}
	top[1] = VM_VARIABLE_INTEGER(result);
	return VM_NONE_E;
}

vm_exception_t vm_lib_array_expand(vm_variable_t* top, uint32_t arguments) {
	ASSERT_ARITY_GE(2);
	uint32_t size = 0;
	for (uint32_t i = 1; i <= arguments; i++) {
		ASSERT_TYPE(i, VM_ARRAY_T);
		size += VM_CAST_ARRAY(top - i)->used;
	}
	vm_array_t* output = VM_CAST_ARRAY(top - 1);
	uint32_t offset = output->used;
	output = vm_array_resize(output, size);
	for (uint32_t i = 2; i <= arguments; i++) {
		vm_array_t* src = VM_CAST_ARRAY(top - i);
		vm_array_write(output, src, offset, src->used);
		offset += src->used;
	}
	vm_reference_unsafe(output);
	top[1] = top[-1];
	return VM_NONE_E;
}

vm_exception_t vm_lib_array_reverse(vm_variable_t* top, uint32_t arguments) {
	ASSERT_ARITY(1);
	ASSERT_TYPE(1, VM_ARRAY_T);
	vm_array_t* array = VM_CAST_ARRAY(top - 1);
	vm_array_reverse(array);
	vm_reference_unsafe(array);
	top[1] = top[-1];
	return VM_NONE_E;
}
