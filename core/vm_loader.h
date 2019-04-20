#pragma once

#include <stdint.h>

typedef enum {
	VM_LOADER_ERROR_NONE,
	VM_LOADER_ERROR_MAGICDWORD,
	VM_LOADER_ERROR_SECTION,
	VM_LOADER_ERROR_EXTERN
} vm_loader_error_t;

vm_loader_error_t vm_loader_load(const uint8_t* data, uint32_t size);
const void* vm_loader_get_error_data(void);
