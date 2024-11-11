; The idea is that we encode a string of characters
; in Morse code into binary numbers as follows:
; 1) We replace '.' with 0, and '-' with 1
; 2) We prepend 1 as the most significant bit
; Example: -.- is encoded as 1101
; We create three helper tables:
; - numbersLookup is for converting digits into Morse-encoded binary
; - lettersLookup is for converting uppercase English letters into Morse-encoded binary
; - morseLookup is for converting Morse-encoded binary back into uppercase English letters and digits
;   (if 0 appears here, it means that the given sequence of dots and dashes
;    does not represent any digit or letter of the English alphabet)
;
; (!) Additionally, in the numbersLookup and lettersLookup tables, the Morse codes are "reversed"
; (i.e. 111110 corresponds to .----) - this simplifies the implementation

bits 64

section .data

    NUMOFFSET equ 48        ; in ASCII, digits have codes from 48 to 57

    numbersLookup db 0b111111, 0b111110, 0b111100, 0b111000
                  db 0b110000, 0b100000, 0b100001, 0b100011
                  db 0b100111, 0b101111

    LETTERSOFFSET equ 65    ; in ASCII, uppercase letters have codes from 65 to 90

    lettersLookup db 0b110, 0b10001, 0b10101, 0b1001, 0b10,
                  db 0b10100, 0b1011, 0b10000, 0b100, 0b11110
                  db 0b1101, 0b10010, 0b111, 0b101, 0b1111
                  db 0b10110, 0b11011, 0b1010, 0b1000, 0b11
                  db 0b1100, 0b11000, 0b1110, 0b11001, 0b11101
                  db 0b10011

    morseLookup db 0, 0, 69, 84, 73, 65, 78, 77, 83, 85
                db 82, 87, 68, 75, 71, 79, 72, 86, 70, 0
                db 76, 0, 80, 74, 66, 88, 67, 89, 90, 81
                db 0, 0, 53, 52, 0, 51, 0, 0, 0, 50, 0, 0
                db 0, 0, 0, 0, 0, 49, 54, 0, 0, 0, 0, 0, 0
                db 0, 55, 0, 0, 0, 56, 0, 57, 48

    BUFFERSIZE equ 1024     ; buffer sizes for reading and writing data
    
section .bss
    inBuffer resb BUFFERSIZE    ; buffer for reading data
    outBuffer resb BUFFERSIZE   ; buffer for writing data
    
section .text
    global _start
_start:
    mov rsi, inBuffer       ; pointer to the input buffer
    xor rdi, rdi            ; file descriptor 0 - stdin
    xor rax, rax            ; sys_read
    mov rdx, BUFFERSIZE
    syscall
    cmp rax, -1             ; check the success of the system call
    je exit_error

    ; r12 holds the number of bytes read in the previous sys_read 
    ; (if r12 < BUFFERSIZE, it means that sys_read encountered end of input stream)
    mov r12, rax

    xor r9, r9      ; r9 will hold the position in the input buffer
    xor r10, r10    ; r10 will hold the position in the output buffer
    xor r11, r11    ; r11 will hold the current Morse code in the buffer
    xor r13, r13    ; r13 will hold the conversion mode information:
                    ; 0 means no non-space character has been encountered yet
                    ; 1 means Morse->English conversion
                    ; 2 means English->Morse conversion
    
    ; r14 will hold the binary representation of the Morse code 
    ; (for Morse->English conversion)
    mov r14, 1
    
conversion:
    ; rbx holds the current character from the buffer
    movzx rbx, byte [inBuffer + r9]
    cmp rbx, 32          ; case where the current character in the buffer is a space
    je .space

    ; here we determine the conversion mode
    cmp r13, 0
    je not_determined
    cmp r13, 1
    je morse_eng
    jmp eng_morse

.space:

    ; here we handle the case where r14 contains a Morse code
    ; (a space indicates the end of the current Morse code)
    cmp r14, 1
    jne morse_eng

    ; if the current character in inBuffer is a space, we write a space to outBuffer and proceed
    mov byte [outBuffer + r10], 32
    inc r10
    inc r9
    jmp handling_buffers

