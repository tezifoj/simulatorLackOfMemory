/*
 * Вывод состояния памяти по шагам
 */

#ifndef VIEW_H
#define VIEW_H

#include "memory.h"
#include "trace.h"

/*
 * Печатает, что произошло на шаге step
 * само обращение, hit/fault, таблицу кадров
 * res -- то, что вернула translate
 */
void show_step(const Memory *mem, int step, Access a, int res);

/*
 * Ждёт Enter. Возвращает 0, если ввели q, иначе 1
 */
int wait_key(void);

#endif