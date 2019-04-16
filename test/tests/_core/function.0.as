@private function vararg a ...b
	return [a b]

@private function optarg a b c?:integer d?
	return {array.join [a b c||0 d||0] ""}

@private function optarg2 a1? a2? a3? a4? a5? a6? a7? a8? a9? a10? a11? a12? a13? a14? a15? a16?
	return a16

function vararg_normal
	local a = {vararg 1 2 3 4 5 6}
	assertEqual 1 (a 0)
	assert {array.compare [2 3 4 5 6] (a 1)}

@throws_type function type_call
	local a = 1
	a 1 2 3

@private function glue ...a
	return {array.join a}

function apply_normal
	assertEqual "" {apply glue []}
	assertEqual "1" {apply glue [1]}
	assertEqual "1 2 3" {apply glue [1 2 3]}

@throws_type function apply_type
	apply array.push 12

function optarg_normal
	assertEqual "1200" {optarg 1 2}
	assertEqual "1230" {optarg 1 2 3}
	assertEqual "1234" {optarg 1 2 3 4}

@throws_none function optarg_grow a?
	set a = (a || 0) + 1
	if a > 38
		assertEqual undefined {optarg2 1 2}
	else
		self a

@throws_arity function optarg_arity1
	optarg 1

@throws_arity function optarg_arity2
	optarg 1 2 3 4 5

@throws_type function optarg_type
	optarg 1 2 "3" 4

@throws_arity function general_arity1
	self 1

@throws_type function general_type
	hashmap.path "a"

@throws_type function async_type
	async "c"