not_determined:
    ; below cases for when the current character in the buffer is "." or "-"
    ; (which indicates Morse->English conversion)
    cmp rbx, 45         ; dash
    je .morse
    cmp rbx, 46         ; dot
    je .morse

    ; if the current character in the buffer is neither space, "." nor "-", it means English->Morse conversion
    jmp .eng
.morse:
    mov r13, 1              ; set conversion direction to Morse->English
    jmp handling_buffers

.eng:                       
    mov r13, 2              ; set conversion direction to English->Morse
    jmp handling_buffers

morse_eng:
    ; check if the current character is a space, dot, or dash (if none of these, exit with code 1)
    cmp rbx, 45             ; dash
    je .dash
    cmp rbx, 46             ; dot
    je .dot
    cmp rbx, 32             ; space
    je .space
    jmp exit_error

.dot:
    ; a dot is converted to 0, so we simply shift r14 bitwise
    shl r14, 1 
    jmp .check_for_error
.dash:
    ; a dash is converted to 1, so we shift r14 bitwise and add 1
    shl r14, 1
    inc r14
    jmp .check_for_error
.space:
    ; end of the current Morse code
    
    ; load the corresponding character from morseLookup
    lea rsi, [morseLookup + r14]
    mov al, [rsi]

    ; check if the given Morse code represented any character
    ; (if the value at that address in morseLookup is 0, it means 
    ; that the code did not represent any character)
    test al, al
    jz exit_error

    ; write the character that the Morse code represented to outBuffer
    ; reset r14
    mov byte [outBuffer + r10], al
    inc r10
    inc r9
    mov r14, 1
    jmp handling_buffers

.check_for_error:
    ; check if the read Morse code is becoming too long 
    ; (the longest Morse code has 5 characters, meaning the largest possible value
    ; representing Morse code is 0b111111)
    cmp r14, 0b111111
    jg exit_error

    ; after reading the given character, increment r9
    inc r9
    jmp handling_buffers

eng_morse:
    ; if rbx contains a value less than 48 or greater than 90, it is definitely not a digit or uppercase letter
    cmp rbx, 48
    jl exit_error
    cmp rbx, 90
    jg exit_error

    inc r9

    ; if rbx is in the range [48, 57], it means it represents a digit
    cmp rbx, 58
    js .number

    ; if rbx is in the range [65, 90], it means it represents a letter
    cmp rbx, 64
    jg .letter

    ; if it is neither a letter nor a digit, it means the input is invalid
    jmp exit_error

.number:
    ; here we load from numbersLookup the Morse code corresponding to the given digit
    lea rsi, [numbersLookup + rbx - NUMOFFSET]
    mov al, [rsi]
    jmp .push_to_out_buffer
.letter:
    ; here we load from lettersLookup the Morse code corresponding to the given letter
    lea rsi, [lettersLookup + rbx - LETTERSOFFSET]
    mov al, [rsi]
.push_to_out_buffer:
    ; write the Morse code to outBuffer
    mov rcx, al
.next_bit:
    ; stop if the value in al is 0
    test rcx, rcx
    jz .next_char

    ; extract the last bit of al and write it to outBuffer
    test rcx, 1
    jz .write_dot
    mov byte [outBuffer + r10], 45   ; dash
    jmp .increment_out_buffer
.write_dot:
    mov byte [outBuffer + r10], 46   ; dot
.increment_out_buffer:
    inc r10
    shr rcx, 1
    jmp .next_bit
.next_char:
    ; after writing the character, add a space to outBuffer
    mov byte [outBuffer + r10], 32
    inc r10
    jmp handling_buffers

handling_buffers:
    ; increment r9 and continue reading the buffer
    cmp r9, r12
    je write_to_stdout
    jmp conversion

write_to_stdout:
    mov rsi, outBuffer
    mov rdx, r10
    mov rdi, 1
    mov rax, 1
    syscall
    cmp rax, -1
    je exit_error
    jmp exit_success

exit_error:
    mov rdi, 1
    mov rax, 60
    syscall

exit_success:
    cmp r14, 1
    jne exit_error
    mov rax, 60             ; sys_exit
    xor rdi, rdi            ; exit(0)
    syscall
