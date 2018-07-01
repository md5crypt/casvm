@private function bor a b
	return a | b
@private function bxor a b
	return a ^ b
@private function band a b
	return a & b
@private function shr a b
	return a >> b
@private function lsr a b
	return a >>> b
@private function shl a b
	return a << b
@private function mod a b
	return a % b
@private function bnot a
	return ~a

function bor_normal
	assertEqual 3 {bor 1 2}
	assertEqual 3 {bor 3 3}
	assertEqual 3 {bor 1.0 2}
	assertEqual 3 {bor 1 2.0}
	assertEqual 3 {bor 1.0 2.0}

@throws_type function bor_type1
	bor 1 true

@throws_type function bor_type2
	bor true 1

@throws_type function bor_type3
	bor true 1.0

function bxor_normal
	assertEqual 3 {bxor 1 2}
	assertEqual 0 {bxor 3 3}
	assertEqual 3 {bxor 1.0 2}
	assertEqual 3 {bxor 1 2.0}
	assertEqual 3 {bxor 1.0 2.0}

@throws_type function bxor_type1
	bxor 1 true

@throws_type function bxor_type2
	bxor true 1

@throws_type function bxor_type3
	bxor true 1.0

function band_normal
	assertEqual 0 {band 1 2}
	assertEqual 3 {band 3 3}
	assertEqual 0 {band 1.0 2}
	assertEqual 0 {band 1 2.0}
	assertEqual 0 {band 1.0 2.0}

@throws_type function band_type1
	band 1 true

@throws_type function band_type2
	band true 1

@throws_type function band_type3
	band true 1.0

function shr_normal
	assertEqual 1 {shr 4 2}
	assertEqual 3 {shr 24 3}
	assertEqual -4 {shr -16 2}
	assertEqual 1 {shr 4 2.0}
	assertEqual 1 {shr 4.0 2}
	assertEqual 1 {shr 4.0 2.0}

@throws_type function shr_type1
	shr 1 true

@throws_type function shr_type2
	shr true 1

@throws_type function shr_type3
	shr true 1.0

function lsr_normal
	assertEqual 1 {lsr 4 2}
	assertEqual 3 {lsr 24 3}
	assertEqual 1073741820 {lsr -16 2}
	assertEqual 1 {lsr 4.0 2}
	assertEqual 1 {lsr 4 2.0}
	assertEqual 1 {lsr 4.0 2.0}

@throws_type function lsr_type1
	lsr 1 true

@throws_type function lsr_type2
	lsr true 1

@throws_type function lsr_type3
	lsr true 1.0

function shl_normal
	assertEqual 4 {shl 1 2}
	assertEqual 24 {shl 3 3}
	assertEqual 4 {shl 1.0 2}
	assertEqual 4 {shl 1 2.0}
	assertEqual 4 {shl 1.0 2.0}

@throws_type function shl_type1
	shl 1 true

@throws_type function shl_type2
	shl true 1

@throws_type function shl_type3
	shl true 1.0

function mod_normal
	assertEqual 1 {mod 7 3}
	assertEqual 3 {mod 24 7}
	assertEqual 1 {mod 7.0 3}
	assertEqual 1 {mod 7 3.0}
	assertEqual 1 {mod 7.0 3.0}

@throws_type function mod_type1
	mod 1 true

@throws_type function mod_type2
	mod true 1

@throws_type function mod_type3
	mod true 1.0

@throws_div0 function mod_div0
	mod 8 0

@throws_div0 function mod_div1
	mod 8 0.0

function bnot_normal
	assertEqual -1 {bnot 0}
	assertEqual -25 {bnot 24}
	assertEqual -25 {bnot 24.0}

@throws_type function bnot_type1
	bnot true