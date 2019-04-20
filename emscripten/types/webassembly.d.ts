declare namespace WebAssembly {
	function instantiate(bufferSource: ArrayBuffer, importObject: Object): Promise<{module: Module, instance: Instance}>

	class Memory {
		constructor(memoryDescriptor: {
			initial: number,
			maximum?: number
		})
		public readonly buffer: ArrayBuffer
		public grow(amount: number): number
	}

	class Table {
		constructor(tableDescriptor: {
			element: "anyfunc",
			initial: number,
			maximum?: number
		})
		public readonly length: number
		public grow(amount: number): number
		public get(index: number): any
		public set(index: number, value: any): void
	}

	class Module {
		constructor(bufferSource: ArrayBuffer)
		static customSections(module: Module, sectionName: string): ArrayBuffer[]
	}

	class Instance {
		constructor(module: Module, importObject: Object)
		public readonly exports: Object
	}
}
