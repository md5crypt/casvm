#include <stdio.h>
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
static struct {
	vm_mmid_t thread;
	vm_exception_t exception;
} last_error;

const vm_destructor_t vm_destructor_lut[VM_TYPE_COUNT] = {
	[VM_ARRAY_T] = vm_array_free,
	[VM_STRING_T] = vm_string_free,
	[VM_THREAD_T] = vm_thread_free
};

void vm_init(){
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
	last_error.exception = VM_NONE_E;
}

void vm_call(uint32_t address){
	vm_thread_t* thread = MMID_TO_PTR(main_thread, vm_thread_t*);
	if(thread->state == VM_THREAD_STATE_FINISHED)
		vm_variable_dereference(thread->stack->variable);
	thread->stack[0].frame = VM_PACK_STACKFRAME(0,0,0);
	thread->stack[1].frame = VM_PACK_STACKFRAME(address,0,0);
	thread->top = 1;
	thread->state = VM_THREAD_STATE_PAUSED;
	vm_thread_push(thread);
}

uint32_t vm_variable_compare(vm_variable_t a, vm_variable_t b){
	if(a.type != b.type)
		return 0;
	if(a.data.i == b.data.i)
		return 1;
	if(a.type == VM_STRING_T)
		return vm_string_cmp(VM_CAST_STRING(&a),VM_CAST_STRING(&b));
	return 0;
}

vm_exception_t vm_run(){
	if(last_error.exception != VM_NONE_E)
		return last_error.exception;
	while(true){
		vm_mmid_t thread_id = vm_thread_pop();
		if(thread_id == MMID_NULL)
			return VM_NONE_E;
		vm_exception_t e = vm_mainloop(thread_id);
		if(e != VM_YIELD_E && e != VM_NONE_E){
			last_error.thread = thread_id;
			last_error.exception = e;
			return e;
		}
		vm_dereference_m(thread_id,VM_THREAD_T);
	}
}

void vm_recover(){
	if(last_error.exception != VM_NONE_E){
		vm_dereference_m(last_error.thread,VM_THREAD_T);
		last_error.exception = VM_NONE_E;
	}
}


bool vm_trace_next(vm_symbols_location_t* loc){
	vm_thread_t* thread = MMID_TO_PTR(last_error.thread,vm_thread_t*);
	if(thread->top != 0){
		uint32_t pc = thread->stack[thread->top].frame.link;
		thread->top = thread->stack[thread->top].frame.base;
		vm_symbols_get_location(pc, loc);
		return true;
	}
	return false;
}