import { AsVm, vm_thread_t, vm_mmid_t} from "./asvm"
import * as fs from "fs"
import * as process from "process"


const image = fs.readFileSync('../emscripten/build/asvm.wasm')
let test_lib_try = 0

AsVm.create(image).then((vm) => {
	vm.addFunction('__tryStart', (_top, argc) => {
		if (argc != 0) {
			vm.$._vm_exception_arity(argc, 1)
			return AsVm.Exception.ARITY
		}
		test_lib_try += 1
		return AsVm.Exception.NONE
	})

	vm.addFunction('__tryEnd', (_top, argc) => {
		if (argc != 0) {
			vm.$._vm_exception_arity(argc, 1)
			return AsVm.Exception.ARITY
		}
		test_lib_try -= 1
		return AsVm.Exception.NONE
	})

	vm.addFunction('__dbgbrk', (_top, _args) => {
		throw new Error("breakpoint")
	})

	vm.addFunction('__print', (top, argc) => {
		if (argc != 1) {
			vm.$._vm_exception_arity(argc, 1)
			return AsVm.Exception.ARITY
		}
		if (vm.getArgType(top, 1) != AsVm.Type.STRING) {
			return AsVm.Exception.TYPE
		}
		process.stdout.write(vm.readVmString(vm.getArgValue(top, 1) as vm_mmid_t))
		return AsVm.Exception.NONE
	})

	vm.vmInit(fs.readFileSync('../test/__output/image.bin'))
	const time = Date.now()
	while (true) {
		const exception = vm.$._vm_run()
		if (exception != AsVm.Exception.NONE) {
			if (test_lib_try == 0) {
				console.log(vm.getExceptionMessage(exception, true))
				break
			}
			const thread = vm.$._vm_memory_get_ptr(vm.$._vm_fault_get_thread()) as vm_thread_t
			vm.$._vm_thread_kill(thread, vm.createVmString(AsVm.exceptionLut[exception]), AsVm.Type.STRING)
			vm.$._vm_fault_recover()
		} else {
			break
		}
	}
	console.log(Date.now() - time)
}).catch(e => console.error(e))
