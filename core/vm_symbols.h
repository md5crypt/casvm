#pragma once

#include <stdint.h>
#include "vm_util.h"

typedef struct {
	uint32_t key;
	uint32_t value;
} vm_symbols_entry_t;

typedef struct {
	wstring_t** strings;
	vm_symbols_entry_t* files;
	vm_symbols_entry_t* functions;
	vm_symbols_entry_t* lines;
	uint32_t files_count;
	uint32_t functions_count;
	uint32_t lines_count;
} vm_symbols_t;

typedef struct {
	uint32_t pc;
	uint32_t line;
	wstring_t* file;
	wstring_t* function;
} vm_symbols_location_t;

extern vm_symbols_t vm_symbols;

uint32_t vm_symbols_get_line(uint32_t offset);

vm_mmid_t vm_symbols_get_function(uint32_t offset);

wstring_t* vm_symbols_get_file(uint32_t offset);

void vm_symbols_clear(void);

void vm_symbols_get_location(uint32_t pc, vm_symbols_location_t* loc);
