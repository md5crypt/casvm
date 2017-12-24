#include "vm_array.h"
#include "vm_util.h"

uint32_t vm_array_set(vm_array_t* array, int32_t pos, vm_variable_t value){
	if(pos < 0)
		pos += array->used;
	if(pos < 0 || (uint32_t)pos >= array->used)
		return 1;
	uint32_t index = (pos+array->offset) & (array->size-1);
	vm_variable_dereference(array->data[index]);
	array->data[index] = value;
	vm_variable_reference(value);
	return 0;
}

vm_variable_t vm_array_get(vm_array_t* array, int32_t pos){
	if(pos < 0)
		pos += array->used;
	if(pos < 0 || (uint32_t)pos >= array->used)
		return (vm_variable_t){.type=VM_INVALID_T};
	uint32_t index = (pos+array->offset) & (array->size-1);
	vm_variable_reference(array->data[index]);
	return array->data[index];
}

vm_mmid_t vm_array_create(uint32_t size){
	uint32_t bsize = size<=VM_ARRAY_SIZE_MIN?(VM_ARRAY_SIZE_MIN*2):npot(size);
	vm_mmid_t id = vm_memory_allocate(&vm_mem_array,bsize*sizeof(vm_variable_t)+sizeof(vm_array_t));
	vm_array_t* ptr = MMID_TO_PTR(id, vm_array_t*);
	ptr->rcnt = 1;
	ptr->size = bsize;
	ptr->used = size;
	ptr->offset = 0;
	for(uint32_t i=0; i<size; i++)
		ptr->data[i].type = VM_UNDEFINED_T;
	return id;
}

void vm_array_free(vm_array_t* array){
	uint32_t offset = array->offset;
	for(uint32_t i=0; i<array->used; i++)
		vm_variable_dereference(array->data[(offset++)&(array->size-1)]);
	vm_memory_free(&vm_mem_array, PTR_TO_MMID(array));
}

vm_mmid_t vm_array_slice(vm_array_t* array, int32_t start, int32_t stop){
	if(start < 0)
		start += array->used;
	if(stop < 0)
		stop += array->used;
	if((uint32_t)start >= array->used || start < 0 || (uint32_t)stop > array->used || stop < 0 || start > stop)
		return MMID_NULL;
	uint32_t size = stop-start;
	vm_mmid_t srcid = PTR_TO_MMID(array);
	vm_mmid_t dstid = vm_array_create(size);
	vm_array_t* src = MMID_TO_PTR(srcid, vm_array_t*);
	vm_array_t* dst = MMID_TO_PTR(dstid, vm_array_t*);
	uint32_t offset = src->offset+start;
	for(uint32_t i=0; i<size; i++){
		dst->data[i] = src->data[(offset++)&(src->size-1)];
		vm_variable_reference(dst->data[i]);
	}
	dst->used = size;
	return dstid;
}

vm_mmid_t vm_array_concat_m(vm_mmid_t a, vm_mmid_t b){
	vm_array_t* A = MMID_TO_PTR(a,vm_array_t*);
	vm_array_t* B = MMID_TO_PTR(b,vm_array_t*);
	vm_array_t* C;
	vm_mmid_t c = vm_array_create(A->used+B->used);
	A = MMID_TO_PTR(a,vm_array_t*);
	B = MMID_TO_PTR(b,vm_array_t*);
	C = MMID_TO_PTR(c,vm_array_t*);
	uint32_t offset = A->offset;
	vm_variable_t* ptr = C->data;
	for(uint32_t i=0; i<A->used; i++){
		*ptr = A->data[(offset++)&(A->size-1)];
		vm_variable_reference((*ptr++));
	}
	offset = B->offset;
	for(uint32_t i=0; i<B->used; i++){
		*ptr = B->data[(offset++)&(B->size-1)];
		vm_variable_reference((*ptr++));
	}
	C->used = A->used+B->used;
	return c;
}

void vm_array_grow(vm_array_t* array){
	vm_mmid_t oldid = PTR_TO_MMID(array);
	vm_mmid_t newid = vm_array_create(array->size*2);
	vm_array_t* oldptr = MMID_TO_PTR(oldid,vm_array_t*);
	vm_array_t* newptr = MMID_TO_PTR(newid,vm_array_t*);
	*newptr = *oldptr;
	uint32_t offset = oldptr->offset;
	for(uint32_t i=0; i<oldptr->used; i++)
		newptr->data[i] = oldptr->data[(offset++)&(oldptr->size-1)];
	vm_memory_replace(&vm_mem_array, oldid, newid);
}

void vm_array_push(vm_array_t* array, vm_variable_t value){
	array->data[((array->used++)+array->offset)&(array->size-1)] = value;
	if(array->used == array->size)
		vm_array_grow(array);
	vm_variable_reference(value);
}

vm_variable_t vm_array_pop(vm_array_t* array){
	if(array->used == 0)
		return (vm_variable_t){.type=VM_UNDEFINED_T};
	return array->data[((--array->used)+array->offset)&(array->size-1)];
}

void vm_array_unshift(vm_array_t* array, vm_variable_t value){
	array->data[(--array->offset)&(array->size-1)] = value;
	if(++array->used == array->size)
		vm_array_grow(array);
	vm_variable_reference(value);
}

vm_variable_t vm_array_shift(vm_array_t* array){
	if(array->used == 0)
		return (vm_variable_t){.type=VM_UNDEFINED_T};
	array->used -= 1;
	return array->data[(array->offset++)&(array->size-1)];
}

