@private function run_tests a
	assertEqual -1 {array.find a 2 2}
	assertEqual -1 {array.find a "2" 0}
	assertEqual -1 {array.find a "3"}
	assertEqual 0 {array.find a "1"}
	assertEqual 1 {array.find a 2}
	assertEqual 0 {array.find a "1" 0}
	assertEqual 2 {array.find a "1" 1}
	assertEqual 2 {array.find a "1" 2}
	assertEqual 2 {array.find a "1" -2}
	assertEqual 0 {array.find a "1" -3}

function normal
	run_tests [{string.from 1} 2 {string.from 1}]

function offset_negative
	local a = [2 {string.from 1}]
	array.unshift a {string.from 1}
	run_tests a

function offset_positive
	local a = [{string.from 4} {string.from 1} 2 {string.from 1}]
	array.shift a
	run_tests a

@throws_type function type2
	array.find "a" 0

@throws_type function type3
	array.find [] 0 "a"

@throws_arity function arity0
	array.find

@throws_arity function arity1
	array.find []

@throws_arity function arity3
	array.find [] 0 0 0

@throws_oob function oob1
	array.find [] 0 0

@throws_oob function oob2
	array.find [] 0 -1
