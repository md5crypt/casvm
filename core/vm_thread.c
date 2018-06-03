#include "vm_thread.h"
#include "vm_util.h"
#include "vm_conf.h"

#include <string.h>

static vm_mmid_t active_stack = MMID_NULL;
static vm_mmid_t active_queue = MMID_NULL;

void vm_thread_wait(vm_thread_t* thread, vm_thread_t* queue) {
	vm_reference(thread);
	thread->next = queue->queue;
	queue->queue = PTR_TO_MMID(thread);
}

vm_mmid_t vm_thread_create(uint32_t size) {
	uint32_t bsize = size<THREAD_INITIAL_SIZE?THREAD_INITIAL_SIZE:npot(size);
	vm_mmid_t id = vm_memory_allocate(&vm_mem_thread,bsize*sizeof(vm_variable_t)+sizeof(vm_thread_t));
	vm_thread_t* ptr = MMID_TO_PTR(id, vm_thread_t*);
	ptr->rcnt = 1;
	ptr->size = bsize;
	ptr->top = 0;
	ptr->next = MMID_NULL;
	ptr->queue = MMID_NULL;
	ptr->state = VM_THREAD_STATE_PAUSED;
	return id;
}

vm_thread_t* vm_thread_grow(vm_thread_t* thread, uint32_t amount) {
	vm_mmid_t oldid = PTR_TO_MMID(thread);
	vm_mmid_t newid = vm_thread_create(thread->size < amount ? thread->size+amount : thread->size*2);
	vm_thread_t* oldptr = MMID_TO_PTR(oldid,vm_thread_t*);
	vm_thread_t* newptr = MMID_TO_PTR(newid,vm_thread_t*);
	newptr->rcnt = oldptr->rcnt;
	newptr->top = oldptr->top;
	newptr->next = oldptr->next;
	newptr->queue = oldptr->queue;
	newptr->state = oldptr->state;
	memcpy(newptr->stack,oldptr->stack,sizeof(vm_variable_t)*oldptr->size);
	vm_memory_replace(&vm_mem_thread, oldid, newid);
	return newptr;
}

bool vm_thread_unwind(vm_thread_t* thread) {
	if (thread->top != 0) {
		uint32_t base = thread->stack[thread->top].frame.base;
		uint32_t top = thread->top - 1;
		while (top > base) {
			vm_variable_dereference(thread->stack[top].variable);
			top -= 1;
		}
		thread->top = base;
		return true;
	}
	return false;
}

void vm_thread_kill(vm_thread_t* thread, vm_variable_t value) {
	if (thread->state == VM_THREAD_STATE_FINISHED)
		return;
	if (thread->queue != MMID_NULL) {
		vm_thread_push(MMID_TO_PTR(thread->queue, vm_thread_t*));
		thread->queue = MMID_NULL;
	}
	while (vm_thread_unwind(thread));
	thread->stack[0].variable = value;
	thread->state = VM_THREAD_STATE_FINISHED;
}

void vm_thread_free(vm_thread_t* thread) {
	if (thread->state == VM_THREAD_STATE_FINISHED) {
		vm_variable_dereference(thread->stack[0].variable);
	} else {
		while (vm_thread_unwind(thread));
	}
	vm_memory_free(&vm_mem_thread, PTR_TO_MMID(thread));
}

void vm_thread_push(vm_thread_t* thread) {
	if (thread->next) {
		vm_thread_t* ptr = MMID_TO_PTR(thread->next,vm_thread_t*);
		while (ptr->next != MMID_NULL)
			ptr = MMID_TO_PTR(thread->queue,vm_thread_t*);
		ptr->next = active_stack;
	} else {
		thread->next = active_stack;
	}
	active_stack = PTR_TO_MMID(thread);
}

vm_mmid_t vm_thread_pop() {
	if (active_queue == MMID_NULL) {
		if (active_stack == MMID_NULL)
			return MMID_NULL;
		vm_mmid_t head_id = active_stack;
		vm_thread_t* tail = MMID_TO_PTR(head_id,vm_thread_t*);
		if (tail->next != MMID_NULL) {
			vm_mmid_t id = tail->next;
			tail->next = MMID_NULL;
			while (id != MMID_NULL) {
				vm_thread_t* ptr = MMID_TO_PTR(id,vm_thread_t*);
				vm_mmid_t next_id = ptr->next;
				ptr->next = head_id;
				head_id = id;
				id = next_id;
			}
		}
		active_queue = head_id;
		active_stack = MMID_NULL;
	}
	vm_mmid_t id = active_queue;
	vm_thread_t* thread = MMID_TO_PTR(id,vm_thread_t*);
	active_queue = thread->next;
	thread->next = MMID_NULL;
	return id;
}

