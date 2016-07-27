	.sect iprog
	.def _ierAddMask
	.def _csrAddMask
	.def _ierClrMask
	.def _csrClrMask
	.def _ierClr
	.def _csrClr
	.def _enableInterrupts
        .def _disableInterrupts
        .def _invokeInterrupt
        .def _clearInterrupts
; ierClr(void)
_ierClr .asmfunc
        b b3
        mvk 1, b0
        mvc b0, ier
        nop 3
        .endasmfunc
        
; csrClr(void)
_csrClr .asmfunc
        b b3
        zero b0
        mvc b0, csr
        nop 3
        .endasmfunc
        
; ierClrMask(int bit)
_ierClrMask     .asmfunc
        mvk 1, a0
        b b3
        shl a0, a4, a0
        not a0, a0
        mvc ier, b0
        and a0, b0, b0
        mvc b0, ier
        .endasmfunc
        
; csrClrMask(int bit)
_csrClrMask     .asmfunc
        mvk 1, a0
        b b3
        shl a0, a4, a0
        not a0, a0
        mvc csr, b0
        and a0, b0, b0
        mvc b0, csr
	.endasmfunc
	; ierAddMask(int bit)
_ierAddMask	.asmfunc
	b b3
	mvk 1, a0
	shl a0, a4, a0
	mvc ier, b1
	or a0, b1, b1
	mvc b1, ier
	.endasmfunc
	
; csrAddMask(int bit)	
_csrAddMask	.asmfunc
	b b3
	mvk 1, a0
	shl a0, a4, a0
	mvc csr, b1
	or a0, b1, b1
	mvc b1, csr
	.endasmfunc
	
; invokeInterrupt(int irq)
_invokeInterrupt	.asmfunc
	mvk 1, a0	
	shl a0, a4, a0
	mv a0, b0
	mvc b0, isr
	nop 8
	nop 8
	nop 8
	b b3
	nop 5
	.endasmfunc

_clearInterrupts        .asmfunc
        mvkl 0xfff0, b0
        mvc b0, icr ; clear interrupts
        zero b0
        mvc b0, isr ; do not set any interrupts
        mvc b0, icr ; do not clear any interrupts
        b b3
        nop 5
        .endasmfunc
        
_enableInterrupts       .asmfunc
; ier
        mvc ier, b0
        b b3
        or b0, 3, b0
        mvc b0, ier
; csr
        mvc csr, b0
        or b0, 1, b0
        mvc b0, csr
        .endasmfunc
        
_disableInterrupts      .asmfunc
; csr
        b b3
        mvc csr, b0
        and -2, b0, b0
        mvc b0, csr
        nop 2
        .endasmfunc

