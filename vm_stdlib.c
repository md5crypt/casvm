#include <stdio.h>
#include <string.h>

#include "vm_stdlib.h"
#include "vm_util.h"
#include "vm_string.h"
#include "vm_hashmap.h"
#include "vm_array.h"

#define UNUSED(x) (void)x

#define ASSERT_ARRITY(cond) if(!(cond)){ return VM_ARRITY_E; }
#define ASSERT_TYPE(n,t) if((top-(n))->type != t){ return VM_TYPE_E; }

#define THROW(str) do{ top[1] = cstring("Negative array size"); return VM_USER_E; } while(0)

static char sprintf_buffer[64];
static vm_mmid_t type_string[VM_TYPE_COUNT];

static vm_mmid_t ascii_to_string(const char* cstr, uint32_t len){
	vm_mmid_t id = vm_string_create(len);
	vm_string_t* str = MMID_TO_PTR(id, vm_string_t*); 
	for(uint32_t i = 0; i<len; i++)
		str->data[i] = cstr[i];
	return id;
}

static vm_mmid_t type_to_string(vm_type_t type){
	return type_string[type];
}

static vm_mmid_t int_to_string(int32_t n){
	uint32_t len = sprintf(sprintf_buffer,"%d",n);
	return ascii_to_string(sprintf_buffer, len);
}

static uint32_t double_to_string(double f){
	uint32_t len = sprintf(sprintf_buffer,"%g",f);
	return ascii_to_string(sprintf_buffer, len);
}

static vm_variable_t cstring(const char* cstr){
	return (vm_variable_t){
		.type=VM_STRING_T,
		.data.m=ascii_to_string(cstr,strlen(cstr))
	};
}

static vm_exception_t lib_itos(vm_variable_t* top, uint32_t arguments){
	ASSERT_ARRITY(arguments==1);
	ASSERT_TYPE(1,VM_INTEGER_T);
	top[1] = (vm_variable_t){.type=VM_STRING_T, .data.m=int_to_string((top-1)->data.i)};
	return VM_NONE_E;
}

static vm_exception_t lib_dtos(vm_variable_t* top, uint32_t arguments){
	ASSERT_ARRITY(arguments==1);
	ASSERT_TYPE(1,VM_FLOAT_T);
	top[1] = (vm_variable_t){.type=VM_STRING_T, .data.m=double_to_string((top-1)->data.f)};
	return VM_NONE_E;
}

static vm_exception_t lib_typeof(vm_variable_t* top, uint32_t arguments){
	ASSERT_ARRITY(arguments==1);
	top[1] = (vm_variable_t){.type=VM_STRING_T, .data.m=type_to_string((top-1)->type)};
	return VM_NONE_E;
}

static vm_exception_t lib_nameof(vm_variable_t* top, uint32_t arguments){
	ASSERT_ARRITY(arguments==1);
	if(!VM_ISTYPE((top-1)->type, VM_OBJECT_T))
		return VM_TYPE_E;
	top[1] = (vm_variable_t){.type=VM_STRING_T, .data.m=MMID_TO_PTR((top-1)->data.m,vm_hashmap_t*)->name};
	return VM_NONE_E;
}

