@private function run_tests a
	local b = [{string.from 1} {string.from 2} {string.from 3} {string.from 4}]
	local c = [{string.from 7}]
	array.shift b
	array.unshift c {string.from 5} {string.from 6}
	array.expand a [] b [] c [] [1 2 3]
	assert {array.compare a ["1" 2 3 "2" "3" "4" "5" "6" "7" 1 2 3]}

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

function empty
	local a = []
	array.expand a [] [] [] [] []
	assert {array.compare a []}

@throws_type function type21
	array.expand [] "a"

@throws_type function type22
	array.expand "a" []

@throws_type function type23
	array.expand [] [] "a"

@throws_arity function arity0
	array.expand

@throws_arity function arity1
	array.expand []
