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
		if !{array.compare reference usage}
			print "memstat before:" reference
			print "memstat after: " usage
			throw "Memory leak detected"

	function enter map:namespace
		for e in {hashmap.values map}
			if {typeof e} == "function" run e
			elseif {typeof e} == "namespace" self e
			else throw "bad test tree element $e"

	enter root.tests