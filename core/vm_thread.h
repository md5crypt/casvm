#pragma once
#include "vm.h"

#define VM_CAST_THREAD(var) MMID_TO_PTR((var)->data.m, vm_thread_t*)

typedef union {
	struct {
		uint32_t arguments;
		uint32_t reserved;
	} upper;
	struct {
		uint32_t base;
		uint32_t link;
	} lower;
} vm_stackframe_t;

typedef union {
	vm_variable_t variable;
	vm_stackframe_t frame;
} vm_stackitem_t;

typedef enum {
	VM_THREAD_STATE_PAUSED,
	VM_THREAD_STATE_FINISHED,
} vm_thread_state_t;

typedef struct {
	uint32_t rcnt;
	uint32_t size;
	uint32_t top;
	vm_thread_state_t state;
	vm_mmid_t next;
	vm_mmid_t prev;
	vm_mmid_t queue;
	vm_stackitem_t stack[];
} vm_thread_t;

vm_mmid_t vm_thread_create(uint32_t size);
vm_mmid_t vm_thread_pop(void);

vm_thread_t* vm_thread_grow(vm_thread_t* thread, uint32_t amount);
void vm_thread_wait(vm_thread_t* thread, vm_thread_t* queue);
void vm_thread_dequeue(vm_thread_t* thread);
void vm_thread_free(vm_thread_t* thread);
void vm_thread_push(vm_thread_t* thread);
void vm_thread_kill(vm_thread_t* thread, vm_variable_t value);
bool vm_thread_unwind(vm_thread_t* thread);

static inline void vm_thread_stackframe_pack(vm_stackframe_t* frame, uint32_t link, uint32_t base, uint32_t arguments) {
	frame[0].lower.base = base;
	frame[0].lower.link = link;
	frame[1].upper.arguments = arguments;
}