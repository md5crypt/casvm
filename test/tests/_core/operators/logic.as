@private function aeq a b
	assert a == b

@private function aneq a b
	assert a != b

@private function aeqeq a b
	assert a === b

@private function aneqneq a b
	assert a !== b

@private function anot a
	assert !a

@private function anotnot a
	assert !!a

@private function ge a b
	return a >= b

@private function gt a b
	return a > b

@private function le a b
	return a <= b

@private function lt a b
	return a < b

function eq
	aeq self self
	aeq 1 1.0
	aeq 1 true
	aeq 0 false
	aeq 1 1
	aeq 1.0 1
	aeq 1.0 true
	aeq 0.0 false
	aeq 1.0 1.0
	aeq false 0
	aeq false 0.0
	aeq true true
	aeq false false
	aeq "1" {string.from 1}
	aeq {string.from 1} "1"
	aeq "1" "1"

function neq
	aneq self self.parent
	aneq [] []
	aneq 1 1.1
	aneq 1 false
	aneq 0 true
	aneq 1 11
	aneq 1.1 1
	aneq 1.1 true
	aneq 1.1 1.0
	aneq true 0
	aneq true 1.1
	aneq true false
	aneq "2" {string.from 1}
	aneq {string.from 2} "1"
	aneq "1" "2"

function eqeq
	aeqeq self self
	aneqneq 1 1.0
	aneqneq 1 true
	aneqneq 0 false
	aeqeq 1 1
	aneqneq 1.0 1
	aneqneq 1.0 true
	aneqneq 0.0 false
	aeqeq 1.0 1.0
	aneqneq false 0
	aneqneq false 0
	aeqeq true true
	aeqeq false false
	aeqeq "1" {string.from 1}
	aeqeq {string.from 1} "1"
	aeqeq "1" "1"

function neqneq
	aneqneq self self.parent
	aneqneq [] []
	aneqneq 1 1.1
	aneqneq 1 false
	aneqneq 0 true
	aneqneq 1 11
	aneqneq 1.1 1
	aneqneq 1.1 true
	aneqneq 1.1 1.0
	aneqneq true 0
	aneqneq true 1.1
	aneqneq true false
	aneqneq "2" {string.from 1}
	aneqneq {string.from 2} "1"
	aneqneq "1" "2"

function `not
	anot false
	anotnot true
	anot 0
	anot 0.0
	anot ""
	anot undefined
	anotnot []
	anotnot self
	anotnot 2
	anotnot 0.1

function inequality
	# lesser then
	assert {lt 1 2}
	assert {lt 1 2.0}
	assert {lt 1.0 2}
	assert {lt 1.0 2.0}
	assert !{lt 1 1}
	assert !{lt 1 1.0}
	assert !{lt 1.0 1}
	assert !{lt 1.0 1.0}
	# lesser equal
	assert {le 1 1}
	assert {le 1 1.0}
	assert {le 1.0 1}
	assert {le 1.0 1.0}
	assert !{le 2 1}
	assert !{le 2 1.0}
	assert !{le 2.0 1}
	assert !{le 2.0 1.0}
	# greater then
	assert {gt 3 2}
	assert {gt 3 2.0}
	assert {gt 3.0 2}
	assert {gt 3.0 2.0}
	assert !{gt 1 1}
	assert !{gt 1 1.0}
	assert !{gt 1.0 1}
	assert !{gt 1.0 1.0}
	# greater equal
	assert {ge 1 1}
	assert {ge 1 1.0}
	assert {ge 1.0 1}
	assert {ge 1.0 1.0}
	assert !{ge 2 3}
	assert !{ge 2 3.0}
	assert !{ge 2.0 3}
	assert !{ge 2.0 3.0}

@throws_type function inequality_lt_type1
	lt true 0

@throws_type function inequality_lt_type2
	lt true 1.0

@throws_type function inequality_lt_type3
	lt 0 true

@throws_type function inequality_le_type1
	le true 0

@throws_type function inequality_le_type2
	le true 1.0

@throws_type function inequality_le_type3
	le 0 true

@throws_type function inequality_gt_type1
	gt true 0

@throws_type function inequality_gt_type2
	gt true 1.0

@throws_type function inequality_gt_type3
	gt 0 true

@throws_type function inequality_ge_type1
	ge true 0

@throws_type function inequality_ge_type2
	ge true 1.0

@throws_type function inequality_ge_type3
	ge 0 true
