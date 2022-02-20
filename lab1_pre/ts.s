# ts.s file:
.section .text

.global getebp
getebp:
        movl %ebp, %eax
        ret
