#include <stdio.h>
#include <string.h>

#include "vm_lib.h"

char sprintf_buffer[64];
vm_mmid_t type_string[VM_TYPE_COUNT];

//static vm_mmid_t type_to_string(vm_type_t type){
//	return type_string[type];
//}

static vm_mmid_t int_to_string(int32_t n){
	uint32_t len = sprintf(sprintf_buffer,"%d",n);
	return ascii_to_string(sprintf_buffer, len);
}

static uint32_t double_to_string(double f){
	uint32_t len = sprintf(sprintf_buffer,"%g",f);
	return ascii_to_string(sprintf_buffer, len);
}

void vm_stdlib_init(){
	for(uint32_t i=0; i<VM_TYPE_COUNT; i++){
		vm_mmid_t id = vm_lib_(vm_type_names[i],strlen(vm_type_names[i]));
		type_string[i] = vm_string_intern(MMID_TO_PTR(id,vm_string_t*));
	}
}


#include "vm_stdlib_exports.h"
