#include "vm_hashmap.h"

#include <string.h>
#include "vm_array.h"
#include "vm_util.h"

#define TOMBSTONE 0xFFFFFFFF

static vm_hashmap_pair_t* get(vm_hashmap_t* map, vm_mmid_t key) {
	uint32_t hash = (key ^ (key >> 20) ^ (key >> 12) ^ (key >> 7) ^ (key >> 4))&(map->size - 1);
	while ((map->data[hash].key != MMID_NULL) && (map->data[hash].key != key)) {
		hash = (hash + 1) & (map->size - 1);
	}
	return map->data + hash;
}

static inline const vm_hashmap_pair_t* get_const(const vm_hashmap_t* map, vm_mmid_t key) {
	return get((vm_hashmap_t*)map, key);
}

static void grow(vm_hashmap_t* map) {
	vm_mmid_t old_id = PTR_TO_MMID(map);
	vm_mmid_t new_id = vm_hashmap_create(
		(map->deleted > (map->used>>1))? map->size : (map->size<<1),
		map->type,
		map->name,
		map->parent,
		(void*)map->code.address
	);
	map = MMID_TO_PTR(old_id, vm_hashmap_t*);
	vm_hashmap_t* newmap = MMID_TO_PTR(new_id, vm_hashmap_t*);
	for (uint32_t i = 0; i < map->size; i++) {
		if (map->data[i].key != MMID_NULL && map->data[i].key != TOMBSTONE) {
			vm_hashmap_pair_t* pair = get(newmap, map->data[i].key);
			*pair = map->data[i];
			newmap->used++;
		}
	}
	vm_memory_replace(&vm_mem_hashmap, old_id, new_id);
}

vm_mmid_t vm_hashmap_create(uint32_t size, vm_type_t type, vm_mmid_t name, vm_mmid_t parent, void* code) {
	vm_mmid_t id = vm_memory_allocate(
		&vm_mem_hashmap,
		(size * sizeof(vm_hashmap_pair_t)) + sizeof(vm_hashmap_t)
	);
	vm_hashmap_t* map = MMID_TO_PTR(id, vm_hashmap_t*);
	memset(
		((uint8_t*)map) + sizeof(vm_hashmap_t), 0,
		size * sizeof(vm_hashmap_pair_t)
	);
	map->size = size;
	map->used = 0;
	map->deleted = 0;
	map->type = type;
	map->name = name;
	map->parent = parent;
	map->code.address = (uint32_t)code;
	return id;
}

void vm_hashmap_set(vm_hashmap_t* map, vm_mmid_t key, vm_variable_data_t value, vm_type_t type) {
	vm_hashmap_pair_t* pair = get(map, key);
	vm_mmid_t id = pair->key;
	if ((id != MMID_NULL) && (id != TOMBSTONE)) {
		vm_variable_dereference(pair->data);
	}
	if (type == VM_UNDEFINED_T) {
		pair->key = TOMBSTONE;
		map->deleted++;
	} else {
		pair->data = VM_VARIABLE(type, value);
		pair->key = key;
	}
	if (id == MMID_NULL) {
		map->used += 1;
		if (map->used >= (map->size>>1)) {
			grow(map);
		}
	}
}

void vm_hashmap_get(const vm_hashmap_t* map, vm_mmid_t key, vm_variable_t* value) {
	const vm_hashmap_pair_t* pair = get_const(map, key);
	if ((pair->key == MMID_NULL) || (pair->key == TOMBSTONE)) {
		value[0] = VM_VARIABLE_OFTYPE(VM_UNDEFINED_T);
	} else {
		vm_variable_reference(pair->data);
		value[0] = pair->data;
	}
}

bool vm_hashmap_has(const vm_hashmap_t* map, vm_mmid_t key) {
	const vm_hashmap_pair_t* pair = get_const(map, key);
	if ((pair->key == MMID_NULL) || (pair->key == TOMBSTONE)) {
		return false;
	}
	return true;
}

vm_mmid_t vm_hashmap_keys(const vm_hashmap_t* map) {
	vm_mmid_t map_id = PTR_TO_MMID(map);
	vm_mmid_t array_id = vm_array_create(map->used - map->deleted);
	map = MMID_TO_PTR(map_id, vm_hashmap_t*);
	vm_array_t* array = MMID_TO_PTR(array_id, vm_array_t*);
	uint32_t offset = 0;
	for (uint32_t i = 0; i < map->size; i++) {
		if ((map->data[i].key != MMID_NULL) && (map->data[i].key != TOMBSTONE)) {
			array->data[offset++] = (vm_variable_t) {.type = VM_STRING_T, .data.m = map->data[i].key};
		}
	}
	return array_id;
}

vm_mmid_t vm_hashmap_values(const vm_hashmap_t* map) {
	vm_mmid_t map_id = PTR_TO_MMID(map);
	vm_mmid_t array_id = vm_array_create(map->used - map->deleted);
	map = MMID_TO_PTR(map_id, vm_hashmap_t*);
	vm_array_t* array = MMID_TO_PTR(array_id, vm_array_t*);
	uint32_t offset = 0;
	for (uint32_t i = 0; i < map->size; i++) {
		if (map->data[i].key != MMID_NULL && map->data[i].key != TOMBSTONE) {
			vm_variable_reference(map->data[i].data);
			array->data[offset++] = map->data[i].data;
		}
	}
	return array_id;
}
