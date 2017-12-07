#include <stdio.h>
#include <string.h>

#include "vm.h"
#include "vm_util.h"
#include "vm_string.h"
#include "vm_hashmap.h"
#include "vm_array.h"
#include "vm_stdlib.h"
#include "vm_op.h"
#include "vm_conf.h"

vm_memory_t vm_mem_level_0;
vm_memory_t vm_mem_level_1;
vm_memory_t vm_mem_level_2;
vm_memory_t vm_mem_level_3;

vm_stackframe_t vm_callstack[CALLSTACK_SIZE];
vm_variable_t vm_stack[STACK_SIZE];

const vm_opcode_t* vm_progmem = NULL;

typedef struct {
	const vm_opcode_t* pc;
	vm_variable_t* top;
	vm_stackframe_t* frame;
} vm_state_t;

vm_state_t vm_state = {NULL, NULL, NULL};

void vm_init(uint32_t mmid_offset){
	vm_memmap_init(MEMMAP_SIZE, MEMMAP_STACK_SIZE);
	vm_memory_init(&vm_mem_level_0, MEMORY_L0_SIZE);
	vm_memory_init(&vm_mem_level_1, MEMORY_L1_SIZE);
	vm_memory_init(&vm_mem_level_2, MEMORY_L2_SIZE);
	vm_memory_init(&vm_mem_level_3, MEMORY_L3_SIZE);
	vm_stringset_init(STRINGSET_SIZE);
	vm_memmap_set_offset(mmid_offset);
}

#define ERROR(e) \
	do{ \
		vm_state = (vm_state_t){ .pc=pc, .top=top, .frame=frame }; \
		return e; \
	}while(0)

#define ASSERT_TYPE(v,t) if((v)->type != t){ ERROR(VM_TYPE_E); }
#define SOFT_ASSERT_TYPE(v,t) if(!VM_ISTYPE((v)->type,t)){ ERROR(VM_TYPE_E); }

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
				ERROR(VM_TYPE_E); \
			} \
		}else{ \
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
				ERROR(VM_TYPE_E); \
			} \
		}else if((top-0)->type == VM_FLOAT_T){ \
			if((top-1)->type == VM_BOOLEAN_T || (top-1)->type == VM_INTEGER_T){ \
				(top-1)->data.i = (top-1)->data.i op (top-0)->data.f; \
			}else if((top-1)->type == VM_FLOAT_T){ \
				(top-1)->data.i = (top-1)->data.f op (top-0)->data.f; \
			}else{ \
				ERROR(VM_TYPE_E); \
			} \
		}else{ \
			ERROR(VM_TYPE_E); \
		} \
		(--top)->type = VM_BOOLEAN_T; \
	}while(0)


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
			vm_dereference(top->data.m, VM_STRING_T);
			return 1;
		}
		vm_dereference(top->data.m, VM_STRING_T);
		return 0;
	}
	if(top->data.m){
		vm_variable_dereference(*top);
		return 1;
	}
	return 0;
}

inline static uint32_t eqeq(vm_variable_t* top){
	if((top+0)->type == (top+1)->type && (top+0)->data.i == (top+1)->data.i){
		vm_variable_dereference(*(top-1));
		vm_variable_dereference(*(top-2));
		return 1;
	}else if((top+0)->type == VM_STRING_T){
		uint32_t r = vm_string_cmp((top+0)->data.m, (top+1)->data.m);
		vm_variable_dereference(*(top-1));
		vm_variable_dereference(*(top-2));
		return r;
	}else{
		vm_variable_dereference(*(top-1));
		vm_variable_dereference(*(top-2));
		return 0;
	}
}

inline static uint32_t eq(vm_variable_t* top){
	if((top+0)->type == (top-1)->type && (top-0)->data.i == (top-1)->data.i){
		vm_variable_dereference(*(top-1));
		vm_variable_dereference(*(top-2));
		return 1;
	}else if((top-0)->type == VM_STRING_T){
		uint32_t r = vm_string_cmp((top-0)->data.m, (top-1)->data.m);
		vm_variable_dereference(*(top-1));
		vm_variable_dereference(*(top-2));
		return r;
	}else{
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
		vm_variable_dereference(*(top-1));
		vm_variable_dereference(*(top-2));
		return 0;
	}
}

