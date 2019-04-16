@throws_user function throw_1
	throw "aaa"

@throws_user function throw_2
	throw

@throws_type function wait_type
	await "a"

function parent_root
	local a = root
	assert a.parent === undefined

@throws_type function parent_type
	local a = "a"
	assert a.parent === undefined

@throws_type function hashmap_get_type1
	return (self 1)

@throws_type function hashmap_get_type2
	local a = "a"
	return a.aaa

@throws_type function argv_type
	return {_argv "a"}

@throws_oob function argv_oob
	return {_argv 0}