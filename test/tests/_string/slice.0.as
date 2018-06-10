@throws_arity function arrity0
	string.slice

@throws_arity function arrity4
	string.slice "a" 1 2 3

@throws_type function type1
	string.slice 0

@throws_type function type2
	string.slice "a" "a"

@throws_type function type3
	string.slice "" 0 "a"

@throws_oob function oob1
	string.slice "" 1 2

@throws_oob function oob2
	string.slice "1" 0 2

@throws_oob function oob3
	string.slice "1" -2 2

@throws_oob function oob4
	string.slice "1" -8 -2

function normal
	local a = "12345"
	assertEqual a {string.slice a}
	assertEqual "2345" {string.slice a 1}
	assertEqual "5" {string.slice a 4}
	assertEqual ""  {string.slice a 5}
	assertEqual "5" {string.slice a -1}
	assertEqual "45" {string.slice a -2}
	assertEqual "1234" {string.slice a 0 -1}
	assertEqual "3" {string.slice a 2 3}
	assertEqual "234" {string.slice a 1 4}
