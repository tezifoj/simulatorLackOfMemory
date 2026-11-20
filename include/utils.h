#ifndef UTILS_H
#define UTILS_H

/*
 * Возвращает индекс минимального элемента в массиве values длины count
 */
int find_min_index(const int *values, int count);

/*
 * Алгоритм Clock: начинает с позиции *hand и ищет кадр с used == 0
 */
int clock_find(int *used, int count, int *hand);

#endif