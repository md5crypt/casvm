#include "vm_lib.h"
#include "vm_thread.h"

vm_exception_t vm_lib_thread_current(vm_variable_t* top, uint32_t arguments) {
	ASSERT_ARITY(0);
	vm_mmid_t mmid = vm_get_current_thread();
	vm_reference_m_inline(mmid);
	top[1] = VM_VARIABLE_MMID(VM_THREAD_T, mmid);
	return VM_NONE_E;
}

vm_exception_t vm_lib_thread_resume(vm_variable_t* top, uint32_t arguments) {
	ASSERT_ARITY(1);
	ASSERT_TYPE(1, VM_THREAD_T);
	vm_thread_t* thread = VM_CAST_THREAD(top - 1);
	if (!(thread->flags & VM_THREAD_FLAG_FINISHED) && (vm_get_current_thread() != top[-1].data.m)) {
		vm_thread_push(VM_CAST_THREAD(top - 1));
	}
	top[1] = VM_VARIABLE_OFTYPE(VM_UNDEFINED_T);
	return VM_NONE_E;
}

vm_exception_t vm_lib_thread_detach(vm_variable_t* top, uint32_t arguments) {
	ASSERT_ARITY(1);
	ASSERT_TYPE(1, VM_THREAD_T);
	vm_thread_t* thread = VM_CAST_THREAD(top - 1);
	if ((thread->flags & (VM_THREAD_FLAG_FINISHED | VM_THREAD_FLAG_DETACHED)) == 0) {
		thread->rcnt += 1;
		thread->flags |= VM_THREAD_FLAG_DETACHED;
	}
	top[1] = VM_VARIABLE_OFTYPE(VM_UNDEFINED_T);
	return VM_NONE_E;
}

vm_exception_t vm_lib_thread_reattach(vm_variable_t* top, uint32_t arguments) {
	ASSERT_ARITY(1);
	ASSERT_TYPE(1, VM_THREAD_T);
	vm_thread_t* thread = VM_CAST_THREAD(top - 1);
	if ((thread->flags & (VM_THREAD_FLAG_FINISHED | VM_THREAD_FLAG_DETACHED)) == VM_THREAD_FLAG_DETACHED) {
		thread->flags &= ~VM_THREAD_FLAG_DETACHED;
		vm_dereference(thread, VM_THREAD_T);
	}
	top[1] = VM_VARIABLE_OFTYPE(VM_UNDEFINED_T);
	return VM_NONE_E;
}
