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
    Memory *mem = create_memory(8, 3);
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
            printf("page fault, загрузили, физ. адрес 0x%04lX\n", (unsigned long)phys);
        } else {
            printf("ошибка (нет свободного кадра)\n");
        }
    }
    printf("\n");

    print_memory(mem);
    free_memory(mem);

    return 0;
}