#include <stdio.h>
#include "vm_lib.h"

vm_exception_t vm_lib_print(vm_variable_t* top, uint32_t arguments){
	vm_variable_t* arg = top-1;
	for(uint32_t i=1; i<=arguments; i++)
		ASSERT_TYPE(i,VM_STRING_T);
	for(uint32_t i=0; i<arguments; i++){
		vm_string_t* str = MMID_TO_PTR(arg->data.m, vm_string_t*);
		for(uint32_t j=0; j<str->size; j++)
			putchar(str->data[j]);
		arg -= 1;
	}
	return VM_NONE_E;
}
