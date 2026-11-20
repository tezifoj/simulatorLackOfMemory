/*
 * Точка входа симулятора
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "addr.h"
#include "memory.h"
#include "trace.h"

static const char *algo_name(Algorithm algo) {
    switch (algo) {
        case ALGO_FIFO:  return "FIFO";
        case ALGO_LRU:   return "LRU";
        case ALGO_CLOCK: return "Clock";
    }
    return "?";
}

/* сколько страниц нужно, чтобы поместились все адреса из трассы */
static int count_pages(const Trace *t) {
    uint64_t max_page = 0;
    for (int i = 0; i < t->count; i++) {
        uint64_t page = get_page_number(t->items[i].addr, OFFSET_BITS);
        if (page > max_page) {
            max_page = page;
        }
    }
    return (int)max_page + 1;
}

/*
 * Прогоняет трассу t на памяти с frames_count кадрами и алгоритмом algo
 * Печатает каждый шаг, возвращает число page fault
 */
static int run_trace(Algorithm algo, int frames_count, const Trace *t) {
    Memory *mem = create_memory(count_pages(t), frames_count, algo);
    if (mem == NULL) {
        printf("не получилось создать память\n");
        return -1;
    }

    printf("\n--- %s, кадров: %d ---\n", algo_name(algo), frames_count);
    int faults = 0;

    for (int i = 0; i < t->count; i++) {
        Access a = t->items[i];
        uint64_t phys = 0;
        int res = translate(mem, a.addr, a.is_write, &phys);
        int page = (int)get_page_number(a.addr, OFFSET_BITS);

        /* %04lX -- число unsigned long в шестнадцатеричном виде */
        printf("%c 0x%04lX (стр %d): ", a.is_write ? 'W' : 'R',
               (unsigned long)a.addr, page);

        if (res == HIT) {
            printf("hit, физ. 0x%04lX\n", (unsigned long)phys);
        } else if (res == PAGE_FAULT) {
            faults++;
            printf("fault -> кадр %d, физ. 0x%04lX",
                   mem->pages[page].frame_num, (unsigned long)phys);
            if (mem->removed_page != EMPTY) {
                printf(", убрали стр %d", mem->removed_page);
            }
            printf("\n");
        } else {
            printf("ошибка (нет такой страницы)\n");
        }
    }

    printf("всего page fault: %d из %d\n", faults, t->count);
    free_memory(mem);
    return faults;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("запуск: %s файл_трассы [кадров]\n", argv[0]);
        return 1;
    }

    int frames = 3;   /* по умолчанию 3 кадра */
    if (argc >= 3) {
        frames = atoi(argv[2]);   /* строку "3" -> в число 3 */
        if (frames <= 0) {
            printf("число кадров должно быть больше 0\n");
            return 1;
        }
    }

    /* читаем обращения из файла */
    Trace *t = load_trace(argv[1]);
    if (t == NULL) {
        return 1;
    }
    if (t->count == 0) {
        printf("в файле нет обращений\n");
        free_trace(t);
        return 1;
    }

    printf("файл: %s, обращений: %d, кадров: %d\n", argv[1], t->count, frames);

    /* одна и та же трасса на трёх алгоритмах */
    int fifo  = run_trace(ALGO_FIFO, frames, t);
    int lru   = run_trace(ALGO_LRU, frames, t);
    int clock = run_trace(ALGO_CLOCK, frames, t);

    printf("\nитого: FIFO = %d, LRU = %d, Clock = %d\n", fifo, lru, clock);

    free_trace(t);
    return 0;
}