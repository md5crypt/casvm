#include <stdlib.h>
#include <string.h>
#include "vm_string.h"
#include "vm_util.h"

typedef struct {
	uint32_t size;
	uint32_t used;
	vm_mmid_t data[0];
} vm_hashset_t;

vm_hashset_t* set;

static uint32_t mkhash(const vm_string_t* key) {
	uint32_t h = 0;
	const uint16_t* data = key->data;
	for (uint32_t i = 0; i < key->size; i++) {
		h = 31 * h + data[i];
	}
	return (h ^ (h >> 20) ^ (h >> 12) ^ (h >> 7) ^ (h >> 4));
}

static uint32_t get(const vm_string_t* key) {
	uint32_t hash = mkhash(key)&(set->size - 1);
	while (set->data[hash] != MMID_NULL) {
		vm_string_t* str = MMID_TO_PTR(set->data[hash], vm_string_t*);
		if ((str->size == key->size) && !memcmp(str->data, key->data, key->size * sizeof(uint16_t))) {
			return hash;
		}
		hash = (hash + 1) & (set->size - 1);
	}
	return hash;
}

static void grow() {
	vm_hashset_t* oldset = set;
	vm_stringset_init(set->size << 1);
	for (uint32_t i = 0; i < oldset->size; i++) {
		if (oldset->data[i] != MMID_NULL) {
			uint32_t key = get(MMID_TO_PTR(oldset->data[i], vm_string_t*));
			set->data[key] = oldset->data[i];
			set->used++;
		}
	}
	free(oldset);
}

void vm_stringset_init(uint32_t size) {
	set = (vm_hashset_t*)malloc((size * sizeof(vm_mmid_t)) + sizeof(vm_hashset_t));
	memset(
		((uint8_t*)set) + sizeof(vm_hashset_t),
		0,
		size * sizeof(vm_mmid_t)
	);
	set->size = size;
	set->used = 0;
}

vm_mmid_t vm_string_intern(vm_string_t* str) {
	if (str->rcnt == VM_CONSTANT) {
		return PTR_TO_MMID(str);
	}
	uint32_t key = get(str);
	vm_mmid_t value = set->data[key];
	if (value == MMID_NULL) {
		value = vm_string_copy(str, true);
		set->data[key] = value;
		set->used += 1;
		if (set->used > (set->size >> 1)) {
			grow();
		}
	}
	vm_dereference(str, VM_STRING_T);
	return value;
}

vm_mmid_t vm_string_create(uint32_t len) {
	vm_mmid_t id = vm_memory_allocate(
		&vm_mem_string,
		(len * sizeof(uint16_t)) + sizeof(vm_string_t)
	);
	vm_string_t* ptr = MMID_TO_PTR(id, vm_string_t*);
	ptr->size = len;
	ptr->rcnt = 1;
	return id;
}

vm_mmid_t vm_string_insert(const uint16_t* data, uint32_t len) {
	vm_mmid_t id = vm_memory_allocate(
		&vm_mem_const,
		(len * sizeof(uint16_t)) + sizeof(vm_string_t)
	);
	vm_string_t* ptr = MMID_TO_PTR(id, vm_string_t*);
	ptr->size = len;
	ptr->rcnt = VM_CONSTANT;
	memcpy(ptr->data, data, len * 2);
	uint32_t hash = get(MMID_TO_PTR(id, vm_string_t*));
	set->data[hash] = id;
	set->used += 1;
	if (set->used > (set->size>>1)) {
		grow();
	}
	return id;
}

vm_mmid_t vm_string_copy(const vm_string_t* string, bool constant) {
	vm_mmid_t src_id = PTR_TO_MMID(string);
	vm_mmid_t dst_id = vm_memory_allocate(
		constant ? &vm_mem_const : &vm_mem_string,
		(string->size * sizeof(uint16_t)) + sizeof(vm_string_t)
	);
	vm_string_t* dst = MMID_TO_PTR(dst_id, vm_string_t*);
	vm_string_t* src = MMID_TO_PTR(src_id, vm_string_t*);
	dst->size = src->size;
	dst->rcnt = constant ? VM_CONSTANT : 1;
	memcpy(dst->data, src->data, src->size * sizeof(uint16_t));
	return dst_id;
}

