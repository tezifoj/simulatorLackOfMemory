section .text
global find_min_index

; Возвращает индекс минимального элемента массива
find_min_index:
    xor eax, eax              ; eax -- индекс минимума
    mov edx, [rdi]
    mov ecx, 1

.loop:
    cmp ecx, esi
    jge .done
    mov r8d, [rdi + rcx*4]
    cmp r8d, edx
    jge .next
    mov edx, r8d
    mov eax, ecx
.next:
    inc ecx
    jmp .loop

.done:
    ret

section .note.GNU-stack noalloc noexec nowrite progbits