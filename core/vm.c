#include <string.h>

#include "vm.h"
#include "vm_thread.h"
#include "vm_util.h"
#include "vm_string.h"
#include "vm_hashmap.h"
#include "vm_array.h"
#include "vm_op.h"
#include "vm_conf.h"
#include "vm_mainloop.h"

vm_memory_t vm_mem_const;
vm_memory_t vm_mem_hashmap;
vm_memory_t vm_mem_array;
vm_memory_t vm_mem_string;
vm_memory_t vm_mem_thread;

const vm_opcode_t* vm_progmem = NULL;
vm_mmid_t vm_root;

static vm_mmid_t current_thread;

static struct {
	vm_mmid_t thread;
	vm_exception_t exception;
} last_fault;

const bool vm_destructor_lut[VM_TYPE_COUNT] = {
	[VM_ARRAY_T] = true,
	[VM_STRING_T] = true,
	[VM_THREAD_T] = true
};

void vm_init() {
	vm_memmap_init(MEMMAP_SIZE, MEMMAP_STACK_SIZE);
	vm_memory_init(&vm_mem_const, MEMORY_CONST_SIZE);
	vm_memory_init(&vm_mem_hashmap, MEMORY_HASHMAP_SIZE);
	vm_memory_init(&vm_mem_array, MEMORY_ARRAY_SIZE);
	vm_memory_init(&vm_mem_string, MEMORY_STRING_SIZE);
	vm_memory_init(&vm_mem_thread, MEMORY_THREAD_SIZE);
	vm_stringset_init(STRINGSET_SIZE);
	vm_symbols_clear();
	last_fault.exception = VM_NONE_E;
}

void vm_call(uint32_t address, uint32_t argc, const vm_variable_t* argv) {
	vm_thread_t* thread = MMID_TO_PTR(vm_thread_create(argc + 4), vm_thread_t*);
	thread->flags |= VM_THREAD_FLAG_DETACHED;
	for (uint32_t i = 0; i < argc; i++) {
		thread->stack[i].variable = argv[i];
	}
	vm_thread_stackframe_pack(&thread->stack[argc + 0].frame, 0, 0xFFFFFFFF, 0);
	vm_thread_stackframe_pack(&thread->stack[argc + 2].frame, address, argc, argc);
	thread->top = argc + 2;
	vm_thread_push(thread);
}

vm_exception_t vm_run() {
	if (last_fault.exception != VM_NONE_E) {
		return last_fault.exception;
	}
	while (true) {
		vm_mmid_t thread_id = vm_thread_pop();
		if (thread_id == MMID_NULL) {
			current_thread = MMID_NULL;
			return VM_NONE_E;
		}
		current_thread = thread_id;
		vm_reference_m(thread_id);
		vm_exception_t e = vm_mainloop(thread_id);
		if (e != VM_YIELD_E && e != VM_NONE_E) {
			current_thread = MMID_NULL;
			last_fault.thread = thread_id;
			last_fault.exception = e;
			return e;
		}
		vm_dereference_m(thread_id, VM_THREAD_T);
	}
}

void vm_fault_recover() {
	if (last_fault.exception != VM_NONE_E) {
		last_fault.exception = VM_NONE_E;
		vm_dereference_m(last_fault.thread, VM_THREAD_T);
	}
}

vm_mmid_t vm_fault_get_thread() {
	return last_fault.thread;
}

bool vm_fault_trace(vm_symbols_location_t* loc) {
	vm_thread_t* thread = MMID_TO_PTR(last_fault.thread, vm_thread_t*);
	vm_symbols_get_location(thread->stack[thread->top].frame.lower.link, loc);
	return vm_thread_unwind(thread);
}

static inline void destructor_dispatch(void* ptr, vm_type_t type) {
	switch (type) {
		case VM_ARRAY_T:
			vm_array_free(ptr);
			break;
		case VM_STRING_T:
			vm_string_free(ptr);
			break;
		case VM_THREAD_T:
			vm_thread_free(ptr);
			break;
		default:
			break;
	}
}

void vm_dereference(void* ptr, vm_type_t type) {
	if (vm_destructor_lut[type]) {
		uint32_t cnt = ((uint32_t*)ptr)[0];
		if (cnt <= 1) {
			destructor_dispatch(ptr, type);
		} else if (cnt != VM_CONSTANT) {
			((uint32_t*)ptr)[0] = cnt - 1;
		}
	}
}

void vm_dereference_m(vm_mmid_t id, vm_type_t type) {
	if (vm_destructor_lut[type]) {
		void* ptr = MMID_TO_PTR(id, void*);
		uint32_t cnt = ((uint32_t*)ptr)[0];
		if (cnt <= 1) {
			destructor_dispatch(ptr, type);
		} else if (cnt != VM_CONSTANT) {
			((uint32_t*)ptr)[0] = cnt - 1;
		}
	}
}

void vm_reference(void* ptr) {
	vm_reference_unsafe(ptr);
}

void vm_reference_m(vm_mmid_t id) {
	vm_reference_m_inline(id);
}

vm_mmid_t vm_get_current_thread() {
	return current_thread;
}
