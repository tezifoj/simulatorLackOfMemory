/*
 * memory.c
 * Создание, удаление, вывод памяти
 */

#include <stdio.h>
#include <stdlib.h>
#include "memory.h"

Memory *create_memory(int pages_count, int frames_count) {
    if (pages_count <= 0 || frames_count <= 0) {
        return NULL;
    }

    Memory *mem = malloc(sizeof(Memory));
    if (mem == NULL) {
        return NULL;
    }

    mem->pages_count = pages_count;
    mem->frames_count = frames_count;

    mem->pages = calloc(pages_count, sizeof(Page));
    mem->frames = malloc(frames_count * sizeof(int));

    if (mem->pages == NULL || mem->frames == NULL) {
        free_memory(mem);
        return NULL;
    }

    /* страницы пока ни в каких кадрах */
    for (int i = 0; i < pages_count; i++) {
        mem->pages[i].frame_num = EMPTY;
    }

    /* все кадры свободны */
    for (int i = 0; i < frames_count; i++) {
        mem->frames[i] = EMPTY;
    }

    return mem;
}

void free_memory(Memory *mem) {
    if (mem == NULL) {
        return;
    }
    free(mem->pages);
    free(mem->frames);
    free(mem);
}

void print_memory(const Memory *mem) {
    printf("Таблица страниц:\n");
    printf(" стр | в памяти | кадр | исп | изм\n");
    printf("-----+----------+------+-----+-----\n");

    for (int i = 0; i < mem->pages_count; i++) {
        Page p = mem->pages[i];

        if (p.in_memory) {
            printf(" %3d |    да    | %4d |  %d  |  %d\n",
                   i, p.frame_num, p.used, p.changed);
        } else {
            printf(" %3d |    нет   |    - |  %d  |  %d\n",
                   i, p.used, p.changed);
        }
    }

    printf("\nКадры:\n");
    for (int i = 0; i < mem->frames_count; i++) {
        if (mem->frames[i] == EMPTY) {
            printf(" кадр %d: свободен\n", i);
        } else {
            printf(" кадр %d: страница %d\n", i, mem->frames[i]);
        }
    }
}