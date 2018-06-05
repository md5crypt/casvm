#include <stddef.h>
#include "vm.h"
#include "vm_symbols.h"
#include "vm_hashmap.h"

vm_symbols_t vm_symbols;

static inline uint32_t upper_bound(vm_symbols_entry_t* data, uint32_t size, uint32_t key) {
	uint32_t lo = 0;
	uint32_t hi = size - 1;
	while (lo < hi) {
		uint32_t mid = lo + ((hi - lo) >> 1);
		if (data[mid].key > key) {
			hi = mid;
		} else if (data[mid].key < key) {
			lo = mid + 1;
		} else {
			return mid;
		}
	}
	return lo;
}

static inline uint32_t lower_bound(vm_symbols_entry_t* data, uint32_t size, uint32_t key) {
	uint32_t lo = 0;
	uint32_t hi = size - 1;
	while (lo < hi) {
		uint32_t mid = lo + ((hi - lo) >> 1);
		if (data[mid].key > key) {
			hi = mid;
		} else if (data[mid].key < key) {
			lo = mid + 1;
		} else {
			return mid;
		}
	}
	return (data[lo].key > key && lo > 0) ? lo - 1 : lo;
}

uint32_t vm_symbols_get_line(uint32_t offset) {
	uint32_t index = upper_bound(vm_symbols.lines, vm_symbols.lines_count, offset);
	return vm_symbols.lines[index].value;
}

vm_mmid_t vm_symbols_get_function(uint32_t offset) {
	uint32_t index = lower_bound(vm_symbols.functions, vm_symbols.functions_count, offset);
	if (vm_symbols.functions[0].key > offset)
		return MMID_NULL;
	return vm_symbols.functions[index].value;
}

wstring_t* vm_symbols_get_file(uint32_t offset) {
	uint32_t index = lower_bound(vm_symbols.files, vm_symbols.files_count, offset);
	if (vm_symbols.files[0].key > offset)
		return NULL;
	return vm_symbols.strings[vm_symbols.files[index].value];
}

void vm_symbols_clear() {
	vm_symbols = (vm_symbols_t) {0};
}

void vm_symbols_get_location(uint32_t pc, vm_symbols_location_t* loc) {
	loc->pc = pc;
	loc->file = (vm_symbols.files == NULL)?NULL:vm_symbols_get_file(pc);
	loc->line = (vm_symbols.lines == NULL)?0:vm_symbols_get_line(pc);
	loc->function = NULL;
	if (vm_symbols.functions != NULL) {
		vm_mmid_t func = vm_symbols_get_function(pc);
		if (func != MMID_NULL) {
			vm_mmid_t name = MMID_TO_PTR(func, vm_hashmap_t*)->name;
			loc->function = (wstring_t*)(MMID_TO_PTR(name, uint8_t*) + 4);
		}
	}
}
