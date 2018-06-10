function normal
	assertEqual "a" ("abcd" 0)
	assertEqual "b" ("abcd" 1)
	assertEqual "c" ("abcd" 2)
	assertEqual "d" ("abcd" 3)
	assertEqual "a" ("abcd" -4)
	assertEqual "b" ("abcd" -3)
	assertEqual "c" ("abcd" -2)
	assertEqual "d" ("abcd" -1)

@throws_type function type
	return ("abcd" "a")

@throws_oob function oob1
	return ("abcd" 4)

@throws_oob function oob2
	return ("abcd" -5)
