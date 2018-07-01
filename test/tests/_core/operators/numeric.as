@private function sub a b
	return a - b

@private function add a b
	return a + b

@private function mul a b
	return a * b

@private function div a b
	return a / b

@private function neg a
	return -a

function sub_normal
	assertEqual -5 {sub 10 15}
	assertEqual -5.0 {sub 10.0 15}
	assertEqual -5.0 {sub 10 15.0}
	assertEqual -5.0 {sub 10.0 15.0}

@throws_type function sub_type1
	sub 1 true

@throws_type function sub_type2
	sub true 1

@throws_type function sub_type3
	sub true 1.0

function mul_normal
	assertEqual 150 {mul 10 15}
	assertEqual 150.0 {mul 10.0 15}
	assertEqual 150.0 {mul 10 15.0}
	assertEqual 150.0 {mul 10.0 15.0}

@throws_type function mul_type1
	mul 1 true

@throws_type function mul_type2
	mul true 1

@throws_type function mul_type3
	mul true 1.0

function div_normal
	assertEqual 5 {div 100 20}
	assertEqual 5.0 {div 100.0 20}
	assertEqual 5.0 {div 100 20.0}
	assertEqual 5.0 {div 100.0 20.0}
	assertEqual 2 {div 5 2}
	assertEqual 2.5 {div 5 2.0}
	assertEqual 1/0.0 {div 5 0.0}
	assertEqual 1/0.0 {div 5.0 0}

@throws_type function div_type1
	div 1 true

@throws_type function div_type2
	div true 1

@throws_type function div_type3
	div true 1.0

@throws_div0 function div_div0
	div 1 0

function add_normal
	assertEqual 25 {add 10 15}
	assertEqual 25.0 {add 10.0 15}
	assertEqual 25.0 {add 10 15.0}
	assertEqual 25.0 {add 10.0 15.0}
	assertEqual "abba" {add "ab" "ba"}

@throws_type function add_type1
	add 1 true

@throws_type function add_type2
	add true 1

@throws_type function add_type3
	add true 1.0

@throws_type function add_type4
	add true "a"

function neg_normal
	assertEqual -2 {neg 2}
	assertEqual -2.0 {neg 2.0}

@throws_type function neg_type1
	neg true
