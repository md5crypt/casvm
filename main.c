#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "vm.h"
#include "vm_hashmap.h"
#include "vm_string.h"

typedef struct {
	uint32_t size;
	uint32_t s_code;
	uint32_t s_object;
	uint32_t s_string;
	uint32_t mmid_offset;
	uint8_t data[0];
} image_header_t;

typedef struct {
	vm_type_t type;
	vm_mmid_t name;
	vm_mmid_t parent;
	uint32_t code;
} object_header_t;

typedef struct {
	uint32_t size;
	uint16_t data[0];
} string_header_t;

image_header_t* readfile(const char* path){
	FILE* fp = fopen(path, "rb");
	if(fp == NULL)
		return NULL;
	fseek(fp, 0, SEEK_END);
	uint32_t size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	void* data = malloc(size);
	if(fread(data, 1, size, fp) != size){
		fclose(fp);
		return NULL;
	}
	fclose(fp);
	image_header_t* const head = (image_header_t*)data;
	if(head->size != 0xB5006BB1)
		return NULL;
	head->size = size;
	return data;
}

int main(){
	image_header_t* head = readfile("C:\\Users\\Administrator\\Desktop\\asc\\__output\\image.bin");
	if(head == NULL){
		puts("error opening file");
		exit(1);
	}
	const vm_opcode_t* code = (vm_opcode_t*)head->data;
	const object_header_t* objects = (object_header_t*)(head->data + head->s_code);
	const string_header_t* strings = (string_header_t*)(head->data + head->s_code + head->s_object);
	const char* externs = (char*)(head->data + head->s_code + head->s_object + head->s_string);
	vm_init(head->mmid_offset);
	vm_progmem = code;
	while((void*)objects < (void*)strings){
		uint32_t mmid = vm_hashmap_create(8,objects->type,objects->name,objects->parent,(void*)(code+objects->code));
		if(objects->type == VM_EXTERN_T){
			if(!vm_extern_resolve(mmid,externs+objects->code)){
				printf("could not link external dependency '%s'\n",externs+objects->code);
				exit(1);
			}
		}
		objects += 1;
	}
	while((void*)strings < (void*)externs){
		vm_string_insert(strings->data,strings->size);
		strings += 1+(strings->size+(strings->size&1))/2;
	}
	vm_stdlib_init();
	vm_call(code);
	printf("%d\n",vm_run());
	return 0;
}
