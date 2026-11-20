section .text
global find_min_index
global clock_find

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

; int clock_find(int *used, int count, int *hand)
; Возвращает номер найденного кадра
clock_find:
    mov ecx, [rdx]

.loop:
    cmp dword [rdi + rcx*4], 0
    je .found
    mov dword [rdi + rcx*4], 0
    inc ecx
    cmp ecx, esi
    jl .loop
    xor ecx, ecx
    jmp .loop

.found:
    mov eax, ecx
    inc ecx
    cmp ecx, esi
    jl .save
    xor ecx, ecx
.save:
    mov [rdx], ecx
    ret

section .note.GNU-stack noalloc noexec nowrite progbits