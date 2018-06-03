@private function test a:array
	assertEqual 5 (a 0)
	assertEqual 1 (a 2)
	assertEqual "8" (a 5)
	assertEqual 1 (a 6)
	assertEqual 1 (a -1)
	assertEqual 3 (a -6)
	assertEqual 5 (a -7)

function normal
	local a = [5 3 1 {string.from 2} 3 {string.from 8} 1]
	test a

function offset
	local a = [3 {string.from 8} 1]
	array.unshift a 5 3 1 {string.from 2}
	test a

function offset_only
	local a = []
	for x in {array.reverse [5 3 1 {string.from 2} 3 {string.from 8} 1]}
		array.unshift a x
	test a

@throws_oob function offset_oob
	local a = []
	array.unshift a 1
	return (a 1)

@throws_oob function oob_positive
	local a = [5 3 1 {string.from 2} 3 {string.from 8} 1]
	return (a 7)

@throws_oob function oob_negative
	local a = [5 3 1 {string.from 2} 3 {string.from 8} 1]
	return (a -8)

@throws_oob function oob_empty_positive
	local a = []
	return (a 0)

@throws_oob function oob_empty_negative
	local a = []
	return (a 0)