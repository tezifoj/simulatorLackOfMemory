/*
 * Точка входа симулятора
 */

#include <stdio.h>
#include <stdint.h>
#include "addr.h"
#include "memory.h"

int main(void) {
    uint64_t addr = 0x3A7F;      /* тестовый виртуальный адрес */

    uint64_t page = get_page_number(addr, OFFSET_BITS);
    uint64_t offset = get_offset(addr, OFFSET_BITS);

    printf("Адрес 0x%lX -> страница %lu, смещение 0x%lX\n",
           (unsigned long)addr, (unsigned long)page, (unsigned long)offset);

    /* память на 8 страниц и 3 кадра */
    Memory *mem = create_memory(8, 3, ALGO_FIFO);
    if (mem == NULL) {
        printf("не получилось создать память\n");
        return 1;
    }

    /* адреса и тип обращения: 0 - чтение, 1 - запись */
    uint64_t addrs[] = {0x0123, 0x1456, 0x0FFF, 0x2000, 0x1004, 0x5ABC};
    int writes[]     = {0,      1,      0,      0,      1,      0};
    int n = sizeof(addrs) / sizeof(addrs[0]);

    for (int i = 0; i < n; i++) {
        uint64_t phys = 0;
        int res = translate(mem, addrs[i], writes[i], &phys);

        /* %04lX -- число unsigned long в шестнадцатеричном виде */
        printf("0x%04lX (%s): ", (unsigned long)addrs[i],
               writes[i] ? "запись" : "чтение");

        if (res == HIT) {
            printf("hit, физ. адрес 0x%04lX\n", (unsigned long)phys);
        } else if (res == PAGE_FAULT) {
            printf("page fault, физ. адрес 0x%04lX", (unsigned long)phys);
            if (mem->removed_page != EMPTY) {
                printf(", убрали стр. %d", mem->removed_page);
            }
            printf("\n");
        } else {
            printf("ошибка (нет такой страницы)\n");
        }
    }
    printf("\n");

    print_memory(mem);
    free_memory(mem);

    printf("\n--- FIFO, 3 кадра ---\n");

    mem = create_memory(8, 3, ALGO_FIFO);
    if (mem == NULL) {
        printf("не получилось создать память\n");
        return 1;
    }

    /* номера страниц, к которым обращаемся */
    int refs[] = {7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3, 2, 1, 2, 0, 1, 7, 0, 1};
    int refs_count = sizeof(refs) / sizeof(refs[0]);
    int faults = 0;

    for (int i = 0; i < refs_count; i++) {
        /* из номера страницы создаем адрес: страница + смещение */
        uint64_t a = ((uint64_t)refs[i] << OFFSET_BITS) | 0x10;
        uint64_t phys = 0;
        int res = translate(mem, a, 0, &phys);

        printf("стр. %d: ", refs[i]);

        if (res == HIT) {
            printf("hit\n");
        } else if (res == PAGE_FAULT) {
            faults++;
            printf("fault -> кадр %d", mem->pages[refs[i]].frame_num);
            if (mem->removed_page != EMPTY) {
                printf(", убрали стр %d", mem->removed_page);
            }
            printf("\n");
        } else {
            printf("ошибка\n");
        }
    }

    printf("\nвсего page fault: %d из %d\n", faults, refs_count);

    free_memory(mem);

    return 0;
}