/*
 * gcc -g -Wall -o astar_test astar_test.c -lcurses
 */

#define ASTAR_IMPLEMENTATION
#include "astar.h"

#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>

#include <curses.h>

int
main(int argc, char *argv[]) {
    srand(time(0));
    int w;
    int h;

    initscr();
    keypad(stdscr, TRUE);
    cbreak();
    noecho();
    curs_set(0);
    start_color();

    init_pair(COLOR_RED, COLOR_RED, COLOR_BLACK);
    init_pair(COLOR_GREEN, COLOR_GREEN, COLOR_BLACK);
    init_pair(COLOR_BLUE, COLOR_BLUE, COLOR_BLACK);

    while (1) {
        getmaxyx(stdscr, h, w);
        struct astar *astar = (struct astar *)malloc(sizeof *astar);
        astar_init(astar, w, h);
        int b[w][h];

        clear();
        int i, j;
        for (i = 0; i < w; i++) {
            for (j = 0; j < h; j++) {
                int g = rand()%100;
                if (i == 0 || j == 0 || i == w-1 || j == h-1 || g < 10) {
                    astar_block(astar, i, j, 0, 1);
                    move(j, i);
                    addch('#' | COLOR_PAIR(COLOR_BLUE));
                } else {
                    g = rand()%10;
                    astar_block(astar, i, j, g, 0);
                    move(j, i);
                    addch(48+g);
                    b[i][j] = 48+g;
                }
            }
        }
        astar_link(astar);
        refresh();

        int sx = rand()%w;
        int sy = rand()%h;
        int ex = rand()%w;
        int ey = rand()%h;

        struct timeval tv1;
        struct timeval tv2;
        gettimeofday(&tv1, 0);
        astar_find(astar, sx, sy, ex, ey, 0);
        gettimeofday(&tv2, 0);
        int t = (tv2.tv_sec - tv1.tv_sec) * 1000 + (tv2.tv_usec - tv1.tv_usec) / 1000;
        attron(A_BOLD);
        mvprintw(0, 1, " elapse: %d ms ", t);
        attroff(A_BOLD);

        move(sy, sx);
        addch('s' | COLOR_PAIR(COLOR_RED));

        move(ey, ex);
        addch('e' | COLOR_PAIR(COLOR_RED));

        int x, y;
        while (0 == astar_next(astar, &x, &y)) {
            move(y, x);
            if (y == sy && x == sx) {
                addch('s' | COLOR_PAIR(COLOR_RED));
            } else if (y == ey && x == ex) {
                addch('e' | COLOR_PAIR(COLOR_RED));
            } else {
                addch(b[x][y] | COLOR_PAIR(COLOR_GREEN));
            }
            refresh();
            usleep(300000);
        }
        astar_unit(astar);
        free(astar);
        sleep(1);
	}
    endwin();
	return 0;
}
