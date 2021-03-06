import * as VmConstants from "../asc/src/vmConstants"

const enum Config {
	MMID_OFFSET = 128,
	TABLE_SIZE = 1024,
	MEMORY_SIZE_INITIAL = 1024 * 1024 * 8,
	MEMORY_SIZE_MAX = 1024 * 1024 * 128
}

export const enum void_ptr_t {}
export const enum vm_mmid_t {}
export const enum vm_variable_t {
	type = 0,
	data = 4,
	__sizeof = 8
}
export const enum vm_thread_t {
	rnct = 0,
	size = 4,
	top = 8,
	state = 12,
	next = 16,
	prev = 20,
	queue = 24,
	stack = 28,
	__sizeof = 32
}
export const enum vm_string_t {
	rcnt = 0,
	size = 4,
	data = 8,
	__sizeof = 12
}
export const enum vm_array_t {
	rcnt = 0,
	size = 4,
	used = 8,
	offset = 12,
	data = 16,
	__sizeof = 20
}
export const enum vm_hashmap_t {
	size = 0,
	used = 4,
	deleted = 8,
	name = 12,
	parent = 16,
	type = 20,
	code = 24,
	dirty = 28,
	data = 32,
	__sizeof = 36
}
export const enum vm_exception_data_t {
	f1 = 0,
	f2 = 4,
	__sizeof = 8
}
export const enum vm_symbols_location_t {
	pc = 0,
	line = 4,
	file = 8,
	function = 12,
	__sizeof = 16
}
export const enum wstring_t {
	size = 0,
	data = 4,
	__sizeof = 8
}
export const enum cstring_t {}
export const enum int32_t {}
export const enum uint32_t {}
export const enum float32_t {}

export type vm_loader_error_t = wstring_t | cstring_t

export type ptr_t = (
	void_ptr_t | vm_variable_t | vm_thread_t | vm_string_t |
	vm_array_t | vm_array_t | vm_hashmap_t | vm_thread_t |
	vm_exception_data_t | vm_symbols_location_t | vm_loader_error_t |
	wstring_t | cstring_t
)

export type vm_variable_data_t = vm_mmid_t | int32_t | uint32_t | float32_t

type AsVmExtern = (top: vm_variable_t, argc: uint32_t) => AsVm.Exception

interface WAsmEnvBase {
	memory: WebAssembly.Memory
	table: WebAssembly.Table
	sbrk: (delta: uint32_t) => void_ptr_t
}

interface AsVmEnv extends WAsmEnvBase {
	emscripten_memcpy_big: (dest: ptr_t, src: ptr_t, num: uint32_t) => ptr_t
	lib_float2str: (value: float32_t) => vm_mmid_t
	lib_int2str: (value: int32_t) => vm_mmid_t
	vm_extern_call: (id: uint32_t, top: vm_variable_t, argc: uint32_t) => AsVm.Exception
	vm_extern_resolve: (str: wstring_t) => uint32_t
}

/*interface EMScriptenMetadata {
	vesrion: [number, number]
	abiVersion: [number, number]
	memSize: number
	tableSize: number
}*/

