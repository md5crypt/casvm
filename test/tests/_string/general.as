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
