﻿; Listing generated by Microsoft (R) Optimizing Compiler Version 16.00.40219.01 

	TITLE	C:\DUII\Simulation\GUI\Widget\CHECKBOX_SkinFlex.c
	.686P
	.XMM
	include listing.inc
	.model	flat

INCLUDELIB LIBCMT
INCLUDELIB OLDNAMES

CONST	SEGMENT
__apProps DD	FLAT:__PropsEnabled
	DD	FLAT:__PropsDisabled
CONST	ENDS
_DATA	SEGMENT
__PropsEnabled DD 0707070H
	DD	0fcfcfcH
	DD	0b9b3aeH
	DD	0d2d2d2H
	DD	0f3f3f3H
	DD	0404040H
	DD	0fH
__PropsDisabled DD 0b5b2adH
	DD	0fcfcfcH
	DD	0f4f4f4H
	DD	0f4f4f4H
	DD	0f4f4f4H
	DD	08b622cH
	DD	0fH
_DATA	ENDS
PUBLIC	_CHECKBOX_DrawSkinFlex
EXTRN	_WM_SetHasTrans:PROC
EXTRN	_WM__IsEnabled:PROC
EXTRN	__RTC_CheckEsp:PROC
EXTRN	__RTC_Shutdown:PROC
EXTRN	__RTC_InitBase:PROC
_BSS	SEGMENT
__aBitmap DB	0a0H DUP (?)
_BSS	ENDS
;	COMDAT rtc$TMZ
; File c:\duii\simulation\gui\widget\checkbox_skinflex.c
rtc$TMZ	SEGMENT
__RTC_Shutdown.rtc$TMZ DD FLAT:__RTC_Shutdown
rtc$TMZ	ENDS
;	COMDAT rtc$IMZ
rtc$IMZ	SEGMENT
__RTC_InitBase.rtc$IMZ DD FLAT:__RTC_InitBase
; Function compile flags: /Odtp /RTCsu /ZI
rtc$IMZ	ENDS
;	COMDAT _CHECKBOX_DrawSkinFlex
_TEXT	SEGMENT
tv131 = -244						; size = 4
_Size$ = -44						; size = 4
_Index$ = -32						; size = 4
_pBitmap$ = -20						; size = 4
_pProps$ = -8						; size = 4
_pDrawItemInfo$ = 8					; size = 4
_CHECKBOX_DrawSkinFlex PROC				; COMDAT
; Line 300
	push	ebp
	mov	ebp, esp
	sub	esp, 244				; 000000f4H
	push	ebx
	push	esi
	push	edi
	lea	edi, DWORD PTR [ebp-244]
	mov	ecx, 61					; 0000003dH
	mov	eax, -858993460				; ccccccccH
	rep stosd
; Line 308
	mov	eax, DWORD PTR _pDrawItemInfo$[ebp]
	cmp	DWORD PTR [eax], 0
	je	SHORT $LN11@CHECKBOX_D
; Line 309
	mov	eax, DWORD PTR _pDrawItemInfo$[ebp]
	mov	ecx, DWORD PTR [eax]
	push	ecx
	call	_WM__IsEnabled
	add	esp, 4
	neg	eax
	sbb	eax, eax
	add	eax, 1
	mov	DWORD PTR _Index$[ebp], eax
; Line 310
	jmp	SHORT $LN10@CHECKBOX_D
$LN11@CHECKBOX_D:
; Line 311
	mov	DWORD PTR _Index$[ebp], 0
$LN10@CHECKBOX_D:
; Line 316
	mov	eax, DWORD PTR _Index$[ebp]
	mov	ecx, DWORD PTR __apProps[eax*4]
	mov	DWORD PTR _pProps$[ebp], ecx
; Line 317
	mov	eax, DWORD PTR _Index$[ebp]
	imul	eax, 80					; 00000050H
	add	eax, OFFSET __aBitmap
	mov	DWORD PTR _pBitmap$[ebp], eax
; Line 321
	mov	eax, DWORD PTR _Index$[ebp]
	imul	eax, 20					; 00000014H
	mov	ecx, DWORD PTR _pBitmap$[ebp]
	cmp	DWORD PTR [ecx+eax+8], 0
	jne	SHORT $LN9@CHECKBOX_D
; Line 322
	mov	eax, DWORD PTR _pProps$[ebp]
	mov	ecx, DWORD PTR [eax+24]
	sub	ecx, 6
	mov	DWORD PTR _Size$[ebp], ecx
; Line 323
	mov	eax, DWORD PTR _pBitmap$[ebp]
	mov	ecx, DWORD PTR _pProps$[ebp]
	mov	edx, DWORD PTR [ecx+20]
	mov	DWORD PTR [eax+76], edx
; Line 324
	mov	eax, DWORD PTR _Index$[ebp]
	imul	eax, 12					; 0000000cH
	mov	ecx, DWORD PTR _pBitmap$[ebp]
	lea	edx, DWORD PTR [ecx+eax+40]
	push	edx
	mov	eax, DWORD PTR _Index$[ebp]
	imul	eax, 20					; 00000014H
	add	eax, DWORD PTR _pBitmap$[ebp]
	push	eax
	mov	ecx, DWORD PTR _pBitmap$[ebp]
	add	ecx, 72					; 00000048H
	push	ecx
	mov	edx, DWORD PTR _Index$[ebp]
	push	edx
	push	1
	push	3
	mov	eax, DWORD PTR _Size$[ebp]
	cdq
	mov	ecx, 5
	idiv	ecx
	push	eax
	mov	edx, DWORD PTR _Size$[ebp]
	push	edx
	mov	eax, DWORD PTR _Size$[ebp]
	push	eax
	call	__GenCheck
	add	esp, 36					; 00000024H
