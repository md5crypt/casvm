#include <string.h>
#include "vm_util.h"
#include "vm_string.h"

#define THOMBSTONE 0xFFFFFFFF

typedef struct {
	uint32_t size;
	uint32_t used;
	vm_mmid_t data[0];
} vm_hashset_t;

vm_hashset_t* set;
vm_mmid_t setid;

static uint32_t mkhash(vm_string_t* key){
	uint32_t h = 0;
	uint16_t* data = (uint16_t*)key->data;
	for(uint32_t i = 0; i<key->size; i++)
		h = 31 * h + data[i];
	return (h ^ (h >> 20) ^ (h >> 12) ^ (h >> 7) ^ (h >> 4));
}

static uint32_t get(vm_string_t* key){
	uint32_t hash = mkhash(key)&(set->size-1); 
	while(set->data[hash] != MMID_NULL){
		vm_string_t* str = MMID_TO_PTR(set->data[hash],vm_string_t*);
		if(str->size == key->size && !memcmp(str->data,key->data,key->size*2))
			return hash;
		hash = (hash+1)&(set->size-1);
	}
	return hash;
}

static void grow(){
	vm_mmid_t oldid = setid;
	vm_hashset_t* oldset = set;
	vm_stringset_init(set->size << 1);
	for(uint32_t i = 0; i<oldset->size; i++){
		if(oldset->data[i] != MMID_NULL && oldset->data[i] != THOMBSTONE){
			uint32_t key = get(MMID_TO_PTR(oldset->data[i],vm_string_t*));
			set->data[key] = oldset->data[i];
			set->used++;
		}
	}
	vm_memory_free(&vm_mem_level_1, oldid);
}

void vm_stringset_init(uint32_t size){
	setid = vm_memory_allocate(&vm_mem_level_1,size*sizeof(vm_mmid_t)+sizeof(vm_hashset_t));
	set = MMID_TO_PTR(setid, vm_hashset_t*);
	set->size = size;
	set->used = 0;
}

vm_mmid_t vm_string_intern(vm_mmid_t id){
	set = MMID_TO_PTR(setid, vm_hashset_t*);
	uint32_t key = get(MMID_TO_PTR(id, vm_string_t*));
	vm_mmid_t value = set->data[key];
	if(value == MMID_NULL){
		value = vm_string_copy(id,true);
		set->data[key] = value;
		if(set->used++ > (set->size>>1))
			grow();
	}
	vm_dereference(id,VM_STRING_T);
	return value;
}

vm_mmid_t vm_string_create(uint8_t* str, uint32_t len, bool constant){
	vm_mmid_t id = vm_memory_allocate(constant?&vm_mem_level_0:&vm_mem_level_3,len*2+sizeof(vm_string_t));
	vm_string_t* ptr = MMID_TO_PTR(id, vm_string_t*);
	ptr->size = len;
	ptr->rcnt = constant?VM_CONSTANT:0;
	memcpy(ptr->data, str, len*2);
	return id;
}

vm_mmid_t vm_string_copy(vm_mmid_t srcid, bool constant){
	vm_string_t* src = MMID_TO_PTR(srcid, vm_string_t*);
	vm_mmid_t dstid = vm_memory_allocate(constant?&vm_mem_level_0:&vm_mem_level_3,src->size*2+sizeof(vm_string_t));
	vm_string_t* dst = MMID_TO_PTR(dstid, vm_string_t*);
	src = MMID_TO_PTR(srcid, vm_string_t*);
	dst->size = src->size;
	dst->rcnt = constant?VM_CONSTANT:0;
	memcpy(dst->data, src->data, src->size*2);
	return dstid;
}

uint32_t vm_string_cmp(vm_mmid_t a, vm_mmid_t b){
	vm_string_t* s1 = MMID_TO_PTR(a, vm_string_t*);
	vm_string_t* s2 = MMID_TO_PTR(b, vm_string_t*);
	return (s1->size == s2->size && !memcmp(s1->data, s2->data, s1->size*2));
}