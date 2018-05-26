#include "vm_lib.h"

vm_exception_t vm_lib_array_create(vm_variable_t* top, uint32_t arguments){
	ASSERT_ARRITY_RANGE(1,2);
	ASSERT_TYPE(1,VM_INTEGER_T);
	int32_t size = top[-1].data.i;
	if(size < 0)
		THROW("Negative array size");
	vm_mmid_t id = vm_array_create(size);
	if(arguments > 1){
		vm_variable_t* data = MMID_TO_PTR(id,vm_array_t*)->data;
		while(size--){
			vm_variable_reference(top[-2]);
			*(data++) = top[-2];
		}
	}
	top[1] = (vm_variable_t){.type=VM_ARRAY_T, .data.m=id};
	return VM_NONE_E;
}

vm_exception_t vm_lib_array_static(vm_variable_t* top, uint32_t arguments){
	vm_mmid_t id = vm_array_create(arguments);
	if(arguments > 0){
		vm_variable_t* data = MMID_TO_PTR(id,vm_array_t*)->data;
		for(uint32_t i=1; i<=arguments; i++){
			vm_variable_reference(*(top-i));
			*(data++) = *(top-i);
		}
	}
	top[1] = (vm_variable_t){.type=VM_ARRAY_T, .data.m=id};
	return VM_NONE_E;
}

vm_exception_t vm_lib_array_push(vm_variable_t* top, uint32_t arguments){
	ASSERT_ARRITY_GE(2);
	ASSERT_TYPE(1,VM_ARRAY_T);
	vm_array_t* array = VM_CAST_ARRAY(top-1);
	for(uint32_t i=2; i<=arguments; i++)
		array = vm_array_push(array,*(top-i));
	top[1] = top[-1];
	vm_reference(array);
	return VM_NONE_E;
}

vm_exception_t vm_lib_array_pop(vm_variable_t* top, uint32_t arguments){
	ASSERT_ARRITY(1);
	ASSERT_TYPE(1,VM_ARRAY_T);
	top[1] = vm_array_pop(VM_CAST_ARRAY(top-1));
	return VM_NONE_E;
}

vm_exception_t vm_lib_array_unshift(vm_variable_t* top, uint32_t arguments){
	ASSERT_ARRITY_GE(2);
	ASSERT_TYPE(1,VM_ARRAY_T);
	vm_array_t* array = VM_CAST_ARRAY(top-1);
	for(uint32_t i=arguments; i>=2; i--)
		array = vm_array_unshift(array,*(top-i));
	top[1] = top[-1];
	vm_reference(array);
	return VM_NONE_E;
}

vm_exception_t vm_lib_array_shift(vm_variable_t* top, uint32_t arguments){
	ASSERT_ARRITY(1);
	ASSERT_TYPE(1,VM_ARRAY_T);
	top[1] = vm_array_shift(VM_CAST_ARRAY(top-1));
	return VM_NONE_E;
}

vm_exception_t vm_lib_array_resize(vm_variable_t* top, uint32_t arguments){
	ASSERT_ARRITY(2);
	ASSERT_TYPE(1,VM_ARRAY_T);
	ASSERT_TYPE(2,VM_INTEGER_T);
	vm_array_t* array = VM_CAST_ARRAY(top-1);
	vm_array_resize(array,top[-2].data.i);
	vm_reference(array);
	top[1] = top[-1];
	return VM_NONE_E;
}

vm_exception_t vm_lib_array_slice(vm_variable_t* top, uint32_t arguments){
	ASSERT_ARRITY_RANGE(1,3);
	ASSERT_TYPE(1,VM_ARRAY_T);
	vm_array_t* array = VM_CAST_ARRAY(top-1);
	int32_t start = 0;
	int32_t stop = array->used;
	if(arguments > 1){
		ASSERT_TYPE(2,VM_INTEGER_T);
		start = top[-2].data.i;
	}
	if(arguments > 2){
		ASSERT_TYPE(3,VM_INTEGER_T);
		stop = top[-3].data.i;
	}
	top[1] = (vm_variable_t){.data.m=vm_array_slice(array,start,stop),.type=VM_ARRAY_T};
	return VM_NONE_E;
}


