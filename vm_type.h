#pragma once
#define VM_TYPE_COUNT	15

typedef enum {
	VM_INVALID_T,
	VM_UNDEFINED_T,
	VM_BOOLEAN_T,
	VM_INTEGER_T,
	VM_FLOAT_T,
	VM_STRING_T,
	VM_ARRAY_T,
	VM_CALLABLE_T,
	VM_FUNCTION_T,
	VM_EXTERN_T,
	VM_NATIVE_T,
	VM_NAMESPACE_T,
	VM_NUMERIC_T,
	VM_VECTOR_T,
	VM_OBJECT_T
} vm_type_t;

extern const vm_type_t vm_type_matrix[VM_TYPE_COUNT*VM_TYPE_COUNT];
extern const char* const vm_type_names[VM_TYPE_COUNT];
