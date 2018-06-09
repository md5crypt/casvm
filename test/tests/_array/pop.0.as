function normal
	local a = [1 3 [3 4] 5 6 {string.from 12}]
	assertEqual "12" {array.pop a}
	assertEqual 6 {array.pop a}
	for x in 0:3 array.pop a
	assertEqual 1 {array.pop a}
	assertEqual 0 {length a}

function offset
	local a = [3 4]
	array.unshift a 1 2
	array.push a 5 6 7 8
	assertEqual 8 {length a}
	for i in 0:8
		assertEqual 8-i {array.pop a}

@throws_arity function arrity0
	array.pop

@throws_arity function arrity2
	array.pop [1] 4

@throws_oob function oob
	array.pop []

@throws_type function type
	array.pop 1
