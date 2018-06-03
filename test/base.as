namespace array
	extern create '__arrayCreate'
	extern static '__arrayStatic'
	extern push '__arrayPush'
	extern pop '__arrayPop'
	extern shift '__arrayShift'
	extern unshift '__arrayUnshift'
	extern resize '__arrayResize'
	extern slice '__arraySlice'
	extern write '__arrayWrite'
	extern fill '__arrayFill'
	extern find '__arrayFind'
	extern expand '__arrayExpand'
	extern reverse '__arrayReverse'
	function join A:array glue?:string
		local n = {length A}
		if n == 0
			return ''
		set glue = glue || ' '
		local str = {string.from (A 0)}
		for i in 1:n
			set str = "$str$glue${string.from (A i)}"
		return str

namespace string
	extern concat '__stringConcat'
	extern find '__stringFind'
	extern slice '__stringSlice'
	function `from value:any
		if {istype value 'string'}
			return "\"$value\""
		if {istype value 'integer'}
			return {itos value}
		if {istype value 'float'}
			return {dtos value}
		if {istype value 'boolean'}
			return value?'true':'false'
		if {istype value 'undefined'}
			return 'undefined'
		if {istype value 'array'}
			return "[${array.join value}]"
		if {istype value 'hashmap'}
			return "<${hashmap.path value}:${typeof value}>"
		return "<:${typeof value}>"

namespace hashmap
	extern keys '__hashmapKeys'
	extern values '__hashmapValues'
	function path node:hashmap
		local path = {nameof node}
		while node.parent
			set node = node.parent
			set path = {nameof node}+'.'+path
		return path

extern typeof '__typeof'
extern nameof '__nameof'
extern length '__length'
extern stdout '__print'
extern itos '__itos'
extern dtos '__dtos'
extern trystart '__tryStart'
extern tryend '__tryEnd'
extern memstat '__memStat'
function print ...
	local cnt = {_argc}
	for i in 0:cnt
		local s = {_argv i}
		if {typeof s} != "string"
			set s = {string.from s}
		stdout s + (i == cnt-1 ? "\n" : " ")
