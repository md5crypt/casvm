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

static vm_mmid_t main_thread;
static vm_mmid_t current_thread;

static struct {
	vm_mmid_t thread;
	vm_exception_t exception;
} last_fault;

const vm_destructor_t vm_destructor_lut[VM_TYPE_COUNT] = {
	[VM_ARRAY_T] = (vm_destructor_t)vm_array_free,
	[VM_STRING_T] = (vm_destructor_t)vm_string_free,
	[VM_THREAD_T] = (vm_destructor_t)vm_thread_free
};

void vm_init() {
	vm_memmap_init(MEMMAP_SIZE, MEMMAP_STACK_SIZE);
	vm_memory_init(&vm_mem_const, MEMORY_CONST_SIZE);
	vm_memory_init(&vm_mem_hashmap, MEMORY_HASHMAP_SIZE);
	vm_memory_init(&vm_mem_array, MEMORY_ARRAY_SIZE);
	vm_memory_init(&vm_mem_string, MEMORY_STRING_SIZE);
	vm_memory_init(&vm_mem_thread, MEMORY_THREAD_SIZE);
	vm_stringset_init(STRINGSET_SIZE);
	main_thread = vm_thread_create(0);
	vm_make_const(main_thread);
	vm_symbols_clear();
	last_fault.exception = VM_NONE_E;
}

void vm_call(uint32_t address) {
	vm_thread_t* thread = MMID_TO_PTR(main_thread, vm_thread_t*);
	if (thread->state == VM_THREAD_STATE_FINISHED) {
		vm_variable_dereference(thread->stack->variable);
	}
	vm_thread_stackframe_pack(&thread->stack[0].frame, 0, 0xFFFFFFFF, 0);
	vm_thread_stackframe_pack(&thread->stack[1].frame, address, 0, 0);
	thread->top = 1;
	thread->state = VM_THREAD_STATE_PAUSED;
	vm_thread_push(thread);
}

vm_exception_t vm_run() {
	if (last_fault.exception != VM_NONE_E) {
		return last_fault.exception;
	}
	while (true) {
		vm_mmid_t thread_id = vm_thread_pop();
		if (thread_id == MMID_NULL) {
			return VM_NONE_E;
		}
		current_thread = thread_id;
		vm_exception_t e = vm_mainloop(thread_id);
		if (e != VM_YIELD_E && e != VM_NONE_E) {
			last_fault.thread = thread_id;
			last_fault.exception = e;
			return e;
		}
	}
	current_thread = MMID_NULL;
}

void vm_fault_recover() {
	if (last_fault.exception != VM_NONE_E) {
		last_fault.exception = VM_NONE_E;
	}
}

vm_mmid_t vm_fault_get_thread() {
	return last_fault.thread;
}

bool vm_fault_trace(vm_symbols_location_t* loc) {
	vm_thread_t* thread = MMID_TO_PTR(main_thread, vm_thread_t*);
	vm_symbols_get_location(thread->stack[thread->top].frame.lower.link, loc);
	return vm_thread_unwind(thread);
}

void vm_dereference(void* ptr, vm_type_t type) {
	vm_destructor_t destructor = vm_destructor_lut[type];
	if (destructor) {
		uint32_t cnt = ((uint32_t*)ptr)[0];
		if (cnt <= 1) {
			destructor(ptr);
		} else if (cnt != VM_CONSTANT) {
			((uint32_t*)ptr)[0] = cnt - 1;
		}
	}
}

void vm_dereference_m(vm_mmid_t id, vm_type_t type) {
	vm_destructor_t destructor = vm_destructor_lut[type];
	if (destructor) {
		void* ptr = MMID_TO_PTR(id, void*);
		uint32_t cnt = ((uint32_t*)ptr)[0];
		if (cnt <= 1) {
			destructor(ptr);
		} else if (cnt != VM_CONSTANT) {
			((uint32_t*)ptr)[0] = cnt - 1;
		}
	}
}

void vm_reference(vm_variable_data_t data, vm_type_t type) {
	vm_reference_inline(data, type);
}

vm_mmid_t vm_get_current_thread() {
	return current_thread;
}
