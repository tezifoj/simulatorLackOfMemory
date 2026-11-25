/*
 * Точка входа симулятора
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "addr.h"
#include "memory.h"
#include "trace.h"
#include "utils.h"
#include "view.h"

/* результаты одного прогона */
typedef struct {
    int hits;
    int faults;
    int disk_writes;
} Result;

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
static Result run_trace(Algorithm algo, int frames_count, const Trace *t, int verbose) {
    Result r = {0, 0, 0};

    Memory *mem = create_memory(count_pages(t), frames_count, algo);
    if (mem == NULL) {
        printf("не получилось создать память\n");
        return r;
    }

    if (verbose) {
        printf("\n--- %s, кадров: %d ---\n", algo_name(algo), frames_count);
    }

    for (int i = 0; i < t->count; i++) {
        Access a = t->items[i];
        uint64_t phys = 0;
        int res = translate(mem, a.addr, a.is_write, &phys);
        int page = (int)get_page_number(a.addr, OFFSET_BITS);

        if (!verbose) {
            continue;
        }

        /* %04lX -- число unsigned long в шестнадцатеричном виде */
        printf("%c 0x%04lX (стр %d): ", a.is_write ? 'W' : 'R',
               (unsigned long)a.addr, page);

        if (res == HIT) {
            printf("hit, физ. 0x%04lX\n", (unsigned long)phys);
        } else if (res == PAGE_FAULT) {
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

    /* счётчики уже посчитала сама память в access_page */
    r.hits = mem->hits;
    r.faults = mem->faults;
    r.disk_writes = mem->disk_writes;

    if (verbose) {
        printf("всего page fault: %d из %d\n", r.faults, t->count);
    }

    free_memory(mem);
    return r;
}

/* одна строка таблицы статистики */
static void print_result(Algorithm algo, Result r, int total) {
    printf(" %-8s | %5d | %4d (%3d%%) | %7d\n",
           algo_name(algo), r.faults, r.hits,
           calc_percent(r.hits, total),   /* из utils.asm */
           r.disk_writes);
}

/* сколько page fault у каждого алгоритма при 1, 2, 3, ... кадрах */
static void compare_frames(const Trace *t) {
    int max_frames = count_pages(t);   /* кадров больше, чем страниц*/
    if (max_frames > 16) {
        max_frames = 16;
    }

    printf("\npage fault при разном числе кадров:\n");
    printf(" кадров | FIFO | LRU | Clock\n");
    printf("--------+------+-----+------\n");

    for (int f = 1; f <= max_frames; f++) {
        Result fifo  = run_trace(ALGO_FIFO, f, t, 0);
        Result lru   = run_trace(ALGO_LRU, f, t, 0);
        Result clock = run_trace(ALGO_CLOCK, f, t, 0);

        printf(" %6d | %4d | %3d | %5d\n", f, fifo.faults, lru.faults, clock.faults);
    }
}

/* перевод строки fifo / lru / clock в алгоритм, иначе 0 */
static int parse_algo(const char *s, Algorithm *algo) {
    if (strcmp(s, "fifo") == 0)  { *algo = ALGO_FIFO;  return 1; }
    if (strcmp(s, "lru") == 0)   { *algo = ALGO_LRU;   return 1; }
    if (strcmp(s, "clock") == 0) { *algo = ALGO_CLOCK; return 1; }
    return 0;
}

/* пошаговый режим: после каждого обращения показываем кадры и ждём Enter */
static void run_steps(Algorithm algo, int frames_count, const Trace *t) {
    Memory *mem = create_memory(count_pages(t), frames_count, algo);
    if (mem == NULL) {
        printf("не получилось создать память\n");
        return;
    }

    printf("пошаговый режим: %s, кадров: %d\n", algo_name(algo), frames_count);

    for (int i = 0; i < t->count; i++) {
        uint64_t phys = 0;
        int res = translate(mem, t->items[i].addr, t->items[i].is_write, &phys);
        show_step(mem, i + 1, t->items[i], res);

        /* после последнего шага не ждём */
        if (i < t->count - 1 && !wait_key()) {
            printf("выход\n");
            break;
        }
    }

    int total = mem->hits + mem->faults;
    printf("\nитого: page fault %d, hit %d (%d%%), на диск %d\n",
           mem->faults, mem->hits, calc_percent(mem->hits, total), mem->disk_writes);

    free_memory(mem);
}

int main(int argc, char *argv[]) {
    /* запуск: ./vmsim файл_трассы [кадров] */
    if (argc < 2) {
        printf("запуск: %s файл_трассы [кадров] [fifo|lru|clock]\n", argv[0]);
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

    /* если указан алгоритм -- пошаговый режим только для него */
    if (argc >= 4) {
        Algorithm algo;
        if (!parse_algo(argv[3], &algo)) {
            printf("алгоритм должен быть fifo, lru или clock\n");
            free_trace(t);
            return 1;
        }
        run_steps(algo, frames, t);
        free_trace(t);
        return 0;
    }

    /* одна и та же трасса на трёх алгоритмах */
    Result fifo  = run_trace(ALGO_FIFO, frames, t, 1);
    Result lru   = run_trace(ALGO_LRU, frames, t, 1);
    Result clock = run_trace(ALGO_CLOCK, frames, t, 1);

    printf("\nстатистика (кадров: %d, обращений: %d):\n", frames, t->count);
    printf(" алгоритм | fault |    hit      | на диск\n");
    printf("----------+-------+-------------+--------\n");
    print_result(ALGO_FIFO, fifo, t->count);
    print_result(ALGO_LRU, lru, t->count);
    print_result(ALGO_CLOCK, clock, t->count);

    compare_frames(t);

    free_trace(t);
    return 0;
}