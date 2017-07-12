#include <stdio.h>

#include "vm.h"
#include "vm_util.h"
#include "vm_string.h"
#include "vm_hashmap.h"
#include "vm_array.h"

#define UNUSED(x) (void)x

vm_exception_t echo(vm_variable_t* top, vm_stackframe_t* frame){
	UNUSED(frame);
	vm_string_t* str = MMID_TO_PTR(top->data.m, vm_string_t*);
	for(uint32_t i=0; i<str->size; i+=2)
		putchar(str->data[i]);
	putchar('\n');
	return VM_NONE_E;
}

vm_native_t vm_native_api[] = {
	echo
};
