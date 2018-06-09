function normal
	local a = [1]
	for x in 0:1024
		array.push a [{string.from x}]
	assertEqual 1025 {length a}
	assertEqual 1 (a 0)
	assertEqual "1023" ((a 1024) 0)

function offset
	local a = [3 4]
	array.unshift a 1 2
	array.push a 5 6 7 8
	assertEqual 8 {length a}
	for i in 0:8
		assertEqual i+1 (a i)

@throws_arity function arrity0
	array.push

@throws_arity function arrity1
	array.push []

@throws_type function type
	array.push 1 1

function arrity4
	local a = []
	array.push a 1 2 3 4
	for i in 0:4
		assertEqual i+1 (a i)
