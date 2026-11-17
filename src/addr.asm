; функции для разбора виртуального адреса на номер страницы и смещение, вызываются из C

section .text
global get_page_number
global get_offset

get_page_number:
    mov rax, rdi        ; rax = addr
    mov rcx, rsi        ; сдвиг на переменную величину возможен только через cl
    shr rax, cl         ; rax = addr >> offset_bits
    ret

get_offset:
    mov rax, 1
    mov rcx, rsi
    shl rax, cl         ; rax = 1 << offset_bits
    dec rax             ; rax = маска
    and rax, rdi        ; rax = addr & маска
    ret

; помечаем, что стек не должен быть исполняемым (иначе линкер ругается)
section .note.GNU-stack noalloc noexec nowrite progbits