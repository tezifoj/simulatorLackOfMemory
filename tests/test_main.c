/*
 * test_main.c
 * Автотесты симулятора. Запуск: make test
 * Каждая проверка печатает ok или FAIL, в конце -- итог
 * Если хоть одна проверка не прошла, программа возвращает 1
 */

#include <stdio.h>
#include <stdint.h>
#include "addr.h"
#include "memory.h"
#include "trace.h"
#include "utils.h"

static int passed = 0;   /* сколько проверок прошло */
static int failed = 0;   /* сколько не прошло */

/* сравнивает полученное значение с ожидаемым,выволит результат */
static void check(const char *name, long long got, long long expected) {
    if (got == expected) {
        passed++;
        printf("  ok   %s\n", name);
    } else {
        failed++;
        printf("  FAIL %s: получили %lld, ожидали %lld\n", name, got, expected);
    }
}

/*
 * Прогоняет обращения к страницам refs, возвращает память
 */
static Memory *run_pages(Algorithm algo, int frames, const int *refs, const int *writes, int n) {
    Memory *mem = create_memory(8, frames, algo);
    if (mem == NULL) {
        return NULL;
    }
    for (int i = 0; i < n; i++) {
        access_page(mem, refs[i], writes ? writes[i] : 0);
    }
    return mem;
}

/* возвращает число page fault после прогона refs */
static int count_faults(Algorithm algo, int frames, const int *refs, int n) {
    Memory *mem = run_pages(algo, frames, refs, NULL, n);
    if (mem == NULL) {
        return -1;
    }
    int faults = mem->faults;
    free_memory(mem);
    return faults;
}

/* функции на ассемблере */

static void test_addr(void) {
    printf("разбор адреса (addr.asm):\n");
    check("номер страницы 0x3A7F",   get_page_number(0x3A7F, 12), 3);
    check("смещение 0x3A7F",         get_offset(0x3A7F, 12), 0xA7F);
    check("номер страницы 0x12345",  get_page_number(0x12345, 12), 0x12);
    check("смещение 0x12345",        get_offset(0x12345, 12), 0x345);
    check("смещение 0x5000",         get_offset(0x5000, 12), 0);
    check("физ. адрес кадр 2",       make_phys_addr(2, 0xA7F, 12), 0x2A7F);
    check("физ. адрес кадр 0",       make_phys_addr(0, 0x123, 12), 0x123);
}

static void test_utils(void) {
    printf("вспомогательные функции (utils.asm):\n");

    int a[] = {5, 2, 8, 2};
    check("минимум - первый из одинаковых", find_min_index(a, 4), 1);

    int b[] = {7};
    check("минимум в массиве из 1 элемента", find_min_index(b, 1), 0);

    int c[] = {-3, 0, -5};
    check("минимум с отрицательными", find_min_index(c, 3), 2);

    int used1[] = {1, 0, 1};
    int hand = 0;
    check("clock: нашли кадр 1", clock_find(used1, 3, &hand), 1);
    check("clock: стрелка сдвинулась на 2", hand, 2);
    check("clock: бит кадра 0 сброшен", used1[0], 0);
    check("clock: бит кадра 2 не трогали", used1[2], 1);

    int used2[] = {1, 1, 1};
    hand = 2;
    check("clock: полный круг, нашли кадр 2", clock_find(used2, 3, &hand), 2);
    check("clock: стрелка ушла по кругу на 0", hand, 0);
    check("clock: все биты сброшены", used2[0] + used2[1] + used2[2], 0);

    check("процент 5 из 20",  calc_percent(5, 20), 25);
    check("процент 1 из 3",   calc_percent(1, 3), 33);
    check("процент 20 из 20", calc_percent(20, 20), 100);
    check("процент из 0",     calc_percent(0, 0), 0);
}

