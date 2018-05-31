#include <stdio.h>
#include <string.h>

#include "vm_mainloop.h"
#include "vm_thread.h"
#include "vm_util.h"
#include "vm_string.h"
#include "vm_hashmap.h"
#include "vm_array.h"
#include "vm_op.h"

#define RELOAD_THREAD(newthread) \
	do{ \
		vm_thread_t* _newthread = (newthread); \
		base = (vm_variable_t*)(_newthread->stack) + (base-bottom); \
		top = (vm_variable_t*)(_newthread->stack) + (top-bottom); \
		bottom = (vm_variable_t*)(_newthread->stack); \
		end = bottom + _newthread->size; \
		thread = _newthread; \
	}while(0)

#define GROW_THREAD(amount) \
	do{ \
		RELOAD_THREAD(vm_thread_grow(thread,(amount))); \
	}while(0)

#define ERROR(e) \
	do{ \
		*((vm_stackframe_t*)(++top)) = CURRENT_STACKFRAME(); \
		if(top >= end) \
			vm_thread_grow(thread,1); \
		thread->top = top-bottom; \
		return e; \
	}while(0)

#define ASSERT_TYPE(v,t) if((v)->type != t){ \
	vm_exception_type(t,(v)->type);          \
	ERROR(VM_TYPE_E);                        \
}

#define SOFT_ASSERT_TYPE(v,t) if(!VM_ISTYPE((v)->type,t)){ \
	vm_exception_type(t,(v)->type);                        \
	ERROR(VM_TYPE_E);                                      \
}                                

#define INT_OP(op) \
	do{ \
		ASSERT_TYPE(top-1,VM_INTEGER_T); \
		ASSERT_TYPE(top-0,VM_INTEGER_T); \
		(top-1)->data.i = (top-1)->data.i op (top-0)->data.i; \
		top -= 1; \
	}while(0)

#define FLOAT_OP(op) \
	do{ \
		if((top-0)->type == VM_BOOLEAN_T || (top-0)->type == VM_INTEGER_T){ \
			if((top-1)->type == VM_BOOLEAN_T || (top-1)->type == VM_INTEGER_T){ \
				(top-1)->data.i = (top-1)->data.i op (top-0)->data.i; \
				(--top)->type = VM_INTEGER_T; \
			}else if((top-1)->type == VM_FLOAT_T){ \
				(top-1)->data.f = (top-1)->data.f op (top-0)->data.i; \
				(--top)->type = VM_FLOAT_T; \
			}else{ \
				vm_exception_type((top-1)->type,VM_NUMERIC_T); \
				ERROR(VM_TYPE_E); \
			} \
		}else if((top-0)->type == VM_FLOAT_T){ \
			if((top-1)->type == VM_BOOLEAN_T || (top-1)->type == VM_INTEGER_T){ \
				(top-1)->data.f = (top-1)->data.i op (top-0)->data.f; \
				(--top)->type = VM_FLOAT_T; \
			}else if((top-1)->type == VM_FLOAT_T){ \
				(top-1)->data.f = (top-1)->data.f op (top-0)->data.f; \
				(--top)->type = VM_FLOAT_T; \
			}else{ \
				vm_exception_type((top-1)->type,VM_NUMERIC_T); \
				ERROR(VM_TYPE_E); \
			} \
		}else{ \
			vm_exception_type((top-0)->type,VM_NUMERIC_T); \
			ERROR(VM_TYPE_E); \
		} \
	}while(0)

#define BOOL_OP(op) \
	do{ \
		if((top-0)->type == VM_BOOLEAN_T || (top-0)->type == VM_INTEGER_T){ \
			if((top-1)->type == VM_BOOLEAN_T || (top-1)->type == VM_INTEGER_T){ \
				(top-1)->data.i = (top-1)->data.i op (top-0)->data.i; \
			}else if((top-1)->type == VM_FLOAT_T){ \
				(top-1)->data.i = (top-1)->data.f op (top-0)->data.i; \
			}else{ \
				vm_exception_type((top-1)->type,VM_NUMERIC_T); \
				ERROR(VM_TYPE_E); \
			} \
		}else if((top-0)->type == VM_FLOAT_T){ \
			if((top-1)->type == VM_BOOLEAN_T || (top-1)->type == VM_INTEGER_T){ \
				(top-1)->data.i = (top-1)->data.i op (top-0)->data.f; \
			}else if((top-1)->type == VM_FLOAT_T){ \
				(top-1)->data.i = (top-1)->data.f op (top-0)->data.f; \
			}else{ \
				vm_exception_type((top-1)->type,VM_NUMERIC_T); \
				ERROR(VM_TYPE_E); \
			} \
		}else{ \
			vm_exception_type((top-0)->type,VM_NUMERIC_T); \
			ERROR(VM_TYPE_E); \
		} \
		(--top)->type = VM_BOOLEAN_T; \
	}while(0)

