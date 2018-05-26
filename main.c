#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "vm.h"
#include "vm_loader.h"
#include "vm_util.h"

typedef struct {
	uint8_t* data;
	uint32_t size;
} filedata_t;

filedata_t readfile(const char* path){
	FILE* fp = fopen(path, "rb");
	if(fp == NULL)
		return (filedata_t){NULL,0};
	fseek(fp, 0, SEEK_END);
	uint32_t size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	uint8_t* data = (uint8_t*)malloc(size);
	if(fread(data, 1, size, fp) != size)
		return (filedata_t){NULL,0};
	fclose(fp);
	return (filedata_t){data,size};
}

static void wstring_print(wstring_t* str){
	for(uint32_t i=0; i<str->size; i++)
		putchar(str->data[i]);
}

static void print_loc(vm_symbols_location_t* loc){
	if(loc->function == NULL){
		printf("vm-internal [pc:%d]\n",loc->pc);
	}else{
		wstring_print(loc->function);
		printf(":%d (",loc->line);
		if(loc->file == NULL){
			printf("unknown");
		}else{
			wstring_print(loc->file);
		}
		printf(") [pc:%d]",loc->pc);
	}
}

static void print_exception(vm_exception_t e){
	static const char* names[] = {
		"NONE","YIELD","USER","OUT-OF-BOUNDS","TYPE","ARRITY","IMMUTABLE","INTERNAL"
	};
	const vm_exception_data_t* data = vm_exception_data_get();
	printf("Exception (%s)",names[e]);
	switch(e){
		case VM_NONE_E:
		case VM_YIELD_E:
		case VM_IMMUTABLE_E:
		case VM_INTERNAL_E:
			break;
		case VM_USER_E:
			if(data->f1){
				printf(": ");
				wstring_print((wstring_t*)data->f1);
			}
			break;
		case VM_OOB_E:
			printf(": accessed element %d out of %d",data->f1,data->f2);
			break;
		case VM_ARRITY_E:
			printf(": passed %d arguments, expected %d",data->f1,data->f2);
			break;
		case VM_TYPE_E:
			printf(": got \"%s\", expected \"%s\"",vm_type_names[data->f1],vm_type_names[data->f2]);
			break;
	}
	putchar('\n');
	vm_symbols_location_t loc;
	while(vm_trace_next(&loc)){
		printf("  at ");
		print_loc(&loc);
		putchar('\n');
	}
}

int main(){
	filedata_t image = readfile("C:\\Users\\Administrator\\Desktop\\asc\\__output\\image.bin");
	if(image.data == NULL){
		puts("error opening file");
		exit(1);
	}
	vm_init();
	vm_loader_error_t ret = vm_loader_load(image.data,image.size);
	switch(ret){
		case VM_LOADER_ERROR_MAGICDWORD:
			puts("invalid magic number");
			exit(1);
		case VM_LOADER_ERROR_SECTION:
			printf("unknown section: '%s'\n",(char*)vm_loader_error_data);
			exit(1);
		case VM_LOADER_ERROR_EXTERN:
			printf("unresolved extern: '");
			wstring_print((wstring_t*)vm_loader_error_data);
			puts("'");
			exit(1);
		default:
			break;
	}
	vm_call(0);
	vm_exception_t e = vm_run();
	if(e != VM_NONE_E){
		print_exception(e);
	}
	return 0;
}
