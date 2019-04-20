import { TextDecoder } from "util"

const enum Config {
	TABLE_BASE = 0,
	TABLE_SIZE = 1024,
	MEMORY_BASE = 1024,
	MEMORY_SIZE_INITIAL = 1024 * 1024 * 8,
	MEMORY_SIZE_MAX = 1024 * 1024 * 128,
	STACK_SIZE = 1024 * 1024 * 2
}

type c_ptr = number
type c_int32 = number
type c_uint32 = number
type c_float = number

type AsVmExtern = (top: c_ptr, argc: c_uint32) => AsVm.Exception

interface WAsmEnvBase {
	memory: WebAssembly.Memory
	table: WebAssembly.Table
	_sbrk: (delta: c_uint32) => c_uint32
	__memory_base: c_uint32
	__table_base: c_uint32
}

interface AsVmEnv extends WAsmEnvBase {
	_emscripten_memcpy_big: (dest: c_ptr, src: c_ptr, num: c_uint32) => c_ptr
	_lib_float2str: (value: c_float) => c_uint32
	_lib_int2str: (value: c_int32) => c_uint32
	_vm_extern_call: (id: c_uint32, top: c_ptr, argc: c_uint32) => AsVm.Exception
	_vm_extern_resolve: (str: c_ptr) => c_uint32
	// ___assert_fail: (assertion: c_ptr, file: c_ptr, line: c_uint32, func: c_ptr) => void
}

interface EMScriptenMetadata {
	vesrion: [number, number]
	abiVersion: [number, number]
	memSize: number
	tableSize: number
}

interface AsVmExports {
	_free: (ptr: c_ptr) => void
	_malloc: (size: c_uint32) => c_ptr
	_memcpy: (dest: c_ptr, src: c_ptr, num: c_uint32) => c_ptr
	_memset: (ptr: c_ptr, value: c_uint32, num: c_uint32) => c_ptr
	_vm_init: () => void
	_vm_run: () => AsVm.Exception
	_vm_call: (mmid: c_ptr) => void
	_vm_array_create: (len: c_uint32) => c_uint32
	_vm_array_fill: (array: c_ptr, value: c_uint32, type: AsVm.Type, offset: c_int32, len: c_int32) => AsVm.Exception
	_vm_array_find: (array: c_ptr, value: c_uint32, type: AsVm.Type, offset: c_int32) => AsVm.Exception
	_vm_array_get: (array: c_ptr, pos: c_int32, value: c_ptr) => AsVm.Exception
	_vm_array_pop: (array: c_ptr, value: c_ptr) => AsVm.Exception
	_vm_array_push: (array: c_ptr, value: c_uint32, type: AsVm.Type) => c_ptr
	_vm_array_resize: (array: c_ptr, size: c_uint32) => c_ptr
	_vm_array_reverse: (array: c_ptr) => void
	_vm_array_set: (array: c_ptr, pos: c_int32, value: c_uint32, type: AsVm.Type) => AsVm.Exception
	_vm_array_shift: (array: c_ptr, value: c_ptr) => AsVm.Exception
	_vm_array_slice: (array: c_ptr, start: c_int32, end: c_int32) => c_uint32
	_vm_array_unshift: (array: c_ptr, value: c_uint32, type: AsVm.Type) => c_ptr
	_vm_array_write: (dst: c_ptr, src: c_ptr, offset: c_int32, len: c_int32) => AsVm.Exception
	_vm_reference: (value: c_uint32, type: AsVm.Type) => void
	_vm_dereference: (value: c_uint32, type: AsVm.Type) => void
	_vm_exception_data_get: () => c_ptr
	_vm_exception_arity: (actual: c_uint32, expected: c_uint32) => void
	_vm_exception_oob: (index: c_uint32, size: c_uint32) => void
	_vm_exception_type: (actual: AsVm.Type, expected: AsVm.Type) => void
	_vm_exception_user: (wstring: c_ptr) => void
	_vm_fault_get_thread: () => c_uint32
	_vm_fault_recover: () => void
	_vm_fault_trace: (loc: c_ptr) => boolean
	_vm_hashmap_get: (hashmap: c_ptr, key: c_uint32, value: c_ptr) => void
	_vm_hashmap_has: (hashmap: c_ptr, key: c_uint32) => boolean
	_vm_hashmap_set: (hashmap: c_ptr, key: c_uint32, value: c_uint32, type: AsVm.Type) => void
	_vm_hashmap_keys: (hashmap: c_ptr) => c_uint32
	_vm_hashmap_values: (hashmap: c_ptr) => c_uint32
	_vm_loader_get_error_data: () => c_ptr
	_vm_loader_load: (data: c_ptr, size: c_uint32) => AsVm.LoaderError
	_vm_memory_get_mmid: (ptr: c_ptr) => c_uint32
	_vm_memory_get_ptr: (mmid: c_uint32) => c_ptr
	_vm_string_cmp: (a: c_ptr, b: c_ptr) => boolean
	_vm_string_concat: (a: c_ptr, b: c_ptr) => c_uint32
	_vm_string_copy: (str: c_ptr, constant: boolean) => c_uint32
	_vm_string_create: (len: c_uint32) => c_uint32
	_vm_string_cstr: (cstr: c_ptr, len: c_uint32) => c_uint32
	_vm_string_find: (str: c_ptr, needle: c_ptr, offset: c_uint32) => c_int32
	_vm_string_get: (str: c_ptr, pos: c_int32, value: c_ptr) => AsVm.Exception
	_vm_string_intern: (str: c_ptr) => c_uint32
	_vm_string_slice: (str: c_ptr, start: c_int32, end: c_int32) => c_uint32
	_vm_thread_kill: (thread: c_ptr, value: c_uint32, type: AsVm.Type) => void
	_vm_thread_push: (thread: c_ptr) => void
}