interface AsVmExports {
	__data_end: number,
	stackSave: () => ptr_t,
	stackAlloc: (amount: uint32_t) => ptr_t,
	stackRestore: (ptr: ptr_t) => void,
	free: (ptr: ptr_t) => void
	malloc: (size: uint32_t) => ptr_t
	memcpy: (dest: ptr_t, src: ptr_t, num: uint32_t) => ptr_t
	memset: (ptr: ptr_t, value: uint32_t, num: uint32_t) => ptr_t
	vm_init: () => void
	vm_run: () => AsVm.Exception
	vm_call: (pc: uint32_t, argc: uint32_t, argv: vm_variable_t) => void
	vm_get_current_thread: () => vm_mmid_t
	vm_array_create: (len: uint32_t) => vm_mmid_t
	vm_array_fill: (array: vm_array_t, value: vm_variable_data_t, type: AsVm.Type, offset: int32_t, len: int32_t) => AsVm.Exception
	vm_array_find: (array: vm_array_t, value: vm_variable_data_t, type: AsVm.Type, offset: int32_t) => AsVm.Exception
	vm_array_get: (array: vm_array_t, pos: int32_t, value: vm_variable_t) => AsVm.Exception
	vm_array_pop: (array: vm_array_t, value: vm_variable_t) => AsVm.Exception
	vm_array_push: (array: vm_array_t, value: vm_variable_data_t, type: AsVm.Type) => vm_array_t
	vm_array_resize: (array: vm_array_t, size: uint32_t) => vm_array_t
	vm_array_reverse: (array: vm_array_t) => void
	vm_array_set: (array: vm_array_t, pos: int32_t, value: vm_variable_data_t, type: AsVm.Type) => AsVm.Exception
	vm_array_shift: (array: vm_array_t, value: vm_variable_t) => AsVm.Exception
	vm_array_slice: (array: vm_array_t, start: int32_t, end: int32_t) => vm_mmid_t
	vm_array_unshift: (array: vm_array_t, value: vm_variable_data_t, type: AsVm.Type) => vm_array_t
	vm_array_write: (dst: vm_array_t, src: vm_array_t, offset: int32_t, len: int32_t) => AsVm.Exception
	vm_reference: (ptr: ptr_t) => void
	vm_reference_m: (mmid: vm_mmid_t) => void
	vm_dereference: (ptr: ptr_t, type: AsVm.Type) => void
	vm_dereference_m: (mmid: vm_mmid_t, type: AsVm.Type) => void
	vm_exception_data_get: () => vm_exception_data_t
	vm_exception_arity: (actual: uint32_t, expected: uint32_t) => void
	vm_exception_oob: (index: int32_t, size: uint32_t) => void
	vm_exception_type: (actual: AsVm.Type, expected: AsVm.Type) => void
	vm_exception_user: (wstring: wstring_t) => void
	vm_fault_get_thread: () => vm_mmid_t
	vm_fault_recover: () => void
	vm_fault_trace: (loc: vm_symbols_location_t) => boolean
	vm_hashmap_get: (hashmap: vm_hashmap_t, key: vm_mmid_t, value: vm_variable_t) => void
	vm_hashmap_has: (hashmap: vm_hashmap_t, key: vm_mmid_t) => boolean
	vm_hashmap_set: (hashmap: vm_hashmap_t, key: vm_mmid_t, value: vm_variable_data_t, type: AsVm.Type) => void
	vm_hashmap_keys: (hashmap: vm_hashmap_t) => vm_mmid_t
	vm_hashmap_values: (hashmap: vm_hashmap_t) => vm_mmid_t
	vm_loader_get_error_data: () => vm_loader_error_t
	vm_loader_load: (data: void_ptr_t, size: uint32_t) => AsVm.LoaderError
	vm_memory_get_mmid: (ptr: ptr_t) => vm_mmid_t
	vm_memory_get_ptr: (mmid: vm_mmid_t) => ptr_t
	vm_string_cmp: (a: vm_string_t, b: vm_string_t) => boolean
	vm_string_concat: (a: vm_string_t, b: vm_string_t) => vm_mmid_t
	vm_string_copy: (str: vm_string_t, constant: boolean) => vm_mmid_t
	vm_string_create: (len: uint32_t) => vm_mmid_t
	vm_string_find: (str: vm_string_t, needle: vm_string_t, offset: int32_t) => int32_t
	vm_string_get: (str: vm_string_t, pos: int32_t, value: vm_variable_data_t) => AsVm.Exception
	vm_string_intern: (str: vm_string_t) => vm_mmid_t
	vm_string_slice: (str: vm_string_t, start: int32_t, end: int32_t) => vm_mmid_t
	vm_thread_kill: (thread: vm_thread_t, value: vm_variable_data_t, type: AsVm.Type) => void
	vm_thread_push: (thread: vm_thread_t) => void
	theta_star: (data: void_ptr_t, start: number, stop: number, opt: number) => void_ptr_t
	find_closest: (data: void_ptr_t, x0: number, y0: number) => number
}

