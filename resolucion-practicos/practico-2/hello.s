	.section	__TEXT,__text,regular,pure_instructions
	.build_version macos, 26, 0	sdk_version 26, 2
	.globl	_hello                          ; -- Begin function hello
	.p2align	2
_hello:                                 ; @hello
	.cfi_startproc
; %bb.0:
	adrp	x0, l_.str@PAGE
	add	x0, x0, l_.str@PAGEOFF
	ret
	.cfi_endproc
                                        ; -- End function
	.section	__TEXT,__cstring,cstring_literals
l_.str:                                 ; @.str
	.asciz	"Hello world"

.subsections_via_symbols