vm_exception_t vm_lib_array_write(vm_variable_t* top, uint32_t arguments){
	ASSERT_ARRITY_RANGE(2,4);
	ASSERT_TYPE(1,VM_ARRAY_T);
	ASSERT_TYPE(2,VM_ARRAY_T);
	vm_array_t* dst = VM_CAST_ARRAY(top-1);
	vm_array_t* src = VM_CAST_ARRAY(top-2);
	int32_t offset = 0;
	int32_t length = src->used;
	if(arguments > 2){
		ASSERT_TYPE(3,VM_INTEGER_T);
		offset = top[-3].data.i;
	}
	if(arguments > 4){
		ASSERT_TYPE(3,VM_INTEGER_T);
		length = top[-4].data.i;
	}
	if(!vm_array_write(dst,src,offset,length)){
		if(iabs(length) > src->used){
			vm_exception_oob(length>0?length-1:length+1, src->used);
		}else{
			vm_exception_oob(
				(-offset > (int32_t)dst->used) ? offset : (
					(length>0?length:length+src->used) +
					(offset>0?offset:offset+dst->used)
				), dst->used
			);
		}
		return VM_OOB_E;
	}
	vm_reference(dst);
	top[1] = top[-1];
	return VM_NONE_E;
}

vm_exception_t vm_lib_array_fill(vm_variable_t* top, uint32_t arguments){
	ASSERT_ARRITY_RANGE(2,4);
	ASSERT_TYPE(1,VM_ARRAY_T);
	vm_array_t* array = VM_CAST_ARRAY(top-1);
	int32_t offset = 0;
	int32_t length;
	if(arguments > 2){
		ASSERT_TYPE(3,VM_INTEGER_T);
		offset = top[-3].data.i;
		if(offset < 0)
			offset += array->used;
	}
	if(arguments > 3){
		ASSERT_TYPE(4,VM_INTEGER_T);
		length = top[-4].data.i;
	}else{
		length = array->used - offset;
	}
	if(!vm_array_fill(array,top[-2],offset,length)){
		vm_exception_oob(
			(uint32_t)(-offset > (int32_t)array->used) ? offset : (
				(-length > (int32_t)array->used) ? length : (
					(length>0?length:length+array->used) +
					(offset>0?offset:offset+array->used)
				)
			), array->used
		);
		return VM_OOB_E;
	}
	vm_reference(array);
	top[1] = top[-1];
	return VM_NONE_E;
}

vm_exception_t vm_lib_array_find(vm_variable_t* top, uint32_t arguments){
	ASSERT_ARRITY_RANGE(2,3);
	ASSERT_TYPE(1,VM_ARRAY_T);
	vm_array_t* array = VM_CAST_ARRAY(top-1);
	int32_t offset = 0;
	if(arguments == 3){
		ASSERT_TYPE(3,VM_INTEGER_T);
		offset = top[-3].data.i;
	}
	int32_t result = vm_array_find(array,top[-2],offset);
	if(result == -2){
		vm_exception_oob(offset, array->used);
		return VM_OOB_E;
	}
	top[1] = (vm_variable_t){.type=VM_INTEGER_T,.data.i=result};
	return VM_NONE_E;
}

vm_exception_t vm_lib_array_expand(vm_variable_t* top, uint32_t arguments){
	ASSERT_ARRITY_GE(2);
	uint32_t size = 0;
	for(uint32_t i=1; i<=arguments; i++){
		ASSERT_TYPE(1,VM_ARRAY_T);
		size += VM_CAST_ARRAY(top-i)->used;
	}
	vm_array_t* output = VM_CAST_ARRAY(top-1);
	uint32_t offset = output->used;
	output = vm_array_resize(output,size);
	for(uint32_t i=2; i<=arguments; i++){
		vm_array_t* src = VM_CAST_ARRAY(top-i);
		output = vm_array_write(output,src,offset,src->used);
		offset += src->used;
	}
	vm_reference(output);
	top[1] = top[-1];
	return VM_NONE_E;
}

vm_exception_t vm_lib_array_reverse(vm_variable_t* top, uint32_t arguments){
	ASSERT_ARRITY(1);
	ASSERT_TYPE(1,VM_ARRAY_T);
	vm_array_t* array = VM_CAST_ARRAY(top-1);
	vm_array_reverse(array);
	vm_reference(array);
	top[1] = top[-1];
	return VM_NONE_E;
}