#define CURRENT_STACKFRAME() VM_PACK_STACKFRAME(pc-vm_progmem,base-bottom,arguments)

inline static uint32_t test(vm_variable_t* top){
	switch(top->type){
	case VM_INVALID_T:
	case VM_UNDEFINED_T:
		return 0;
	case VM_BOOLEAN_T:
	case VM_INTEGER_T:
		return !!top->data.i;
	case VM_FLOAT_T:
		return !!top->data.f;
	case VM_STRING_T:
		if(MMID_TO_PTR(top->data.m, vm_string_t*)->size){
			vm_dereference_m(top->data.m, VM_STRING_T);
			return 1;
		}
		vm_dereference_m(top->data.m, VM_STRING_T);
		return 0;
	}
	if(top->data.m){
		vm_variable_dereference(*top);
		return 1;
	}
	return 0;
}

inline static uint32_t eqeq(vm_variable_t* top){
	if((top+0)->type != (top+1)->type){
		vm_variable_dereference(*(top-0));
		vm_variable_dereference(*(top-1));
		return 0;
	}
	if((top+0)->data.i == (top+1)->data.i){
		vm_variable_dereference(*(top-0));
		vm_variable_dereference(*(top-1));
		return 1;
	}
	if((top+0)->type == VM_STRING_T){
		uint32_t r = vm_string_cmp(VM_CAST_STRING(top-0), VM_CAST_STRING(top-1));
		vm_variable_dereference(*(top-0));
		vm_variable_dereference(*(top-1));
		return r;
	}
	vm_variable_dereference(*(top-0));
	vm_variable_dereference(*(top-1));
	return 0;
}

inline static uint32_t eq(vm_variable_t* top){
	if((top+0)->type == (top-1)->type && (top-0)->data.i == (top-1)->data.i){
		vm_variable_dereference(*(top-0));
		vm_variable_dereference(*(top-1));
		return 1;
	}
	if((top-0)->type == VM_STRING_T && (top-1)->type == VM_STRING_T){
		uint32_t r = vm_string_cmp(VM_CAST_STRING(top-0), VM_CAST_STRING(top-1));
		vm_variable_dereference(*(top-0));
		vm_variable_dereference(*(top-1));
		return r;
	}
	if((top-0)->type == VM_BOOLEAN_T || (top-0)->type == VM_INTEGER_T){
		if((top-1)->type == VM_BOOLEAN_T || (top-1)->type == VM_INTEGER_T)
			return (top-1)->data.i == (top-0)->data.i;
		if((top-1)->type == VM_FLOAT_T)
			return (top-1)->data.f == (top-0)->data.i;
	}else if((top-0)->type == VM_FLOAT_T){
		if((top-1)->type == VM_BOOLEAN_T || (top-1)->type == VM_INTEGER_T)
			return (top-1)->data.i == (top-0)->data.f;
		if((top-1)->type == VM_FLOAT_T)
			return (top-1)->data.f == (top-0)->data.f;
	}
	vm_variable_dereference(*(top-0));
	vm_variable_dereference(*(top-1));
	return 0;
}

