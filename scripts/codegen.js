/*eslint semi: ["error", "never"]*/
const fs = require('fs')
const path = require('path')

function cammel(x) {
	return x.split(/[-_]/)
	.map((x, i) => i == 0 ? x : x.charAt(0).toUpperCase() + x.slice(1))
	.join('')
}

function readArgs(help) {
	if (process.argv.length == 2) {
		console.log('parameters:')
		console.log(help)
		process.exit(0)
	}
	const params = help.split('\n').filter(x=>x).map(x=>x.split(/\s+/))
	params.map(x => cammel(x[1].slice(2)))
		.forEach(x => params[x] = [])
	for (let i = 2; i < process.argv.length; i++) {
		let arg = process.argv[i]
		let j = params.findIndex(x => (x[0] == arg) || (x[1] == arg))
		if (j < 0) {
			console.log(`Error: parameter "${arg}" not recognized`)
			process.exit(1)
		}
		let k = cammel(params[j][1].slice(2))
		params[k].push(process.argv[++i])
	}
	return params;
}

function crateTypeFiles(path, outDir) {
	const types = JSON.parse(fs.readFileSync(path))
	const map = {}
	for (let i = 0; i < types.length; i++) {
		map[types[i][0]] = i
	}
	const typeEnum = `typedef enum {\n${types.map(o => `\tVM_${o[0].toUpperCase()}_T`).join(',\n')}\n} vm_type_t;`
	const typeNames = `const char* const vm_type_names[] = {\n${types.map(o => `\t"${o[0]}"`).join(',\n')}\n};`
	const n = types.length
	const matrix = new Uint8Array(n*n)
	for (let k = 0; k < n; k++){ //haha, I'm lazy
		for (let i = 0; i < n; i++){
			matrix[i + (i * n)] = 1
			if (types[i][1]) {
				const b = map[types[i][1]]
				for(let j=0; j<n; j++) {
					matrix[j + (i * n)] = matrix[j + (b * n)]
				}
				matrix[i + (i * n)] = 1
			}
		}
	}
	let typeMatrix = 'const vm_type_t vm_type_matrix[] = {\n'
	for (let i = 0; i < n; i++) {
		typeMatrix += `\t${matrix.slice(i * n, (i + 1) *n).join(',\t')},\n`
	}
	typeMatrix =  typeMatrix.slice(0,-2) + '\n};'
	console.log("codegen.js: creating vm_type.h")
	fs.writeFileSync(outDir+'vm_type.h',
		'#pragma once\n\n'+
		`#define VM_TYPE_COUNT    ${n}\n`+
		`${typeEnum}\n\n`+
		'extern const vm_type_t vm_type_matrix[VM_TYPE_COUNT * VM_TYPE_COUNT];\n'+
		'extern const char* const vm_type_names[VM_TYPE_COUNT];\n'
	)
	console.log("codegen.js: creating vm_type.c")
	fs.writeFileSync(outDir+'vm_type.c',
		'#include "vm_type.h"\n\n'+
		`${typeMatrix}\n\n`+
		`${typeNames}\n`
	)
}

function crateOpcodeFiles(path, outDir) {
	const op = JSON.parse(fs.readFileSync(path))
	console.log("codegen.js: creating vm_op.h")
	fs.writeFileSync(outDir+'vm_op.h',
		'#pragma once\n\n' +
		'typedef enum {\n' +
		op.map(x => '\tVM_OP_' + x.toUpperCase()).join(',\n') +
		'\n} vm_op_t;\n'
	)
}

function crateExternFiles(paths, outDir) {
	const libFiles = paths.map(x => fs.lstatSync(x).isDirectory() ? x.readdirSync().filter(x => x.match(/.c$/)) : [x])
	const libData = Array.prototype.concat.apply([],libFiles)
		.map(x => fs.readFileSync(x, 'utf8')).join('/n')
		.match(/^vm_exception_t\s*vm_lib_[^(]+\([^{\n]+{$/gm)
		.map(x => x.match(/vm_lib_([^(\s]+)/)[1]).sort()
	console.log("codegen.js: creating vm_extern_native.c")
	fs.writeFileSync(outDir+'vm_extern_native.c',
		'#include <stddef.h>\n\n' +
		'#include "vm_extern.h"\n\n' +
		libData.map(x => `extern vm_exception_t ${'vm_lib_' + x}(vm_variable_t* top, uint32_t arguments);`).join('\n') +
		'\n\nconst vm_extern_t vm_extern_native[] = {\n' +
		libData.map(x => `\t{"__${cammel(x)}", vm_lib_${x} },`).join('\n') +
		'\n\t{NULL, NULL}\n};\n'
	)
}

const args = readArgs(`
-t --types      [file]
-c --opcodes    [file]
-l --lib        [file/directory]
-o --output-dir [directory]
`)

const outDir = path.normalize((args.outputDir[0] || '.')+'/')
if (args.types.length > 0) {
	crateTypeFiles(args.types[0], outDir)
}
if (args.opcodes.length > 0) {
	crateOpcodeFiles(args.opcodes[0], outDir)
}
if (args.lib.length > 0) {
	crateExternFiles(args.lib, outDir)
}