static void test_algorithms(void) {
    printf("алгоритмы замещения:\n");

    /* последовательность из учебника Зильбершаца (Основы ОС): FIFO = 15, LRU = 12 при 3 кадрах */
    int refs[] = {7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3, 2, 1, 2, 0, 1, 7, 0, 1};
    int n = sizeof(refs) / sizeof(refs[0]);

    check("FIFO, 3 кадра",  count_faults(ALGO_FIFO, 3, refs, n), 15);
    check("LRU, 3 кадра",   count_faults(ALGO_LRU, 3, refs, n), 12);
    check("Clock, 3 кадра", count_faults(ALGO_CLOCK, 3, refs, n), 14);

    /* кадров хватает на все 6 разных страниц -- каждую загружаем один раз */
    check("FIFO, 8 кадров",  count_faults(ALGO_FIFO, 8, refs, n), 6);
    check("LRU, 8 кадров",   count_faults(ALGO_LRU, 8, refs, n), 6);
    check("Clock, 8 кадров", count_faults(ALGO_CLOCK, 8, refs, n), 6);

    /* 1 кадр -- каждое обращение к другой странице это fault */
    check("LRU, 1 кадр", count_faults(ALGO_LRU, 1, refs, n), 20);

    /* аномалия Белади: у FIFO при 4 кадрах fault больше, чем при 3 */
    int bel[] = {1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5};
    int bn = sizeof(bel) / sizeof(bel[0]);
    check("Белади, FIFO 3 кадра", count_faults(ALGO_FIFO, 3, bel, bn), 9);
    check("Белади, FIFO 4 кадра", count_faults(ALGO_FIFO, 4, bel, bn), 10);

    /* записи на диск */
    int bel_w[] = {0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0};
    Memory *mem = run_pages(ALGO_FIFO, 3, bel, bel_w, bn);
    if (mem != NULL) {
        check("Белади, записи на диск", mem->disk_writes, 3);
        check("hit + fault = все обращения", mem->hits + mem->faults, bn);
        free_memory(mem);
    }
}

static void test_translate(void) {
    printf("трансляция адреса и ошибки:\n");

    Memory *mem = create_memory(8, 3, ALGO_FIFO);
    if (mem == NULL) {
        check("создание памяти", 0, 1);
        return;
    }

    uint64_t phys = 0;
    check("первое обращение - fault", translate(mem, 0x2A7F, 0, &phys), PAGE_FAULT);
    check("страница 2 попала в кадр 0", phys, 0x0A7F);
    check("повторно та же страница - hit", translate(mem, 0x2004, 1, &phys), HIT);
    check("физ. адрес при hit", phys, 0x0004);
    check("после записи страница изменена", mem->pages[2].changed, 1);

    check("страница 100 не существует", access_page(mem, 100, 0), ACCESS_ERR);
    check("страница -1 не существует", access_page(mem, -1, 0), ACCESS_ERR);
    free_memory(mem);

    check("память на 0 страниц не создаётся", create_memory(0, 3, ALGO_FIFO) == NULL, 1);
    check("память на 0 кадров не создаётся", create_memory(8, 0, ALGO_FIFO) == NULL, 1);
}

static void test_trace(void) {
    printf("чтение трасс:\n");

    Trace *t = load_trace("traces/textbook.txt");
    check("textbook.txt прочитался", t != NULL, 1);
    if (t != NULL) {
        check("textbook.txt: 20 обращений", t->count, 20);
        check("первый адрес", t->items[0].addr, 0x7010);
        check("первое обращение - чтение", t->items[0].is_write, 0);
        free_trace(t);
    }

    t = load_trace("traces/belady.txt");
    check("belady.txt прочитался", t != NULL, 1);
    if (t != NULL) {
        check("belady.txt: 12 обращений", t->count, 12);
        check("второе обращение - запись", t->items[1].is_write, 1);
        free_trace(t);
    }

    check("несуществующий файл -> NULL", load_trace("traces/missing.txt") == NULL, 1);
}

int main(void) {
    test_addr();
    test_utils();
    test_algorithms();
    test_translate();
    test_trace();

    printf("\nпрошло: %d, не прошло: %d\n", passed, failed);
    return failed == 0 ? 0 : 1;
}