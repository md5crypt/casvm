#pragma once
#include <inttypes.h>

#define MMID_NULL ((vm_mmid_t)0)
#define MMID_TO_PTR(a,b) ((b)vm_memmap.bottom[a])
#define PTR_TO_MMID(a) (((vm_memblock_t*)((uint8_t*)a-sizeof(vm_memblock_t)))->id)

typedef uint32_t vm_mmid_t;

typedef struct {
	uint32_t size;
	uint32_t used;
	void** top;
	void** bottom;
	struct {
		uint32_t size;
		uint32_t used;
		vm_mmid_t* top;
		vm_mmid_t* bottom;
	} stack;
} vm_memmap_t;

typedef struct {
	uint32_t size;
	vm_mmid_t id;
	uint8_t data[0];
} vm_memblock_t;

typedef struct {
	uint32_t size;
	uint32_t used;
	uint32_t free;
	uint32_t available;
	void* top;
	void* bottom;
} vm_memory_t;

extern vm_memmap_t vm_memmap;

/*
 * initializes the memory system with
 * given initial sizes
 */
void vm_memmap_init(uint32_t mapsize, uint32_t stacksize);

/*
 * creates a GCed memory space with
 * a given initial size
 */
void vm_memory_init(vm_memory_t* mem, uint32_t size);

/*
 * sets mmid offset
 */
void vm_memmap_set_offset(uint32_t offset);

/*
 * allocates a block of memory in a given
 * memory space
 */
vm_mmid_t vm_memory_allocate(vm_memory_t* mem, uint32_t size);

/*
 * replaces dst with src, frees src
 */
void vm_memory_replace(vm_memory_t* mem, vm_mmid_t dst, vm_mmid_t src);

/*
 * frees a block of memory in a given
 * memory space
 */
void vm_memory_free(vm_memory_t* mem, vm_mmid_t id);
