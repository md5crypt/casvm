#include <stddef.h>
#include <stdbool.h>
#include "vm_array.h"
#include "vm_util.h"
#include "vm_conf.h"

vm_exception_t vm_array_set(vm_array_t* array, int32_t pos, vm_variable_t value) {
	if (pos < 0) {
		pos += array->used;
	}
	if (pos < 0 || ((uint32_t)pos >= array->used)) {
		vm_exception_oob(pos, array->used);
		return VM_OOB_E;
	}
	uint32_t index = (pos + array->offset) & (array->size - 1);
	vm_variable_dereference(array->data[index]);
	array->data[index] = value;
	return VM_NONE_E;
}

vm_exception_t vm_array_get(const vm_array_t* array, int32_t pos, vm_variable_t* value) {
	if (pos < 0) {
		pos += array->used;
	}
	if ((pos < 0) || ((uint32_t)pos >= array->used)) {
		value[0] = VM_VARIABLE(VM_INVALID_T);
		vm_exception_oob(pos, array->used);
		return VM_OOB_E;
	}
	uint32_t index = (pos + array->offset) & (array->size - 1);
	vm_variable_reference(array->data[index]);
	value[0] = array->data[index];
	return VM_NONE_E;
}

vm_mmid_t vm_array_create(uint32_t size) {
	uint32_t bsize = (size <= VM_ARRAY_SIZE_MIN) ? (VM_ARRAY_SIZE_MIN * 2) : npot(size);
	vm_mmid_t id = vm_memory_allocate(
		&vm_mem_array,
		(bsize * sizeof(vm_variable_t)) + sizeof(vm_array_t)
	);
	vm_array_t* ptr = MMID_TO_PTR(id, vm_array_t*);
	ptr->rcnt = 1;
	ptr->size = bsize;
	ptr->used = size;
	ptr->offset = 0;
	for (uint32_t i = 0; i < size; i++) {
		ptr->data[i] = VM_VARIABLE(VM_UNDEFINED_T);
	}
	return id;
}

void vm_array_free(vm_array_t* array) {
	uint32_t offset = array->offset;
	for (uint32_t i = 0; i < array->used; i++) {
		vm_variable_dereference(array->data[offset & (array->size - 1)]);
		offset += 1;
	}
	vm_memory_free(&vm_mem_array, PTR_TO_MMID(array));
}

vm_mmid_t vm_array_slice(const vm_array_t* array, int32_t start, int32_t stop) {
	if (start < 0) {
		start += array->used;
	}
	if (stop < 0) {
		stop += array->used;
	}
	if (start >= stop) {
		return vm_array_create(0);
	}
	if ((start < 0) || ((uint32_t)start > array->used)) {
		vm_exception_oob((stop < 0) ? stop : (int32_t)(array->used - 1), array->used);
		return MMID_NULL;
	}
	if ((uint32_t)stop > array->used) {
		vm_exception_oob((int32_t)(array->used - 1), array->used);
		return MMID_NULL;
	}
	uint32_t size = stop - start;
	vm_mmid_t src_id = PTR_TO_MMID(array);
	vm_mmid_t dst_id = vm_array_create(size);
	vm_array_t* src = MMID_TO_PTR(src_id, vm_array_t*);
	vm_array_t* dst = MMID_TO_PTR(dst_id, vm_array_t*);
	uint32_t offset = src->offset + start;
	for (uint32_t i = 0; i < size; i++) {
		dst->data[i] = src->data[offset & (src->size - 1)];
		offset += 1;
		vm_variable_reference(dst->data[i]);
	}
	dst->used = size;
	return dst_id;
}

static vm_array_t* grow(vm_array_t* array, uint32_t newsize) {
	vm_mmid_t old_id = PTR_TO_MMID(array);
	vm_mmid_t new_id = vm_array_create(newsize ? newsize : (array->size * 2));
	vm_array_t* oldptr = MMID_TO_PTR(old_id, vm_array_t*);
	vm_array_t* newptr = MMID_TO_PTR(new_id, vm_array_t*);
	newptr->rcnt = oldptr->rcnt;
	newptr->used = oldptr->used;
	uint32_t offset = oldptr->offset;
	for (uint32_t i = 0; i < oldptr->used; i++) {
		newptr->data[i] = oldptr->data[offset & (oldptr->size - 1)];
		offset += 1;
	}
	vm_memory_replace(&vm_mem_array, old_id, new_id);
	return newptr;
}

vm_array_t* vm_array_resize(vm_array_t* array, uint32_t size) {
	if (array->size < size) {
		array = grow(array, size);
	}
	if (array->used > size) {
		uint32_t diff = array->used - size;
		uint32_t offset = array->offset + array->used - 1;
		while (diff--) {
			vm_variable_dereference(array->data[offset & (array->size - 1)]);
			offset -= 1;
		}
	} else {
		uint32_t diff = size - array->used;
		uint32_t offset = array->offset + array->used;
		while (diff--) {
			array->data[offset & (array->size - 1)] = VM_VARIABLE(VM_UNDEFINED_T);
			offset += 1;
		}
	}
	array->used = size;
	return array;
}

