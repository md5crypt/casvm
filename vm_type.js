var fs = require('fs');

var types = [
	['VM_INVALID_T',null],
	['VM_UNDEFINED_T',null],
	
	['VM_SCALAR_T',null],
	['VM_BOOLEAN_T','VM_SCALAR_T'],
	['VM_INTEGER_T','VM_SCALAR_T'],
	['VM_FLOAT_T','VM_SCALAR_T'],
	
	['VM_VECTOR_T',null],
	['VM_STRING_T','VM_VECTOR_T'],
	['VM_ARRAY_T','VM_VECTOR_T'],
	
	['VM_OBJECT_T',null],
	['VM_FUNCTION_T','VM_OBJECT_T'],
	['VM_NAMESPACE_T','VM_OBJECT_T']
];

var map = {};
for(var i=0; i<types.length; i++)
	map[types[i][0]] = i;

var enumstr = 'typedef enum {\n'+types.map(o=>'\t'+o[0]).join(',\n')+'\n} vm_type_t;';

var n = types.length;
var matrix = new Uint8Array(n*n);

for(var i=0; i<n; i++){
	matrix[i+i*n] = 1;
	if(!types[i][1])
		continue;
	var b = map[types[i][1]];
	for(var j=0; j<n; j++)
		matrix[j+i*n] = matrix[j+b*n];
	matrix[i+i*n] = 1;
}

var matrixstr = 'const vm_type_t vm_type_matrix[] = {\n';
for(var i=0; i<n; i++)
	matrixstr += '\t'+matrix.slice(i*n, (i+1)*n).join(',\t')+','+'\n';
matrixstr =  matrixstr.slice(0,-2)+'\n};';

console.log("vm_type.js: creating vm_type.h");
fs.writeFileSync('vm_type.h','#pragma once\n#define VM_TYPE_COUNT\t'+n+'\n\n'+enumstr+'\n\nextern const vm_type_t vm_type_matrix[];\n');
console.log("vm_type.js: creating vm_type.c");
fs.writeFileSync('vm_type.c','#include "vm_type.h"\n\n'+matrixstr+'\n');