$LN9@CHECKBOX_D:
; Line 329
	mov	eax, DWORD PTR _pDrawItemInfo$[ebp]
	mov	ecx, DWORD PTR [eax+4]
	mov	DWORD PTR tv131[ebp], ecx
	cmp	DWORD PTR tv131[ebp], 23		; 00000017H
	ja	SHORT $LN7@CHECKBOX_D
	mov	edx, DWORD PTR tv131[ebp]
	movzx	eax, BYTE PTR $LN14@CHECKBOX_D[edx]
	jmp	DWORD PTR $LN15@CHECKBOX_D[eax*4]
$LN6@CHECKBOX_D:
; Line 331
	mov	eax, DWORD PTR _pProps$[ebp]
	push	eax
	mov	ecx, DWORD PTR _pDrawItemInfo$[ebp]
	push	ecx
	call	__DrawBackground
	add	esp, 8
; Line 332
	jmp	SHORT $LN7@CHECKBOX_D
$LN5@CHECKBOX_D:
; Line 334
	mov	eax, DWORD PTR _pBitmap$[ebp]
	push	eax
	mov	ecx, DWORD PTR _pProps$[ebp]
	push	ecx
	mov	edx, DWORD PTR _pDrawItemInfo$[ebp]
	push	edx
	call	__DrawBitmap
	add	esp, 12					; 0000000cH
; Line 335
	jmp	SHORT $LN7@CHECKBOX_D
$LN4@CHECKBOX_D:
; Line 337
	mov	eax, DWORD PTR _pProps$[ebp]
	push	eax
	mov	ecx, DWORD PTR _pDrawItemInfo$[ebp]
	push	ecx
	call	__DrawText
	add	esp, 8
; Line 338
	jmp	SHORT $LN7@CHECKBOX_D
$LN3@CHECKBOX_D:
; Line 340
	mov	eax, DWORD PTR _pProps$[ebp]
	push	eax
	mov	ecx, DWORD PTR _pDrawItemInfo$[ebp]
	push	ecx
	call	__DrawFocus
	add	esp, 8
; Line 341
	jmp	SHORT $LN7@CHECKBOX_D
$LN2@CHECKBOX_D:
; Line 343
	mov	eax, DWORD PTR _pProps$[ebp]
	mov	eax, DWORD PTR [eax+24]
	jmp	SHORT $LN12@CHECKBOX_D
$LN1@CHECKBOX_D:
; Line 345
	mov	eax, DWORD PTR _pDrawItemInfo$[ebp]
	mov	ecx, DWORD PTR [eax]
	push	ecx
	call	_WM_SetHasTrans
	add	esp, 4
$LN7@CHECKBOX_D:
; Line 348
	xor	eax, eax
$LN12@CHECKBOX_D:
; Line 349
	pop	edi
	pop	esi
	pop	ebx
	add	esp, 244				; 000000f4H
	cmp	ebp, esp
	call	__RTC_CheckEsp
	mov	esp, ebp
	pop	ebp
	ret	0
	npad	2
$LN15@CHECKBOX_D:
	DD	$LN1@CHECKBOX_D
	DD	$LN5@CHECKBOX_D
	DD	$LN6@CHECKBOX_D
	DD	$LN3@CHECKBOX_D
	DD	$LN4@CHECKBOX_D
	DD	$LN2@CHECKBOX_D
	DD	$LN7@CHECKBOX_D
$LN14@CHECKBOX_D:
	DB	0
	DB	6
	DB	6
	DB	6
	DB	1
	DB	2
	DB	6
	DB	6
	DB	3
	DB	6
	DB	6
	DB	6
	DB	6
	DB	6
	DB	6
	DB	6
	DB	4
	DB	6
	DB	6
	DB	6
	DB	6
	DB	6
	DB	6
	DB	5
_CHECKBOX_DrawSkinFlex ENDP
_TEXT	ENDS
EXTRN	_GUI_DrawGradientV:PROC
EXTRN	_GUI_DrawRect:PROC
EXTRN	_GUI_SetColor:PROC
; Function compile flags: /Odtp /RTCsu /ZI
;	COMDAT __DrawBackground
_TEXT	SEGMENT
_i$ = -8						; size = 4
_pDrawItemInfo$ = 8					; size = 4
_pProps$ = 12						; size = 4
__DrawBackground PROC					; COMDAT
; Line 89
	push	ebp
	mov	ebp, esp
	sub	esp, 204				; 000000ccH
	push	ebx
	push	esi
	push	edi
	lea	edi, DWORD PTR [ebp-204]
	mov	ecx, 51					; 00000033H
	mov	eax, -858993460				; ccccccccH
	rep stosd
; Line 92
	mov	DWORD PTR _i$[ebp], 0
	jmp	SHORT $LN3@DrawBackgr
$LN2@DrawBackgr:
	mov	eax, DWORD PTR _i$[ebp]
	add	eax, 1
	mov	DWORD PTR _i$[ebp], eax
$LN3@DrawBackgr:
	cmp	DWORD PTR _i$[ebp], 3
	jae	SHORT $LN1@DrawBackgr
; Line 93
	mov	eax, DWORD PTR _i$[ebp]
	mov	ecx, DWORD PTR _pProps$[ebp]
	mov	edx, DWORD PTR [ecx+eax*4]
	push	edx
	call	_GUI_SetColor
	add	esp, 4
