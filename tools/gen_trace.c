/*
 * gen_trace.c
 * Генератор трасс для симулятора
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memory.h"

#define PAGE_SIZE (1 << OFFSET_BITS)

/* выбирает страницу для i-го обращения */
static int next_page(const char *mode, int i, int pages, int *base) {
    if (strcmp(mode, "loop") == 0) {
        return i % pages;
    }

    if (strcmp(mode, "local") == 0) {
        if (i % 50 == 0) {
            *base = rand() % pages;
        }
        if (rand() % 10 < 9) {
            return (*base + rand() % 3) % pages;
        }
        return rand() % pages;
    }

    return rand() % pages;   /* random */
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "запуск: %s random|local|loop обращений страниц [seed]\n", argv[0]);
        return 1;
    }

    const char *mode = argv[1];
    if (strcmp(mode, "random") != 0 && strcmp(mode, "local") != 0 && strcmp(mode, "loop") != 0) {
        fprintf(stderr, "режим должен быть random, local или loop\n");
        return 1;
    }

    int count = atoi(argv[2]);
    int pages = atoi(argv[3]);
    if (count <= 0 || pages <= 0) {
        fprintf(stderr, "обращений и страниц должно быть больше 0\n");
        return 1;
    }

    unsigned seed = 1;   /* по умолчанию 1, чтобы трасса каждый раз была одинаковая */
    if (argc >= 5) {
        seed = (unsigned)atoi(argv[4]);
    }
    srand(seed);

    printf("# %s, обращений: %d, страниц: %d, seed: %u\n", mode, count, pages, seed);

    int base = 0;   /* с какой страницы начинается рабочая область (для local) */
    for (int i = 0; i < count; i++) {
        int page = next_page(mode, i, pages, &base);
        int offset = rand() % PAGE_SIZE;
        char op = (rand() % 10 < 3) ? 'W' : 'R';   /* 30% записей */

        unsigned long addr = (unsigned long)page * PAGE_SIZE + offset;
        printf("%c 0x%lx\n", op, addr);
    }

    return 0;
}