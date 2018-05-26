#include <stdlib.h>
#include <stdio.h>
#include "vm_lib.h"

static char sprintf_buffer[64];

vm_mmid_t lib_int2str(int32_t value){
	return vm_string_cstr(sprintf_buffer,sprintf(sprintf_buffer,"%d",value));
}

vm_mmid_t lib_float2str(float value){
	return vm_string_cstr(sprintf_buffer,sprintf(sprintf_buffer,"%g",value));
}

vm_mmid_t lib_type2str(vm_type_t value){
	static vm_mmid_t type_strings[VM_TYPE_COUNT] = {0};
	if(type_strings[0] == 0){
		for(uint32_t i=0; i<VM_TYPE_COUNT; i++){
			vm_mmid_t id = vm_string_cstr(vm_type_names[i],strlen(vm_type_names[i]));
			type_strings[i] = vm_string_intern(MMID_TO_PTR(id,vm_string_t*));
		}
	}
	return type_strings[value];
}

vm_exception_t vm_lib_itos(vm_variable_t* top, uint32_t arguments){
	ASSERT_ARRITY(1);
	ASSERT_TYPE(1,VM_INTEGER_T);
	top[1] = (vm_variable_t){.type=VM_STRING_T, .data.m=lib_int2str((top-1)->data.i)};
	return VM_NONE_E;
}

vm_exception_t vm_lib_dtos(vm_variable_t* top, uint32_t arguments){
	ASSERT_ARRITY(1);
	ASSERT_TYPE(1,VM_FLOAT_T);
	top[1] = (vm_variable_t){.type=VM_STRING_T, .data.m=lib_float2str((top-1)->data.f)};
	return VM_NONE_E;
}

vm_exception_t vm_lib_typeof(vm_variable_t* top, uint32_t arguments){
	ASSERT_ARRITY(1);
	top[1] = (vm_variable_t){.type=VM_STRING_T, .data.m=lib_type2str((top-1)->type)};
	return VM_NONE_E;
}

vm_exception_t vm_lib_nameof(vm_variable_t* top, uint32_t arguments){
	ASSERT_ARRITY(1);
	ASSERT_TYPE_WEAK(1,VM_HASHMAP_T);
	top[1] = (vm_variable_t){.type=VM_STRING_T, .data.m=MMID_TO_PTR((top-1)->data.m,vm_hashmap_t*)->name};
	return VM_NONE_E;
}

vm_exception_t vm_lib_length(vm_variable_t* top, uint32_t arguments){
	ASSERT_ARRITY(1);
	vm_type_t type = (top-1)->type;
	if(type == VM_ARRAY_T){
		top[1] = (vm_variable_t){.type=VM_INTEGER_T, .data.m=VM_CAST_ARRAY(top-1)->used};
	}else if(type == VM_STRING_T){
		top[1] = (vm_variable_t){.type=VM_INTEGER_T, .data.m=VM_CAST_STRING(top-1)->size};
	}else if(VM_ISTYPE(type,VM_HASHMAP_T)){
		vm_hashmap_t* map = VM_CAST_HASHMAP(top-1);
		top[1] = (vm_variable_t){.type=VM_INTEGER_T, .data.m=map->used-map->deleted};
	}else{
		vm_exception_type((top-1)->type, VM_INDEXABLE_T);
		return VM_TYPE_E;
	}
	return VM_NONE_E;
}