; Line 97
	mov	eax, DWORD PTR _pDrawItemInfo$[ebp]
	mov	ecx, DWORD PTR [eax+12]
	mov	edx, DWORD PTR _pProps$[ebp]
	add	ecx, DWORD PTR [edx+24]
	sub	ecx, DWORD PTR _i$[ebp]
	sub	ecx, 1
	push	ecx
	mov	eax, DWORD PTR _pDrawItemInfo$[ebp]
	mov	ecx, DWORD PTR [eax+12]
	mov	edx, DWORD PTR _pProps$[ebp]
	add	ecx, DWORD PTR [edx+24]
	sub	ecx, DWORD PTR _i$[ebp]
	sub	ecx, 1
	push	ecx
	mov	eax, DWORD PTR _pDrawItemInfo$[ebp]
	mov	ecx, DWORD PTR [eax+16]
	add	ecx, DWORD PTR _i$[ebp]
	push	ecx
	mov	edx, DWORD PTR _pDrawItemInfo$[ebp]
	mov	eax, DWORD PTR [edx+12]
	add	eax, DWORD PTR _i$[ebp]
	push	eax
	call	_GUI_DrawRect
	add	esp, 16					; 00000010H
; Line 98
	jmp	SHORT $LN2@DrawBackgr
$LN1@DrawBackgr:
; Line 103
	mov	eax, DWORD PTR _pProps$[ebp]
	mov	ecx, DWORD PTR [eax+16]
	push	ecx
	mov	edx, DWORD PTR _pProps$[ebp]
	mov	eax, DWORD PTR [edx+12]
	push	eax
	mov	ecx, DWORD PTR _pDrawItemInfo$[ebp]
	mov	edx, DWORD PTR [ecx+12]
	mov	eax, DWORD PTR _pProps$[ebp]
	add	edx, DWORD PTR [eax+24]
	sub	edx, DWORD PTR _i$[ebp]
	sub	edx, 1
	push	edx
	mov	ecx, DWORD PTR _pDrawItemInfo$[ebp]
	mov	edx, DWORD PTR [ecx+12]
	mov	eax, DWORD PTR _pProps$[ebp]
	add	edx, DWORD PTR [eax+24]
	sub	edx, DWORD PTR _i$[ebp]
	sub	edx, 1
	push	edx
	mov	ecx, DWORD PTR _pDrawItemInfo$[ebp]
	mov	edx, DWORD PTR [ecx+16]
	add	edx, DWORD PTR _i$[ebp]
	push	edx
	mov	eax, DWORD PTR _pDrawItemInfo$[ebp]
	mov	ecx, DWORD PTR [eax+12]
	add	ecx, DWORD PTR _i$[ebp]
	push	ecx
	call	_GUI_DrawGradientV
	add	esp, 24					; 00000018H
; Line 104
	pop	edi
	pop	esi
	pop	ebx
	add	esp, 204				; 000000ccH
	cmp	ebp, esp
	call	__RTC_CheckEsp
	mov	esp, ebp
	pop	ebp
	ret	0
__DrawBackground ENDP
_TEXT	ENDS
EXTRN	_GUI__memset:PROC
EXTRN	_GUI_ALLOC_GetFixedBlock:PROC
; Function compile flags: /Odtp /RTCsu /ZI
;	COMDAT __GenCheck
_TEXT	SEGMENT
_State$ = -116						; size = 4
_i$ = -104						; size = 4
_xs$ = -92						; size = 4
_y$ = -80						; size = 4
_x$ = -68						; size = 4
_h2$ = -56						; size = 4
_b2$ = -44						; size = 4
_BytesRequired$ = -32					; size = 4
_BytesPerLine$ = -20					; size = 4
_pData$ = -8						; size = 4
_b$ = 8							; size = 4
_h$ = 12						; size = 4
_s$ = 16						; size = 4
_w$ = 20						; size = 4
_Frame$ = 24						; size = 4
_Dither$ = 28						; size = 4
_pColor$ = 32						; size = 4
_pBitmap$ = 36						; size = 4
_pPalette$ = 40						; size = 4
__GenCheck PROC						; COMDAT
; Line 123
	push	ebp
	mov	ebp, esp
	sub	esp, 312				; 00000138H
	push	ebx
	push	esi
	push	edi
	lea	edi, DWORD PTR [ebp-312]
	mov	ecx, 78					; 0000004eH
	mov	eax, -858993460				; ccccccccH
	rep stosd
; Line 131
	mov	eax, DWORD PTR _b$[ebp]
	cdq
	sub	eax, edx
	sar	eax, 1
	mov	DWORD PTR _b2$[ebp], eax
; Line 132
	mov	eax, DWORD PTR _h$[ebp]
	cdq
	sub	eax, edx
	sar	eax, 1
	mov	DWORD PTR _h2$[ebp], eax
; Line 133
	mov	eax, DWORD PTR _h2$[ebp]
	cdq
	sub	eax, edx
	sar	eax, 1
	mov	ecx, DWORD PTR _h2$[ebp]
	sub	ecx, eax
	add	ecx, DWORD PTR _s$[ebp]
	mov	eax, DWORD PTR _w$[ebp]
	cdq
	sub	eax, edx
	sar	eax, 1
	sub	ecx, eax
	add	ecx, DWORD PTR _Frame$[ebp]
	mov	DWORD PTR _y$[ebp], ecx
; Line 134
	mov	eax, DWORD PTR _b2$[ebp]
	sub	eax, DWORD PTR _s$[ebp]
	mov	DWORD PTR _xs$[ebp], eax
; Line 138
	mov	eax, DWORD PTR _b$[ebp]
	add	eax, 7
	cdq
	and	edx, 7
	add	eax, edx
	sar	eax, 3
	mov	DWORD PTR _BytesPerLine$[ebp], eax
