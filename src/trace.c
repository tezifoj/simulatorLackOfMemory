/*
 * Чтение трассы обращений из текстового файла
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "trace.h"

/*
 * Добавляет обращение в конец трассы
 */
static int add_access(Trace *t, uint64_t addr, int is_write) {
    if (t->count == t->capacity) {
        int new_capacity = (t->capacity == 0) ? 16 : t->capacity * 2;
        Access *tmp = realloc(t->items, new_capacity * sizeof(Access));
        if (tmp == NULL) {
            return 0;
        }
        t->items = tmp;
        t->capacity = new_capacity;
    }

    t->items[t->count].addr = addr;
    t->items[t->count].is_write = is_write;
    t->count++;
    return 1;
}

Trace *load_trace(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        printf("не получилось открыть файл %s\n", filename);
        return NULL;
    }

    /* calloc: items = NULL, count = 0, capacity = 0 */
    Trace *t = calloc(1, sizeof(Trace));
    if (t == NULL) {
        fclose(f);
        return NULL;
    }

    char line[256];
    int line_num = 0;

    /* fgets читает по одной строке, пока файл не кончится */
    while (fgets(line, sizeof(line), f) != NULL) {
        line_num++;

        char *s = line;
        while (isspace((unsigned char)*s)) {
            s++;
        }

        if (*s == '\0' || *s == '#') {
            continue;
        }

        char op;
        unsigned long addr;
        if (sscanf(s, "%c %lx", &op, &addr) != 2) {
            printf("строка %d: непонятный формат, пропуск\n", line_num);
            continue;
        }

        op = toupper((unsigned char)op);
        if (op != 'R' && op != 'W') {
            printf("строка %d: ожидалось R или W, пропуск\n", line_num);
            continue;
        }

        if (!add_access(t, addr, op == 'W')) {
            printf("не хватило памяти\n");
            free_trace(t);
            fclose(f);
            return NULL;
        }
    }

    fclose(f);
    return t;
}

void free_trace(Trace *t) {
    if (t == NULL) {
        return;
    }
    free(t->items);
    free(t);
}