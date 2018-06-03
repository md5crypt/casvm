#include "vm_hashmap.h"

#include <string.h>
#include "vm_array.h"
#include "vm_util.h"

#define THOMBSTONE 0xFFFFFFFF

static vm_hashmap_pair_t* get(vm_hashmap_t* map, vm_mmid_t key){
	uint32_t hash = (key ^ (key >> 20) ^ (key >> 12) ^ (key >> 7) ^ (key >> 4))&(map->size-1); 
	while(map->data[hash].key != MMID_NULL && map->data[hash].key != key)
		hash = (hash+1)&(map->size-1);
	return map->data+hash;
}

static void grow(vm_hashmap_t* map){
	vm_mmid_t id = vm_hashmap_create(
		map->deleted > map->used?map->size:(map->size<<1),
		map->type,
		map->name,
		map->parent,
		(void*)map->code.address
	);
	vm_hashmap_t* newmap = MMID_TO_PTR(id, vm_hashmap_t*);
	for(uint32_t i = 0; i<map->size; i++){
		if(map->data[i].key != MMID_NULL && map->data[i].key != THOMBSTONE){
			vm_hashmap_pair_t* pair = get(newmap, map->data[i].key);
			*pair = map->data[i];
			newmap->used++;
		}
	}
	vm_memory_replace(&vm_mem_hashmap, PTR_TO_MMID(map), id);
}

vm_mmid_t vm_hashmap_create(uint32_t size, vm_type_t type, vm_mmid_t name, vm_mmid_t parent, void* code){
	vm_mmid_t id = vm_memory_allocate(&vm_mem_hashmap,size*sizeof(vm_hashmap_pair_t)+sizeof(vm_hashmap_t));
	vm_hashmap_t* map = MMID_TO_PTR(id,vm_hashmap_t*);
	memset(((uint8_t*)map)+sizeof(vm_hashmap_t),0,size*sizeof(vm_hashmap_pair_t));
	map->size = size;
	map->used = 0;
	map->deleted = 0;
	map->type = type;
	map->name = name;
	map->parent = parent;
	map->code.address = (uint32_t)code;
	return id;
}

void vm_hashmap_set(vm_hashmap_t* map, vm_mmid_t key, vm_variable_t value){
	vm_hashmap_pair_t* pair = get(map,key);
	vm_mmid_t id = pair->key;
	if(id != MMID_NULL && id != THOMBSTONE)
		vm_variable_dereference(pair->data);
	if(value.type == VM_UNDEFINED_T){
		pair->key = THOMBSTONE;
		map->deleted++;
	}else{
		pair->data = value;
		pair->key = key;
	}
	if(id == MMID_NULL && map->used++ > (map->size>>1))
		grow(map);
}

vm_variable_t vm_hashmap_get(vm_hashmap_t* map, vm_mmid_t key){
	vm_hashmap_pair_t* pair = get(map,key);
	if(pair->key == MMID_NULL || pair->key == THOMBSTONE)
		return (vm_variable_t){.type=VM_UNDEFINED_T};
	vm_variable_reference(pair->data);
	return pair->data;
}

vm_mmid_t vm_hashmap_keys(vm_hashmap_t* map){
	vm_mmid_t mapid = PTR_TO_MMID(map);
	vm_mmid_t arrayid = vm_array_create(map->used-map->deleted);
	map = MMID_TO_PTR(mapid,vm_hashmap_t*);
	vm_array_t* array = MMID_TO_PTR(arrayid,vm_array_t*);
	uint32_t offset = 0;
	for(uint32_t i=0; i<map->size; i++){
		if(map->data[i].key != MMID_NULL && map->data[i].key != THOMBSTONE)
			array->data[offset++] = (vm_variable_t){.type=VM_STRING_T, .data.m=map->data[i].key};
	}
	return arrayid;
}

vm_mmid_t vm_hashmap_values(vm_hashmap_t* map){
	vm_mmid_t mapid = PTR_TO_MMID(map);
	vm_mmid_t arrayid = vm_array_create(map->used-map->deleted);
	map = MMID_TO_PTR(mapid,vm_hashmap_t*);
	vm_array_t* array = MMID_TO_PTR(arrayid,vm_array_t*);
	uint32_t offset = 0;
	for(uint32_t i=0; i<map->size; i++){
		if(map->data[i].key != MMID_NULL && map->data[i].key != THOMBSTONE){
			vm_variable_reference(map->data[i].data);
			array->data[offset++] = map->data[i].data;
		}
	}
	return arrayid;
}