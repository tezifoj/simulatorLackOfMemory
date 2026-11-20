/*
 * trace.h
 * Трасса -- список обращений к памяти, который читается из файла
 */

#ifndef TRACE_H
#define TRACE_H

#include <stdint.h>

/* одно обращение */
typedef struct {
    uint64_t addr;
    int is_write;
} Access;

/* все обращения из файла */
typedef struct {
    Access *items;
    int count;
    int capacity;
} Trace;

/*
 * Читает трассу из файла
 */
Trace *load_trace(const char *filename);

/* Освобождает трассу */
void free_trace(Trace *t);

#endif