vm_array_t* vm_array_push(vm_array_t* array, vm_variable_t value) {
	vm_variable_reference(value);
	array->data[(array->used + array->offset) & (array->size - 1)] = value;
	array->used += 1;
	if (array->used == array->size) {
		return grow(array, 0);
	}
	return array;
}

vm_exception_t vm_array_write(vm_array_t* dst, const vm_array_t* src, int32_t offset, int32_t len) {
	if (offset < 0) {
		offset += dst->used;
	}
	if (len <= 0) {
		return VM_NONE_E;
	}
	if (src->used < (uint32_t)len) {
		vm_exception_oob((int32_t)(src->used - 1), src->used);
		return VM_OOB_E;
	}
	if ((offset < 0) || (dst->used < (uint32_t)(offset + len))) {
		vm_exception_oob((offset < 0) ? offset : (int32_t)(dst->used - 1), dst->used);
		return VM_OOB_E;
	}
	uint32_t offset_dst = dst->offset + offset;
	uint32_t offset_src = src->offset;
	while (len--) {
		offset_src &= (src->size - 1);
		offset_dst &= (dst->size - 1);
		vm_variable_reference(src->data[offset_src]);
		vm_variable_dereference(dst->data[offset_dst]);
		dst->data[offset_dst] = src->data[offset_src];
		offset_dst += 1;
		offset_src += 1;
	}
	return VM_NONE_E;
}

vm_exception_t vm_array_fill(vm_array_t* array, vm_variable_t var, int32_t offset, int32_t len) {
	if (offset < 0) {
		offset += array->used;
	}
	if (len <= 0) {
		return VM_NONE_E;
	}
	if ((offset < 0) || ((uint32_t)(offset + len) > array->used)) {
		vm_exception_oob((offset < 0) ? offset : (int32_t)(array->used - 1), array->used);
		return VM_OOB_E;
	}
	offset = array->offset + offset;
	while (len--) {
		offset &= array->size - 1;
		vm_variable_dereference(array->data[offset]);
		array->data[offset] = var;
		vm_variable_reference(var);
		offset += 1;
	}
	return VM_NONE_E;
}

void vm_array_reverse(vm_array_t* array) {
	uint32_t left = array->offset;
	uint32_t right = array->offset + array->used - 1;
	uint32_t cnt = array->used >> 1;
	while (cnt--) {
		left &= (array->size - 1);
		right &= (array->size - 1);
		vm_variable_t var = array->data[left];
		array->data[left] = array->data[right];
		array->data[right] = var;
		left += 1;
		right -= 1;
	}
}

int32_t vm_array_find(vm_array_t* array, vm_variable_t var, int32_t offset) {
	if (offset < 0) {
		offset += array->used;
	}
	if (offset < 0 || (uint32_t)offset >= array->used) {
		vm_exception_oob((offset < 0) ? offset : (int32_t)(array->used - 1), array->used);
		return -2;
	}
	uint32_t cnt = array->used - offset;
	offset = offset + array->offset;
	while (cnt--) {
		if (vm_variable_compare(var, array->data[offset & (array->size - 1)])) {
			return (offset - array->offset) & (array->size - 1);
		}
		offset += 1;
	}
	return -1;
}

vm_exception_t vm_array_pop(vm_array_t* array, vm_variable_t* value) {
	if (array->used == 0) {
		vm_exception_oob(0, 0);
		value[0] = VM_VARIABLE(VM_UNDEFINED_T);
		return VM_OOB_E;
	}
	array->used -= 1;
	value[0] = array->data[(array->used + array->offset) & (array->size - 1)];
	return VM_NONE_E;
}

vm_array_t* vm_array_unshift(vm_array_t* array, vm_variable_t value) {
	vm_variable_reference(value);
	array->offset -= 1;
	array->used += 1;
	array->data[array->offset & (array->size - 1)] = value;
	if (array->used == array->size) {
		return grow(array, 0);
	}
	return array;
}

vm_exception_t vm_array_shift(vm_array_t* array, vm_variable_t* value) {
	if (array->used == 0) {
		vm_exception_oob(0, 0);
		value[0] = VM_VARIABLE(VM_UNDEFINED_T);
		return VM_OOB_E;
	}
	value[0] = array->data[array->offset & (array->size - 1)];
	array->offset += 1;
	array->used -= 1;
	return VM_NONE_E;
}

vm_variable_t* vm_array_apply(const vm_array_t* array, vm_variable_t* top) {
	uint32_t offset = array->offset + array->used - 1;
	for (uint32_t i = 0; i < array->used; i++) {
		top[0] = array->data[offset & (array->size - 1)];
		offset -= 1;
		vm_variable_reference(top[0]);
		top += 1;
	}
	return top - 1;
}
