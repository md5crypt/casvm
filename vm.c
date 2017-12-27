#include <stdio.h>
#include <string.h>

#include "vm.h"
#include "vm_thread.h"
#include "vm_util.h"
#include "vm_string.h"
#include "vm_hashmap.h"
#include "vm_array.h"
#include "vm_stdlib.h"
#include "vm_op.h"
#include "vm_conf.h"
#include "vm_mainloop.h"

vm_memory_t vm_mem_const;
vm_memory_t vm_mem_hashmap;
vm_memory_t vm_mem_array;
vm_memory_t vm_mem_string;
vm_memory_t vm_mem_thread;
vm_mmid_t main_thread;

const vm_opcode_t* vm_progmem = NULL;

const vm_destructor_t vm_destructor_lut[VM_TYPE_COUNT] = {
	[VM_ARRAY_T] = vm_array_free,
	[VM_STRING_T] = vm_string_free,
	[VM_THREAD_T] = vm_thread_free
};

void vm_init(uint32_t mmid_offset){
	vm_memmap_init(MEMMAP_SIZE, MEMMAP_STACK_SIZE);
	vm_memory_init(&vm_mem_const, MEMORY_CONST_SIZE);
	vm_memory_init(&vm_mem_hashmap, MEMORY_HASHMAP_SIZE);
	vm_memory_init(&vm_mem_array, MEMORY_ARRAY_SIZE);
	vm_memory_init(&vm_mem_string, MEMORY_STRING_SIZE);
	vm_memory_init(&vm_mem_thread, MEMORY_THREAD_SIZE);
	vm_stringset_init(STRINGSET_SIZE);
	main_thread = vm_thread_create(0);
	vm_make_const(main_thread);
	vm_memmap_set_offset(mmid_offset);
}

bool vm_extern_resolve(vm_mmid_t hashmap, const char* name){
	const vm_stdlib_t* item = vm_stdlib;
	while(item->name != NULL){
		if(strcmp(item->name,name)==0){
			vm_hashmap_t* h = MMID_TO_PTR(hashmap, vm_hashmap_t*);
			h->code.native = item->function;
			h->type = VM_NATIVE_T;
			return true;
		}
		item += 1;
	}
	return false;
}

void vm_call(const vm_opcode_t* address){
	vm_thread_t* thread = MMID_TO_PTR(main_thread, vm_thread_t*);
	if(thread->state == VM_THREAD_STATE_FINISHED)
		vm_variable_dereference(thread->stack->variable);
	thread->stack[0].frame = VM_PACK_STACKFRAME(0,0,0);
	thread->stack[1].frame = VM_PACK_STACKFRAME(address-vm_progmem,0,0);
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
	while(true){
		vm_mmid_t thread_id = vm_thread_pop();
		if(thread_id == MMID_NULL)
			return VM_NONE_E;
		vm_exception_t e = vm_mainloop(thread_id);
		vm_dereference_m(thread_id,VM_THREAD_T);
		if(e != VM_YIELD_E && e != VM_NONE_E)
			return e;
	}
}