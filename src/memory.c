/*
 * memory.c
 * Создание, удаление, вывод памяти
 */

#include <stdio.h>
#include <stdlib.h>
#include "memory.h"
#include "addr.h"
#include "utils.h"

Memory *create_memory(int pages_count, int frames_count, Algorithm algo) {
    if (pages_count <= 0 || frames_count <= 0) {
        return NULL;
    }

    Memory *mem = malloc(sizeof(Memory));
    if (mem == NULL) {
        return NULL;
    }

    mem->pages_count = pages_count;
    mem->frames_count = frames_count;
    mem->algo = algo;
    mem->time = 0;
    mem->removed_page = EMPTY;
    mem->clock_hand = 0;
    mem->hits = 0;
    mem->faults = 0;
    mem->disk_writes = 0;

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
    p->load_time = mem->time;
}

/* FIFO: ищет кадр, где лежит страница, которую загрузили вначале */
static int fifo_find_oldest(const Memory *mem) {
    int times[mem->frames_count];   /* время загрузки страницы в каждом кадре */

    for (int i = 0; i < mem->frames_count; i++) {
        times[i] = mem->pages[mem->frames[i]].load_time;
    }
    return find_min_index(times, mem->frames_count);   /* из utils.asm */
}

/* LRU: ищет кадр со страницей, к которой дольше всего не обращались */
static int lru_find_oldest(const Memory *mem) {
    int times[mem->frames_count];   /* время последнего обращения для каждого кадра */

    for (int i = 0; i < mem->frames_count; i++) {
        times[i] = mem->pages[mem->frames[i]].last_used;
    }
    return find_min_index(times, mem->frames_count);
}

/* Clock */
static int clock_find_frame(Memory *mem) {
    int used[mem->frames_count];   /* бит обращения для каждого кадра */

    for (int i = 0; i < mem->frames_count; i++) {
        used[i] = mem->pages[mem->frames[i]].used;
    }

    int frame = clock_find(used, mem->frames_count, &mem->clock_hand);

    /* после того как asm сбросил часть битов записываем их обратно в таблицу страниц */
    for (int i = 0; i < mem->frames_count; i++) {
        mem->pages[mem->frames[i]].used = used[i];
    }
    return frame;
}

/* какой кадр освободить */
static int find_frame_to_free(Memory *mem) {
    switch (mem->algo) {
        case ALGO_FIFO:
            return fifo_find_oldest(mem);
        case ALGO_LRU:
            return lru_find_oldest(mem);
        case ALGO_CLOCK:
            return clock_find_frame(mem);
    }
    return 0;
}

/* удалить страницу из кадра frame_num */
static void unload_page(Memory *mem, int frame_num) {
    int page_num = mem->frames[frame_num];
    Page *p = &mem->pages[page_num];

    if (p->changed) {
        mem->disk_writes++;   /* сохранение на диск при записи на странице */
    }

    p->in_memory = 0;
    p->frame_num = EMPTY;
    p->used = 0;
    p->changed = 0;

    mem->frames[frame_num] = EMPTY;
    mem->removed_page = page_num;
}

int access_page(Memory *mem, int page_num, int is_write) {
    if (page_num < 0 || page_num >= mem->pages_count) {
        return ACCESS_ERR;
    }

    mem->time++;
    mem->removed_page = EMPTY;

    Page *p = &mem->pages[page_num];
    int result = HIT;

    if (!p->in_memory) {
        result = PAGE_FAULT;

        int frame = find_free_frame(mem);
        if (frame == EMPTY) {
            frame = find_frame_to_free(mem);
            unload_page(mem, frame);
        }
        load_page(mem, page_num, frame);
    }

    p->used = 1;
    p->last_used = mem->time;   /* для LRU */
    if (is_write) {
        p->changed = 1;
    }
    if (result == HIT) {
        mem->hits++;
    } else {
        mem->faults++;
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