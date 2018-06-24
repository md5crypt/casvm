function fibo n?:integer
	if {istype n "undefined"}
		assertEqual 10946 {length {string.split {fibo 20} "+"}}
	else
		if n < 2
			return "1"
		local t1 = {async fibo n-1}
		local t2 = {async fibo n-2}
		return "${await t2}+${await t1}"

@throws_none function stack_attack1 a?
	set a = (a || 0) + 1
	return a > 2048 ? false : {stack_attack1 a}

@throws_none function stack_attack2 a?
	set a = {array.create 256}
	set (a 0) = []
	apply array.push a
	return false

@private function rekureku a b
	if a != b
		await {async rekureku a b+1}

@private function waitfor t n?
	await t
	return n

@private function waitfor2 t n
	await (t 0)
	array.push t n

function queue1
	local a = {async fibo 4}
	local b = {async waitfor a}
	await b

function queue2
	local a = []
	local b = {async waitfor2 a 1}
	local c = {async waitfor2 a 2}
	local d = {async waitfor2 a 3}
	array.push a {async fibo 4}
	await (a 0)
	await b
	await c
	await d
	assert {array.compare [1 2 3] {array.slice a 1}}

@private function selfthread t
	local a = {async waitfor (t 0)}
	local b = {async waitfor (t 0)}
	await {async fibo 4}

function queue3
	local t = []
	local a = {async selfthread t}
	array.push t a
	await a

function queue4
	local a = {async fibo 4}
	local b = {async fibo 4}
	local c = {async fibo 4}
	unset a

function gc1
	local a = {async fibo 4}
	local b = {async waitfor a}
	local c = {async waitfor a}