; Line 139
	mov	eax, DWORD PTR _BytesPerLine$[ebp]
	imul	eax, DWORD PTR _h$[ebp]
	mov	DWORD PTR _BytesRequired$[ebp], eax
; Line 140
	mov	eax, DWORD PTR _BytesRequired$[ebp]
	push	eax
	call	_GUI_ALLOC_GetFixedBlock
	add	esp, 4
	mov	DWORD PTR _pData$[ebp], eax
; Line 141
	mov	eax, DWORD PTR _BytesRequired$[ebp]
	push	eax
	push	0
	mov	ecx, DWORD PTR _pData$[ebp]
	push	ecx
	call	_GUI__memset
	add	esp, 12					; 0000000cH
; Line 145
	mov	eax, DWORD PTR _Frame$[ebp]
	mov	DWORD PTR _x$[ebp], eax
	jmp	SHORT $LN14@GenCheck
$LN13@GenCheck:
	mov	eax, DWORD PTR _x$[ebp]
	add	eax, 1
	mov	DWORD PTR _x$[ebp], eax
	mov	ecx, DWORD PTR _y$[ebp]
	add	ecx, 1
	mov	DWORD PTR _y$[ebp], ecx
$LN14@GenCheck:
	mov	eax, DWORD PTR _x$[ebp]
	cmp	eax, DWORD PTR _xs$[ebp]
	jge	SHORT $LN12@GenCheck
; Line 146
	mov	DWORD PTR _State$[ebp], 1
; Line 147
	mov	eax, DWORD PTR _y$[ebp]
	mov	DWORD PTR _i$[ebp], eax
	jmp	SHORT $LN11@GenCheck
$LN10@GenCheck:
	mov	eax, DWORD PTR _i$[ebp]
	add	eax, 1
	mov	DWORD PTR _i$[ebp], eax
$LN11@GenCheck:
	mov	eax, DWORD PTR _y$[ebp]
	add	eax, DWORD PTR _w$[ebp]
	cmp	DWORD PTR _i$[ebp], eax
	jge	SHORT $LN9@GenCheck
; Line 148
	cmp	DWORD PTR _State$[ebp], 0
	je	SHORT $LN8@GenCheck
	mov	eax, DWORD PTR _i$[ebp]
	cmp	eax, DWORD PTR _h$[ebp]
	jge	SHORT $LN8@GenCheck
; Line 149
	mov	eax, DWORD PTR _pData$[ebp]
	push	eax
	mov	ecx, DWORD PTR _BytesPerLine$[ebp]
	push	ecx
	mov	edx, DWORD PTR _i$[ebp]
	push	edx
	mov	eax, DWORD PTR _x$[ebp]
	push	eax
	call	__SetBitmapPixel_1bpp
	add	esp, 16					; 00000010H
$LN8@GenCheck:
; Line 151
	mov	eax, DWORD PTR _State$[ebp]
	xor	eax, DWORD PTR _Dither$[ebp]
	mov	DWORD PTR _State$[ebp], eax
; Line 152
	jmp	SHORT $LN10@GenCheck
$LN9@GenCheck:
; Line 153
	jmp	SHORT $LN13@GenCheck
$LN12@GenCheck:
; Line 154
	jmp	SHORT $LN7@GenCheck
$LN6@GenCheck:
	mov	eax, DWORD PTR _x$[ebp]
	add	eax, 1
	mov	DWORD PTR _x$[ebp], eax
	mov	ecx, DWORD PTR _y$[ebp]
	sub	ecx, 1
	mov	DWORD PTR _y$[ebp], ecx
$LN7@GenCheck:
	mov	eax, DWORD PTR _b$[ebp]
	sub	eax, DWORD PTR _Frame$[ebp]
	cmp	DWORD PTR _x$[ebp], eax
	jge	SHORT $LN5@GenCheck
; Line 155
	mov	DWORD PTR _State$[ebp], 1
; Line 156
	mov	eax, DWORD PTR _y$[ebp]
	mov	DWORD PTR _i$[ebp], eax
	jmp	SHORT $LN4@GenCheck
$LN3@GenCheck:
	mov	eax, DWORD PTR _i$[ebp]
	add	eax, 1
	mov	DWORD PTR _i$[ebp], eax
$LN4@GenCheck:
	mov	eax, DWORD PTR _y$[ebp]
	add	eax, DWORD PTR _w$[ebp]
	cmp	DWORD PTR _i$[ebp], eax
	jge	SHORT $LN2@GenCheck
; Line 157
	cmp	DWORD PTR _State$[ebp], 0
	je	SHORT $LN1@GenCheck
	mov	eax, DWORD PTR _i$[ebp]
	cmp	eax, DWORD PTR _h$[ebp]
	jge	SHORT $LN1@GenCheck
; Line 158
	mov	eax, DWORD PTR _pData$[ebp]
	push	eax
	mov	ecx, DWORD PTR _BytesPerLine$[ebp]
	push	ecx
	mov	edx, DWORD PTR _i$[ebp]
	push	edx
	mov	eax, DWORD PTR _x$[ebp]
	push	eax
	call	__SetBitmapPixel_1bpp
	add	esp, 16					; 00000010H
$LN1@GenCheck:
; Line 160
	mov	eax, DWORD PTR _State$[ebp]
	xor	eax, DWORD PTR _Dither$[ebp]
	mov	DWORD PTR _State$[ebp], eax
; Line 161
	jmp	SHORT $LN3@GenCheck
$LN2@GenCheck:
; Line 162
	jmp	SHORT $LN6@GenCheck
$LN5@GenCheck:
; Line 166
	mov	eax, DWORD PTR _pPalette$[ebp]
	mov	DWORD PTR [eax], 2
