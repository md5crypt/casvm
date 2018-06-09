function normal
	local a = {array.create 0}
	local b = {array.create 1}
	local c = {array.create 1024}
	local d = {array.create 4 {string.from 100}}
	assert {array.compare a []}
	assert {array.compare b [undefined]}
	assertEqual 1024 {length c}
	for i in 0:1024
		assertEqual undefined (c i)
	assert {array.compare d ["100" "100" "100" "100"]}

@throws_type function type
	array.create "a"

@throws_user function negative
	array.create -10

@throws_arity function arity0
	array.create

@throws_arity function arity3
	array.create 1 2 4