export class AsVm {
	public $!: AsVmExports
	public $u32!: Uint32Array
	public $32!: Int32Array
	public $u16!: Uint16Array
	public $16!: Int16Array
	public $u8!: Uint8Array
	public $8!: Int8Array
	public metadata!: EMScriptenMetadata

	private static asciiDecoder = new TextDecoder('utf-8')
	private static utf16Decoder = new TextDecoder('utf-16le')

	private externTable: AsVmExtern[]
	private externMap: Map<string, number>
	private memory: WebAssembly.Memory
	private table: WebAssembly.Table
	private heapTop: number
	private heapEnd: number
	private vStack: number[]
	private vStackTop: number

	private constructor() {
		this.memory = new WebAssembly.Memory({
			'initial': Config.MEMORY_SIZE_INITIAL >> 16,
			'maximum': Config.MEMORY_SIZE_MAX >> 16,
		})
		this.table = new WebAssembly.Table({
			'initial': Config.TABLE_SIZE,
			'maximum': Config.TABLE_SIZE,
			'element': 'anyfunc'
		})
		this.externTable = []
		this.externMap = new Map()
		this.heapTop = 0
		this.heapEnd = 0
		this.vStack = []
		this.vStackTop = 0
	}

	private async loadWasm(image: ArrayBuffer): Promise<AsVm> {
		const env: AsVmEnv = {
			memory: this.memory,
			table: this.table,
			__memory_base: Config.MEMORY_BASE,
			__table_base: Config.TABLE_BASE,
			_emscripten_memcpy_big: (dest, src, num) => (this.$u8.set(this.$u8.subarray(src, src + num), dest), dest),
			_sbrk: (delta) => this.sbrk(delta),
			_lib_float2str: (value) => this.createVmString((Math.round(value * 10000000) / 10000000).toString()),
			_lib_int2str: (value) => this.createVmString(value.toString()),
			_vm_extern_call: (id, top, argc) => this.externTable[id](top, argc),
			_vm_extern_resolve: (str) => {
				const id = this.externMap.get(this.readWideString(str))
				return (id === undefined) ? 0xFFFFFFFF : id
			},
			// ___assert_fail: (assertion, file, line, func) => console.log(`${this.readCString(assertion)} (in ${this.readCString(file)}:${this.readCString(func)}:${line}`)
		}
		return WebAssembly.instantiate(image, {env}).then((o) => {
			this.$ = <AsVmExports>o.instance.exports
			this.metadata = AsVm.readMetadata(o.module)
			this.heapTop = this.metadata.memSize + Config.MEMORY_BASE + Config.STACK_SIZE
			this.heapEnd = Config.MEMORY_SIZE_INITIAL
			this.vStackTop = this.heapTop
			this.updateHeapViews()
			return this
		})
	}