; Line 167
	mov	eax, DWORD PTR _pPalette$[ebp]
	mov	BYTE PTR [eax+4], 1
; Line 168
	mov	eax, DWORD PTR _pPalette$[ebp]
	mov	ecx, DWORD PTR _pColor$[ebp]
	mov	DWORD PTR [eax+8], ecx
; Line 172
	mov	eax, 1
	mov	ecx, DWORD PTR _pBitmap$[ebp]
	mov	WORD PTR [ecx+6], ax
; Line 173
	mov	eax, DWORD PTR _pBitmap$[ebp]
	mov	cx, WORD PTR _BytesPerLine$[ebp]
	mov	WORD PTR [eax+4], cx
; Line 174
	mov	eax, DWORD PTR _pBitmap$[ebp]
	mov	ecx, DWORD PTR _pData$[ebp]
	mov	DWORD PTR [eax+8], ecx
; Line 175
	mov	eax, DWORD PTR _pBitmap$[ebp]
	mov	DWORD PTR [eax+16], 0
; Line 176
	mov	eax, DWORD PTR _pBitmap$[ebp]
	mov	ecx, DWORD PTR _pPalette$[ebp]
	mov	DWORD PTR [eax+12], ecx
; Line 177
	mov	eax, DWORD PTR _pBitmap$[ebp]
	mov	cx, WORD PTR _b$[ebp]
	mov	WORD PTR [eax], cx
; Line 178
	mov	eax, DWORD PTR _pBitmap$[ebp]
	mov	cx, WORD PTR _h$[ebp]
	mov	WORD PTR [eax+2], cx
; Line 179
	pop	edi
	pop	esi
	pop	ebx
	add	esp, 312				; 00000138H
	cmp	ebp, esp
	call	__RTC_CheckEsp
	mov	esp, ebp
	pop	ebp
	ret	0
__GenCheck ENDP
; Function compile flags: /Odtp /RTCsu /ZI
_TEXT	ENDS
;	COMDAT __SetBitmapPixel_1bpp
_TEXT	SEGMENT
_Mask$ = -17						; size = 1
_Off$ = -8						; size = 4
_x$ = 8							; size = 4
_y$ = 12						; size = 4
_BytesPerLine$ = 16					; size = 4
_pData$ = 20						; size = 4
__SetBitmapPixel_1bpp PROC				; COMDAT
; Line 110
	push	ebp
	mov	ebp, esp
	sub	esp, 216				; 000000d8H
	push	ebx
	push	esi
	push	edi
	lea	edi, DWORD PTR [ebp-216]
	mov	ecx, 54					; 00000036H
	mov	eax, -858993460				; ccccccccH
	rep stosd
; Line 114
	mov	eax, DWORD PTR _BytesPerLine$[ebp]
	imul	eax, DWORD PTR _y$[ebp]
	mov	ecx, DWORD PTR _x$[ebp]
	sar	ecx, 3
	add	eax, ecx
	mov	DWORD PTR _Off$[ebp], eax
; Line 115
	mov	ecx, DWORD PTR _x$[ebp]
	and	ecx, 7
	mov	eax, 128				; 00000080H
	sar	eax, cl
	mov	BYTE PTR _Mask$[ebp], al
; Line 116
	movzx	eax, BYTE PTR _Mask$[ebp]
	mov	ecx, DWORD PTR _pData$[ebp]
	add	ecx, DWORD PTR _Off$[ebp]
	movzx	edx, BYTE PTR [ecx]
	or	edx, eax
	mov	eax, DWORD PTR _pData$[ebp]
	add	eax, DWORD PTR _Off$[ebp]
	mov	BYTE PTR [eax], dl
; Line 117
	pop	edi
	pop	esi
	pop	ebx
	mov	esp, ebp
	pop	ebp
	ret	0
__SetBitmapPixel_1bpp ENDP
_TEXT	ENDS
EXTRN	_GUI_DrawBitmap:PROC
; Function compile flags: /Odtp /RTCsu /ZI
;	COMDAT __DrawBitmap
_TEXT	SEGMENT
_Size$ = -20						; size = 4
_Index$ = -8						; size = 4
_pDrawItemInfo$ = 8					; size = 4
_pProps$ = 12						; size = 4
_pBitmap$ = 16						; size = 4
__DrawBitmap PROC					; COMDAT
; Line 185
	push	ebp
	mov	ebp, esp
	sub	esp, 216				; 000000d8H
	push	ebx
	push	esi
	push	edi
	lea	edi, DWORD PTR [ebp-216]
	mov	ecx, 54					; 00000036H
	mov	eax, -858993460				; ccccccccH
	rep stosd
; Line 188
	mov	eax, DWORD PTR _pDrawItemInfo$[ebp]
	mov	ecx, DWORD PTR [eax+8]
	sub	ecx, 1
	mov	DWORD PTR _Index$[ebp], ecx
; Line 189
	mov	eax, DWORD PTR _pProps$[ebp]
	mov	ecx, DWORD PTR [eax+24]
	sub	ecx, 6
	mov	DWORD PTR _Size$[ebp], ecx
; Line 193
	mov	eax, DWORD PTR _Index$[ebp]
	imul	eax, 20					; 00000014H
	mov	ecx, DWORD PTR _pBitmap$[ebp]
	cmp	DWORD PTR [ecx+eax+8], 0
	jne	SHORT $LN1@DrawBitmap
; Line 194
	mov	eax, DWORD PTR _pBitmap$[ebp]
	mov	ecx, DWORD PTR _pProps$[ebp]
	mov	edx, DWORD PTR [ecx+20]
	mov	DWORD PTR [eax+76], edx
