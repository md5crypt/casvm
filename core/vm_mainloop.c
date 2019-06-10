#include <string.h>

#include "vm_mainloop.h"
#include "vm_thread.h"
#include "vm_util.h"
#include "vm_string.h"
#include "vm_hashmap.h"
#include "vm_array.h"
#include "vm_extern.h"
#include "vm_op.h"

#define RELOAD_THREAD(newthread) \
	do{ \
		vm_thread_t* _newthread = (newthread); \
		base = (vm_variable_t*)(_newthread->stack) + (base - bottom); \
		top = (vm_variable_t*)(_newthread->stack) + (top - bottom); \
		bottom = (vm_variable_t*)(_newthread->stack); \
		end = bottom + _newthread->size; \
		thread = _newthread; \
	}while (0)

#define GROW_THREAD(amount) \
	do{ \
		RELOAD_THREAD(vm_thread_grow(thread, (amount))); \
	}while (0)

#define ERROR(e) \
	do{ \
		vm_thread_stackframe_pack( \
			(vm_stackframe_t*)(top + 1), \
			pc - vm_progmem, \
			base - bottom, \
			arguments \
		); \
		thread->top = top - bottom + 1; \
		return e; \
	}while (0)

#define ASSERT_TYPE(v, t) if ((v)->type != t) { \
	vm_exception_type((v)->type, t); \
	ERROR(VM_TYPE_E); \
}

#define SOFT_ASSERT_TYPE(v, t) if (!VM_ISTYPE((v)->type, t)) { \
	vm_exception_type((v)->type, t); \
	ERROR(VM_TYPE_E); \
}

#define INT_OP(op, cast, z_check) \
	do{ \
		if (top[0].type == VM_INTEGER_T) { \
			if (z_check && (top[0].data.i == 0)) { \
				ERROR(VM_DIV0_E); \
			} \
			if (top[-1].type == VM_INTEGER_T) { \
				top[-1].data.i = (cast)top[-1].data.i op (cast)top[0].data.i; \
			} else if (top[-1].type == VM_FLOAT_T) { \
				top[-1].data.i = (cast)top[-1].data.f op (cast)top[0].data.i; \
			} else { \
				vm_exception_type(top[-1].type, VM_NUMERIC_T); \
				ERROR(VM_TYPE_E); \
			} \
		} else if (top[0].type == VM_FLOAT_T) { \
			if (z_check && ((cast)top[0].data.f == 0)) { \
				ERROR(VM_DIV0_E); \
			} \
			if (top[-1].type == VM_INTEGER_T) { \
				top[-1].data.i = (cast)top[-1].data.i op (cast)top[0].data.f; \
			} else if (top[-1].type == VM_FLOAT_T) { \
				top[-1].data.i = (cast)top[-1].data.f op (cast)top[0].data.f; \
			} else { \
				vm_exception_type(top[-1].type, VM_NUMERIC_T); \
				ERROR(VM_TYPE_E); \
			} \
		} else { \
			vm_exception_type(top[0].type, VM_NUMERIC_T); \
			ERROR(VM_TYPE_E); \
		} \
		(--top)->type = VM_INTEGER_T; \
	}while (0)