vm_mmid_t vm_string_slice(const vm_string_t* string, int32_t start, int32_t stop) {
	if (start < 0) {
		start += string->size;
	}
	if (stop < 0) {
		stop += string->size;
	}
	if (start >= stop) {
		return vm_string_create(0);
	}
	if ((start < 0) || ((uint32_t)start > string->size)) {
		vm_exception_oob((stop < 0) ? stop : (int32_t)(string->size - 1), string->size);
		return MMID_NULL;
	}
	if ((uint32_t)stop > string->size) {
		vm_exception_oob((int32_t)(string->size - 1), string->size);
		return MMID_NULL;
	}
	uint32_t size = stop - start;
	vm_mmid_t src_id = PTR_TO_MMID(string);
	vm_mmid_t dst_id = vm_string_create(size);
	vm_string_t* src = MMID_TO_PTR(src_id, vm_string_t*);
	vm_string_t* dst = MMID_TO_PTR(dst_id, vm_string_t*);
	memcpy(dst->data, src->data + start, size * sizeof(uint16_t));
	return dst_id;
}

vm_mmid_t vm_string_concat(const vm_string_t* s1, const vm_string_t* s2) {
	vm_mmid_t a = PTR_TO_MMID(s1);
	vm_mmid_t b = PTR_TO_MMID(s2);
	vm_mmid_t id = vm_string_create(s1->size + s2->size);
	s1 = MMID_TO_PTR(a, vm_string_t*);
	s2 = MMID_TO_PTR(b, vm_string_t*);
	vm_string_t* c = MMID_TO_PTR(id, vm_string_t*);
	memcpy(c->data, s1->data, s1->size * 2);
	memcpy(c->data + s1->size, s2->data, s2->size * 2);
	return id;
}

uint32_t vm_string_cmp(const vm_string_t* a, const vm_string_t* b) {
	if (a == b) {
		return 1;
	}
	return (a->size == b->size) && !memcmp(a->data, b->data, a->size * 2);
}

vm_exception_t vm_string_get(const vm_string_t* str, int32_t index, vm_variable_t* value) {
	if (index < 0) {
		index += str->size;
	}
	if ((index < 0) || ((uint32_t)index >= str->size)) {
		vm_exception_oob(index, str->size);
		return VM_OOB_E;
	}
	uint16_t c = str->data[index];
	vm_mmid_t out_id = vm_string_create(1);
	vm_string_t* outstr = MMID_TO_PTR(out_id, vm_string_t*);
	outstr->data[0] = c;
	value[0] = VM_VARIABLE_MMID(VM_STRING_T, out_id);
	return VM_NONE_E;
}

int32_t vm_string_find(const vm_string_t* str, const vm_string_t* needle, int32_t offset) {
	if (offset < 0) {
		offset += str->size;
	}
	if (offset < 0 || (uint32_t)offset >= str->size) {
		vm_exception_oob((offset < 0) ? offset : (int32_t)(str->size - 1), str->size);
		return -2;
	}
	if (needle->size == 0) {
		return -1;
	}
	int32_t cnt = str->size - offset - needle->size;
	while (cnt-- >= 0) {
		if (!memcmp(str->data + offset, needle->data, needle->size * sizeof(uint16_t))) {
			return offset;
		}
		offset += 1;
	}
	return -1;
}

void vm_string_free(vm_string_t* str) {
	vm_memory_free(&vm_mem_string, PTR_TO_MMID(str));
}

vm_mmid_t vm_string_cstr(const char* cstr, uint32_t len) {
	if (len == 0) {
		len = strlen(cstr);
	}
	vm_mmid_t id = vm_string_create(len);
	vm_string_t* str = MMID_TO_PTR(id, vm_string_t*);
	for (uint32_t i = 0; i < len; i++) {
		str->data[i] = cstr[i];
	}
	return id;
}
