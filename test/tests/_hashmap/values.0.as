@throws_type function type
	hashmap.values 0

@throws_arity function arity0
	hashmap.values

@throws_arity function arity2
	hashmap.values self 0

function normal
	namespace map
	set (map "0") = "aba"
	set (map "11") = "cba"
	set (map "33") = 12
	local a = {hashmap.values map}
	assertEqual 3 {length a}
	assert {array.find a "aba"} >= 0
	assert {array.find a "cba"} >= 0
	assert {array.find a 12} >= 0
