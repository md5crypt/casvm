#include "vm.h"
#include "vm_string.h"
#include "vm_conf.h"

vm_memory_t vm_mem_level_0;
vm_memory_t vm_mem_level_1;
vm_memory_t vm_mem_level_2;
vm_memory_t vm_mem_level_3; 

void vm_init(){
	vm_memmap_init(MEMMAP_SIZE, MEMMAP_STACK_SIZE);
	vm_memory_init(&vm_mem_level_0, MEMORY_L0_SIZE);
	vm_memory_init(&vm_mem_level_1, MEMORY_L1_SIZE);
	vm_memory_init(&vm_mem_level_2, MEMORY_L2_SIZE);
	vm_memory_init(&vm_mem_level_3, MEMORY_L3_SIZE);
	vm_stringset_init(STRINGSET_SIZE);
}
