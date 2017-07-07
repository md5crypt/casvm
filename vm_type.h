#pragma once
#define VM_TYPE_COUNT	11

typedef enum {
	VM_UNDEFINED_T,
	VM_SCALAR_T,
	VM_BOOLEAN_T,
	VM_FLOAT_T,
	VM_INTEGER_T,
	VM_VECTOR_T,
	VM_STRING_T,
	VM_ARRAY_T,
	VM_OBJECT_T,
	VM_FUNCTION_T,
	VM_NAMESPACE_T
} vm_type_t;

extern const vm_type_t vm_type_matrix[];
