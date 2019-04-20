#include <stddef.h>
#include "vm.h"
#include "vm_util.h"

#include "vm_extern.h"

vm_native_t vm_extern_native_resolve(const wstring_t* str) {
	const vm_extern_t* item = vm_extern_native;
	while (item->name != NULL) {
		if (wstrcmp8(str, item->name) == 0) {
			return item->function;
		}
		item += 1;
	}
	return NULL;
}
