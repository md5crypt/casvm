#pragma once

typedef enum {
	VM_NONE_E,
	VM_YIELD_E,
	VM_USER_E,
	VM_OOB_E,
	VM_TYPE_E,
	VM_ARRITY_E,
	VM_INVALID_OP_E
} vm_exception_t;
