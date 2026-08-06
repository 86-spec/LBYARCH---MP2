; imgcvt.asm

; void imgCvtGrayIntToDouble(unsigned char *intImg, double *dblImg,
;                             int height, int width);

; Converts a uint8 grayscale image (0-255) to a double grayscale image
; ([0.0, 1.0]) using f = i * (1/255).


section .rodata
    align 8
    scale_factor: dq 0.0039215686274509803921568627451   ; 1/255

section .text
    global imgCvtGrayIntToDouble

imgCvtGrayIntToDouble:
    ; save the two nonvolatile registers we actually use
    push    r12
    push    r13
    sub     rsp, 8              ; padding: keeps rsp on a true 16-byte
                                 ; boundary (rsp % 16 == 8 at entry, since
                                 ; CALL just pushed an 8-byte return addr)

    mov     eax, r8d            ; eax = height
    imul    eax, r9d            ; eax = height * width
    mov     r13d, eax           ; r13d = pixel count / down-counter

    test    r13d, r13d
    jle     .done

    mov     r12, rcx            ; r12 = intImg pointer (walks +1 byte)
    mov     r10, rdx            ; r10 = dblImg pointer (walks +8 bytes)

    movsd   xmm1, [rel scale_factor]   ; load 1/255 once

.loop:
    movzx   eax, byte [r12]     ; load one pixel byte
    cvtsi2sd xmm0, eax          ; int -> double
    mulsd   xmm0, xmm1          ; scale to [0,1]
    movsd   [r10], xmm0         ; store result

    add     r12, 1
    add     r10, 8
    dec     r13d
    jnz     .loop

.done:
    add     rsp, 8
    pop     r13
    pop     r12
    ret
