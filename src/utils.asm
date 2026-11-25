section .text
global find_min_index
global clock_find
global calc_percent

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

; int calc_percent(int part, int total)
; Возвращает part * 100 / total, если total = 0 -- возвращает 0
calc_percent:
    xor eax, eax
    test esi, esi
    jz .zero                ; на ноль делить нельзя
    imul eax, edi, 100      ; eax = part * 100
    cdq                     ; расширяем eax в edx:eax до 64-битного значения
    idiv esi                ; eax = edx:eax / total, остаток пишем в edx
.zero:
    ret

section .note.GNU-stack noalloc noexec nowrite progbits