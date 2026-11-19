/*
 * memory.c
 * Создание, удаление, вывод памяти
 */

#include <stdio.h>
#include <stdlib.h>
#include "memory.h"
#include "addr.h"

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

static int find_free_frame(const Memory *mem) {
    for (int i = 0; i < mem->frames_count; i++) {
        if (mem->frames[i] == EMPTY) {
            return i;
        }
    }
    return EMPTY;
}

/* положить страницу page_num в кадр frame_num */
static void load_page(Memory *mem, int page_num, int frame_num) {
    Page *p = &mem->pages[page_num];   /* указатель на одну запись в странице */
    p->in_memory = 1;
    p->frame_num = frame_num;
    p->used = 0;
    p->changed = 0;
    mem->frames[frame_num] = page_num;
}

int access_page(Memory *mem, int page_num, int is_write) {
    if (page_num < 0 || page_num >= mem->pages_count) {
        return ACCESS_ERR;
    }

    Page *p = &mem->pages[page_num];
    int result = HIT;

    if (!p->in_memory) {
        result = PAGE_FAULT;

        int frame = find_free_frame(mem);
        if (frame == EMPTY) {
            return ACCESS_ERR;   /* замещение будет на следующем шаге */
        }
        load_page(mem, page_num, frame);
    }

    p->used = 1;
    if (is_write) {
        p->changed = 1;
    }
    return result;
}

int translate(Memory *mem, uint64_t virt_addr, int is_write, uint64_t *phys_addr) {
    /* делим адрес функциями из addr.asm */
    uint64_t page = get_page_number(virt_addr, OFFSET_BITS);
    uint64_t offset = get_offset(virt_addr, OFFSET_BITS);

    int result = access_page(mem, (int)page, is_write);
    if (result == ACCESS_ERR) {
        return ACCESS_ERR;
    }

    /* собираем физический адрес */
    int frame = mem->pages[page].frame_num;
    *phys_addr = make_phys_addr(frame, offset, OFFSET_BITS);
    return result;
}