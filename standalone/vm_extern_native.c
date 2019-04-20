#include <stddef.h>

#include "vm_extern.h"

extern vm_exception_t vm_lib_array_create(vm_variable_t* top, uint32_t arguments);
extern vm_exception_t vm_lib_array_expand(vm_variable_t* top, uint32_t arguments);
extern vm_exception_t vm_lib_array_fill(vm_variable_t* top, uint32_t arguments);
extern vm_exception_t vm_lib_array_find(vm_variable_t* top, uint32_t arguments);
extern vm_exception_t vm_lib_array_pop(vm_variable_t* top, uint32_t arguments);
extern vm_exception_t vm_lib_array_push(vm_variable_t* top, uint32_t arguments);
extern vm_exception_t vm_lib_array_resize(vm_variable_t* top, uint32_t arguments);
extern vm_exception_t vm_lib_array_reverse(vm_variable_t* top, uint32_t arguments);
extern vm_exception_t vm_lib_array_shift(vm_variable_t* top, uint32_t arguments);
extern vm_exception_t vm_lib_array_slice(vm_variable_t* top, uint32_t arguments);
extern vm_exception_t vm_lib_array_static(vm_variable_t* top, uint32_t arguments);
extern vm_exception_t vm_lib_array_unshift(vm_variable_t* top, uint32_t arguments);
extern vm_exception_t vm_lib_array_write(vm_variable_t* top, uint32_t arguments);
extern vm_exception_t vm_lib_dbgbrk(vm_variable_t* top, uint32_t arguments);
extern vm_exception_t vm_lib_dtos(vm_variable_t* top, uint32_t arguments);
extern vm_exception_t vm_lib_hashmap_has(vm_variable_t* top, uint32_t arguments);
extern vm_exception_t vm_lib_hashmap_keys(vm_variable_t* top, uint32_t arguments);
extern vm_exception_t vm_lib_hashmap_values(vm_variable_t* top, uint32_t arguments);
extern vm_exception_t vm_lib_itos(vm_variable_t* top, uint32_t arguments);
extern vm_exception_t vm_lib_length(vm_variable_t* top, uint32_t arguments);
extern vm_exception_t vm_lib_mem_stat(vm_variable_t* top, uint32_t arguments);
extern vm_exception_t vm_lib_nameof(vm_variable_t* top, uint32_t arguments);
extern vm_exception_t vm_lib_print(vm_variable_t* top, uint32_t arguments);
extern vm_exception_t vm_lib_string_concat(vm_variable_t* top, uint32_t arguments);
extern vm_exception_t vm_lib_string_find(vm_variable_t* top, uint32_t arguments);
extern vm_exception_t vm_lib_string_slice(vm_variable_t* top, uint32_t arguments);
extern vm_exception_t vm_lib_try_end(vm_variable_t* top, uint32_t arguments);
extern vm_exception_t vm_lib_try_start(vm_variable_t* top, uint32_t arguments);
extern vm_exception_t vm_lib_typeof(vm_variable_t* top, uint32_t arguments);

const vm_extern_t vm_extern_native[] = {
	{"__arrayCreate", vm_lib_array_create },
	{"__arrayExpand", vm_lib_array_expand },
	{"__arrayFill", vm_lib_array_fill },
	{"__arrayFind", vm_lib_array_find },
	{"__arrayPop", vm_lib_array_pop },
	{"__arrayPush", vm_lib_array_push },
	{"__arrayResize", vm_lib_array_resize },
	{"__arrayReverse", vm_lib_array_reverse },
	{"__arrayShift", vm_lib_array_shift },
	{"__arraySlice", vm_lib_array_slice },
	{"__arrayStatic", vm_lib_array_static },
	{"__arrayUnshift", vm_lib_array_unshift },
	{"__arrayWrite", vm_lib_array_write },
	{"__dbgbrk", vm_lib_dbgbrk },
	{"__dtos", vm_lib_dtos },
	{"__hashmapHas", vm_lib_hashmap_has },
	{"__hashmapKeys", vm_lib_hashmap_keys },
	{"__hashmapValues", vm_lib_hashmap_values },
	{"__itos", vm_lib_itos },
	{"__length", vm_lib_length },
	{"__memStat", vm_lib_mem_stat },
	{"__nameof", vm_lib_nameof },
	{"__print", vm_lib_print },
	{"__stringConcat", vm_lib_string_concat },
	{"__stringFind", vm_lib_string_find },
	{"__stringSlice", vm_lib_string_slice },
	{"__tryEnd", vm_lib_try_end },
	{"__tryStart", vm_lib_try_start },
	{"__typeof", vm_lib_typeof },
	{NULL, NULL}
};