; Line 195
	mov	eax, DWORD PTR _Index$[ebp]
	imul	eax, 12					; 0000000cH
	mov	ecx, DWORD PTR _pBitmap$[ebp]
	lea	edx, DWORD PTR [ecx+eax+40]
	push	edx
	mov	eax, DWORD PTR _Index$[ebp]
	imul	eax, 20					; 00000014H
	add	eax, DWORD PTR _pBitmap$[ebp]
	push	eax
	mov	ecx, DWORD PTR _pBitmap$[ebp]
	add	ecx, 72					; 00000048H
	push	ecx
	mov	edx, DWORD PTR _Index$[ebp]
	push	edx
	push	1
	push	3
	mov	eax, DWORD PTR _Size$[ebp]
	cdq
	mov	ecx, 5
	idiv	ecx
	push	eax
	mov	edx, DWORD PTR _Size$[ebp]
	push	edx
	mov	eax, DWORD PTR _Size$[ebp]
	push	eax
	call	__GenCheck
	add	esp, 36					; 00000024H
$LN1@DrawBitmap:
; Line 200
	push	3
	push	3
	mov	eax, DWORD PTR _Index$[ebp]
	imul	eax, 20					; 00000014H
	add	eax, DWORD PTR _pBitmap$[ebp]
	push	eax
	call	_GUI_DrawBitmap
	add	esp, 12					; 0000000cH
; Line 201
	pop	edi
	pop	esi
	pop	ebx
	add	esp, 216				; 000000d8H
	cmp	ebp, esp
	call	__RTC_CheckEsp
	mov	esp, ebp
	pop	ebp
	ret	0
__DrawBitmap ENDP
_TEXT	ENDS
EXTRN	_GUI_DispStringInRect:PROC
EXTRN	_GUI_SetTextMode:PROC
EXTRN	_GUI_SetFont:PROC
EXTRN	_WM_GetClientRect:PROC
EXTRN	_GUI_ALLOC_UnlockH:PROC
EXTRN	_CHECKBOX_LockH:PROC
EXTRN	@_RTC_CheckStackVars@8:PROC
; Function compile flags: /Odtp /RTCsu /ZI
;	COMDAT __DrawText
_TEXT	SEGMENT
_Color$ = -84						; size = 4
_pFont$ = -72						; size = 4
_s$ = -60						; size = 4
_Rect$ = -48						; size = 8
_Spacing$ = -32						; size = 4
_Align$ = -20						; size = 4
_pObj$ = -8						; size = 4
_pDrawItemInfo$ = 8					; size = 4
_pProps$ = 12						; size = 4
__DrawText PROC						; COMDAT
; Line 207
	push	ebp
	mov	ebp, esp
	sub	esp, 280				; 00000118H
	push	ebx
	push	esi
	push	edi
	lea	edi, DWORD PTR [ebp-280]
	mov	ecx, 70					; 00000046H
	mov	eax, -858993460				; ccccccccH
	rep stosd
; Line 218
	mov	eax, DWORD PTR _pDrawItemInfo$[ebp]
	mov	ecx, DWORD PTR [eax]
	push	ecx
	call	_CHECKBOX_LockH
	add	esp, 4
	mov	DWORD PTR _pObj$[ebp], eax
; Line 219
	mov	eax, DWORD PTR _pObj$[ebp]
	movsx	ecx, WORD PTR [eax+88]
	mov	DWORD PTR _Align$[ebp], ecx
; Line 220
	mov	eax, DWORD PTR _pObj$[ebp]
	mov	ecx, DWORD PTR [eax+56]
	mov	DWORD PTR _pFont$[ebp], ecx
; Line 221
	mov	eax, DWORD PTR _pObj$[ebp]
	movzx	ecx, BYTE PTR [eax+90]
	mov	DWORD PTR _Spacing$[ebp], ecx
; Line 222
	mov	eax, DWORD PTR _pObj$[ebp]
	mov	ecx, DWORD PTR [eax+72]
	mov	DWORD PTR _Color$[ebp], ecx
; Line 223
	call	_GUI_ALLOC_UnlockH
	mov	DWORD PTR _pObj$[ebp], 0
; Line 227
	lea	eax, DWORD PTR _Rect$[ebp]
	push	eax
	call	_WM_GetClientRect
	add	esp, 4
; Line 228
	mov	eax, DWORD PTR _pProps$[ebp]
	mov	ecx, DWORD PTR [eax+24]
	add	ecx, DWORD PTR _Spacing$[ebp]
	movsx	edx, WORD PTR _Rect$[ebp]
	add	edx, ecx
	mov	WORD PTR _Rect$[ebp], dx
; Line 229
	mov	eax, DWORD PTR _pDrawItemInfo$[ebp]
	mov	ecx, DWORD PTR [eax+28]
	mov	DWORD PTR _s$[ebp], ecx
; Line 230
	mov	eax, DWORD PTR _pFont$[ebp]
	push	eax
	call	_GUI_SetFont
	add	esp, 4
; Line 231
	push	2
	call	_GUI_SetTextMode
	add	esp, 4
; Line 232
	mov	eax, DWORD PTR _Color$[ebp]
	push	eax
	call	_GUI_SetColor
	add	esp, 4
; Line 233
	mov	eax, DWORD PTR _Align$[ebp]
	push	eax
	lea	ecx, DWORD PTR _Rect$[ebp]
	push	ecx
	mov	edx, DWORD PTR _s$[ebp]
	push	edx
	call	_GUI_DispStringInRect
	add	esp, 12					; 0000000cH
