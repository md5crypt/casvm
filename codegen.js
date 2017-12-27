/*eslint semi: ["error", "never"]*/
const fs = require('fs')

const types = require('./vm_type.json')

const map = {}
for(let i=0; i<types.length; i++)
	map[types[i][0]] = i

const typeEnum = `typedef enum {\n${types.map(o => `\tVM_${o[0].toUpperCase()}_T`).join(',\n')}\n} vm_type_t;`
const typeNames = `const char* const vm_type_names[] = {\n${types.map(o => `\t"${o[0]}"`).join(',\n')}\n};`

const n = types.length
const matrix = new Uint8Array(n*n)

for(let k=0; k<n; k++){ //haha, I'm lazy
	for(let i=0; i<n; i++){
		matrix[i+i*n] = 1
		if(!types[i][1])
			continue
		const b = map[types[i][1]]
		for(let j=0; j<n; j++)
			matrix[j+i*n] = matrix[j+b*n]
		matrix[i+i*n] = 1
	}
}

let typeMatrix = 'const vm_type_t vm_type_matrix[] = {\n'
for(let i=0; i<n; i++)
	typeMatrix += `\t${matrix.slice(i*n, (i+1)*n).join(',\t')},\n`
typeMatrix =  typeMatrix.slice(0,-2)+'\n};'

console.log("codegen.js: creating vm_type.h")
fs.writeFileSync('vm_type.h',`#pragma once\n#define VM_TYPE_COUNT\t${n}\n\n${typeEnum}\n\nextern const vm_type_t vm_type_matrix[VM_TYPE_COUNT*VM_TYPE_COUNT];\nextern const char* const vm_type_names[VM_TYPE_COUNT];\n`)
console.log("codegen.js: creating vm_type.c")
fs.writeFileSync('vm_type.c', `#include "vm_type.h"\n\n${typeMatrix}\n\n${typeNames}\n`)
console.log("codegen.js: creating vm_op.h")
fs.writeFileSync('vm_op.h', `#pragma once\n\ntypedef enum {\n${require('./vm_op.json').map(x => '\tVM_OP_' + x.toUpperCase()).join(',\n')}\n} vm_op_t;\n`)
console.log("codegen.js: creating vm_stdlib_exports.h")
const data = fs.readFileSync('vm_stdlib.c', 'utf-8')
    .match(/^static\s*vm_exception_t\s*lib_[^(]+\([^{\n]+{$/gm)
    .map(x => x.match(/lib_([^(\s]+)/)[1])
    .map(x => `\t{"__${x}",lib_${x}},`).join('\n');
fs.writeFileSync('vm_stdlib_exports.h', `const vm_stdlib_t vm_stdlib[] = {\n${data}\n\t{NULL,NULL}\n};\n`)