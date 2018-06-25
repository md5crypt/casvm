function mmid_overflow
	local a = {array.create 256*256*2}
	for i in 0:{length a}
		set (a i) = {itos i}

function itos_normal
	assertEqual "123" {itos 123}

@throws_type function itos_type
	itos 1.21

@throws_arity function itos_arrity0
	itos

@throws_arity function itos_arrity2
	itos 1 2

function dtos_normal
	assertEqual "1.23" {dtos 1.23}

@throws_type function dtos_type
	dtos 0

@throws_arity function dtos_arrity0
	dtos

@throws_arity function dtos_arrity2
	dtos 1.2 2.2

function typeof_normal
	assertEqual "integer" {typeof 1}
	assertEqual "string" {typeof "1"}
	assertEqual "array" {typeof []}
	assertEqual "function" {typeof self}
	assertEqual "namespace" {typeof self.parent}

@throws_arity function typeof_arrity0
	typeof

@throws_arity function typeof_arrity2
	typeof 1.2 2.2

function nameof_normal
	assertEqual "nameof_normal" {nameof self}
	assertEqual "tests" {nameof self.parent}

@throws_type function nameof_type
	nameof []

@throws_arity function nameof_arrity0
	nameof

@throws_arity function nameof_arrity2
	nameof self self

@leaks_hashmap function length_normal
	assertEqual 4 {length [1 2 3 4]}
	assertEqual 4 {length "1234"}
	in self set
		a = 1
		b = 2
		c = 3
		d = 4
	assertEqual 5 {length self}

@throws_type function length_type
	length 0

@throws_arity function length_arrity0
	length

@throws_arity function length_arrity2
	length self self