; Line 234
	push	edx
	mov	ecx, ebp
	push	eax
	lea	edx, DWORD PTR $LN5@DrawText
	call	@_RTC_CheckStackVars@8
	pop	eax
	pop	edx
	pop	edi
	pop	esi
	pop	ebx
	add	esp, 280				; 00000118H
	cmp	ebp, esp
	call	__RTC_CheckEsp
	mov	esp, ebp
	pop	ebp
	ret	0
$LN5@DrawText:
	DD	1
	DD	$LN4@DrawText
$LN4@DrawText:
	DD	-48					; ffffffd0H
	DD	8
	DD	$LN3@DrawText
$LN3@DrawText:
	DB	82					; 00000052H
	DB	101					; 00000065H
	DB	99					; 00000063H
	DB	116					; 00000074H
	DB	0
__DrawText ENDP
_TEXT	ENDS
EXTRN	_GUI_DrawFocusRect:PROC
EXTRN	_GUI_GetYSizeOfFont:PROC
EXTRN	_GUI_GetStringDistX:PROC
; Function compile flags: /Odtp /RTCsu /ZI
;	COMDAT __DrawFocus
_TEXT	SEGMENT
tv128 = -308						; size = 4
tv84 = -308						; size = 4
_pFont$ = -108						; size = 4
_s$ = -96						; size = 4
_FocusColor$ = -84					; size = 4
_Rect$ = -72						; size = 8
_ySizeText$ = -56					; size = 4
_xSizeText$ = -44					; size = 4
_Spacing$ = -32						; size = 4
_Align$ = -20						; size = 4
_pObj$ = -8						; size = 4
_pDrawItemInfo$ = 8					; size = 4
_pProps$ = 12						; size = 4
__DrawFocus PROC					; COMDAT
; Line 240
	push	ebp
	mov	ebp, esp
	sub	esp, 308				; 00000134H
	push	ebx
	push	esi
	push	edi
	lea	edi, DWORD PTR [ebp-308]
	mov	ecx, 77					; 0000004dH
	mov	eax, -858993460				; ccccccccH
	rep stosd
; Line 251
	mov	eax, DWORD PTR _pDrawItemInfo$[ebp]
	mov	ecx, DWORD PTR [eax]
	push	ecx
	call	_CHECKBOX_LockH
	add	esp, 4
	mov	DWORD PTR _pObj$[ebp], eax
; Line 252
	mov	eax, DWORD PTR _pObj$[ebp]
	movsx	ecx, WORD PTR [eax+88]
	mov	DWORD PTR _Align$[ebp], ecx
; Line 253
	mov	eax, DWORD PTR _pObj$[ebp]
	mov	ecx, DWORD PTR [eax+56]
	mov	DWORD PTR _pFont$[ebp], ecx
; Line 254
	mov	eax, DWORD PTR _pObj$[ebp]
	movzx	ecx, BYTE PTR [eax+90]
	mov	DWORD PTR _Spacing$[ebp], ecx
; Line 255
	mov	eax, DWORD PTR _pObj$[ebp]
	mov	ecx, DWORD PTR [eax+76]
	mov	DWORD PTR _FocusColor$[ebp], ecx
; Line 256
	call	_GUI_ALLOC_UnlockH
	mov	DWORD PTR _pObj$[ebp], 0
; Line 260
	lea	eax, DWORD PTR _Rect$[ebp]
	push	eax
	call	_WM_GetClientRect
	add	esp, 4
; Line 261
	mov	eax, DWORD PTR _pProps$[ebp]
	mov	ecx, DWORD PTR [eax+24]
	add	ecx, DWORD PTR _Spacing$[ebp]
	movsx	edx, WORD PTR _Rect$[ebp]
	add	edx, ecx
	mov	WORD PTR _Rect$[ebp], dx
; Line 262
	mov	eax, DWORD PTR _pDrawItemInfo$[ebp]
	mov	ecx, DWORD PTR [eax+28]
	mov	DWORD PTR _s$[ebp], ecx
; Line 263
	mov	eax, DWORD PTR _s$[ebp]
	push	eax
	call	_GUI_GetStringDistX
	add	esp, 4
	mov	DWORD PTR _xSizeText$[ebp], eax
; Line 264
	mov	eax, DWORD PTR _pFont$[ebp]
	push	eax
	call	_GUI_GetYSizeOfFont
	add	esp, 4
	mov	DWORD PTR _ySizeText$[ebp], eax
; Line 265
	mov	eax, DWORD PTR _Align$[ebp]
	and	eax, -4					; fffffffcH
	mov	DWORD PTR tv84[ebp], eax
	cmp	DWORD PTR tv84[ebp], 4
	je	SHORT $LN5@DrawFocus
	cmp	DWORD PTR tv84[ebp], 12			; 0000000cH
	je	SHORT $LN6@DrawFocus
	jmp	SHORT $LN7@DrawFocus
$LN6@DrawFocus:
; Line 267
	movsx	eax, WORD PTR _Rect$[ebp+6]
	sub	eax, DWORD PTR _ySizeText$[ebp]
	add	eax, 1
	cdq
	sub	eax, edx
	sar	eax, 1
	mov	WORD PTR _Rect$[ebp+2], ax
; Line 268
	jmp	SHORT $LN7@DrawFocus
$LN5@DrawFocus:
; Line 270
	movsx	eax, WORD PTR _Rect$[ebp+6]
	sub	eax, DWORD PTR _ySizeText$[ebp]
	mov	WORD PTR _Rect$[ebp+2], ax
$LN7@DrawFocus:
; Line 273
	mov	eax, DWORD PTR _Align$[ebp]
	and	eax, -13				; fffffff3H
	mov	DWORD PTR tv128[ebp], eax
	cmp	DWORD PTR tv128[ebp], 1
	je	SHORT $LN1@DrawFocus
	cmp	DWORD PTR tv128[ebp], 2
	je	SHORT $LN2@DrawFocus
	jmp	SHORT $LN3@DrawFocus
