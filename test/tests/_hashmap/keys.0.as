@throws_type function type
	hashmap.keys 0

@throws_arity function arity0
	hashmap.keys

@throws_arity function arity2
	hashmap.keys self 0

function normal
	namespace map
	set (map "0")
	set (map "11")
	set (map "33")
	local a = {hashmap.keys map}
	assertEqual 3 {length a}
	assert {array.find a "0"} >= 0
	assert {array.find a "11"} >= 0
	assert {array.find a "33"} >= 0