vm_exception_t vm_run(){
	vm_stackframe_t* frame = vm_state.frame;
	const vm_opcode_t* pc = vm_state.pc;
	vm_variable_t* top = vm_state.top;
	while(true){
		//printf("%08X %d\n",pc-vm_progmem,pc->o16.op);
		vm_opcode_t opcode = *pc++;
		switch(opcode.o16.op){
			case VM_OP_PUSH_VALUE:
				*(++top) = (vm_variable_t){ .type=opcode.o16.type, .data.i=(pc++)->o32 };
				break;
			case VM_OP_PUSH_CONST:
				*(++top) = (vm_variable_t){ .type=opcode.o16.type, .data.i=opcode.o16.value };
				break;
			case VM_OP_PUSH_LOCAL:
				*(++top) = *(frame->base + opcode.o24.value);
				vm_variable_reference(*top);
				break;
			case VM_OP_PUSH_PARENT: {
				SOFT_ASSERT_TYPE(top, VM_OBJECT_T);
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
					vm_variable_t tmp = vm_array_get((top-1)->data.m, (top-0)->data.i);
					if(tmp.type == VM_INVALID_T)
						ERROR(VM_OOB_E);
					top -= 1;
					vm_variable_dereference(*top);
					*top = tmp;
				}else if(VM_ISTYPE((top-1)->type, VM_OBJECT_T)){
					ASSERT_TYPE(top-0, VM_STRING_T);
					*(top-1) = vm_hashmap_get((top-1)->data.m, vm_string_intern((top-0)->data.i));
					top -= 1;
				}else if((top-1)->type == VM_STRING_T){
					ASSERT_TYPE(top-0, VM_INTEGER_T);
					vm_variable_t tmp = vm_string_get((top-1)->data.m, (top-0)->data.i);
					if(tmp.type == VM_INVALID_T)
						ERROR(VM_OOB_E);
					top -= 1;
					vm_variable_dereference(*top);
					*top = tmp;
				}else{
					ERROR(VM_TYPE_E);
				}
				break;
			case VM_OP_PUSH_MEMBER_UNSAFE:
				SOFT_ASSERT_TYPE(top-1, VM_OBJECT_T);
				vm_variable_dereference(*(top-1));
				*(top-1) = vm_hashmap_get((top-1)->data.m, (top-0)->data.i);
				top -= 1;
				break;
			case VM_OP_PUSH_MEMBER_CONST:
				vm_variable_dereference(*top);
				*top = vm_hashmap_get(top->data.m, opcode.o24.value);
				break;
			case VM_OP_PUSH_ARGUMENT:
				ASSERT_TYPE(top, VM_INTEGER_T);
				if((uint32_t)top->data.i > frame->arguments)
					ERROR(VM_OOB_E);
				*top = *(frame->base - top->data.i);
				break;
			case VM_OP_PUSH_ARGUMENT_ARRAY: {
				const uint32_t diff = frame->arguments-opcode.o24.value;
				const vm_mmid_t id = vm_array_create(diff);
				vm_array_t* const array = MMID_TO_PTR(id, vm_array_t*);
				for(uint32_t i=0; i<diff; i+=1){
					vm_variable_t* arg = frame->base - i - opcode.o24.value;
					vm_variable_reference(*arg);
					array->data[i] = *arg;
				}
				*(++top) = (vm_variable_t){.type=VM_ARRAY_T,.data.m=id};
				break;
			} case VM_OP_PUSH_ARGUMENT_COUNT:
				*(++top) = (vm_variable_t){.type=VM_INTEGER_T,.data.i=frame->arguments};
				break;
			case VM_OP_DEALLOC:
				for(int32_t i = 0; i < opcode.o24.value; i++)
					vm_variable_dereference(*(top--));
				break;
			case VM_OP_ALLOC:
				for(int32_t i = 0; i < opcode.o24.value; i++)
					*(++top) = (vm_variable_t){.type=VM_UNDEFINED_T};
				break;
			case VM_OP_DUP:
				vm_variable_reference(*top);
				*(top+1) = *top;
				top += 1;
				break;
			case VM_OP_SET_LOCAL:
				vm_variable_dereference(*(frame->base + opcode.o24.value));
				*(frame->base + opcode.o24.value) = *(top--);
				break;
			case VM_OP_SET_MEMBER:
				if((top-1)->type == VM_ARRAY_T){
					ASSERT_TYPE(top-0, VM_INTEGER_T);
					if(vm_array_set((top-1)->data.m, (top-0)->data.i,*(top-2)))
						ERROR(VM_OOB_E);
					vm_variable_dereference(*(top-0));
					vm_variable_dereference(*(top-1));
					top -= 3;
				}else if(VM_ISTYPE((top-1)->type, VM_OBJECT_T)){
					ASSERT_TYPE(top-0, VM_STRING_T);
					vm_hashmap_set((top-1)->data.m, vm_string_intern((top-0)->data.i), *(top-2));
					vm_variable_dereference(*(top-0));
					vm_variable_dereference(*(top-1));
					top -= 3;
					break;
				}else{
					ERROR(VM_TYPE_E);
				}
				break;
			case VM_OP_SET_MEMBER_UNSAFE:
				SOFT_ASSERT_TYPE(top-1, VM_OBJECT_T);
				vm_hashmap_set((top-1)->data.m, (top-0)->data.i, *(top-2));
				vm_variable_dereference(*(top-0));
				vm_variable_dereference(*(top-1));
				top -= 3;
				break;
			case VM_OP_SET_MEMBER_CONST:
				vm_hashmap_set((top-0)->data.m, opcode.o24.value, *(top-1));
				vm_variable_dereference(*(top-0));
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
			case VM_OP_CALL:
				if(top->type == VM_EXTERN_T){
					vm_hashmap_t* hashmap = MMID_TO_PTR(top->data.m, vm_hashmap_t*);
					if(hashmap->type == VM_NATIVE_T){
						*(top+1) = (vm_variable_t){ .type=VM_UNDEFINED_T, .data.i=0 };
						vm_exception_t e = hashmap->code.native(top,opcode.o24.value);
						if(e != VM_NONE_E)
							ERROR(e);
						for(int32_t i = 0; i < opcode.o24.value; i++)
							vm_variable_dereference(*(top-i-1));
						*(top-opcode.o24.value) = *(top+1);
						top -= opcode.o24.value;
					}else{
						ERROR(VM_TYPE_E);
					}
				}else if(VM_ISTYPE(top->type, VM_FUNCTION_T)){
					*(++frame) = (vm_stackframe_t){ .arguments=opcode.o24.value, .link=pc, .base=top-1 };
					pc = MMID_TO_PTR(top->data.m, vm_hashmap_t*)->code.address;
					top -= 1;
				}else{
					ERROR(VM_TYPE_E);
				}
				break;
			case VM_OP_CALL_UNSAFE:
				*(++frame) = (vm_stackframe_t){ .arguments=opcode.o24.value, .link=pc, .base=top-1 };
				pc = MMID_TO_PTR(top->data.m, vm_hashmap_t*)->code.address;
				top -= 1;
				break;
			case VM_OP_RET: {
				vm_variable_t tmp = *(top--);
				for(int32_t i = (top - frame->base) + frame->arguments; i > 0; i--)
					vm_variable_dereference(*(top--));
				*(++top) = tmp;
				if(frame == vm_callstack)
					return VM_NONE_E;
				pc = (frame--)->link;
				break;
			}
			case VM_OP_YIELD:
				ERROR(VM_YIELD_E);
				break;
			case VM_OP_THROW:
				ERROR(VM_USER_E);
				break;
			case VM_OP_ISTYPE:
				vm_variable_dereference(*top);
				top->data.i = VM_ISTYPE(top->type, opcode.o16.type);
				top->type = VM_BOOLEAN_T;
				break;
			case VM_OP_ASSERT_TYPE:
				if(!VM_ISTYPE((frame->base + opcode.o16.value)->type, opcode.o16.type))
					ERROR(VM_TYPE_E);
				break;
			case VM_OP_ASSERT_ARG_TYPE:
				if(-opcode.o16.value < (int32_t)frame->arguments && !VM_ISTYPE((frame->base + opcode.o16.value)->type, opcode.o16.type))
					ERROR(VM_TYPE_E);
				break;
			case VM_OP_ASSERT_ARRITY_EQ:
				if(frame->arguments != (uint32_t)opcode.o24.value)
					ERROR(VM_ARRITY_E);
				break;
			case VM_OP_ASSERT_ARRITY_GE:
				if(frame->arguments < (uint32_t)opcode.o24.value)
					ERROR(VM_ARRITY_E);
				break;
			case VM_OP_SET_ARRITY:
				if(frame->arguments < (uint32_t)opcode.o24.value){
					const uint32_t diff = (uint32_t)opcode.o24.value - frame->arguments;
					for(uint32_t i=0; i<frame->arguments; i++)
						*(top+diff-i) = *(top-i);
					for(uint32_t i=0; i<diff; i++)
						*(top-frame->arguments+i+1) = (vm_variable_t){ .type=VM_UNDEFINED_T, .data.i=0 };
					top += diff;
					frame->base = top;
					frame->arguments = opcode.o24.value;
				}else if(frame->arguments != (uint32_t)opcode.o24.value){
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
					if((top-1)->type != VM_STRING_T)
						ERROR(VM_TYPE_E);
					vm_mmid_t s = vm_string_concat((top-1)->data.m, (top-0)->data.m);
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
				}else{
					ERROR(VM_TYPE_E);
				}
				break;
			default:
				ERROR(VM_INVALID_OP_E);
		}
	}
}

void vm_reset(){
	vm_state.top = vm_stack-1;
	vm_state.frame = vm_callstack;
}

void vm_push(vm_variable_t var){
	vm_variable_reference(var);
	*(++vm_state.top) = var;
}

vm_variable_t vm_pop(){
	return *(vm_state.top--);
}

void vm_call(const vm_opcode_t*  address){
	vm_callstack[0] = (vm_stackframe_t){ .arguments=vm_state.top-vm_stack+1, .link=NULL, .base=vm_state.top };
	vm_state.frame = vm_callstack;
	vm_state.top = vm_stack;
	vm_state.pc = address;
}

bool vm_extern_resolve(vm_mmid_t hashmap, const char* name){
	const vm_stdlib_t* item = vm_stdlib;
	while(item->name != NULL){
		if(strcmp(item->name,name)==0){
			vm_hashmap_t* h = MMID_TO_PTR(hashmap, vm_hashmap_t*);
			h->code.native = item->function;
			h->type = VM_NATIVE_T;
			return true;
		}
		item += 1;
	}
	return false;
}