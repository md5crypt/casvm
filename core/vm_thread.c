#include "vm_thread.h"
#include "vm_util.h"
#include "vm_conf.h"

#include <string.h>

static struct {
	vm_mmid_t head;
	vm_mmid_t tail;
} active_queue = {MMID_NULL, MMID_NULL};

static vm_thread_t* traverse_backward(vm_thread_t* thread) {
	vm_thread_t* ptr = thread;
	if (thread->prev != MMID_NULL) {
		ptr = MMID_TO_PTR(thread->prev, vm_thread_t*);
		while (ptr->prev != MMID_NULL) {
			ptr = MMID_TO_PTR(ptr->prev, vm_thread_t*);
		}
	}
	return ptr;
}

vm_mmid_t vm_thread_create(uint32_t size) {
	uint32_t bsize = size < THREAD_INITIAL_SIZE ? THREAD_INITIAL_SIZE : npot(size);
	vm_mmid_t id = vm_memory_allocate(
		&vm_mem_thread,
		(bsize * sizeof(vm_variable_t)) + sizeof(vm_thread_t)
	);
	vm_thread_t* ptr = MMID_TO_PTR(id, vm_thread_t*);
	ptr->rcnt = 1;
	ptr->size = bsize;
	ptr->top = 0;
	ptr->next = MMID_NULL;
	ptr->prev = MMID_NULL;
	ptr->queue = MMID_NULL;
	ptr->state = VM_THREAD_STATE_PAUSED;
	return id;
}

vm_thread_t* vm_thread_grow(vm_thread_t* thread, uint32_t amount) {
	vm_mmid_t old_id = PTR_TO_MMID(thread);
	vm_mmid_t new_id = vm_thread_create(thread->size < amount ? thread->size + amount : thread->size * 2);
	vm_thread_t* old_ptr = MMID_TO_PTR(old_id, vm_thread_t*);
	vm_thread_t* new_ptr = MMID_TO_PTR(new_id, vm_thread_t*);
	new_ptr->rcnt = old_ptr->rcnt;
	new_ptr->top = old_ptr->top;
	new_ptr->next = old_ptr->next;
	new_ptr->prev = old_ptr->prev;
	new_ptr->queue = old_ptr->queue;
	new_ptr->state = old_ptr->state;
	memcpy(new_ptr->stack, old_ptr->stack, sizeof(vm_variable_t) * old_ptr->size);
	vm_memory_replace(&vm_mem_thread, old_id, new_id);
	return new_ptr;
}

bool vm_thread_unwind(vm_thread_t* thread) {
	uint32_t base = thread->stack[thread->top].frame.lower.base;
	if (base == 0xFFFFFFFF) {
		if (thread->top > 0) {
			uint32_t top = thread->top;
			while (top) {
				top -= 1;
				vm_variable_dereference(thread->stack[top].variable);
			}
		}
		return false;
	}
	uint32_t top = thread->top - 1;
	while (top > (base + 1)) {
		vm_variable_dereference(thread->stack[top].variable);
		top -= 1;
	}
	thread->top = base;
	return true;
}

void vm_thread_kill(vm_thread_t* thread, vm_variable_data_t value, vm_type_t type) {
	if (thread->state == VM_THREAD_STATE_FINISHED) {
		return;
	}
	if (active_queue.head == PTR_TO_MMID(thread)) {
		active_queue.head = thread->next;
	}
	if (active_queue.tail == PTR_TO_MMID(thread)) {
		active_queue.tail = thread->prev;
	}
	if (thread->next != MMID_NULL) {
		vm_thread_t* next = MMID_TO_PTR(thread->next, vm_thread_t*);
		if (next->queue == PTR_TO_MMID(thread)) {
			next->queue = thread->prev;
		} else {
			next->prev = thread->prev;
		}
	}
	if (thread->prev != MMID_NULL) {
		vm_thread_t* prev = MMID_TO_PTR(thread->prev, vm_thread_t*);
		prev->next = thread->next;
	}
	thread->next = MMID_NULL;
	thread->prev = MMID_NULL;
	vm_thread_dequeue(thread);
	while (vm_thread_unwind(thread));
	thread->stack[0].variable = VM_VARIABLE(type, value);
	thread->state = VM_THREAD_STATE_FINISHED;
}

void vm_thread_free(vm_thread_t* thread) {
	vm_thread_kill(thread, 0, VM_UNDEFINED_T);
	vm_variable_dereference(thread->stack[0].variable);
	vm_memory_free(&vm_mem_thread, PTR_TO_MMID(thread));
}

void vm_thread_wait(vm_thread_t* thread, vm_thread_t* queue) {
	vm_mmid_t id = PTR_TO_MMID(thread);
	if (queue->queue != MMID_NULL) {
		vm_thread_t* tail = MMID_TO_PTR(queue->queue, vm_thread_t*);
		tail->next = id;
		thread->prev = queue->queue;
		thread->next = PTR_TO_MMID(queue);
	}
	queue->queue = id;
}

void vm_thread_dequeue(vm_thread_t* thread) {
	if (thread->queue != MMID_NULL) {
		vm_thread_t* tail = MMID_TO_PTR(thread->queue, vm_thread_t*);
		tail->next = MMID_NULL;
		vm_thread_push(tail);
		thread->queue = MMID_NULL;
	}
}

void vm_thread_push(vm_thread_t* thread) {
	vm_thread_t* first = traverse_backward(thread);
	if (active_queue.tail == MMID_NULL) {
		active_queue.head = PTR_TO_MMID(first);
	} else {
		vm_thread_t* tail = MMID_TO_PTR(active_queue.tail, vm_thread_t*);
		tail->next = PTR_TO_MMID(first);
		first->prev = active_queue.tail;
	}
	active_queue.tail = PTR_TO_MMID(thread);
}

vm_mmid_t vm_thread_pop() {
	if (active_queue.head == MMID_NULL) {
		return MMID_NULL;
	}
	vm_mmid_t ret = active_queue.head;
	vm_thread_t* head = MMID_TO_PTR(ret, vm_thread_t*);
	active_queue.head = head->next;
	if (head->next != MMID_NULL) {
		MMID_TO_PTR(head->next, vm_thread_t*)->prev = MMID_NULL;
		head->next = MMID_NULL;
	} else {
		active_queue.tail = MMID_NULL;
	}
	return ret;
}

