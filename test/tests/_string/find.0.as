function normal
	assertEqual 0 {string.find "abbaa" "a"}
	assertEqual 1 {string.find "abbaa" "b"}
	assertEqual 3 {string.find "abbaa" "a" 1}
	assertEqual -1 {string.find "abbaa" "b" -2}
	assertEqual -1 {string.find "abbaa" ""}
	assertEqual 2 {string.find "abbaa" "b" -3}
	assertEqual 0 {string.find "abbaa" "abbaa"}
	assertEqual -1 {string.find "abbaa" "abbaa" 1}
	assertEqual -1 {string.find "abbaa" "abbaaa" -1}
	assertEqual 2 {string.find "abbaa" "ba" 0}
	assert {array.compare {findall "banananabanna" "ana"} [1 3 5]}

@private function findall a b
	local p = 0
	local T = []
	while true
		set p = {string.find a b p}
		if p == -1
			return T
		array.push T p
		set p = p + 1

@throws_type function type21
	string.find "a" 0

@throws_type function type22
	string.find 0 "a"

@throws_type function type3
	string.find "a" "a" "a"

@throws_arity function arity0
	string.find

@throws_arity function arity1
	string.find "a"

@throws_arity function arity4
	string.find "a" "a" 0 0

@throws_oob function oob1
	string.find "a" "a" 1

@throws_oob function oob2
	string.find "a" "a" -2
