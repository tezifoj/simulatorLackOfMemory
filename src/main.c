/*
 * Точка входа симулятора
 */

#include <stdio.h>
#include <stdint.h>
#include "addr.h"

int main(void) {
    uint64_t offset_bits = 12;   /* размер страницы 2**12 = 4096 байт */
    uint64_t addr = 0x3A7F;      /* тестовый виртуальный адрес */

    uint64_t page = get_page_number(addr, offset_bits);
    uint64_t offset = get_offset(addr, offset_bits);

    printf("Адрес 0x%lX -> страница %lu, смещение 0x%lX\n",
           (unsigned long)addr, (unsigned long)page, (unsigned long)offset);

    return 0;
}