#pragma once

#include <string.h>
#include "../vm.h"
#include "../vm_util.h"
#include "../vm_string.h"
#include "../vm_array.h"
#include "../vm_hashmap.h"

#define ASSERT_ARRITY(n) if(arguments!=n){ \
	vm_exception_arrity(arguments, n);     \
	return VM_ARRITY_E;                    \
}

#define ASSERT_ARRITY_GE(a) if(arguments < a){ \
	vm_exception_arrity(arguments, a);         \
	return VM_ARRITY_E;                        \
}

#define ASSERT_ARRITY_RANGE(a,b)           \
	if(arguments < a){                     \
		vm_exception_arrity(arguments, a); \
		return VM_ARRITY_E;                \
	}else if(arguments > b){               \
		vm_exception_arrity(arguments, b); \
		return VM_ARRITY_E;                \
	}

#define ASSERT_TYPE(n,t) if((top-(n))->type != t){    \
	vm_exception_type(t,(top-(n))->type);             \
	return VM_TYPE_E;                                 \
}                  

#define ASSERT_TYPE_WEAK(n,t) if(!VM_ISTYPE((top-(n))->type,t)){    \
	vm_exception_type(t,(top-(n))->type);                           \
	return VM_TYPE_E;                                               \
}

#define THROW(cstr)                                           \
do{                                                           \
	vm_exception_user(                                        \
		vm_string_to_wstr(                                    \
			MMID_TO_PTR(vm_string_cstr(cstr,0), vm_string_t*) \
		)                                                     \
	);                                                        \
	return VM_USER_E;                                         \
} while(0)

vm_mmid_t lib_int2str(int32_t value);
vm_mmid_t lib_float2str(float value);
vm_mmid_t lib_type2str(vm_type_t value);