#define FLOAT_OP(op, z_check) \
	do{ \
		if (top[0].type == VM_INTEGER_T) { \
			if (top[-1].type == VM_INTEGER_T) { \
				if (z_check && (top[0].data.i == 0)) { \
					ERROR(VM_DIV0_E); \
				} \
				top[-1].data.i = top[-1].data.i op top[0].data.i; \
				(--top)->type = VM_INTEGER_T; \
			} else if (top[-1].type == VM_FLOAT_T) { \
				top[-1].data.f = top[-1].data.f op top[0].data.i; \
				(--top)->type = VM_FLOAT_T; \
			} else { \
				vm_exception_type(top[-1].type, VM_NUMERIC_T); \
				ERROR(VM_TYPE_E); \
			} \
		} else if (top[0].type == VM_FLOAT_T) { \
			if (top[-1].type == VM_INTEGER_T) { \
				top[-1].data.f = top[-1].data.i op top[0].data.f; \
				(--top)->type = VM_FLOAT_T; \
			} else if (top[-1].type == VM_FLOAT_T) { \
				top[-1].data.f = top[-1].data.f op top[0].data.f; \
				(--top)->type = VM_FLOAT_T; \
			} else { \
				vm_exception_type(top[-1].type, VM_NUMERIC_T); \
				ERROR(VM_TYPE_E); \
			} \
		} else { \
			vm_exception_type(top[0].type, VM_NUMERIC_T); \
			ERROR(VM_TYPE_E); \
		} \
	}while (0)

#define LOGIC_OP(op) \
	do{ \
		if (top[0].type == VM_INTEGER_T) { \
			if (top[-1].type == VM_INTEGER_T) { \
				top[-1].data.i = top[-1].data.i op top[0].data.i; \
			} else if (top[-1].type == VM_FLOAT_T) { \
				top[-1].data.i = top[-1].data.f op top[0].data.i; \
			} else { \
				vm_exception_type(top[-1].type, VM_NUMERIC_T); \
				ERROR(VM_TYPE_E); \
			} \
		} else if (top[0].type == VM_FLOAT_T) { \
			if (top[-1].type == VM_INTEGER_T) { \
				top[-1].data.i = top[-1].data.i op top[0].data.f; \
			} else if (top[-1].type == VM_FLOAT_T) { \
				top[-1].data.i = top[-1].data.f op top[0].data.f; \
			} else { \
				vm_exception_type(top[-1].type, VM_NUMERIC_T); \
				ERROR(VM_TYPE_E); \
			} \
		} else { \
			vm_exception_type(top[0].type, VM_NUMERIC_T); \
			ERROR(VM_TYPE_E); \
		} \
		(--top)->type = VM_BOOLEAN_T; \
	}while (0)

inline static bool test(vm_variable_t* top) {
	bool result;
	switch (top->type) {
		case VM_INVALID_T:
		case VM_UNDEFINED_T:
			result = false;
			break;
		case VM_BOOLEAN_T:
		case VM_INTEGER_T:
			result = (top->data.i != 0);
			break;
		case VM_FLOAT_T:
			result = (top->data.f != 0);
			break;
		case VM_STRING_T:
			result = (MMID_TO_PTR(top->data.m, vm_string_t*)->size);
			vm_dereference_m(top->data.m, VM_STRING_T);
			break;
		default:
			vm_variable_dereference(top[0]);
			result = true;
			break;
	}
	return result;
}

inline static bool eq(vm_variable_t* top, bool strict) {
	bool result = (
		((top[-1].type == top[0].type) &&
			((top[-1].data.i == top[0].data.i) ||
			((top[-1].type == VM_STRING_T) &&
				vm_string_cmp(VM_CAST_STRING(top - 1), VM_CAST_STRING(top - 0))))) ||
		(!strict &&
			(((top[-1].type == VM_BOOLEAN_T) &&
				(((top[0].type == VM_INTEGER_T) && (top[-1].data.i == top[0].data.i)) ||
				((top[0].type == VM_FLOAT_T) && (top[-1].data.i == top[0].data.f)))) ||
			((top[-1].type == VM_INTEGER_T) &&
				(((top[0].type == VM_BOOLEAN_T) && (top[-1].data.i == top[0].data.i)) ||
				((top[0].type == VM_FLOAT_T) && (top[-1].data.i == top[0].data.f)))) ||
			((top[-1].type == VM_FLOAT_T) &&
				((top[0].type == VM_INTEGER_T) || (top[0].type == VM_BOOLEAN_T)) &&
				(top[-1].data.f == top[0].data.i))))
	);
	vm_variable_dereference(top[-1]);
	vm_variable_dereference(top[0]);
	return result;
}

