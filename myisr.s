%macro save_context_ISR 0
		push ax
		push bx
		push cx
		push dx
		push si
		push di
		push bp
		push es
		push ds
	%endmacro

	%macro restore_context_ISR 0
		pop ds
		pop es
		pop bp
		pop di
		pop si
		pop dx
		pop cx
		pop bx
		pop ax
	%endmacro

	%macro EOI_command 0
		mov al, 0x20
		out 0x20, al
	%endmacro

InterruptTick:
	save_context_ISR
	call YKEnterISR
	call YKTickHandler	
	sti
	call InterruptTicker
	cli
	EOI_command
	call YKExitISR
	restore_context_ISR
	iret

InterruptKeyboard:
	save_context_ISR
	call YKEnterISR
	sti
	call InterruptKeyboarder
	cli
	EOI_command
	call YKExitISR
	restore_context_ISR
	iret

InterruptReset:
	save_context_ISR
	sti
	call InterruptReseter
	cli
	EOI_command
	restore_context_ISR
	iret

gameOver:
	save_context_ISR
	call YKEnterISR	
	sti
	call gameOver_handler
	cli
	EOI_command
	call YKExitISR
	restore_context_ISR
	iret

newPiece:
	save_context_ISR
	call YKEnterISR	
	sti
	call newPiece_handler
	cli
	EOI_command
	call YKExitISR
	restore_context_ISR
	iret

cmdReceived:
	save_context_ISR
	call YKEnterISR	
	sti
	call cmdReceived_handler
	cli
	EOI_command
	call YKExitISR
	restore_context_ISR
	iret

touchDown:
	save_context_ISR
	call YKEnterISR	
	sti
	call touchDown_handler
	cli
	EOI_command
	call YKExitISR
	restore_context_ISR
	iret

lineClr:
	save_context_ISR
	call YKEnterISR	
	sti
	call lineClr_handler
	cli
	EOI_command
	call YKExitISR
	restore_context_ISR
	iret

