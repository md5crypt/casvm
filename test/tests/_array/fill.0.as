@private function run_tests a
	array.fill a {string.from 8}
	array.fill a {string.from 7} 0 0
	assert {array.compare a ["8" "8" "8"]}
	array.fill a {string.from 7} -1
	assert {array.compare a ["8" "8" "7"]}
	array.fill a {string.from 6} -2 1
	assert {array.compare a ["8" "6" "7"]}

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


@throws_type function type2
	array.fill "a" 0

@throws_type function type3
	array.fill [] 0 "a"

@throws_type function type4
	array.fill [] 0 0 "a"

@throws_arity function arity0
	array.fill

@throws_arity function arity1
	array.fill []

@throws_arity function arity5
	array.fill [] 0 0 0 0

@throws_oob function oob1
	array.fill [1 2 3] 0 3 1

@throws_oob function oob2
	array.fill [1 2 3] 0 -4

@throws_oob function oob3
	array.fill [1 2 3] 0 4 1

@throws_oob function oob4
	array.fill [1 2 3] 0 0 4