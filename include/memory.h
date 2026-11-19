/*
 * Модель памяти: таблица страниц, список кадров
 */

#ifndef MEMORY_H
#define MEMORY_H

#define EMPTY (-1)   /* кадр свободен либо страница не в кадре */

/* Информация о виртуальной странице */
typedef struct {
    int in_memory;   /* 1 -- страница в оперативке, 0 - нет */
    int frame_num;   /* в каком кадре лежит (если in_memory = 1) */
    int used;        /* было ли обращение */
    int changed;     /* 1 -- в страницу писали, при вытеснении сохраняем на диск */
} Page;

/* Вся память симулятора */
typedef struct {
    int pages_count;    /* сколько виртуальных страниц */
    int frames_count;   /* сколько кадров в оперативной памяти */
    Page *pages;        /* таблица страниц, pages_count штук */
    int *frames;        /* frames[i] - какая страница в кадре i */
} Memory;

Memory *create_memory(int pages_count, int frames_count);

void free_memory(Memory *mem);

void print_memory(const Memory *mem);

#endif