	public vStackPush(amount: number): c_ptr {
		this.vStack.push(this.vStackTop)
		this.vStackTop -= amount
		return this.vStackTop
	}

	public vStackPop(): void {
		if (this.vStack.length == 0) {
			throw new Error('VStack is empty!')
		}
		this.vStackTop = this.vStack.pop()!
	}

	public static async create(image: ArrayBuffer): Promise<AsVm> {
		const vm = new AsVm()
		return vm.loadWasm(image)
	}

	public readWideString(ptr: c_ptr): string {
		return AsVm.utf16Decoder.decode(this.$u8.subarray(ptr + 4, ptr + 4 + (this.$u32[ptr / 4] * 2)))
	}

	public readCString(ptr: c_ptr): string {
		return AsVm.asciiDecoder.decode(this.$u8.subarray(ptr, this.$u8.indexOf(0, ptr)))
	}

	public writeAsciiString(ptr: c_ptr, str: string, noNull?: boolean): number {
		for (let i = 0; i < str.length; i++) {
			this.$u8[ptr + i] = str.charCodeAt(i)
		}
		if (!noNull) {
			this.$u8[ptr + str.length] = 0
			return str.length + 1
		}
		return str.length
	}

	public createVmString(str: string): c_uint32 {
		const mmid = this.$._vm_string_create(str.length)
		const base = (this.$._vm_memory_get_ptr(mmid) + 8) / 2
		for (let i = 0; i < str.length; i++) {
			this.$u16[base + i] = str.charCodeAt(i)
		}
		return mmid
	}

	public readVmString(mmid: c_uint32): string {
		return this.readWideString(this.$._vm_memory_get_ptr(mmid) + 4)
	}

	public getExceptionMessage(exception: AsVm.Exception, trace?: boolean): string {
		const ptr = this.$._vm_exception_data_get()
		const f1 = this.$u32[ptr / 4]
		const f2 = this.$u32[(ptr / 4) + 1]
		let msg = `Exception ${AsVm.exceptionLut[exception]}`
		switch (exception) {
			case AsVm.Exception.USER:
				if (f1) {
					msg += `: ${this.readWideString(f1)}`
				}
				break
			case AsVm.Exception.OOB:
				msg += `: accessed element ${f1} out of ${f2}`
				break
			case AsVm.Exception.ARITY:
				msg += `: passed ${f1} arguments, expected ${f2}`
				break
			case  AsVm.Exception.TYPE:
				msg += `: got "${AsVm.typeLut[f1]}", expected "${AsVm.typeLut[f2]}"`
				break
		}
		if (trace) {
			msg += '\n' + this.readTrace().map((loc) => `  at ${loc.func}:${loc.line} (${loc.file || 'unknown'}) [pc:${loc.pc}]`).join('\n')
		}
		return msg
	}

	public readTrace(): AsVm.Trace[] {
		const trace: AsVm.Trace[] = []
		const loc = this.vStackPush(24)
		while (this.$._vm_fault_trace(loc)) {
			const file = this.$u32[(loc / 4) + 2]
			const func = this.$u32[(loc / 4) + 3]
			const o: AsVm.Trace = {
				pc: this.$u32[(loc / 4) + 0],
				line: this.$u32[(loc / 4) + 1]
			}
			if (file) {
				o.file = this.readWideString(file)
			}
			if (func) {
				o.func = this.readWideString(func)
			}
			trace.push(o)
		}
		this.vStackPop()
		return trace
	}

	public addFunction(name: string, func: AsVmExtern): void {
		this.externMap.set(name, this.externTable.length)
		this.externTable.push(func)
	}

	public loaderRun(data: Uint8Array): AsVm.LoaderError {
		const ptr = this.$._malloc(data.length)
		this.$u8.set(data, ptr)
		return this.$._vm_loader_load(ptr, data.length)
	}

