#include "vm_thread.h"
#include "vm_util.h"
#include "vm_conf.h"

#include <string.h>

static vm_mmid_t active_stack = MMID_NULL;
static vm_mmid_t active_queue = MMID_NULL;

void vm_thread_wait(vm_thread_t* thread, vm_thread_t* queue){
	if(queue->queue != MMID_NULL)
		thread->next = queue->queue;
	queue->queue = PTR_TO_MMID(thread);
}

vm_mmid_t vm_thread_create(uint32_t size){
	uint32_t bsize = size<THREAD_INITIAL_SIZE?THREAD_INITIAL_SIZE:npot(size);
	vm_mmid_t id = vm_memory_allocate(&vm_mem_thread,bsize*sizeof(vm_variable_t)+sizeof(vm_thread_t));
	vm_thread_t* ptr = MMID_TO_PTR(id, vm_thread_t*);
	ptr->rcnt = 1;
	ptr->size = bsize;
	ptr->top = 0;
	ptr->next = MMID_NULL;
	ptr->queue = MMID_NULL;
	return id;
}

void vm_thread_grow(vm_thread_t* thread, uint32_t amount){
	vm_mmid_t oldid = PTR_TO_MMID(thread);
	vm_mmid_t newid = vm_thread_create(thread->size < amount ? thread->size+amount : thread->size*2);
	vm_thread_t* oldptr = MMID_TO_PTR(oldid,vm_thread_t*);
	vm_thread_t* newptr = MMID_TO_PTR(newid,vm_thread_t*);
	*newptr = *oldptr;
	memcpy(newptr->stack,oldptr->stack,sizeof(vm_variable_t)*oldptr->size);
	vm_memory_replace(&vm_mem_array, oldid, newid);
}

void vm_thread_kill(vm_thread_t* thread){
	if(thread->state == VM_THREAD_STATE_FINISHED)
		return;
	if(thread->queue){
		vm_thread_push_m(thread->queue);
		thread->queue = MMID_NULL;
	}
	vm_stackitem_t* ptr = thread->stack+thread->top;
	vm_stackitem_t* base;
	do{
		base = thread->stack+(ptr--)->frame.base;
		while(ptr > base)
			vm_variable_dereference((ptr--)->variable);
	}while(base > thread->stack);
	thread->top = 0;
	thread->stack->variable = (vm_variable_t){.type=VM_UNDEFINED_T, .data.m=0};
	thread->state = VM_THREAD_STATE_FINISHED;
}

void vm_thread_free(vm_thread_t* thread){
	vm_thread_kill(thread);
	vm_variable_dereference(thread->stack->variable);
	vm_memory_free(&vm_mem_thread, PTR_TO_MMID(thread));
}

void vm_thread_push(vm_thread_t* thread){
	if(active_stack == MMID_NULL){
		active_stack = PTR_TO_MMID(thread);
	}else if(thread->next == MMID_NULL){
		thread->next = active_stack;
	}else{
		vm_thread_t* ptr = thread;
		while(ptr->next != MMID_NULL)
			ptr = MMID_TO_PTR(ptr->next,vm_thread_t*);
		ptr->next = active_stack;
	}
	active_stack = PTR_TO_MMID(thread);
	vm_reference(thread);
}

vm_mmid_t vm_thread_pop(){
	if(active_queue == MMID_NULL){
		if(active_stack == MMID_NULL)
			return MMID_NULL;
		vm_mmid_t head_id = active_stack;
		vm_thread_t* tail = MMID_TO_PTR(head_id,vm_thread_t*);
		if(tail->next != MMID_NULL){
			vm_mmid_t id = tail->next;
			tail->next = MMID_NULL;
			while(id != MMID_NULL){
				vm_thread_t* ptr = MMID_TO_PTR(id,vm_thread_t*);
				vm_mmid_t next_id = ptr->next;
				ptr->next = head_id;
				head_id = id;
				id = next_id;
			}
		}
		active_queue = MMID_NULL;
		active_stack = head_id;
	}
	vm_mmid_t id = active_queue;
	active_queue = MMID_TO_PTR(id,vm_thread_t*)->next;
	return id;
}
