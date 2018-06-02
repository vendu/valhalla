.include <stdio.def>

msg:	.asciz	"hello, world\n"

.align	4

_start:
	ldr		$msg,   %r0     ; r0 = message-string address
	ldrb	*%r0, %r1   	; r1 = first character
	ldr		$STDOUT, %r2    ; r2 = file descriptor for stdout
	cmp		$0x00,  %r1     ; check if first character is 0
	biz	    done        	; yes
loop:
	inc	    %r0         	; increment address
	iwrb	%r2, %r1    	; write character to stdout
	ldrb	*%r0, %r1		; read new character
	cmp		$NUL, %r1       ; check if character non-zero
	bnz		loop            ; no
done:
	hlt

