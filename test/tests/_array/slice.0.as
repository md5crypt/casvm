@throws_arity function arrity0
	array.slice

@throws_arity function arrity4
	array.slice [] 1 2 3

@throws_type function type1
	array.slice 0

@throws_type function type2
	array.slice [] "a"

@throws_type function type3
	array.slice [] 0 "a"

@throws_oob function oob1
	array.slice [] 1 2

@throws_oob function oob2
	array.slice [1] 0 2

@throws_oob function oob3
	array.slice [1] -2 2

@throws_oob function oob4
	array.slice [1] -8 -2

@private function single a:array
	assert {array.slice a} != a
	assert {array.compare {array.slice a} a}
	assert {array.compare {array.slice a 0} a}
	assert {array.compare {array.slice a 1} [2 3 4 5]}
	assert {array.compare {array.slice a 4} [5]}
	assert {array.compare {array.slice a 5} []}
	assert {array.compare {array.slice a -1} [5]}
	assert {array.compare {array.slice a -2} [4 5]}

@private function double a:array
	assert {array.compare {array.slice a 0 5} a}
	assert {array.compare {array.slice a 0 -1} [1 2 3 4]}
	assert {array.compare {array.slice a 2 3} [3]}
	assert {array.compare {array.slice a 1 4} [2 3 4]}

function normal
	local a = [1 2 3 4 5]
	single a
	double a

function offset
	local a = [4 5]
	array.unshift a 1 2 3
	single a
	double a

function gc
	array.slice [{string.from 1234} [1 2 3 4]]
