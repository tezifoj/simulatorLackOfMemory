/*
 * Модель памяти: таблица страниц, список кадров
 */

#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

#define EMPTY (-1)   /* кадр свободен либо страница не в кадре */

/* Информация о виртуальной странице */
typedef struct {
    int in_memory;   /* 1 -- страница в оперативке, 0 - нет */
    int frame_num;   /* в каком кадре лежит (если in_memory = 1) */
    int used;        /* было ли обращение */
    int changed;     /* 1 -- в страницу писали, при вытеснении сохраняем на диск */
    int load_time;
    int last_used;
} Page;

/* алгоритм замещения */
typedef enum {
    ALGO_FIFO,
    ALGO_LRU,
    ALGO_CLOCK
} Algorithm;

/* Вся память симулятора */
typedef struct {
    int pages_count;    /* сколько виртуальных страниц */
    int frames_count;   /* сколько кадров в оперативной памяти */
    Page *pages;        /* таблица страниц, pages_count штук */
    int *frames;        /* frames[i] - какая страница в кадре i */
    Algorithm algo;     /* алгоритм замещения */
    int time;
    int removed_page;   /* что вытесняется после обращения ( или empty) */
    int clock_hand;
    int hits;
    int faults;         /* сколько было page fault */
    int disk_writes;    /* сколько раз выгрузили изменённую страницу */
} Memory;

Memory *create_memory(int pages_count, int frames_count, Algorithm algo);

void free_memory(Memory *mem);

void print_memory(const Memory *mem);

#define OFFSET_BITS 12   /* размер страницы 4096 байт */

/* что получилось при обращении */
#define HIT         0    /* страница уже была в памяти */
#define PAGE_FAULT  1    /* страницы не было -- загрузили */
#define ACCESS_ERR (-1)  /* страницы с таким номером нет */

/*
 * Обращение к странице page_num
 * is_write = 1 - запись, 0 - чтение
 * Если страницы нет в памяти -- кладём её в свободный кадр
 * Возвращает HIT, PAGE_FAULT или ACCESS_ERR
 */
int access_page(Memory *mem, int page_num, int is_write);

/*
 * Виртуальный адрес в физический
 * Возвращает HIT, PAGE_FAULT или ACCESS_ERR
 */
int translate(Memory *mem, uint64_t virt_addr, int is_write, uint64_t *phys_addr);

#endif