	public getLoaderErrorMessage(error: AsVm.LoaderError): string {
		switch (error) {
			case AsVm.LoaderError.NONE:
				return 'no error'
			case AsVm.LoaderError.MAGICDWORD:
				return "invalid magic number"
			case AsVm.LoaderError.SECTION:
				return `unknown section: ${this.readCString(this.$._vm_loader_get_error_data())}`
			case AsVm.LoaderError.EXTERN:
				return `unresolved extern: ${this.readWideString(this.$._vm_loader_get_error_data())}`
			default:
				return 'unkown error'
		}
	}

	private sbrk(delta: c_uint32): c_ptr {
		const top = this.heapTop
		this.heapTop += delta
		if (this.heapEnd < this.heapTop) {
			const amount = ((this.heapTop - this.heapEnd) >> 16) + 1
			this.memory.grow(amount)
			this.heapEnd += (amount << 16)
			this.updateHeapViews()
		}
		return top
	}

	private updateHeapViews(): void {
		this.$u32 = new Uint32Array(this.memory.buffer)
		this.$32 = new Int32Array(this.memory.buffer)
		this.$u16 = new Uint16Array(this.memory.buffer)
		this.$16 = new Int16Array(this.memory.buffer)
		this.$u8 = new Uint8Array(this.memory.buffer)
		this.$8 = new Int8Array(this.memory.buffer)
	}

	private static readMetadata(module: WebAssembly.Module): EMScriptenMetadata {
		const sections = WebAssembly.Module.customSections(module, 'emscripten_metadata')
		if (sections.length != 1) {
			throw new Error('no metadata found')
		}
		const values = AsVm.delebify(new Uint8Array(sections[0]))
		return {
			vesrion: [values[0], values[1]],
			abiVersion: [values[2], values[3]],
			memSize: values[4],
			tableSize: values[5],
		}
	}

	private static delebify(buffer: Uint8Array): number[] {
		const output: number[] = []
		let accumulator = 0
		let shift = 0
		for (let i=0; i < buffer.length; i++) {
			const byte = buffer[i]
			if (!(byte & 0x80)) {
				output.push(accumulator | (byte << shift))
				accumulator = 0
				shift = 0
			} else {
				accumulator |= ((byte&0x7F) << shift)
				shift += 7
			}
		}
		return output
	}
}

export namespace AsVm {
	export const enum Exception {
		NONE,
		YIELD,
		USER,
		OOB,
		TYPE,
		ARITY,
		IMMUTABLE,
		DIV,
		INTERNAL
	}

	export const exceptionLut = [
		"NONE_",
		"YIELD",
		"USER",
		"OUT-OF-BOUNDS",
		"TYPE",
		"ARITY",
		"IMMUTABLE",
		"DIV0",
		"INTERNAL"
	]

	export const enum LoaderError {
		NONE,
		MAGICDWORD,
		SECTION,
		EXTERN
	}

	export const loaderErrorLut = [
		"NONE",
		"MAGICDWORD",
		"SECTION",
		"EXTERN"
	]

	export const threadStateLut = [
		"PAUSED",
		"FINISHED"
	]

	export const enum ThreadState {
		PAUSED,
		FINISHED
	}

	export const enum Type {
		INVALID,
		UNDEFINED,
		BOOLEAN,
		INTEGER,
		FLOAT,
		STRING,
		ARRAY,
		CALLABLE,
		FUNCTION,
		EXTERN,
		NATIVE,
		NAMESPACE,
		FRAME,
		VECTOR,
		HASHMAP,
		NUMERIC,
		INDEXABLE,
		THREAD
	}

	export const typeLut = [
		"INVALID",
		"UNDEFINED",
		"BOOLEAN",
		"INTEGER",
		"FLOAT",
		"STRING",
		"ARRAY",
		"CALLABLE",
		"FUNCTION",
		"EXTERN",
		"NATIVE",
		"NAMESPACE",
		"FRAME",
		"VECTOR",
		"HASHMAP",
		"NUMERIC",
		"INDEXABLE",
		"THREAD"
	]

	export interface Trace {
		pc: number
		line: number
		file?: string
		func?: string
	}
}
