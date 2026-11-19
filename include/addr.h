/*
 * Объявления функций разбора адреса (для вызова из C),
 * Сами функции написаны на ассемблере в src/addr.asm,
 */

#ifndef ADDR_H   /* защита от повторного включения */
#define ADDR_H

#include <stdint.h>

/* Возвращает номер страницы (addr >> offset_bits) */
uint64_t get_page_number(uint64_t addr, uint64_t offset_bits);

/* Возвращает смещение внутри страницы (младшие offset_bits бит адреса) */
uint64_t get_offset(uint64_t addr, uint64_t offset_bits);

/* Собирает физический адрес: (frame << offset_bits) | offset */
uint64_t make_phys_addr(uint64_t frame, uint64_t offset, uint64_t offset_bits);

#endif