function concat
	assertEqual "0.2 times 5 is <:boolean>" {string.concat 0.2 " times " 5 " is " true}
	assertEqual "" {string.concat}

@leaks_const function grow
	namespace map
	for x in 0:1024
		local key = {string.from x}
		set (map key) = "aia!"
		assertEqual "aia!" (map key)
		unset (map key)
		assertEqual undefined (map key)
	for x in 0:1024
		local key = {string.from x}
		set (map key) = "aia!"
		assertEqual "aia!" (map key)
		unset (map key)
		assertEqual undefined (map key)

function intern
	local c = "aabb"
	local a = "aa"
	local b = "bb"
	assertEqual c {_intern a + b}

@throws_type function intern_type
	return {_intern 0}
