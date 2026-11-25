/*
 * Пошаговый вывод: таблица кадров после каждого обращения
 */

#include <stdio.h>
#include "view.h"
#include "addr.h"

/* ANSI-коды цветов для терминала */
#define RED    "\033[31m"
#define GREEN  "\033[32m"
#define YELLOW "\033[33m"
#define RESET  "\033[0m"

void show_step(const Memory *mem, int step, Access a, int res) {
    int page = (int)get_page_number(a.addr, OFFSET_BITS);

    printf("\nшаг %d: %c 0x%04lX (стр %d) -> ", step,
           a.is_write ? 'W' : 'R', (unsigned long)a.addr, page);

    if (res == HIT) {
        printf(GREEN "HIT" RESET);
    } else if (res == PAGE_FAULT) {
        printf(RED "PAGE FAULT" RESET);
        if (mem->removed_page != EMPTY) {
            printf(", убрали стр %d", mem->removed_page);
        }
    } else {
        printf(RED "ошибка (нет такой страницы)" RESET);
    }
    printf("\n");

    /* таблица кадров */
    printf(" кадр | стр | исп | изм | загр | обр\n");
    printf("------+-----+-----+-----+------+-----\n");

    for (int i = 0; i < mem->frames_count; i++) {
        int p = mem->frames[i];

        if (p == EMPTY) {
            printf(" %4d |   - |     |     |      |", i);
        } else {
            Page pg = mem->pages[p];
            printf(" %4d | %3d |  %d  |  %d  | %4d | %4d",
                   i, p, pg.used, pg.changed, pg.load_time, pg.last_used);
        }

        /* отметили кадр, к которому сейчас обращались */
        if (p == page && res == PAGE_FAULT) {
            printf(RED "  <- загрузили" RESET);
        } else if (p == page && res == HIT) {
            printf(GREEN "  <- hit" RESET);
        }

        /* для Clock показываем, где стрелка */
        if (mem->algo == ALGO_CLOCK && i == mem->clock_hand) {
            printf(YELLOW "  (стрелка)" RESET);
        }
        printf("\n");
    }
}

int wait_key(void) {
    printf("Enter - дальше, q - выход: ");

    int c = getchar();
    int quit = (c == 'q' || c == 'Q');

    /* дочитываем строку до конца, чтобы лишние символы не попали в следующий шаг */
    while (c != '\n' && c != EOF) {
        c = getchar();
    }
    return !quit;
}