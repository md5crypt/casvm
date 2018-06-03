#include "vm_lib.h"

vm_exception_t vm_lib_string_slice(vm_variable_t* top, uint32_t arguments) {
	ASSERT_ARITY_RANGE(1,3);
	ASSERT_TYPE(1,VM_STRING_T);
	vm_string_t* string = VM_CAST_STRING(top-1);
	int32_t start = 0;
	int32_t stop = string->size;
	if (arguments > 1) {
		ASSERT_TYPE(2,VM_INTEGER_T);
		start = top[-2].data.i;
	}
	if (arguments > 2) {
		ASSERT_TYPE(3,VM_INTEGER_T);
		stop = top[-3].data.i;
	}
	top[1] = (vm_variable_t) {.data.m=vm_string_slice(string,start,stop),.type=VM_STRING_T};
	return VM_NONE_E;
}

vm_exception_t vm_lib_string_concat(vm_variable_t* top, uint32_t arguments) {
	vm_variable_t* arg = top-1;
	uint32_t len = 0;
	for (uint32_t i=0; i<arguments; i++) {
		switch(arg->type) {
			case VM_STRING_T:
				len += VM_CAST_STRING(arg)->size;
				break;
			case VM_INTEGER_T:{
				vm_mmid_t id = lib_int2str(arg->data.i);
				len += MMID_TO_PTR(id,vm_string_t*)->size;
				*arg = (vm_variable_t) {.type=VM_STRING_T, .data.m=id};
				break;
			}case VM_FLOAT_T:{
				vm_mmid_t id = lib_float2str(arg->data.f);
				len += MMID_TO_PTR(id,vm_string_t*)->size;
				*arg = (vm_variable_t) {.type=VM_STRING_T, .data.m=id};
				break;
			}default:{
				vm_mmid_t id = lib_type2str(arg->type);
				len += MMID_TO_PTR(id,vm_string_t*)->size + 3;
			}
		}
		arg -= 1;
	}
	arg = top-1;
	vm_mmid_t id = vm_string_create(len);
	uint16_t* dest = MMID_TO_PTR(id,vm_string_t*)->data;
	for (uint32_t i=0; i<arguments; i++) {
		if (arg->type == VM_STRING_T) {
			vm_string_t* src = VM_CAST_STRING(arg);
			for (uint32_t j=0; j<src->size; j++)
				*(dest++) = src->data[j];
		} else {
			*(dest++) = '<';
			*(dest++) = ':';
			vm_string_t* src = MMID_TO_PTR(lib_type2str(arg->type),vm_string_t*);
			for (uint32_t j=0; j<src->size; j++)
				*(dest++) = src->data[j];
			*(dest++) = '>';
		}
		arg -= 1;
	}
	top[1] = (vm_variable_t) {.type=VM_STRING_T, .data.m=id};
	return VM_NONE_E;
}

vm_exception_t vm_lib_string_find(vm_variable_t* top, uint32_t arguments) {
	ASSERT_ARITY_RANGE(2,3);
	ASSERT_TYPE(1,VM_STRING_T);
	ASSERT_TYPE(2,VM_STRING_T);
	vm_string_t* haystack = VM_CAST_STRING(top-1);
	vm_string_t* needle = VM_CAST_STRING(top-2);
	int32_t offset = 0;
	if (arguments == 3) {
		ASSERT_TYPE(3,VM_INTEGER_T);
		offset = top[-3].data.i;
	}
	int32_t result = vm_string_find(haystack,needle,offset);
	if (result == -2) {
		vm_exception_oob(offset, haystack->size);
		return VM_OOB_E;
	}
	top[1] = (vm_variable_t) {.type=VM_INTEGER_T,.data.i=result};
	return VM_NONE_E;
}
