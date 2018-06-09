function normal
	local a = [0]
	apply array.push [a 1 2 3]
	assert {array.compare a [0 1 2 3]}
	local b = {apply array.expand [[] {array.create 1024 {string.from 1}}]}
	assertEqual 1024 {length b}
	assertEqual "1" (b 0)
	assertEqual "1" (b 511)
	assertEqual "1" (b -1)

function offset_negative
	local a = [2 3]
	array.unshift a {string.from 1}
	apply array.push [a 1 2 3]
	assert {array.compare a ["1" 2 3 1 2 3]}

function offset_positive
	local a = [{string.from 4} {string.from 1} 2 3]
	array.shift a
	apply array.push [a 1 2 3]
	assert {array.compare a ["1" 2 3 1 2 3]}