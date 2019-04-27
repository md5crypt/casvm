#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "vm.h"
#include "vm_loader.h"
#include "vm_util.h"
#include "vm_thread.h"
#include "vm_string.h"
#include "vm_extern.h"

typedef struct {
	uint8_t* data;
	uint32_t size;
} filedata_t;

static const char* exception_names[] = {
	"NONE", "YIELD", "USER", "OUT-OF-BOUNDS", "TYPE", "ARITY", "IMMUTABLE", "DIV0", "INTERNAL"
};

extern uint32_t test_lib_try;

filedata_t readfile(const char* path) {
	FILE* fp = fopen(path, "rb");
	if (fp == NULL) {
		return (filedata_t) {NULL, 0};
	}
	fseek(fp, 0, SEEK_END);
	uint32_t size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	uint8_t* data = (uint8_t*)malloc(size);
	if (fread(data, 1, size, fp) != size) {
		return (filedata_t) {NULL, 0};
	}
	fclose(fp);
	return (filedata_t) {data, size};
}

static void wstring_print(wstring_t* str) {
	for (uint32_t i = 0; i < str->size; i++) {
		putchar(str->data[i]);
	}
}

static void print_loc(vm_symbols_location_t* loc) {
	if (loc->function == NULL) {
		printf("vm-internal [pc:%d]\n", loc->pc);
	} else {
		wstring_print(loc->function);
		printf(":%d (", loc->line);
		if (loc->file == NULL) {
			printf("unknown");
		} else {
			wstring_print(loc->file);
		}
		printf(") [pc:%d]", loc->pc);
	}
}

static void print_exception(vm_exception_t e) {
	const vm_exception_data_t* data = vm_exception_data_get();
	printf("Exception (%s)", exception_names[e]);
	switch (e) {
		case VM_NONE_E:
		case VM_YIELD_E:
		case VM_IMMUTABLE_E:
		case VM_INTERNAL_E:
		case VM_DIV0_E:
			break;
		case VM_USER_E:
			if (data->f1) {
				printf(": ");
				wstring_print((wstring_t*)data->f1);
			}
			break;
		case VM_OOB_E:
			printf(": accessed element %d out of %d", data->f1, data->f2);
			break;
		case VM_ARITY_E:
			printf(": passed %d arguments, expected %d", data->f1, data->f2);
			break;
		case VM_TYPE_E:
			printf(": got \"%s\", expected \"%s\"", vm_type_names[data->f1], vm_type_names[data->f2]);
			break;
	}
	putchar('\n');
	vm_symbols_location_t loc;
	while (vm_fault_trace(&loc)) {
		printf("  at ");
		print_loc(&loc);
		putchar('\n');
	}
}

bool run() {
	while (true) {
		vm_exception_t e = vm_run();
		if (e != VM_NONE_E) {
			if (test_lib_try == 0) {
				print_exception(e);
				return false;
			}
			vm_thread_t* thread = MMID_TO_PTR(vm_fault_get_thread(), vm_thread_t*);
			vm_thread_kill(thread, vm_string_cstr(exception_names[e], 0), VM_STRING_T);
			vm_fault_recover();
		} else {
			return true;
		}
	}
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		puts("usage: vm [input file]");
		return 1;
	}
	filedata_t image = readfile(argv[1]);
	if (image.data == NULL) {
		puts("error opening file");
		exit(1);
	}
	vm_init();
	vm_loader_error_t ret = vm_loader_load(image.data, image.size);
	switch (ret) {
		case VM_LOADER_ERROR_MAGICDWORD:
			puts("invalid magic number");
			exit(1);
		case VM_LOADER_ERROR_SECTION:
			printf("unknown section: '%s'\n", (char*)vm_loader_get_error_data());
			exit(1);
		case VM_LOADER_ERROR_EXTERN:
			printf("unresolved extern: '");
			wstring_print((wstring_t*)vm_loader_get_error_data());
			puts("'");
			exit(1);
		default:
			break;
	}
	vm_call(0);
	vm_run();
	vm_call(1);
	return !run();
}

vm_exception_t vm_extern_call(uint32_t id, vm_variable_t* top, uint32_t arguments) {
	(void) id;
	(void) top;
	(void) arguments;
	puts("vm_extern_call not implemented");
	exit(1);
}

uint32_t vm_extern_resolve(const wstring_t* str) {
	(void) str;
	return 0xFFFFFFFF;
}

vm_mmid_t lib_int2str(int32_t value) {
	char buffer[64];
	return vm_string_cstr(buffer, sprintf(buffer, "%d", value));
}

vm_mmid_t lib_float2str(float value) {
	char buffer[64];
	return vm_string_cstr(buffer, sprintf(buffer, "%g", value));
}
