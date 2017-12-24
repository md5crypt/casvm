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

static uint32_t mkhash(vm_string_t* key){
	uint32_t h = 0;
	uint16_t* data = key->data;
	for(uint32_t i = 0; i<key->size; i++)
		h = 31 * h + data[i];
	return (h ^ (h >> 20) ^ (h >> 12) ^ (h >> 7) ^ (h >> 4));
}

static uint32_t get(vm_string_t* key){
	uint32_t hash = mkhash(key)&(set->size-1); 
	while(set->data[hash] != MMID_NULL){
		vm_string_t* str = MMID_TO_PTR(set->data[hash],vm_string_t*);
		if(str->size == key->size && !memcmp(str->data,key->data,key->size*sizeof(uint16_t)))
			return hash;
		hash = (hash+1)&(set->size-1);
	}
	return hash;
}

static void grow(){
	vm_hashset_t* oldset = set;
	vm_stringset_init(set->size << 1);
	for(uint32_t i = 0; i<oldset->size; i++){
		if(oldset->data[i] != MMID_NULL){
			uint32_t key = get(MMID_TO_PTR(oldset->data[i],vm_string_t*));
			set->data[key] = oldset->data[i];
			set->used++;
		}
	}
	free(oldset);
}

void vm_stringset_init(uint32_t size){
	set = (vm_hashset_t*)malloc(size*sizeof(vm_mmid_t)+sizeof(vm_hashset_t));
	memset(((uint8_t*)set)+sizeof(vm_hashset_t),0,size*sizeof(vm_mmid_t));
	set->size = size;
	set->used = 0;
}

vm_mmid_t vm_string_intern_m(vm_mmid_t id){
	vm_string_t* str = MMID_TO_PTR(id, vm_string_t*);
	if(str->rcnt == VM_CONSTANT)
		return id;
	uint32_t key = get(str);
	vm_mmid_t value = set->data[key];
	if(value == MMID_NULL){
		value = vm_string_copy(str,true);
		set->data[key] = value;
		if(set->used++ > (set->size>>1))
			grow();
	}
	vm_dereference_m(id,VM_STRING_T);
	return value;
}

vm_mmid_t vm_string_create(uint32_t len){
	vm_mmid_t id = vm_memory_allocate(&vm_mem_string,len*sizeof(uint16_t)+sizeof(vm_string_t));
	vm_string_t* ptr = MMID_TO_PTR(id, vm_string_t*);
	ptr->size = len;
	ptr->rcnt = 1;
	return id;
}

vm_mmid_t vm_string_insert(const uint16_t* data, uint32_t len){
	vm_mmid_t id = vm_memory_allocate(&vm_mem_const,len*sizeof(uint16_t)+sizeof(vm_string_t));
	vm_string_t* ptr = MMID_TO_PTR(id, vm_string_t*);
	ptr->size = len;
	ptr->rcnt = VM_CONSTANT;
	memcpy(ptr->data, data, len*2);
	set->data[get(MMID_TO_PTR(id, vm_string_t*))] = id;
	if(set->used++ > (set->size>>1))
		grow();
	return id;
}

vm_mmid_t vm_string_copy(vm_string_t* string, bool constant){
	vm_mmid_t srcid = PTR_TO_MMID(string);
	vm_mmid_t dstid = vm_memory_allocate(constant?&vm_mem_const:&vm_mem_string,string->size*sizeof(uint16_t)+sizeof(vm_string_t));
	vm_string_t* dst = MMID_TO_PTR(dstid, vm_string_t*);
	vm_string_t* src = MMID_TO_PTR(srcid, vm_string_t*);
	dst->size = src->size;
	dst->rcnt = constant?VM_CONSTANT:1;
	memcpy(dst->data,src->data,src->size*sizeof(uint16_t));
	return dstid;
}

vm_mmid_t vm_string_slice(vm_string_t* string, int32_t start, int32_t stop){
	if(start < 0)
		start += string->size;
	if(stop < 0)
		stop += string->size;
	if((uint32_t)start >= string->size || start < 0 || (uint32_t)stop > string->size || stop < 0 || start > stop)
		return MMID_NULL;
	uint32_t size = stop-start;
	vm_mmid_t srcid = PTR_TO_MMID(string);
	vm_mmid_t dstid = vm_string_create(size);
	vm_string_t* src = MMID_TO_PTR(srcid, vm_string_t*);
	vm_string_t* dst = MMID_TO_PTR(dstid, vm_string_t*);
	memcpy(dst->data,src->data+start*sizeof(uint16_t),size*sizeof(uint16_t));
	return dstid;
}

vm_mmid_t vm_string_concat_m(vm_mmid_t a, vm_mmid_t b){
	vm_string_t* s1 = MMID_TO_PTR(a, vm_string_t*);
	vm_string_t* s2 = MMID_TO_PTR(b, vm_string_t*);
	vm_mmid_t id = vm_string_create(s1->size+s2->size);
	s1 = MMID_TO_PTR(a, vm_string_t*);
	s2 = MMID_TO_PTR(b, vm_string_t*);
	vm_string_t* c = MMID_TO_PTR(id, vm_string_t*);
	memcpy(c->data, s1->data, s1->size*2);
	memcpy(c->data+s1->size, s2->data, s2->size*2);
	return id;
}

uint32_t vm_string_cmp(vm_string_t* a, vm_string_t* b){
	if(a == b)
		return 1;
	return (a->size == b->size && !memcmp(a->data, b->data, a->size*2));
}

uint32_t vm_string_cmp_m(vm_mmid_t a, vm_mmid_t b){
	if(a == b)
		return 1;
	vm_string_t* s1 = MMID_TO_PTR(a, vm_string_t*);
	vm_string_t* s2 = MMID_TO_PTR(b, vm_string_t*);
	return (s1->size == s2->size && !memcmp(s1->data, s2->data, s1->size*2));
}

vm_variable_t vm_string_get(vm_string_t* str, int32_t index){
	if(index < 0)
		index += str->size;
	if(index < 0 || (uint32_t)index >= str->size)
		return (vm_variable_t){.type=VM_INVALID_T};
	uint16_t c = str->data[index];
	vm_mmid_t outid = vm_string_create(1);
	vm_string_t* outstr = MMID_TO_PTR(outid, vm_string_t*);
	outstr->data[0] = c;
	return (vm_variable_t){.type=VM_STRING_T,.data.m=outid};
}

void vm_string_free(vm_string_t* str){
	vm_memory_free(&vm_mem_string, PTR_TO_MMID(str));
}