@private function run_tests a
	array.resize a 4
	assert {array.compare a ["1" 2 undefined undefined]}
	array.resize a 6 {string.from 3}
	assert {array.compare a ["1" 2 undefined undefined "3" "3"]}
	array.resize a 1024
	assertEqual 1024 {length a}
	assert {array.compare a ["1" 2 undefined undefined "3" "3"] 6}
	for i in 6:1024
		assertEqual undefined (a i)
	array.resize a 3
	assert {array.compare a ["1" 2 undefined]}

function normal
	run_tests [{string.from 1} 2]

function offset_negative
	local a = [2]
	array.unshift a {string.from 1}
	run_tests a

function offset_positive
	local a = [{string.from 4} {string.from 1} 2]
	array.shift a
	run_tests a

@throws_type function type1
	array.resize "a" 10

@throws_type function type2
	array.resize [] "a"

@throws_user function negative
	array.resize [] -10

@throws_arity function arity0
	array.resize

@throws_arity function arity1
	array.resize []

@throws_arity function arity4
	array.resize [] 4 4 4