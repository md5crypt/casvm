#include <stddef.h>
#include "vm_array.h"
#include "vm_util.h"
#include "vm_conf.h"

vm_array_t* vm_array_set(vm_array_t* array, int32_t pos, vm_variable_t value){
	if(pos < 0)
		pos += array->used;
	if(pos < 0 || (uint32_t)pos >= array->used)
		return NULL;
	uint32_t index = (pos+array->offset) & (array->size-1);
	vm_variable_dereference(array->data[index]);
	array->data[index] = value;
	return array;
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

static vm_array_t* grow(vm_array_t* array, uint32_t newsize){
	vm_mmid_t oldid = PTR_TO_MMID(array);
	vm_mmid_t newid = vm_array_create(newsize?newsize:array->size*2);
	vm_array_t* oldptr = MMID_TO_PTR(oldid,vm_array_t*);
	vm_array_t* newptr = MMID_TO_PTR(newid,vm_array_t*);
	newptr->rcnt = oldptr->rcnt;
	newptr->used = oldptr->used;
	uint32_t offset = oldptr->offset;
	for(uint32_t i=0; i<oldptr->used; i++)
		newptr->data[i] = oldptr->data[(offset++)&(oldptr->size-1)];
	vm_memory_replace(&vm_mem_array, oldid, newid);
	return newptr;
}

vm_array_t* vm_array_resize(vm_array_t* array, uint32_t size){
	if(array->size < size)
		array = grow(array,size);
	uint32_t offset = (array->offset + array->used)&(array->size-1);
	if(array->used > size){
		uint32_t diff = array->used - size;
		while(diff--)
			vm_variable_reference(array->data[(--offset)&(array->size-1)]);
	}else{
		uint32_t diff = size-array->used;
		while(diff--)
			array->data[(offset++)&(array->size-1)].type = VM_UNDEFINED_T;
	}
	array->used = size;
	return array;
}

vm_array_t* vm_array_push(vm_array_t* array, vm_variable_t value){
	vm_variable_reference(value);
	array->data[((array->used++)+array->offset)&(array->size-1)] = value;
	if(array->used == array->size)
		return grow(array,0);
	return array;
}

vm_array_t* vm_array_write(vm_array_t* dst, vm_array_t* src, int32_t offset, int32_t len){
	if(offset < 0)
		offset += dst->used;
	if(len < 0)
		len += src->used;
	if(dst->used < (uint32_t)(offset+len) || len < 0 || offset < 0 || (uint32_t)offset > dst->used)
		return NULL;
	uint32_t offset_dst = dst->offset + offset;
	uint32_t offset_src = src->offset;
	while(len--){
		vm_variable_t var = src->data[(offset_src++)&(src->size-1)];
		vm_variable_reference(var);
		dst->data[(offset_dst++)&(dst->size-1)] = var;
	}
	return dst;
}

vm_array_t* vm_array_fill(vm_array_t* array, vm_variable_t var, int32_t offset, int32_t len){
	if(offset < 0)
		offset += array->used;
	if(len < 0)
		len += array->used;
	if(array->used < (uint32_t)(offset+len) || len < 0 || offset < 0 || (uint32_t)offset > array->used)
		return NULL;
	offset = array->offset + offset;
	while(len--){
		vm_variable_dereference(array->data[offset&(array->size-1)]);
		array->data[offset&(array->size-1)] = var;
		vm_variable_reference(var);
		offset += 1;
	}
	return array;
}

void vm_array_reverse(vm_array_t* array){
	uint32_t left = array->offset;
	uint32_t right = array->offset + array->used - 1;
	uint32_t cnt = array->used>>1;
	while(cnt--){
		vm_variable_t var = array->data[left&(array->size-1)];
		array->data[left&(array->size-1)] = array->data[right&(array->size-1)];
		array->data[right&(array->size-1)] = var;
		left += 1;
		right -= 1;
	}
}

int32_t vm_array_find(vm_array_t* array, vm_variable_t var, int32_t offset){
	if(offset < 0)
		offset += array->used;
	if(offset < 0 || (uint32_t)offset >= array->used)
		return -2;
	uint32_t cnt = array->used - offset;
	offset += array->offset;
	while(cnt--){
		if(vm_variable_compare(var,array->data[offset&(array->size-1)]))
			return offset;
		offset += 1;
	}
	return -1;
}

vm_variable_t vm_array_pop(vm_array_t* array){
	if(array->used == 0)
		return (vm_variable_t){.type=VM_UNDEFINED_T};
	return array->data[((--array->used)+array->offset)&(array->size-1)];
}

vm_array_t* vm_array_unshift(vm_array_t* array, vm_variable_t value){
	vm_variable_reference(value);
	array->data[(--array->offset)&(array->size-1)] = value;
	if(++array->used == array->size)
		return grow(array,0);
	return array;
}

vm_variable_t vm_array_shift(vm_array_t* array){
	if(array->used == 0)
		return (vm_variable_t){.type=VM_UNDEFINED_T};
	array->used -= 1;
	return array->data[(array->offset++)&(array->size-1)];
}

vm_variable_t* vm_array_apply(vm_array_t* array, vm_variable_t* top){
	uint32_t offset = array->offset+array->used-1;
	for(uint32_t i=0; i<array->used; i++){
		top[0] = array->data[(offset--)&(array->size-1)];
		vm_variable_reference(top[0]);
		top += 1;
	}
	return top-1;
}
