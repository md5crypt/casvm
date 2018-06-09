@throws_arity function arrity0
	array.shift

@throws_arity function arrity2
	array.shift [1] 4

@throws_oob function oob
	array.shift []

@throws_type function type
	array.shift 1

function normal
	local a = [1 3 [3 4] 5 6 {string.from 12}]
	assertEqual 1 {array.shift a}
	assertEqual 3 {array.shift a}
	for x in 0:3 array.shift a
	assertEqual "12" {array.shift a}
	assertEqual 0 {length a}

function offset
	local a = [3 4]
	array.unshift a 1 2
	array.push a 5 6 7 8
	assertEqual 8 {length a}
	for i in 0:7
		assertEqual i+1 {array.shift a}
	array.push a 9
	assertEqual 9 {array.pop a}
	assertEqual 8 {array.pop a}
	assertEqual 0 {length a}
