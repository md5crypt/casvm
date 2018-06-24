@throws_type function type21
	hashmap.has 0 "aa"

@throws_type function type22
	hashmap.has self 0

@throws_arity function arity0
	hashmap.has

@throws_arity function arity1
	hashmap.has self

@throws_arity function arity3
	hashmap.has self "aa" "c"

function normal
	namespace map
	set (map "0")
	set (map "11")
	set (map "33")
	assert {hashmap.has map "0"}
	assert {hashmap.has map "11"}
	assert {hashmap.has map "33"}
	assert !{hashmap.has map "22"}