static vm_exception_t lib_string_concat(vm_variable_t* top, uint32_t arguments){
	vm_variable_t* arg = top-1;
	uint32_t len = 0;
	for(uint32_t i=0; i<arguments; i++){
		switch(arg->type){
			case VM_STRING_T:
				len += MMID_TO_PTR(arg->data.m,vm_string_t*)->size;
				break;
			case VM_INTEGER_T:{
				vm_mmid_t id = int_to_string(arg->data.i);
				len += MMID_TO_PTR(id,vm_string_t*)->size;
				*arg = (vm_variable_t){.type=VM_STRING_T, .data.m=id};
				break;
			}case VM_FLOAT_T:{
				vm_mmid_t id = double_to_string(arg->data.f);
				len += MMID_TO_PTR(id,vm_string_t*)->size;
				*arg = (vm_variable_t){.type=VM_STRING_T, .data.m=id};
				break;
			}default:{
				vm_mmid_t id = type_to_string(arg->type);
				len += MMID_TO_PTR(id,vm_string_t*)->size + 3;
			}
		}
		arg -= 1;
	}
	arg = top-1;
	vm_mmid_t id = vm_string_create(len);
	uint16_t* dest = MMID_TO_PTR(id,vm_string_t*)->data;
	for(uint32_t i=0; i<arguments; i++){
		if(arg->type == VM_STRING_T){
			vm_string_t* src = MMID_TO_PTR(arg->data.m,vm_string_t*);
			for(uint32_t j=0; j<src->size; j++)
				*(dest++) = src->data[j];
		}else{
			*(dest++) = '[';
			*(dest++) = ':';
			vm_string_t* src = MMID_TO_PTR(type_to_string(arg->type),vm_string_t*);
			for(uint32_t j=0; j<src->size; j++)
				*(dest++) = src->data[j];
			*(dest++) = ']';
		}
		arg -= 1;
	}
	top[1] = (vm_variable_t){.type=VM_STRING_T, .data.m=id};
	return VM_NONE_E;
}

static vm_exception_t lib_print(vm_variable_t* top, uint32_t arguments){
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

static vm_exception_t lib_array_create(vm_variable_t* top, uint32_t arguments){
	ASSERT_ARRITY(arguments>=1);
	ASSERT_TYPE(1,VM_INTEGER_T);
	int32_t size = (top-1)->data.i;
	if(size < 0)
		THROW("Negative array size");
	vm_mmid_t id = vm_array_create(size);
	if(arguments > 1){
		vm_variable_t* data = MMID_TO_PTR(id,vm_array_t*)->data;
		for(uint32_t i=2; i<=arguments; i++)
			*(data++) = *(top-i);
	}
	top[1] = (vm_variable_t){.type=VM_ARRAY_T, .data.m=id};
	return VM_NONE_E;
}

static vm_exception_t lib_array_push(vm_variable_t* top, uint32_t arguments){
	ASSERT_ARRITY(arguments>1);
	ASSERT_TYPE(1,VM_ARRAY_T);
	vm_mmid_t array = (top-1)->data.m;
	for(uint32_t i=2; i<=arguments; i++)
		vm_array_push(array,*(top-i));
	top[1] = *(top-1);
	vm_variable_reference(top[1]);
	return VM_NONE_E;
}

static vm_exception_t lib_array_pop(vm_variable_t* top, uint32_t arguments){
	ASSERT_ARRITY(arguments==1);
	ASSERT_TYPE(1,VM_ARRAY_T);
	top[1] = vm_array_pop((top-1)->data.m);
	return VM_NONE_E;
}

static vm_exception_t lib_length(vm_variable_t* top, uint32_t arguments){
	ASSERT_ARRITY(arguments==1);
	vm_type_t type = (top-1)->type;
	if(type == VM_ARRAY_T){
		top[1] = (vm_variable_t){.type=VM_INTEGER_T, .data.m=MMID_TO_PTR((top-1)->data.m,vm_array_t*)->used};
	}else if(type == VM_STRING_T){
		top[1] = (vm_variable_t){.type=VM_INTEGER_T, .data.m=MMID_TO_PTR((top-1)->data.m,vm_string_t*)->size};
	}else{
		return VM_TYPE_E;
	}
	return VM_NONE_E;
}

const vm_stdlib_t vm_stdlib[] = {
	{"__itos", lib_itos}, 
	{"__dtos", lib_dtos}, 
	{"__typeof", lib_typeof}, 
	{"__nameof", lib_nameof}, 
	{"__string_concat", lib_string_concat}, 
	{"__print", lib_print}, 
	{"__array_create", lib_array_create}, 
	{"__array_push", lib_array_push}, 
	{"__array_pop", lib_array_pop}, 
	{"__length", lib_length}, 
	{NULL, NULL}
};

void vm_stdlib_init(){
	for(uint32_t i=0; i<VM_TYPE_COUNT; i++){
		vm_mmid_t id = ascii_to_string(vm_type_names[i],strlen(vm_type_names[i]));
		type_string[i] = vm_string_intern(id);
	}
}