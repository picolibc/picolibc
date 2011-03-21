	.text
	.global	___trap0
___trap0:
	trap 31
	tst r11,r11
	bz .L0
	movhi hi(_errno),r0,r6
	st.w r11,lo(_errno)[r6]
.L0:
	jmp [r31]
