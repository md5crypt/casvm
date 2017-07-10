#include "vm_array.h"
#include "vm_util.h"

uint32_t vm_array_set(vm_mmid_t id, int32_t pos, vm_variable_t value){
	vm_array_t* array = MMID_TO_PTR(id, vm_array_t*);
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

vm_variable_t vm_array_get(vm_mmid_t id, int32_t pos){
	vm_array_t* array = MMID_TO_PTR(id, vm_array_t*);
	if(pos < 0)
		pos += array->used;
	if(pos < 0 || (uint32_t)pos >= array->used)
		return (vm_variable_t){.type=VM_INVALID_T};
	uint32_t index = (pos+array->offset) & (array->size-1);
	vm_variable_reference(array->data[index]);
	return array->data[index];
}

vm_mmid_t vm_array_create(uint32_t size){
	uint32_t bsize = size<=8?16:npot(size*2);
	vm_mmid_t id = vm_memory_allocate(&vm_mem_level_2,bsize*sizeof(vm_variable_t)+sizeof(vm_array_t));
	vm_array_t* ptr = MMID_TO_PTR(id, vm_array_t*);
	ptr->rcnt = 0;
	ptr->size = bsize;
	ptr->used = size;
	ptr->offset = 0;
	for(uint32_t i=0; i<size; i++)
		ptr->data[i].type = VM_UNDEFINED_T;
	return id;
}

void vm_array_free(vm_mmid_t id){
	vm_array_t* array = MMID_TO_PTR(id, vm_array_t*);
	uint32_t offset = array->offset;
	for(uint32_t i=0; i<array->used; i++)
		vm_variable_dereference(array->data[(offset++)&(array->size-1)]);
}

vm_mmid_t vm_array_copy(vm_mmid_t srcid, uint32_t size){
	size = imax(size,MMID_TO_PTR(srcid, vm_array_t*)->used);
	vm_mmid_t dstid = vm_array_create(size==0?MMID_TO_PTR(srcid, vm_array_t*)->used:size);
	vm_array_t* src = MMID_TO_PTR(srcid, vm_array_t*);
	vm_array_t* dst = MMID_TO_PTR(dstid, vm_array_t*);
	uint32_t offset = src->offset;
	uint32_t p = 0;
	for(uint32_t i=0; i<src->used; i++)
		dst->data[p++] = src->data[(offset++)&(src->size-1)];
	while(p < size)
		dst->data[p++].type = VM_UNDEFINED_T;
	return dstid;
}

void vm_array_resize(vm_mmid_t id, uint32_t size){
	vm_mmid_t newid = vm_array_copy(id, size);
	vm_array_free(id);
	vm_memory_replace(&vm_mem_level_2, id, newid);
}

void vm_array_push(vm_mmid_t id, vm_variable_t value){
	vm_array_t* array = MMID_TO_PTR(id, vm_array_t*);
	array->data[((array->used++)+array->offset)&(array->size-1)] = value;
	if(array->used == array->size)
		vm_array_resize(id, 0);
	vm_variable_reference(value);
}

vm_variable_t vm_array_pop(vm_mmid_t id){
	vm_array_t* array = MMID_TO_PTR(id, vm_array_t*);
	if(array->used == 0)
		return (vm_variable_t){.type=VM_UNDEFINED_T};
	return array->data[((--array->used)+array->offset)&(array->size-1)];
}

void vm_array_unshift(vm_mmid_t id, vm_variable_t value){
	vm_array_t* array = MMID_TO_PTR(id, vm_array_t*);
	array->data[(--array->offset)&(array->size-1)] = value;
	if(++array->used == array->size)
		vm_array_resize(id, 0);
	vm_variable_reference(value);
}

vm_variable_t vm_array_shift(vm_mmid_t id){
	vm_array_t* array = MMID_TO_PTR(id, vm_array_t*);
	if(array->used == 0)
		return (vm_variable_t){.type=VM_UNDEFINED_T};
	array->used -= 1;
	return array->data[(array->offset++)&(array->size-1)];
}

