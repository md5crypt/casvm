function __mod_leaks_hashmap func:function
	if !{hashmap.has func "leaks"}
		set func.leaks = 0
	set func.leaks = func.leaks | (1<<1)

function __mod_leaks_const func:function
	if !{hashmap.has func "leaks"}
		set func.leaks = 0
	set func.leaks = func.leaks | (1<<0) | (1<<5)

function __mod_throws_oob func:function
	set func.throws = "OUT-OF-BOUNDS"

function __mod_throws_user func:function
	set func.throws = "USER"

function __mod_throws_type func:function
	set func.throws = "TYPE"

function __mod_throws_arity func:function
	set func.throws = "ARITY"

function __mod_throws_immutable func:function
	set func.throws = "IMMUTABLE"

function __mod_throws_internal func:function
	set func.throws = "INTERNAL"

function __mod_private func:function
	set func.private

function assert a
	if !a throw "Assertion failed"

function assertEqual a b
	if a != b throw "Expected value '${string.from a }', got '${string.from b}'"

function assertType a b:string
	if {typeof a} != b throw "Expected type '${typeof a}', got '$b'"

namespace main
	function run func
		if func.private return
		local reference = {array.create 6}
		local usage = {array.create 6}
		memstat reference
		print "Run ${hashmap.path func}..."
		if func.throws
			trystart
			local e = {await {async func}}
			tryend
			if e != func.throws
				throw "Expected exception '$func.throws', got '$(e || "NONE")'"
			unset e
		else
			func
		memstat usage
		local leaks = func.leaks || 0
		for i in 0:{length reference}
			if !(leaks&(1<<i)) && (reference i) != (usage i)
				print "memstat before:" {memstr reference}
				print "memstat after:" {memstr usage}
				throw "Memory leak detected"

	function memstr a
		return "{ c:$(a 0), h:$(a 1), a:$(a 2), s:$(a 3), t:$(a 4), m:$(a 5) }"

	function enter map:namespace
		for e in {hashmap.values map}
			if {typeof e} == "function" run e
			elseif {typeof e} == "namespace" self e
			else throw "bad test tree element $e"

	enter root.tests