vm_exception_t vm_mainloop(vm_mmid_t thread_id){
	vm_thread_t* thread = MMID_TO_PTR(thread_id, vm_thread_t*);
	if(thread->state == VM_THREAD_STATE_FINISHED)
		return VM_NONE_E;
	vm_variable_t* bottom = (vm_variable_t*)thread->stack;
	vm_variable_t* top = bottom + thread->top;
	vm_variable_t* end = bottom + thread->size;
	vm_variable_t* base = bottom + ((vm_stackframe_t*)top)->base;
	const vm_opcode_t* pc = vm_progmem + ((vm_stackframe_t*)top)->link;
	uint32_t arguments = ((vm_stackframe_t*)top)->arguments_low | (((vm_stackframe_t*)top)->arguments_high<<8); 
	top -= 1;
	while(true){
		//printf("%03X\t%03d\n",pc-vm_progmem,top-bottom);
		vm_opcode_t opcode = *pc++;
		switch(opcode.o16.op){
			case VM_OP_PUSH_VALUE:
				*(++top) = (vm_variable_t){ .type=opcode.o16.type, .data.i=(pc++)->o32 };
				if(top >= end)
					GROW_THREAD(1);
				break;
			case VM_OP_PUSH_CONST:
				*(++top) = (vm_variable_t){ .type=opcode.o16.type, .data.i=opcode.o16.value };
				if(top >= end)
					GROW_THREAD(1);
				break;
			case VM_OP_PUSH_LOCAL:
				*(++top) = *(base + opcode.o24.value);
				vm_variable_reference(*top);
				if(top >= end)
					GROW_THREAD(1);
				break;
			case VM_OP_PUSH_PARENT: {
				SOFT_ASSERT_TYPE(top, VM_HASHMAP_T);
				vm_mmid_t mmid = MMID_TO_PTR(top->data.m, vm_hashmap_t*)->parent;
				if(mmid == 0xFFFFFFFF)
					*top = (vm_variable_t){.type=VM_UNDEFINED_T, .data.m=0};
				else
					*top = (vm_variable_t){.type=MMID_TO_PTR(mmid, vm_hashmap_t*)->type, .data.m=mmid};
				break;
			}
			case VM_OP_PUSH_MEMBER:
				if((top-1)->type == VM_ARRAY_T){
					ASSERT_TYPE(top-0, VM_INTEGER_T);
					vm_variable_t tmp = vm_array_get(VM_CAST_ARRAY(top-1), top[0].data.i);
					if(tmp.type == VM_INVALID_T){
						vm_exception_oob(top[0].data.i, VM_CAST_ARRAY(top-1)->used);
						ERROR(VM_OOB_E);
					}
					vm_variable_dereference(top[-1]);
					top[-1] = tmp;
					top -= 1;
				}else if(VM_ISTYPE((top-1)->type, VM_HASHMAP_T)){
					ASSERT_TYPE(top-0, VM_STRING_T);
					top[-1] = vm_hashmap_get(VM_CAST_HASHMAP(top-1), vm_string_intern(VM_CAST_STRING(top-0)));
					top -= 1;
				}else if((top-1)->type == VM_STRING_T){
					ASSERT_TYPE(top-0, VM_INTEGER_T);
					vm_variable_t tmp = vm_string_get(VM_CAST_STRING(top-1), top[0].data.i);
					if(tmp.type == VM_INVALID_T){
						vm_exception_oob(top[0].data.i, VM_CAST_STRING(top-1)->size);
						ERROR(VM_OOB_E);
					}
					vm_variable_dereference(top[-1]);
					top[-1] = tmp;
					top -= 1;
				}else{
					vm_exception_type((top-1)->type, VM_INDEXABLE_T);
					ERROR(VM_TYPE_E);
				}
				break;
			case VM_OP_PUSH_MEMBER_UNSAFE:
				SOFT_ASSERT_TYPE(top-1, VM_HASHMAP_T);
				*(--top) = vm_hashmap_get(VM_CAST_HASHMAP(top-1), (top-0)->data.i);
				break;
			case VM_OP_PUSH_MEMBER_CONST:
				top[0] = vm_hashmap_get(VM_CAST_HASHMAP(top-0), opcode.o24.value);
				break;
			case VM_OP_PUSH_ARGUMENT:
				ASSERT_TYPE(top, VM_INTEGER_T);
				if((uint32_t)top->data.i > arguments){
					vm_exception_oob(top[0].data.i, arguments);
					ERROR(VM_OOB_E);
				}
				*top = *(base - top->data.i - 1);
				vm_variable_reference(*top);
				break;
			case VM_OP_PUSH_ARGUMENT_ARRAY: {
				const uint32_t diff = arguments - opcode.o24.value;
				const vm_mmid_t id = vm_array_create(diff);
				vm_array_t* const array = MMID_TO_PTR(id, vm_array_t*);
				for(uint32_t i=0; i<diff; i+=1){
					vm_variable_t* arg = base - i - opcode.o24.value - 1;
					vm_variable_reference(*arg);
					array->data[i] = *arg;
				}
				*(++top) = (vm_variable_t){.type=VM_ARRAY_T, .data.m=id};
				if(top >= end)
					GROW_THREAD(1);
				break;
			} case VM_OP_PUSH_ARGUMENT_COUNT:
				*(++top) = (vm_variable_t){.type=VM_INTEGER_T,.data.i=arguments};
				if(top >= end)
					GROW_THREAD(1);
				break;
			case VM_OP_DEALLOC:
				for(int32_t i = 0; i < opcode.o24.value; i++)
					vm_variable_dereference(*(top--));
				break;
			case VM_OP_ALLOC:
				if(top+opcode.o24.value >= end)
					GROW_THREAD(opcode.o24.value-(end-top));
				for(int32_t i = 0; i < opcode.o24.value; i++)
					*(++top) = (vm_variable_t){.type=VM_UNDEFINED_T};
				break;
			case VM_OP_DUP: {
				vm_variable_t var = *top;
				if(top+opcode.o24.value >= end)
					GROW_THREAD(opcode.o24.value-(end-top));
				for(int32_t i = 0; i < opcode.o24.value; i++){
					*(++top) = var;
					vm_variable_reference(var);
				}
				break;
			} case VM_OP_SET_LOCAL:
				vm_variable_dereference(*(base + opcode.o24.value));
				*(base + opcode.o24.value) = *(top--);
				break;
			case VM_OP_SET_MEMBER:
				if((top-1)->type == VM_ARRAY_T){
					ASSERT_TYPE(top-0, VM_INTEGER_T);
					if(!vm_array_set(VM_CAST_ARRAY(top-1),top[0].data.i,top[-2])){
						vm_exception_oob(top[0].data.i,  VM_CAST_ARRAY(top-1)->used);
						ERROR(VM_OOB_E);
					}
					vm_variable_dereference(top[-1]);
					top -= 3;
				}else if(VM_ISTYPE((top-1)->type, VM_HASHMAP_T)){
					ASSERT_TYPE(top-0, VM_STRING_T);
					vm_hashmap_set(VM_CAST_HASHMAP(top-1), vm_string_intern(VM_CAST_STRING(top-0)), top[-2]);
					top -= 3;
					break;
				}else if((top-1)->type == VM_STRING_T){
					ERROR(VM_IMMUTABLE_E);
				}else{
					vm_exception_type((top-1)->type, VM_INDEXABLE_T);
					ERROR(VM_TYPE_E);
				}
				break;
			case VM_OP_SET_MEMBER_UNSAFE:
				SOFT_ASSERT_TYPE(top-1, VM_HASHMAP_T);
				vm_hashmap_set(VM_CAST_HASHMAP(top-1), top[0].data.i, top[-2]);
				top -= 3;
				break;
			case VM_OP_SET_MEMBER_CONST:
				vm_hashmap_set(VM_CAST_HASHMAP(top-0), opcode.o24.value, top[-1]);
				top -= 2;
				break;
			case VM_OP_JMP:
				pc += opcode.o24.value-1;
				break;
			case VM_OP_JE:
				if(test(top--))
					pc += opcode.o24.value-1;
				break;
			case VM_OP_JNE:
				if(!test(top--))
					pc += opcode.o24.value-1;
				break;
			case VM_OP_APPLY:
			case VM_OP_CALL: {
				uint32_t args = opcode.o24.value;
				if(opcode.o16.op == VM_OP_APPLY){
					ASSERT_TYPE(top-1,VM_ARRAY_T);
					vm_array_t* array = VM_CAST_ARRAY(top-1);
					args = array->used;
					if(top+args >= end)
						GROW_THREAD((top+args)-end);
					vm_variable_t func = top[0];
					top = vm_array_apply(array,top-1);
					vm_dereference(array,VM_ARRAY_T);
					*(++top) = func;
				}
				if(top->type == VM_EXTERN_T){
					vm_hashmap_t* hashmap = MMID_TO_PTR(top->data.m, vm_hashmap_t*);
					if(hashmap->type == VM_NATIVE_T){
						if(top+1 >= end)
							GROW_THREAD(1);
						*(top+1) = (vm_variable_t){ .type=VM_UNDEFINED_T, .data.i=0 };
						vm_exception_t e = hashmap->code.native(top,args);
						if(e != VM_NONE_E)
							ERROR(e);
						for(uint32_t i = 0; i < args; i++)
							vm_variable_dereference(*(top-i-1));
						*(top-args) = top[1];
						top -= args;
					}else{
						ERROR(VM_INTERNAL_E);
					}
				}else if(VM_ISTYPE(top->type, VM_FUNCTION_T)){
					vm_hashmap_t* func = MMID_TO_PTR(top->data.m, vm_hashmap_t*);
					*((vm_stackframe_t*)top) = CURRENT_STACKFRAME();
					arguments = args;
					pc = vm_progmem+func->code.address;
					base = top;					
				}else{
					vm_exception_type(top->type, VM_CALLABLE_T);
					ERROR(VM_TYPE_E);
				}
				break;
			} case VM_OP_CALL_UNSAFE: {
				vm_hashmap_t* func = MMID_TO_PTR(top->data.m, vm_hashmap_t*);
				*((vm_stackframe_t*)top) = CURRENT_STACKFRAME();
				arguments = opcode.o24.value;
				pc = vm_progmem+func->code.address;
				base = top;	
				break;
			} case VM_OP_CALL_ASYNC: {
				SOFT_ASSERT_TYPE(top, VM_FUNCTION_T);
				vm_hashmap_t* func = MMID_TO_PTR(top->data.m, vm_hashmap_t*);
				vm_mmid_t child_id = vm_thread_create(opcode.o24.value+1);
				vm_thread_t* tmp =  MMID_TO_PTR(thread_id, vm_thread_t*);
				if(tmp != thread)
					RELOAD_THREAD(tmp);
				vm_thread_t* child = MMID_TO_PTR(child_id, vm_thread_t*);
				vm_stackitem_t* child_top = child->stack;
				for(vm_variable_t* ptr = top-opcode.o24.value; ptr < top; ptr++)
					(child_top++)->variable = *ptr;
				(child_top++)->frame = VM_PACK_STACKFRAME(pc - vm_progmem,0,0);
				child_top->frame = VM_PACK_STACKFRAME(func->code.address,opcode.o24.value,opcode.o24.value);
				child->top = child_top-child->stack;
				vm_thread_push(child);
				vm_reference(child);
				*top = (vm_variable_t){.type=VM_THREAD_T, .data.m=child_id};
				break;
			} case VM_OP_WAIT:{
				ASSERT_TYPE(top, VM_THREAD_T);
				vm_thread_t* child = MMID_TO_PTR(top->data.m, vm_thread_t*);
				if(child->state == VM_THREAD_STATE_FINISHED){
					*top = child->stack->variable;
					vm_variable_reference(*top);
					vm_dereference(child,VM_THREAD_T);
				}else{	
					pc -= 1;
					vm_thread_wait(thread,child);
					ERROR(VM_YIELD_E);
				}
				break;
			} case VM_OP_RET: {
				for(vm_variable_t* ptr = top-1; ptr > base; ptr--)
					vm_variable_dereference(*ptr);
				for(vm_variable_t* ptr = base-arguments; ptr < base; ptr++)
					vm_variable_dereference(*ptr);
				if(base - arguments == bottom){
					*bottom = *top;
					thread->top = 0;
					thread->state = VM_THREAD_STATE_FINISHED;
					if(thread->queue){
						vm_thread_push(MMID_TO_PTR(thread->queue,vm_thread_t*));
						thread->queue = MMID_NULL;
					}
					return VM_NONE_E;
				}
				vm_stackframe_t* frame = (vm_stackframe_t*)base;
				vm_variable_t ret = *top;
				top = base - arguments;
				base = bottom + frame->base;
				pc = vm_progmem + frame->link;
				arguments = frame->arguments_low | (frame->arguments_high<<8);
				*top = ret;
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
			case VM_OP_ISTYPE:
				vm_variable_dereference(*top);
				top->data.i = VM_ISTYPE(top->type, opcode.o16.type);
				top->type = VM_BOOLEAN_T;
				break;
			case VM_OP_ASSERT_TYPE:
				if(!VM_ISTYPE((base + opcode.o16.value)->type, opcode.o16.type)){
					vm_exception_type((base + opcode.o16.value)->type, opcode.o16.type);
					ERROR(VM_TYPE_E);
				}
				break;
			case VM_OP_ASSERT_ARG_TYPE:
				if(opcode.o16.value < (int32_t)arguments){
					vm_exception_oob(opcode.o16.value, arguments);
					ERROR(VM_OOB_E);
				}
				if(!VM_ISTYPE((base - opcode.o16.value - 1)->type, opcode.o16.type)){
					vm_exception_type((base - opcode.o16.value - 1)->type, opcode.o16.type);
					ERROR(VM_TYPE_E);
				}
				break;
			case VM_OP_ASSERT_ARITY_EQ:
				if(arguments != (uint32_t)opcode.o24.value){
					vm_exception_arrity(arguments, opcode.o24.value);
					ERROR(VM_ARRITY_E);
				}
				break;
			case VM_OP_ASSERT_ARITY_GE:
				if(arguments < (uint32_t)opcode.o24.value){
					vm_exception_arrity(arguments, opcode.o24.value);
					ERROR(VM_ARRITY_E);
				}
				break;
			case VM_OP_SET_ARITY:
				if(arguments < (uint32_t)opcode.o24.value){
					const uint32_t diff = (uint32_t)opcode.o24.value - arguments;
					if(top+diff >= end)
						GROW_THREAD(diff-(end-top));
					for(uint32_t i=0; i<arguments+1; i++)
						*(top+diff-i) = *(top-i);
					for(uint32_t i=0; i<diff; i++)
						*(top-arguments-i) = (vm_variable_t){ .type=VM_UNDEFINED_T, .data.i=0 };
					top += diff;
					base += diff;
					arguments = opcode.o24.value;
				}else if(arguments != (uint32_t)opcode.o24.value){
					vm_exception_arrity(opcode.o24.value, arguments);
					ERROR(VM_ARRITY_E);
				}
				break;
			case VM_OP_BOR:
				INT_OP(|);
				break;
			case VM_OP_BXOR:
				INT_OP(^);
				break;
			case VM_OP_BAND:
				INT_OP(&);
				break;
			case VM_OP_EQ:
				(top-1)->data.i = eq(top);
				(--top)->type = VM_BOOLEAN_T;
				break;
			case VM_OP_EQEQ:
				(top-1)->data.i = eqeq(top);
				(--top)->type = VM_BOOLEAN_T;
				break;
			case VM_OP_NEQ:
				(top-1)->data.i = !eq(top);
				(--top)->type = VM_BOOLEAN_T;
				break;
			case VM_OP_NEQNEQ:
				(top-1)->data.i = !eqeq(top);
				(--top)->type = VM_BOOLEAN_T;
				break;
			case VM_OP_GT:
				BOOL_OP(>);
				break;
			case VM_OP_GE:
				BOOL_OP(>=);
				break;
			case VM_OP_LT:
				BOOL_OP(<);
				break;
			case VM_OP_LE:
				BOOL_OP(<=);
				break;
			case VM_OP_SHR:
				INT_OP(>>);
				break;
			case VM_OP_SHL:
				INT_OP(<<);
				break;
			case VM_OP_ADD:
				if(top->type == VM_STRING_T){
					if((top-1)->type != VM_STRING_T){
						vm_exception_type((top-1)->type, VM_STRING_T);
						ERROR(VM_TYPE_E);
					}
					vm_mmid_t s = vm_string_concat(VM_CAST_STRING(top-1), VM_CAST_STRING(top-0));
					vm_variable_dereference(*(top-1));
					vm_variable_dereference(*(top-0));
					(--top)->data.m = s;
				}else{
					FLOAT_OP(+);
				}
				break;
			case VM_OP_SUB:
				FLOAT_OP(-);
				break;
			case VM_OP_MUL:
				FLOAT_OP(*);
				break;
			case VM_OP_DIV:
				FLOAT_OP(/);
				break;
			case VM_OP_MOD:
				INT_OP(%);
				break;
			case VM_OP_TEST:
				top->data.i = test(top);
				top->type = VM_BOOLEAN_T;
				break;
			case VM_OP_NOT:
				top->data.i = !test(top);
				top->type = VM_BOOLEAN_T;
				break;
			case VM_OP_BNOT:
				ASSERT_TYPE(top,VM_INTEGER_T);
				top->data.i = ~top->data.i;
				break;
			case VM_OP_NEG:
				if(top->type == VM_INTEGER_T){
					top->data.i = -top->data.i;
				}else if(top->type == VM_FLOAT_T){
					top->data.f = -top->data.f;
				}else if(top->type == VM_BOOLEAN_T){
					top->data.i = !top->data.i;
				}else{
					vm_exception_type(top->type, VM_NUMERIC_T);
					ERROR(VM_TYPE_E);
				}
				break;
			default:
				ERROR(VM_INTERNAL_E);
		}
	}
}
