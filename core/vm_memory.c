#include <stdlib.h>
#include <string.h>

#include "vm_memory.h"
#include "vm_util.h"

vm_memmap_t vm_memmap;

static void memmap_grow(uint32_t size) {
	vm_memmap.bottom = (void**)realloc(vm_memmap.bottom, sizeof(void**) * size);
	vm_memmap.top = vm_memmap.bottom + vm_memmap.size;
	vm_memmap.size = size;
}

static vm_mmid_t memmap_new() {
	if (vm_memmap.stack.used > 0) {
		vm_memmap.stack.used -= 1;
		vm_memmap.stack.top -= 1;
		return vm_memmap.stack.top[0];
	}
	if (vm_memmap.used == vm_memmap.size) {
		memmap_grow(vm_memmap.size * 2);
	}
	return vm_memmap.used++;
}

static void memmap_free(vm_mmid_t id) {
	if (vm_memmap.stack.used == vm_memmap.stack.size) {
		vm_memmap.stack.bottom = (vm_mmid_t*)realloc(
			vm_memmap.stack.bottom,
			(2 * sizeof(vm_mmid_t)) * vm_memmap.stack.size
		);
		vm_memmap.stack.top = vm_memmap.stack.bottom + vm_memmap.stack.size;
		vm_memmap.stack.size <<= 1;
	}
	vm_memmap.stack.used += 1;
	*(vm_memmap.stack.top++) = id;
}

static void grow(vm_memory_t* mem, uint32_t newsize) {
	uint8_t* bottom = malloc(newsize);
	uint8_t* top = bottom;
	vm_memblock_t* block = (vm_memblock_t*)mem->bottom;
	vm_memblock_t* end = (vm_memblock_t*)mem->top;
	while (block < end) {
		if (block->id > 0) {
			vm_memmap.bottom[block->id] = ((vm_memblock_t*)top)->data;
			memcpy(top, block, block->size);
			top += block->size;
		}
		block = (vm_memblock_t*)((uint8_t*)block + block->size);
	}
	free(mem->bottom);
	mem->bottom = bottom;
	mem->top = top;
	mem->size = newsize;
	mem->used = top - bottom;
	mem->free = newsize - mem->used;
	mem->available = mem->free;
}

void vm_memmap_init(uint32_t mapsize, uint32_t stacksize) {
	vm_memmap.size = mapsize;
	vm_memmap.used = 1;
	vm_memmap.bottom = (void**)malloc(sizeof(void*) * mapsize);
	vm_memmap.top = vm_memmap.top + 1;
	vm_memmap.stack.size = stacksize;
	vm_memmap.stack.used = 0;
	vm_memmap.stack.top = (vm_mmid_t*)malloc(sizeof(vm_mmid_t) * stacksize);
	vm_memmap.stack.bottom = vm_memmap.stack.top;
}

void vm_memmap_set_offset(uint32_t offset) {
	if (vm_memmap.size < offset) {
		memmap_grow(npot(offset * 2));
	}
	vm_memmap.used = offset;
	vm_memmap.stack.used = 0;
}

void vm_memory_init(vm_memory_t* mem, uint32_t size) {
	mem->top = malloc(size);
	mem->bottom = mem->top;
	mem->size = size;
	mem->used = 0;
	mem->free = size;
	mem->available = size;
}

uint32_t vm_memory_allocate(vm_memory_t* mem, uint32_t size) {
	size += sizeof(vm_memblock_t);
	if (size&3) {
		size += 4 - (size&3);
	}
	if (mem->available < size) {
		uint32_t newsize = mem->size;
		while (newsize < (mem->used + size) * 2) {
			newsize <<= 1;
		}
		grow(mem, newsize);
	}
	vm_memblock_t* block = (vm_memblock_t*)mem->top;
	mem->used += size;
	mem->free -= size;
	mem->available -= size;
	mem->top = (uint8_t*)mem->top + size;
	block->size = size;
	block->id = memmap_new();
	vm_memmap.bottom[block->id] = block->data;
	return block->id;
}

void vm_memory_replace(vm_memory_t* mem, vm_mmid_t dst, vm_mmid_t src) {
	vm_memblock_t* dstblock = (vm_memblock_t*)((uint8_t*)vm_memmap.bottom[dst] - sizeof(vm_memblock_t));
	vm_memblock_t* srcblock = (vm_memblock_t*)((uint8_t*)vm_memmap.bottom[src] - sizeof(vm_memblock_t));
	dstblock->id = 0;
	mem->used -= dstblock->size;
	srcblock->id = dst;
	vm_memmap.bottom[dst] = srcblock->data;
	memmap_free(src);
}

void vm_memory_free(vm_memory_t* mem, vm_mmid_t id) {
	vm_memblock_t* block = (vm_memblock_t*)((uint8_t*)vm_memmap.bottom[id] - sizeof(vm_memblock_t));
	block->id = 0;
	mem->used -= block->size;
	memmap_free(id);
}

vm_mmid_t vm_memory_get_mmid(void* ptr) {
	return ((vm_memblock_t*)((uint8_t*)ptr - sizeof(vm_memblock_t)))->id;
}

void* vm_memory_get_ptr(vm_mmid_t mmid) {
	return vm_memmap.bottom[mmid];
}
