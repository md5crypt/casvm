@private function test a:array
	set (a 0) = {string.from 1}
	set (a 3) = "b"
	set (a 6) = "c"
	assertEqual "1" (a 0)
	assertEqual 3 (a 1)
	assertEqual 1 (a 2)
	assertEqual "b" (a 3)
	assertEqual 3 (a 4)
	assertEqual "8" (a 5)
	assertEqual "c" (a 6)
	set (a -1) = {string.from 2}
	set (a -4) = "e"
	set (a -7) = "c"
	assertEqual "c" (a 0)
	assertEqual 3 (a 1)
	assertEqual 1 (a 2)
	assertEqual "e" (a 3)
	assertEqual 3 (a 4)
	assertEqual "8" (a 5)
	assertEqual "2" (a 6)

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
	set (a 1)

@throws_oob function oob_positive
	local a = [5 3 1 {string.from 2} 3 {string.from 8} 1]
	set (a 7)

@throws_oob function oob_negative
	local a = [5 3 1 {string.from 2} 3 {string.from 8} 1]
	set (a -8)

@throws_oob function oob_empty_positive
	local a = []
	set (a 0)

@throws_oob function oob_empty_negative
	local a = []
	set (a 0)

@throws_type function type_not_array
	set (1 0)

@throws_type function type_not_integer
	set ([1] "a")