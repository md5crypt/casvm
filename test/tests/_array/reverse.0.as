@private function run_tests a
	array.reverse a
	assert {array.compare a [3 2 "1"]}

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


@throws_type function type1
	array.reverse "a"

@throws_arity function arity0
	array.reverse

@throws_arity function arity2
	array.reverse [] 0