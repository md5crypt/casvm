#pragma once

typedef enum {
	VM_OOB_E
} vm_exception_t;

static inline void vm_throw(vm_exception_t e){
	e = e;
}