$LN2@DrawFocus:
; Line 275
	movsx	eax, WORD PTR _Rect$[ebp+4]
	movsx	ecx, WORD PTR _Rect$[ebp]
	sub	eax, ecx
	sub	eax, DWORD PTR _xSizeText$[ebp]
	cdq
	sub	eax, edx
	sar	eax, 1
	movsx	edx, WORD PTR _Rect$[ebp]
	add	edx, eax
	mov	WORD PTR _Rect$[ebp], dx
; Line 276
	jmp	SHORT $LN3@DrawFocus
$LN1@DrawFocus:
; Line 278
	movsx	eax, WORD PTR _Rect$[ebp+4]
	movsx	ecx, WORD PTR _Rect$[ebp]
	sub	eax, ecx
	sub	eax, DWORD PTR _xSizeText$[ebp]
	movsx	edx, WORD PTR _Rect$[ebp]
	add	edx, eax
	mov	WORD PTR _Rect$[ebp], dx
$LN3@DrawFocus:
; Line 281
	movsx	eax, WORD PTR _Rect$[ebp]
	mov	ecx, DWORD PTR _xSizeText$[ebp]
	lea	edx, DWORD PTR [eax+ecx-1]
	mov	WORD PTR _Rect$[ebp+4], dx
; Line 282
	movsx	eax, WORD PTR _Rect$[ebp+2]
	mov	ecx, DWORD PTR _ySizeText$[ebp]
	lea	edx, DWORD PTR [eax+ecx-1]
	mov	WORD PTR _Rect$[ebp+6], dx
; Line 286
	mov	eax, DWORD PTR _FocusColor$[ebp]
	push	eax
	call	_GUI_SetColor
	add	esp, 4
; Line 287
	push	-1
	lea	eax, DWORD PTR _Rect$[ebp]
	push	eax
	call	_GUI_DrawFocusRect
	add	esp, 8
; Line 288
	push	edx
	mov	ecx, ebp
	push	eax
	lea	edx, DWORD PTR $LN13@DrawFocus
	call	@_RTC_CheckStackVars@8
	pop	eax
	pop	edx
	pop	edi
	pop	esi
	pop	ebx
	add	esp, 308				; 00000134H
	cmp	ebp, esp
	call	__RTC_CheckEsp
	mov	esp, ebp
	pop	ebp
	ret	0
	npad	3
$LN13@DrawFocus:
	DD	1
	DD	$LN12@DrawFocus
$LN12@DrawFocus:
	DD	-72					; ffffffb8H
	DD	8
	DD	$LN11@DrawFocus
$LN11@DrawFocus:
	DB	82					; 00000052H
	DB	101					; 00000065H
	DB	99					; 00000063H
	DB	116					; 00000074H
	DB	0
__DrawFocus ENDP
_TEXT	ENDS
PUBLIC	_CHECKBOX_SetSkinFlexProps
; Function compile flags: /Odtp /RTCsu /ZI
;	COMDAT _CHECKBOX_SetSkinFlexProps
_TEXT	SEGMENT
_pProps$ = 8						; size = 4
_Index$ = 12						; size = 4
_CHECKBOX_SetSkinFlexProps PROC				; COMDAT
; Line 355
	push	ebp
	mov	ebp, esp
	sub	esp, 192				; 000000c0H
	push	ebx
	push	esi
	push	edi
	lea	edi, DWORD PTR [ebp-192]
	mov	ecx, 48					; 00000030H
	mov	eax, -858993460				; ccccccccH
	rep stosd
; Line 356
	cmp	DWORD PTR _Index$[ebp], 2
	jae	SHORT $LN2@CHECKBOX_S
; Line 357
	mov	esi, DWORD PTR _pProps$[ebp]
	mov	eax, DWORD PTR _Index$[ebp]
	mov	edi, DWORD PTR __apProps[eax*4]
	mov	ecx, 7
	rep movsd
$LN2@CHECKBOX_S:
; Line 359
	pop	edi
	pop	esi
	pop	ebx
	mov	esp, ebp
	pop	ebp
	ret	0
_CHECKBOX_SetSkinFlexProps ENDP
_TEXT	ENDS
PUBLIC	_CHECKBOX_GetSkinFlexProps
; Function compile flags: /Odtp /RTCsu /ZI
;	COMDAT _CHECKBOX_GetSkinFlexProps
_TEXT	SEGMENT
_pProps$ = 8						; size = 4
_Index$ = 12						; size = 4
_CHECKBOX_GetSkinFlexProps PROC				; COMDAT
; Line 365
	push	ebp
	mov	ebp, esp
	sub	esp, 192				; 000000c0H
	push	ebx
	push	esi
	push	edi
	lea	edi, DWORD PTR [ebp-192]
	mov	ecx, 48					; 00000030H
	mov	eax, -858993460				; ccccccccH
	rep stosd
; Line 366
	cmp	DWORD PTR _Index$[ebp], 2
	jae	SHORT $LN2@CHECKBOX_G
; Line 367
	mov	eax, DWORD PTR _Index$[ebp]
	mov	esi, DWORD PTR __apProps[eax*4]
	mov	ecx, 7
	mov	edi, DWORD PTR _pProps$[ebp]
	rep movsd
$LN2@CHECKBOX_G:
; Line 369
	pop	edi
	pop	esi
	pop	ebx
	mov	esp, ebp
	pop	ebp
	ret	0
_CHECKBOX_GetSkinFlexProps ENDP
_TEXT	ENDS
END