export class AsVm {
	public $!: AsVmExports
	public $u32!: Uint32Array
	public $32!: Int32Array
	public $u16!: Uint16Array
	public $16!: Int16Array
	public $u8!: Uint8Array
	public $8!: Int8Array
	public $f32!: Float32Array

	private static asciiDecoder = new TextDecoder('utf-8')
	private static utf16Decoder = new TextDecoder('utf-16le')
	private static flConverterIn = new Float32Array(1)
	private static flConverterOut = new Uint32Array(AsVm.flConverterIn.buffer)

	private externTable: AsVmExtern[]
	private externMap: Map<string, number>
	private memory: WebAssembly.Memory
	private table: WebAssembly.Table
	private heapTop: number
	private heapEnd: number
	private vStack: number[]
	private internCache: Map<string, vm_mmid_t>

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
		this.internCache = new Map()
	}

	private async loadWasm(image: ArrayBuffer): Promise<AsVm> {
		const env: AsVmEnv = {
			memory: this.memory,
			table: this.table,
			emscripten_memcpy_big: (dest, src, num) => (this.$u8.set(this.$u8.subarray(src, src + num), dest), dest),
			sbrk: (delta) => this.sbrk(delta),
			lib_float2str: (value) => this.createVmString((Math.round(value * 10000000) / 10000000).toString()),
			lib_int2str: (value) => this.createVmString(value.toString()),
			vm_extern_call: (id, top, argc) => this.externTable[id](top, argc),
			vm_extern_resolve: (str) => {
				const id = this.externMap.get(this.readWideString(str))
				return (id === undefined) ? 0xFFFFFFFF : id
			}
		}
		; (env as any).__assert_fail = () => 0
		; (env as any).debug_log = (x: number) => console.log(x)
		; (env as any).debug_logf = (x: number) => console.log(x)
		return WebAssembly.instantiate(image, {env}).then((o) => {
			this.$ = <AsVmExports>o.instance.exports
			this.heapTop = this.$.stackSave()
			this.heapEnd = Config.MEMORY_SIZE_INITIAL
			this.updateHeapViews()
			return this
		})
	}

	public vStackPush(amount: number): ptr_t {
		this.vStack.push(this.$.stackSave())
		return this.$.stackAlloc(amount)
	}

	public vStackPop(): void {
		if (this.vStack.length == 0) {
			throw new Error('VStack is empty!')
		}
		this.$.stackRestore(this.vStack.pop()!)
	}

	public static async create(image: ArrayBuffer): Promise<AsVm> {
		const vm = new AsVm()
		return vm.loadWasm(image)
	}

	public readWideString(ptr: wstring_t): string {
		const size = this.$u32[(ptr + wstring_t.size) / 4] * 2
		const data = ptr + wstring_t.data
		return AsVm.utf16Decoder.decode(this.$u8.subarray(data, data + size))
	}

	public readCString(ptr: cstring_t): string {
		return AsVm.asciiDecoder.decode(this.$u8.subarray(ptr, this.$u8.indexOf(0, ptr)))
	}

	public createVmString(str: string): vm_mmid_t {
		const mmid = this.$.vm_string_create(str.length)
		const base = (this.$.vm_memory_get_ptr(mmid) + vm_string_t.data) / 2
		for (let i = 0; i < str.length; i++) {
			this.$u16[base + i] = str.charCodeAt(i)
		}
		return mmid
	}

	public readVmString(mmid: vm_mmid_t): string {
		return this.readWideString(this.$.vm_memory_get_ptr(mmid) + vm_string_t.size)
	}

	public getExceptionMessage(exception: AsVm.Exception, trace?: boolean): string {
		const ptr = this.$.vm_exception_data_get()
		const f1 = this.$u32[(ptr + vm_exception_data_t.f1) / 4]
		const f2 = this.$u32[(ptr + vm_exception_data_t.f2) / 4]
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
			case AsVm.Exception.TYPE:
				msg += `: got "${AsVm.typeLut[f1]}", expected "${AsVm.typeLut[f2]}"`
				break
		}
		if (trace) {
			msg += '\n' + this.readTrace().map((loc) => loc.func ? `  at ${loc.func}:${loc.line} (${loc.file || 'unknown'}) [pc:${loc.pc}]` : `  at vm-internal [pc:${loc.pc}]`).join('\n')
		}
		return msg
	}

	public readTrace(): AsVm.Trace[] {
		const trace: AsVm.Trace[] = []
		const loc = this.vStackPush(vm_symbols_location_t.__sizeof) as vm_symbols_location_t
		while (this.$.vm_fault_trace(loc)) {
			const file = this.$u32[(loc + vm_symbols_location_t.file) / 4]
			const func = this.$u32[(loc + vm_symbols_location_t.function) / 4]
			const o: AsVm.Trace = {
				pc: this.$u32[(loc + vm_symbols_location_t.pc) / 4],
				line: this.$u32[(loc + vm_symbols_location_t.line) / 4]
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

	public vmInit(data: Uint8Array): void {
		const ptr = this.$.malloc(data.length) as void_ptr_t
		this.$u8.set(data, ptr)
		this.$.vm_init()
		const error = this.$.vm_loader_load(ptr, data.length)
		if (error != AsVm.LoaderError.NONE) {
			throw new Error(this.getLoaderErrorMessage(error))
		}
		this.$.vm_call(0, 0, 0) // set up object tree
		this.vmRun()
		this.$.vm_call(1, 0, 0) // call constructors
	}

	public vmRun(): void {
		const exception = this.$.vm_run()
		if (exception != AsVm.Exception.NONE) {
			throw this.getExceptionMessage(exception, true)
		}
	}

	public vmCall(mmid: vm_mmid_t, ...args: AsVm.Variable[]) {
		const hashmap = this.$.vm_memory_get_ptr(mmid) as vm_hashmap_t
		let ptr: vm_variable_t = 0
		if (args.length) {
			ptr = this.vStackPush(args.length * vm_variable_t.__sizeof) as vm_variable_t
			let offset = ptr
			for (let i = 0; i < args.length; i++) {
				this.$u32[(offset + vm_variable_t.type) / 4] = args[i].type
				this.$u32[(offset + vm_variable_t.data) / 4] = args[i].value
				offset += vm_variable_t.__sizeof
			}
		}
		this.$.vm_call(this.$u32[(hashmap + vm_hashmap_t.code) / 4], args.length, ptr)
		if (ptr) {
			this.vStackPop()
		}
	}

	public getLoaderErrorMessage(error: AsVm.LoaderError): string {
		switch (error) {
			case AsVm.LoaderError.NONE:
				return 'no error'
			case AsVm.LoaderError.MAGICDWORD:
				return "invalid magic number"
			case AsVm.LoaderError.SECTION:
				return `unknown section: ${this.readCString(this.$.vm_loader_get_error_data() as cstring_t)}`
			case AsVm.LoaderError.EXTERN:
				return `unresolved extern: ${this.readWideString(this.$.vm_loader_get_error_data() as wstring_t)}`
			default:
				return 'unkown error'
		}
	}

	private sbrk(delta: uint32_t): void_ptr_t {
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
		this.$f32 = new Float32Array(this.memory.buffer)
	}

	/*private static readMetadata(module: WebAssembly.Module): EMScriptenMetadata {
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
	}*/

	public static isType(child: AsVm.Type, parent: AsVm.Type){
		return AsVm.typeMatrix[AsVm.typeLut.length * child + parent] == 1
	}

	public resolveAndSet(path: string, value: vm_variable_data_t, type: AsVm.Type, context?: vm_mmid_t) {
		const pathList = path.split('.')
		const key = pathList.pop()!
		const variable = this.resolve(pathList, context)
		if (!AsVm.isType(variable.type, AsVm.Type.HASHMAP)) {
			throw new Error(`Failed to set '${path}': target is of type '${AsVm.typeLut[type]}', expected 'hashmap'`)
		}
		this.$.vm_hashmap_set(this.$.vm_memory_get_ptr(variable.value as vm_mmid_t) as vm_hashmap_t, this.intern(key), value, type)
	}

	public intern(str: string): vm_mmid_t {
		const cached = this.internCache.get(str)
		if (cached !== undefined) {
			return cached
		} else {
			const mmid = this.$.vm_string_intern(this.$.vm_memory_get_ptr(this.createVmString(str)) as vm_string_t)
			this.internCache.set(str, mmid)
			return mmid
		}
	}

	public getHashmapPath(mmid: vm_mmid_t): string {
		const stack: string[] = []
		let current = mmid
		while (current != 0) {
			const hashmap = this.$.vm_memory_get_ptr(current) as vm_hashmap_t
			stack.push(this.readVmString(this.$u32[(hashmap + vm_hashmap_t.name) / 4]))
			current = this.$u32[(hashmap + vm_hashmap_t.parent) / 4]
		}
		return stack.reverse().join('.')
	}

	public resolve(path: string | string[], context?: vm_mmid_t): AsVm.Variable {
		let current: vm_mmid_t = Config.MMID_OFFSET as number
		const pathList = path instanceof Array ? path : path.split('.')
		let i = 0
		if (!pathList[0] || (pathList[0] == 'root')) {
			i += 1
		} else if (context !== undefined) {
			current = context
		}
		if (pathList.length == 0) {
			return {
				value: current as number,
				type:this.$u32[(this.$.vm_memory_get_ptr(current) + vm_hashmap_t.parent) / 4]
			}
		}
		const out = this.vStackPush(vm_variable_t.__sizeof) as vm_variable_t
		while (true) {
			const key = pathList[i]
			const hashmap = this.$.vm_memory_get_ptr(current) as vm_hashmap_t
			let type: AsVm.Type
			if (key == 'parent') {
				current = this.$u32[(hashmap + vm_hashmap_t.parent) / 4]
				type = this.$u32[(this.$.vm_memory_get_ptr(current) + vm_hashmap_t.parent) / 4]
			} else {
				this.$.vm_hashmap_get(hashmap, this.intern(key), out)
				current = this.$u32[(out + vm_variable_t.data) / 4]
				type = this.$u32[(out + vm_variable_t.type) / 4]
				this.$.vm_dereference_m(current, type)
			}
			i += 1
			if (i < pathList.length) {
				if (!AsVm.isType(type, AsVm.Type.HASHMAP)) {
					throw new Error(`Failed to resolve '${key}' in '${pathList.slice(0, i).join('.')}': expected 'hashmap' got '${AsVm.typeLut[type]}'`)
				}
			} else {
				this.vStackPop()
				return {value: current as number, type: type}
			}
		}
	}

	public getArgType(top: vm_variable_t, arg: number): AsVm.Type {
		return this.$u32[((top + vm_variable_t.type) - (vm_variable_t.__sizeof * arg)) / 4]
	}

	public getArgValue(top: vm_variable_t, arg: number): vm_variable_data_t {
		const type = this.getArgType(top, arg)
		const index = ((top + vm_variable_t.data) - (vm_variable_t.__sizeof * arg)) / 4
		if (type == AsVm.Type.INTEGER) {
			return this.$32[index]
		}
		if (type == AsVm.Type.FLOAT) {
			return this.$f32[index]
		}
		return this.$u32[index]
	}

	public setReturnValue(top: vm_variable_t, value: vm_variable_data_t, type: AsVm.Type): void {
		this.$u32[(top + vm_variable_t.type + vm_variable_t.__sizeof) / 4] = type
		const index = (top + vm_variable_t.data + vm_variable_t.__sizeof) / 4
		if (type == AsVm.Type.INTEGER) {
			this.$32[index] = value
		} else if (type == AsVm.Type.FLOAT) {
			this.$f32[index] = value
		} else {
			this.$u32[index] = value
		}
	}

	public readHashmapKeys(hashmap: vm_hashmap_t, keys: AsVm.HashmapKeyList, output?: Object): Object {
		const v = this.vStackPush(vm_hashmap_t.__sizeof) as vm_variable_t
		const o: {[key: string]: any} = output || {}
		for (const [key, expectedType] of keys) {
			this.$.vm_hashmap_get(hashmap, this.intern(key), v)
			const type: AsVm.Type = this.$u32[(v + vm_variable_t.type) / 4]
			let value: number
			if (type == AsVm.Type.INTEGER) {
				value = this.$32[(v + vm_variable_t.data) / 4]
			} else if (type == AsVm.Type.FLOAT) {
				value = this.$f32[(v + vm_variable_t.data) / 4]
			} else {
				value = this.$u32[(v + vm_variable_t.data) / 4]
			}
			this.$.vm_dereference_m(value as vm_mmid_t, type)
			if (AsVm.isType(type, expectedType)) {
				o[key] = value
			}
		}
		this.vStackPop()
		return o
	}

	public readArray(array: vm_array_t): AsVm.Variable[] {
		const v = this.vStackPush(vm_hashmap_t.__sizeof) as vm_variable_t
		const size = this.$u32[(array + vm_array_t.used) / 4]
		const out: AsVm.Variable[] = []
		for (let i = 0; i < size; i++) {
			const exception = this.$.vm_array_get(array, i, v)
			if (exception != AsVm.Exception.NONE) {
				throw new Error(AsVm.exceptionLut[exception])
			}
			const type: AsVm.Type = this.$u32[(v + vm_variable_t.type) / 4]
			const value: vm_variable_data_t = this.$u32[(v + vm_variable_t.data) / 4]
			this.$.vm_dereference_m(value as vm_mmid_t, type)
			out.push({type, value})
		}
		this.vStackPop()
		return out
	}

	public createArray(data: AsVm.Variable[]) {
		const mmid = this.$.vm_array_create(data.length)
		const ptr = this.$.vm_memory_get_ptr(mmid)
		this.$u32[(ptr + vm_array_t.used) / 4] = data.length
		for (let i = 0; i < data.length; i++) {
			this.$u32[(ptr + vm_array_t.data + (vm_variable_t.__sizeof * i) + vm_variable_t.type) / 4] = data[i].type
			const index = (ptr + vm_array_t.data + (vm_variable_t.__sizeof * i) + vm_variable_t.data) / 4
			if (data[i].type == AsVm.Type.INTEGER) {
				this.$32[index] = data[i].value
			} else if (data[i].type == AsVm.Type.FLOAT) {
				this.$f32[index] = data[i].value
			} else {
				this.$u32[index] = data[i].value
			}
		}
		return mmid
	}

	public static floatToRaw(n: number): uint32_t {
		AsVm.flConverterIn[0] = n
		return AsVm.flConverterOut[0]
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

	export import Type = VmConstants.Type

	export import typeLut = VmConstants.typeLut

	export import typeMatrix = VmConstants.typeMatrix

	export interface Trace {
		pc: number
		line: number
		file?: string
		func?: string
	}

	export interface Variable {
		value: vm_variable_data_t
		type: Type
	}

	export type HashmapKeyList = [string, AsVm.Type][]
}