vm_exception_t vm_mainloop(vm_mmid_t thread_id) {
	vm_thread_t* thread = MMID_TO_PTR(thread_id, vm_thread_t*);
	if (thread->flags & VM_THREAD_FLAG_FINISHED) {
		return VM_NONE_E;
	}
	vm_variable_t* bottom = (vm_variable_t*)thread->stack;
	vm_variable_t* top = bottom + thread->top;
	vm_variable_t* end = bottom + thread->size;
	vm_variable_t* base = bottom + ((vm_stackframe_t*)top)[0].lower.base;
	const vm_opcode_t* pc = vm_progmem + ((vm_stackframe_t*)top)[0].lower.link;
	uint32_t arguments = ((vm_stackframe_t*)top)[1].upper.arguments;
	top -= 1;
	while (true) {
		if ((top + 4) >= end) {
			GROW_THREAD(4);
		}
		//printf("%03X\t%03d\n", pc - vm_progmem, top - bottom);
		vm_opcode_t opcode = pc[0];
		pc += 1;
		switch (opcode.o16.op) {
			case VM_OP_PUSH_VALUE:
				top += 1;
				top[0] = VM_VARIABLE_MMID(opcode.o16.type, pc->o32);
				pc += 1;
				break;
			case VM_OP_PUSH_CONST:
				top += 1;
				top[0] = VM_VARIABLE_MMID(opcode.o16.type, opcode.o16.value);
				break;
			case VM_OP_PUSH_LOCAL:
				top +=1;
				top[0] = base[opcode.o24.value];
				vm_variable_reference(top[0]);
				break;
			case VM_OP_PUSH_PARENT: {
				SOFT_ASSERT_TYPE(top, VM_HASHMAP_T);
				vm_mmid_t mmid = MMID_TO_PTR(top->data.m, vm_hashmap_t*)->parent;
				top[0] = (mmid == MMID_NULL) ?
					VM_VARIABLE_OFTYPE(VM_UNDEFINED_T) :
					VM_VARIABLE_MMID(MMID_TO_PTR(mmid, vm_hashmap_t*)->type, mmid);
				break;
			}
			case VM_OP_PUSH_MEMBER:
				if (top[-1].type == VM_ARRAY_T) {
					ASSERT_TYPE(top, VM_INTEGER_T);
					vm_array_t* array = VM_CAST_ARRAY(top - 1);
					vm_exception_t e = vm_array_get(array, top[0].data.i, top - 1);
					vm_dereference(array, VM_ARRAY_T);
					top -= 1;
					if (e != VM_NONE_E) {
						ERROR(e);
					}
				} else if (VM_ISTYPE(top[-1].type, VM_HASHMAP_T)) {
					ASSERT_TYPE(top, VM_STRING_T);
					vm_hashmap_t* map = VM_CAST_HASHMAP(top - 1);
					vm_hashmap_get(map, vm_string_intern(VM_CAST_STRING(top - 0)), top - 1);
					top -= 1;
				} else if (top[-1].type == VM_STRING_T) {
					ASSERT_TYPE(top, VM_INTEGER_T);
					vm_string_t* string = VM_CAST_STRING(top - 1);
					vm_exception_t e = vm_string_get(string, top[0].data.i, top - 1);
					vm_dereference(string, VM_STRING_T);
					top -= 1;
					if (e != VM_NONE_E) {
						ERROR(e);
					}
				} else {
					vm_exception_type(top[-1].type, VM_INDEXABLE_T);
					ERROR(VM_TYPE_E);
				}
				break;
			case VM_OP_PUSH_MEMBER_UNSAFE:
				SOFT_ASSERT_TYPE(top - 1, VM_HASHMAP_T);
				vm_hashmap_get(VM_CAST_HASHMAP(top - 1), top[0].data.i, top - 1);
				top -= 1;
				break;
			case VM_OP_PUSH_MEMBER_CONST:
				SOFT_ASSERT_TYPE(top, VM_HASHMAP_T);
				vm_hashmap_get(VM_CAST_HASHMAP(top), opcode.o24.value, top);
				break;
			case VM_OP_PUSH_ARGUMENT:
				ASSERT_TYPE(top, VM_INTEGER_T);
				if ((uint32_t)top->data.i >= arguments) {
					vm_exception_oob(top[0].data.i, arguments);
					ERROR(VM_OOB_E);
				}
				top[0] = base[-top->data.i - 1];
				vm_variable_reference(top[0]);
				break;
			case VM_OP_PUSH_ARGUMENT_ARRAY: {
				const uint32_t diff = arguments - opcode.o24.value;
				const vm_mmid_t id = vm_array_create(diff);
				vm_array_t* const array = MMID_TO_PTR(id, vm_array_t*);
				for (uint32_t i = 0; i < diff; i += 1) {
					vm_variable_t* arg = base - i - opcode.o24.value - 1;
					vm_variable_reference(arg[0]);
					array->data[i] = arg[0];
				}
				top += 1;
				top[0] = VM_VARIABLE_MMID(VM_ARRAY_T, id);
				break;
			} case VM_OP_PUSH_ARGUMENT_COUNT:
				top += 1;
				top[0] = VM_VARIABLE_INTEGER(arguments);
				break;
			case VM_OP_DEALLOC:
				for (uint32_t i = 0; i < (uint32_t)opcode.o24.value; i++) {
					vm_variable_dereference(top[0]);
					top -= 1;
				}
				break;
			case VM_OP_ALLOC:
				if ((top + opcode.o24.value) >= end) {
					GROW_THREAD(opcode.o24.value - (end - top));
				}
				for (int32_t i = 0; i < opcode.o24.value; i++) {
					top += 1;
					top[0] = VM_VARIABLE_OFTYPE(VM_UNDEFINED_T);
				}
				break;
			case VM_OP_DUP: {
				vm_variable_t var = top[0];
				if ((top + opcode.o24.value) >= end) {
					GROW_THREAD(opcode.o24.value - (end - top));
				}
				for (int32_t i = 0; i < opcode.o24.value; i++) {
					top += 1;
					top[0] = var;
					vm_variable_reference(var);
				}
				break;
			} case VM_OP_SET_LOCAL:
				vm_variable_dereference(base[opcode.o24.value]);
				base[opcode.o24.value] = top[0];
				top -= 1;
				break;
			case VM_OP_SET_MEMBER:
				if (top[-1].type == VM_ARRAY_T) {
					ASSERT_TYPE(top - 0, VM_INTEGER_T);
					vm_exception_t e = vm_array_set(VM_CAST_ARRAY(top - 1), top[0].data.i, top[-2].data.i, top[-2].type);
					vm_variable_dereference(top[-1]);
					top -= 3;
					if (e != VM_NONE_E) {
						ERROR(e);
					}
				} else if (VM_ISTYPE(top[-1].type, VM_HASHMAP_T)) {
					ASSERT_TYPE(top - 0, VM_STRING_T);
					vm_hashmap_set(
						VM_CAST_HASHMAP(top - 1),
						vm_string_intern(VM_CAST_STRING(top - 0)),
						top[-2].data.i,
						top[-2].type
					);
					top -= 3;
					break;
				} else if (top[-1].type == VM_STRING_T) {
					ERROR(VM_IMMUTABLE_E);
				} else {
					vm_exception_type(top[-1].type, VM_INDEXABLE_T);
					ERROR(VM_TYPE_E);
				}
				break;
			case VM_OP_SET_MEMBER_UNSAFE:
				SOFT_ASSERT_TYPE(top - 1, VM_HASHMAP_T);
				vm_hashmap_set(VM_CAST_HASHMAP(top - 1), top[0].data.i, top[-2].data.i, top[-2].type);
				top -= 3;
				break;
			case VM_OP_SET_MEMBER_CONST:
				SOFT_ASSERT_TYPE(top - 0, VM_HASHMAP_T);
				vm_hashmap_set(VM_CAST_HASHMAP(top - 0), opcode.o24.value, top[-1].data.i, top[-1].type);
				top -= 2;
				break;
			case VM_OP_JMP:
				pc += opcode.o24.value - 1;
				break;
			case VM_OP_JE:
				if (test(top)) {
					pc += opcode.o24.value - 1;
				}
				top -= 1;
				break;
			case VM_OP_JNE:
				if (!test(top)) {
					pc += opcode.o24.value - 1;
				}
				top -= 1;
				break;
			case VM_OP_APPLY:
			case VM_OP_CALL: {
				uint32_t args = opcode.o24.value;
				if (opcode.o24.op == VM_OP_APPLY) {
					ASSERT_TYPE(top - 1, VM_ARRAY_T);
					vm_array_t* array = VM_CAST_ARRAY(top - 1);
					args = array->used;
					if ((top + args) >= end) {
						GROW_THREAD((top + args) - end);
					}
					vm_variable_t func = top[0];
					top = vm_array_apply(array, top - 1);
					vm_dereference(array, VM_ARRAY_T);
					top += 1;
					top[0] = func;
				}
				if (top->type == VM_EXTERN_T) {
					vm_hashmap_t* hashmap = MMID_TO_PTR(top->data.m, vm_hashmap_t*);
					top[1] = VM_VARIABLE_OFTYPE(VM_UNDEFINED_T);
					vm_exception_t e = (
						(hashmap->type == VM_NATIVE_T) ?
						hashmap->code.native(top, args) :
						vm_extern_call(hashmap->code.address, top, args)
					);
					if (e != VM_NONE_E) {
						if (e == VM_USER_E) {
							vm_exception_user(vm_string_to_wstr(MMID_TO_PTR(top[1].data.m, vm_string_t*))); \
						}
						top += 1;
						ERROR(e);
					}
					for (uint32_t i = 0; i < args; i++) {
						vm_variable_dereference(top[-i - 1]);
					}
					top[-args] = top[1];
					top -= args;
				} else if (VM_ISTYPE(top->type, VM_FUNCTION_T)) {
					vm_hashmap_t* func = MMID_TO_PTR(top->data.m, vm_hashmap_t*);
					vm_thread_stackframe_pack(
						(vm_stackframe_t*)top,
						pc - vm_progmem,
						base - bottom,
						arguments
					);
					arguments = args;
					pc = vm_progmem + func->code.address;
					base = top;
					top += 1;
				} else {
					vm_exception_type(top->type, VM_CALLABLE_T);
					ERROR(VM_TYPE_E);
				}
				break;
			} case VM_OP_CALL_ASYNC: {
				SOFT_ASSERT_TYPE(top, VM_FUNCTION_T);
				vm_hashmap_t* func = MMID_TO_PTR(top[0].data.m, vm_hashmap_t*);
				vm_mmid_t child_id = vm_thread_create(opcode.o24.value + 4);
				vm_thread_t* tmp = MMID_TO_PTR(thread_id, vm_thread_t*);
				if (tmp != thread) {
					RELOAD_THREAD(tmp);
				}
				vm_thread_t* child = MMID_TO_PTR(child_id, vm_thread_t*);
				top -= opcode.o24.value;
				for (uint32_t i = 0; i < (uint32_t)opcode.o24.value; i++) {
					child->stack[i].variable = top[i];
				}
				vm_thread_stackframe_pack(
					&child->stack[opcode.o24.value + 0].frame,
					pc - vm_progmem,
					0xFFFFFFFF,
					0
				);
				vm_thread_stackframe_pack(
					&child->stack[opcode.o24.value + 2].frame,
					func->code.address,
					opcode.o24.value,
					opcode.o24.value
				);
				child->top = opcode.o24.value + 2;
				vm_thread_push(child);
				top[0] = VM_VARIABLE_MMID(VM_THREAD_T, child_id);
				break;
			} case VM_OP_WAIT:{
				ASSERT_TYPE(top, VM_THREAD_T);
				vm_thread_t* child = MMID_TO_PTR(top->data.m, vm_thread_t*);
				if (child->flags & VM_THREAD_FLAG_FINISHED) {
					top[0] = child->stack->variable;
					vm_variable_reference(top[0]);
					vm_dereference(child, VM_THREAD_T);
				} else {
					pc -= 1;
					vm_thread_wait(thread, child);
					ERROR(VM_YIELD_E);
				}
				break;
			} case VM_OP_RET: {
				for (vm_variable_t* ptr = top - 1; ptr > (base + 1); ptr--) {
					vm_variable_dereference(ptr[0]);
				}
				for (vm_variable_t* ptr = base - arguments; ptr < base; ptr++) {
					vm_variable_dereference(ptr[0]);
				}
				if ((base - arguments) == bottom) {
					bottom[0] = top[0];
					thread->top = 0;
					thread->flags |= VM_THREAD_FLAG_FINISHED;
					vm_thread_dequeue(thread);
					if (thread->flags & VM_THREAD_FLAG_DETACHED) {
						vm_dereference(thread, VM_THREAD_T);
					}
					return VM_NONE_E;
				}
				vm_stackframe_t* frame = (vm_stackframe_t*)base;
				vm_variable_t ret = top[0];
				top = base - arguments;
				base = bottom + frame[0].lower.base;
				pc = vm_progmem + frame[0].lower.link;
				arguments = frame[1].upper.arguments;
				top[0] = ret;
				break;
			}
			case VM_OP_YIELD:
				ERROR(VM_YIELD_E);
			case VM_OP_THROW:
				vm_exception_user((top->type == VM_STRING_T) ?
					vm_string_to_wstr(MMID_TO_PTR(top->data.m, vm_string_t*)) :
					NULL
				);
				ERROR(VM_USER_E);
			case VM_OP_INTERN:
				ASSERT_TYPE(top, VM_STRING_T);
				top[0].data.m = vm_string_intern(MMID_TO_PTR(top->data.m, vm_string_t*));
				break;
			case VM_OP_ISTYPE:
				vm_variable_dereference(top[0]);
				top[0] = VM_VARIABLE_BOOL(VM_ISTYPE(top->type, opcode.o16.type));
				break;
			case VM_OP_ASSERT_TYPE:
				if (!VM_ISTYPE((base + opcode.o16.value)->type, opcode.o16.type)) {
					vm_exception_type((base + opcode.o16.value)->type, opcode.o16.type);
					ERROR(VM_TYPE_E);
				}
				break;
			case VM_OP_ASSERT_TYPE_SOFT: {
				vm_type_t type = (base + opcode.o16.value)->type;
				if (type != VM_UNDEFINED_T && !VM_ISTYPE(type, opcode.o16.type)) {
					vm_exception_type((base + opcode.o16.value)->type, opcode.o16.type);
					ERROR(VM_TYPE_E);
				}
				break;
			}
			case VM_OP_ASSERT_ARITY_EQ:
				if (arguments != (uint32_t)opcode.o24.value) {
					vm_exception_arity(arguments, opcode.o24.value);
					ERROR(VM_ARITY_E);
				}
				break;
			case VM_OP_ASSERT_ARITY_GE:
				if (arguments < (uint32_t)opcode.o24.value) {
					vm_exception_arity(arguments, opcode.o24.value);
					ERROR(VM_ARITY_E);
				}
				break;
			case VM_OP_SET_ARITY:
				if (arguments < (uint32_t)opcode.o24.value) {
					const uint32_t diff = (uint32_t)opcode.o24.value - arguments;
					if ((top + diff) >= end) {
						GROW_THREAD(diff - (end - top));
					}
					for (uint32_t i = 0; i < (arguments + 2); i++) {
						top[diff - i] = top[-i];
					}
					top += diff;
					base += diff;
					for (uint32_t i = 0; i < diff; i++) {
						base[-arguments - i - 1] = VM_VARIABLE_OFTYPE(VM_UNDEFINED_T);
					}
					arguments = opcode.o24.value;
				} else if (arguments != (uint32_t)opcode.o24.value) {
					vm_exception_arity(opcode.o24.value, arguments);
					ERROR(VM_ARITY_E);
				}
				break;
			case VM_OP_BOR:
				INT_OP(|, int32_t, false);
				break;
			case VM_OP_BXOR:
				INT_OP(^, int32_t, false);
				break;
			case VM_OP_BAND:
				INT_OP(&, int32_t, false);
				break;
			case VM_OP_EQ:
				top -= 1;
				top[0] = VM_VARIABLE_BOOL(eq(top + 1, false));
				break;
			case VM_OP_EQEQ:
				top -= 1;
				top[0] = VM_VARIABLE_BOOL(eq(top + 1, true));
				break;
			case VM_OP_NEQ:
				top -= 1;
				top[0] = VM_VARIABLE_BOOL(!eq(top + 1, false));
				break;
			case VM_OP_NEQNEQ:
				top -= 1;
				top[0] = VM_VARIABLE_BOOL(!eq(top + 1, true));
				break;
			case VM_OP_GT:
				LOGIC_OP(>);
				break;
			case VM_OP_GE:
				LOGIC_OP(>=);
				break;
			case VM_OP_LT:
				LOGIC_OP(<);
				break;
			case VM_OP_LE:
				LOGIC_OP(<=);
				break;
			case VM_OP_SHR:
				INT_OP(>>, int32_t, false);
				break;
			case VM_OP_LSR:
				INT_OP(>>, uint32_t, false);
				break;
			case VM_OP_SHL:
				INT_OP(<<, int32_t, false);
				break;
			case VM_OP_ADD:
				if (top->type == VM_STRING_T) {
					if (top[-1].type != VM_STRING_T) {
						vm_exception_type(top[-1].type, VM_STRING_T);
						ERROR(VM_TYPE_E);
					}
					vm_mmid_t s = vm_string_concat(VM_CAST_STRING(top - 1), VM_CAST_STRING(top - 0));
					vm_variable_dereference(top[-1]);
					vm_variable_dereference(top[0]);
					(--top)->data.m = s;
				} else {
					FLOAT_OP(+, false);
				}
				break;
			case VM_OP_SUB:
				FLOAT_OP(-, false);
				break;
			case VM_OP_MUL:
				FLOAT_OP(*, false);
				break;
			case VM_OP_DIV:
				FLOAT_OP(/, true);
				break;
			case VM_OP_MOD:
				INT_OP(%, int32_t, true);
				break;
			case VM_OP_NOT:
				top->data.i = !test(top);
				top->type = VM_BOOLEAN_T;
				break;
			case VM_OP_BNOT:
				if (top->type == VM_INTEGER_T) {
					top->data.i = ~top->data.i;
					break;
				} else if (top->type == VM_FLOAT_T) {
					top->data.i = ~((int32_t)top->data.f);
					top->type = VM_INTEGER_T;
					break;
				} else {
					vm_exception_type(top->type, VM_NUMERIC_T);
					ERROR(VM_TYPE_E);
				}
			case VM_OP_NEG:
				if (top->type == VM_INTEGER_T) {
					top->data.i = -top->data.i;
				} else if (top->type == VM_FLOAT_T) {
					top->data.f = -top->data.f;
				} else {
					vm_exception_type(top->type, VM_NUMERIC_T);
					ERROR(VM_TYPE_E);
				}
				break;
			default:
				ERROR(VM_INTERNAL_E);
		}
	}
}
