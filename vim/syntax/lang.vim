syntax keyword langKeyword if else while skip stop function return and or not for include
syntax keyword langType int float string bool object
syntax keyword langBoolean true false

syntax match langNumber  /\v(\d+\.\d+|\d+|\.\d+|0x[0-9A-Fa-f]+)/ 

syntax match langComment "\~.*"

syntax match langString "\"[^\"\\]*\""

syntax region langBlockComment start="\~\~" end="\~\~" containedin=langComment

highlight def link langKeyword Keyword
highlight def link langType Type
highlight def link langBoolean Boolean
highlight def link langNumber Number
highlight def link langComment Comment
highlight def link langBlockComment Comment
highlight def link langString String


setlocal autoindent
"setlocal smartindent
