const fs = require("fs")
const AsVm = require("../emscripten/build/asvm").AsVm

const image = fs.readFileSync('../emscripten/build/asvm.wasm')

let test_lib_try = 0

AsVm.create(image).then((vm) => {
	vm.addFunction('__tryStart', (_top, _args) => {
		test_lib_try += 1
		return 0
	})

	vm.addFunction('__tryEnd', (_top, _args) => {
		test_lib_try -= 1
		return 0
	})

	vm.addFunction('__dbgbrk', (_top, _args) => {
		throw new Error("breakpoint")
	})

	vm.addFunction('__print', (top, _args) => {
		console.log(vm.readVmString(vm.$u32[(top / 4) - 3]))
		return 0
	})

	vm.$._vm_init()

	const error = vm.loaderRun(fs.readFileSync('../test/__output/image.bin'))
	if (error != 0) {
		throw new Error(vm.getLoaderErrorMessage(error))
	}

	vm.$._vm_call(0)

	const time = Date.now()

	while (true) {
		const exception = vm.$._vm_run()
		if (exception != 0) {
			if (test_lib_try == 0) {
				console.log(vm.getExceptionMessage(exception, true))
				break
			}
			const thread = vm.$._vm_memory_get_ptr(vm.$._vm_fault_get_thread())
			vm.$._vm_thread_kill(thread, vm.createVmString(AsVm.exceptionLut[exception]), 5)
			vm.$._vm_fault_recover()
		} else {
			break
		}
	}
	console.log(Date.now() - time)
}).catch(e => console.error(e))
