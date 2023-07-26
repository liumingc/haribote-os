; naskfunc
; TAB=4

[FORMAT "WCOFF"]				; オブジェクトファイルを作るモード	
[BITS 32]						; 32ビットモード用の機械語を作らせる
[INSTRSET "i486"]

; オブジェクトファイルのための情報

[FILE "naskfunc.nas"]			; ソースファイル名情報

		GLOBAL	_io_hlt			; このプログラムに含まれる関数名
		GLOBAL	_io_in8, _io_in16, _io_in32
		GLOBAL	_io_out8, _io_out16, _io_out32
		GLOBAL	_io_cli, _io_sti
		GLOBAL	_io_load_eflags, _io_store_eflags


; 以下は実際の関数

[SECTION .text]		; オブジェクトファイルではこれを書いてからプログラムを書く

_io_hlt:	; void io_hlt(void);
		HLT
		RET


_io_cli:
	CLI
	RET

_io_sti:
	STI
	RET

_io_in8:	; int io_in8(int port)
	MOV EDX, [ESP+4]
	MOV EAX, 0
	IN AL, DX
	RET

_io_in16:	; int io_in16(int port)
	MOV EDX, [ESP+4]
	MOV EAX, 0
	IN AX, DX
	RET

_io_in32:	; int io_in16(int port)
	MOV EDX, [ESP+4]
	IN EAX, DX
	RET

_io_out8: ; void io_out8(int port, char data)
	MOV EDX, [ESP+4] ; port
	MOV AL, [ESP+8] ; data
	OUT DX, AL
	RET

_io_out16: ; void io_out16(int port, char data)
	MOV EDX, [ESP+4] ; port
	MOV AX, [ESP+8] ; data
	OUT DX, AX
	RET

_io_out32: ; void io_out32(int port, char data)
	MOV EDX, [ESP+4] ; port
	MOV AX, [ESP+8] ; data
	OUT DX, EAX
	RET

_io_load_eflags: ; int io_load_eflags();
	PUSHFD
	POP EAX
	RET

_io_store_eflags: ; void io_store_eflags(int eflags)
	MOV EAX, [ESP+4] ; eflags
	PUSH EAX
	POPFD
	RET

