@throws_arity function arrity0
	array.unshift

@throws_arity function arrity1
	array.unshift []

@throws_type function type
	array.unshift 1 1

function normal
	local a = [1]
	for x in 0:1024
		array.unshift a [{string.from x}]
	assertEqual 1025 {length a}
	assertEqual 1 (a 1024)
	assertEqual "1023" ((a 0) 0)
