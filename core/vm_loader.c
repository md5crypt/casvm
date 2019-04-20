#include <string.h>
#include <stdlib.h>
#include "vm_loader.h"
#include "vm.h"
#include "vm_util.h"
#include "vm_extern.h"
#include "vm_hashmap.h"
#include "vm_string.h"
#include "vm_symbols.h"

typedef struct {
	char name[16];
	uint32_t size;
	uint8_t data[];
} vm_loader_section_t;

typedef struct {
	vm_type_t type;
	vm_mmid_t name;
	vm_mmid_t parent;
	uint32_t code;
} vm_loader_object_t;

typedef vm_loader_error_t (*section_loader_t)(const uint8_t* data, uint32_t size);

typedef struct {
	const char* name;
	const section_loader_t func;
} section_loader_def_t;

static wstring_t** externs = NULL;
void* vm_loader_error_data = NULL;

static wstring_t** section_create_stringmap(const uint8_t* data) {
	uint32_t count = ((uint32_t*)data)[0];
	data += 4;
	wstring_t** map = (wstring_t**)malloc(count * sizeof(wstring_t**));
	for (uint32_t i = 0; i < count; i++) {
		map[i] = (wstring_t*)data;
		uint32_t size = map[i]->size;
		data += 4 + ((size + (size&1)) * 2);
	}
	return map;
}

static vm_loader_error_t section_loader_progmem(const uint8_t* data, uint32_t size) {
	UNUSED(size);
	vm_progmem = (vm_opcode_t*)data;
	return VM_LOADER_ERROR_NONE;
}

static vm_loader_error_t section_loader_shift(const uint8_t* data, uint32_t size) {
	UNUSED(size);
	vm_memmap_set_offset(((uint32_t*)data)[0]);
	return VM_LOADER_ERROR_NONE;
}

static vm_loader_error_t section_loader_object(const uint8_t* data, uint32_t size) {
	vm_loader_object_t* objects = (vm_loader_object_t*)data;
	vm_loader_object_t* end = (vm_loader_object_t*)(data + size);
	while (objects < end) {
		void* address = (void*)objects->code;
		vm_type_t type = objects->type;
		if (objects->type == VM_EXTERN_T) {
			address = (void*)vm_extern_native_resolve(externs[objects->code]);
			if (address == NULL) {
				address = (void*)vm_extern_resolve(externs[objects->code]);
				if (address == (void*)0xFFFFFFFF) {
					vm_loader_error_data = externs[objects->code];
					return VM_LOADER_ERROR_EXTERN;
				} else {
					type = VM_EXTERN_T;
				}
			} else {
				type = VM_NATIVE_T;
			}
		}
		uint32_t mmid = vm_hashmap_create(8, type, objects->name, objects->parent, address);
		if (objects->parent == MMID_NULL) {
			vm_root = mmid;
		}
		objects += 1;
	}
	return VM_LOADER_ERROR_NONE;
}

static vm_loader_error_t section_loader_string(const uint8_t* data, uint32_t size) {
	UNUSED(size);
	uint32_t count = ((uint32_t*)data)[0];
	data += 4;
	for (uint32_t i = 0; i < count; i++) {
		wstring_t* str = (wstring_t*)data;
		vm_string_insert(str->data, str->size);
		data += 4 + ((str->size + (str->size&1)) * 2);
	}
	return VM_LOADER_ERROR_NONE;
}

static vm_loader_error_t section_loader_extern(const uint8_t* data, uint32_t size) {
	UNUSED(size);
	externs = section_create_stringmap(data);
	return VM_LOADER_ERROR_NONE;
}

static vm_loader_error_t section_loader_sym_string(const uint8_t* data, uint32_t size) {
	UNUSED(size);
	vm_symbols.strings = section_create_stringmap(data);
	return VM_LOADER_ERROR_NONE;
}

static vm_loader_error_t section_loader_sym_file(const uint8_t* data, uint32_t size) {
	vm_symbols.files = (vm_symbols_entry_t*)data;
	vm_symbols.files_count = size / sizeof(vm_symbols_entry_t);
	return VM_LOADER_ERROR_NONE;
}

static vm_loader_error_t section_loader_sym_func(const uint8_t* data, uint32_t size) {
	vm_symbols.functions = (vm_symbols_entry_t*)data;
	vm_symbols.functions_count = size / sizeof(vm_symbols_entry_t);
	return VM_LOADER_ERROR_NONE;
}

static vm_loader_error_t section_loader_sym_line(const uint8_t* data, uint32_t size) {
	vm_symbols.lines = (vm_symbols_entry_t*)data;
	vm_symbols.lines_count = size / sizeof(vm_symbols_entry_t);
	return VM_LOADER_ERROR_NONE;
}

const section_loader_def_t section_loaders[] = {
	{"PROGMEM",    section_loader_progmem},
	{"SHIFT",      section_loader_shift},
	{"OBJECT",     section_loader_object},
	{"STRING",     section_loader_string},
	{"EXTERN",     section_loader_extern},
	{"SYM_STRING", section_loader_sym_string},
	{"SYM_FILE",   section_loader_sym_file},
	{"SYM_FUNC",   section_loader_sym_func},
	{"SYM_LINE",   section_loader_sym_line},
};

vm_loader_error_t vm_loader_load(const uint8_t* data, uint32_t size) {
	if (strncmp((char*)data, "ASB", 4) != 0) {
		return VM_LOADER_ERROR_MAGICDWORD;
	}
	const uint8_t* end = data + size;
	data += 4;
	while (data < end) {
		vm_loader_section_t* section = (vm_loader_section_t*)data;
		section_loader_t func = NULL;
		for (uint32_t i = 0; i < sizeof(section_loaders) / sizeof(section_loaders[0]); i++) {
			if (strncmp(section->name, section_loaders[i].name, 16) == 0) {
				func = section_loaders[i].func;
				break;
			}
		}
		if (func == NULL) {
			vm_loader_error_data = section->name;
			return VM_LOADER_ERROR_SECTION;
		}
		vm_loader_error_t ret = func(section->data, section->size);
		if (ret != VM_LOADER_ERROR_NONE) {
			return ret;
		}
		data += section->size + sizeof(vm_loader_section_t);
	}
	return VM_LOADER_ERROR_NONE;
}

const void* vm_loader_get_error_data(void) {
	return vm_loader_error_data;
}
