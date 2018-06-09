@private function run_tests a
	array.write a [3 {string.from 4}]
	assert {array.compare a [3 "4" 3]}
	array.write a [5 6 7 8] 1 2
	assert {array.compare a [3 5 6]}
	array.write a [9] -1
	assert {array.compare a [3 5 9]}
	local b = [1]
	array.unshift b {string.from 2}
	array.write a b 1 1
	assert {array.compare a [3 "2" 9]}
	set b = [1 {string.from 8} 4]
	array.shift b
	array.write a b
	assert {array.compare a ["8" 4 9]}
	array.write a [1 1 1] 0 -1
	assert {array.compare a ["8" 4 9]}

function normal
	run_tests [{string.from 1} 2 3]

function offset_negative
	local a = [2 3]
	array.unshift a {string.from 1}
	run_tests a

function offset_positive
	local a = [{string.from 4} {string.from 1} 2 3]
	array.shift a
	run_tests a

@throws_type function type21
	array.write [] "a"

@throws_type function type22
	array.write "a" []

@throws_type function type23
	array.write "a" "a"

@throws_type function type3
	array.write [] [] "a"

@throws_type function type4
	array.write [] [] 0 "a"

@throws_arity function arity0
	array.write

@throws_arity function arity1
	array.write []

@throws_arity function arity5
	array.write [] [] 0 0 0

@throws_oob function oob1
	array.write [1 2 3] [4 5] 2

@throws_oob function oob2
	array.write [1 2 3] [4 5] -4

@throws_oob function oob3
	array.write [1 2 3] [4 5] 0 3
