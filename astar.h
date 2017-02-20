/**
 * author: zhoukk
 * link: https://github.com/zhoukk/astar
 *
 * Path Find base on A star, bresenham and floyd
 *
 * LD_FLAGS += -lcurses
 *
 * example:
 *
 * #define ASTAR_IMPLEMENTATION
 * #include "astar.h"
 * 
 * #include <unistd.h>
 * #include <time.h>
 * #include <sys/time.h>
 * #include <stdlib.h>
 * 
 * #include <curses.h>
 * 
 * int
 * main(int argc, char *argv[]) {
 *     srand(time(0));
 *     int w;
 *     int h;
 * 
 *     initscr();
 *     keypad(stdscr, TRUE);
 *     cbreak();
 *     noecho();
 *     curs_set(0);
 *     start_color();
 * 
 *     init_pair(COLOR_RED, COLOR_RED, COLOR_BLACK);
 *     init_pair(COLOR_GREEN, COLOR_GREEN, COLOR_BLACK);
 *     init_pair(COLOR_BLUE, COLOR_BLUE, COLOR_BLACK);
 * 
 *     while (1) {
 *         getmaxyx(stdscr, h, w);
 *         struct astar *astar = (struct astar *)malloc(sizeof *astar);
 *         astar_init(astar, w, h);
 *         int b[w][h];
 * 
 *         clear();
 *         int i, j;
 *         for (i = 0; i < w; i++) {
 *             for (j = 0; j < h; j++) {
 *                 int g = rand()%100;
 *                 if (i == 0 || j == 0 || i == w-1 || j == h-1 || g < 10) {
 *                     astar_block(astar, i, j, 0, 1);
 *                     move(j, i);
 *                     addch('#' | COLOR_PAIR(COLOR_BLUE));
 *                 } else {
 *                     g = rand()%10;
 *                     astar_block(astar, i, j, g, 0);
 *                     move(j, i);
 *                     addch(48+g);
 *                     b[i][j] = 48+g;
 *                 }
 *             }
 *         }
 *         astar_link(astar);
 *         refresh();
 * 
 *         int sx = rand()%w;
 *         int sy = rand()%h;
 *         int ex = rand()%w;
 *         int ey = rand()%h;
 * 
 *         struct timeval tv1;
 *         struct timeval tv2;
 *         gettimeofday(&tv1, 0);
 *         astar_find(astar, sx, sy, ex, ey, 0);
 *         gettimeofday(&tv2, 0);
 *         int t = (tv2.tv_sec - tv1.tv_sec) * 1000 + (tv2.tv_usec - tv1.tv_usec) / 1000;
 *         attron(A_BOLD);
 *         mvprintw(0, 1, " elapse: %d ms ", t);
 *         attroff(A_BOLD);
 * 
 *         move(sy, sx);
 *         addch('s' | COLOR_PAIR(COLOR_RED));
 * 
 *         move(ey, ex);
 *         addch('e' | COLOR_PAIR(COLOR_RED));
 * 
 *         int x, y;
 *         while (0 == astar_next(astar, &x, &y)) {
 *             move(y, x);
 *             if (y == sy && x == sx) {
 *                 addch('s' | COLOR_PAIR(COLOR_RED));
 *             } else if (y == ey && x == ex) {
 *                 addch('e' | COLOR_PAIR(COLOR_RED));
 *             } else {
 *                 addch(b[x][y] | COLOR_PAIR(COLOR_GREEN));
 *             }
 *             refresh();
 *             usleep(300000);
 *         }
 *         astar_unit(astar);
 *         free(astar);
 *         sleep(1);
 *     }
 *     endwin();
 *     return 0;
 * }
 *
 */


#ifndef _astar_h_
#define _astar_h_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef ASTAR_API
#define ASTAR_API extern
#endif // ASTAR_API

struct astar;

ASTAR_API int astar_memsize(void);

ASTAR_API void astar_init(struct astar *astar, int width, int height);

ASTAR_API void astar_unit(struct astar *astar);

ASTAR_API int astar_valid_pt(struct astar *astar, int x, int y);

ASTAR_API int astar_cross(struct astar *astar, int sx, int sy, int ex, int ey, int (*f)(struct astar *, int, int));

ASTAR_API void astar_block(struct astar *astar, int x, int y, int g, int block);

ASTAR_API void astar_link(struct astar *astar);

ASTAR_API int astar_find(struct astar *astar, int sx, int sy, int ex, int ey, int opt);

ASTAR_API int astar_next(struct astar *astar, int *px, int *py);

#ifdef __cplusplus
}
#endif

#endif // _astar_h_


#ifdef ASTAR_IMPLEMENTATION

//link: gist.github.com/zhoukk/f5366ce217e614b60ea4
#define HEAP_IMPLEMENTATION
#include "heap.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define _USE_MATH_DEFINES
#include <math.h>

#ifndef min
#define min(x,y) ({\
	typeof(x) _x = (x);\
	typeof(y) _y = (y);\
	(void)(&_x == &y);\
	_x < _y ? _x : _y;})
#endif

#ifndef max
#define max(x,y) ({\
	typeof(x) _x = (x);\
	typeof(y) _y = (y);\
	(void)(&_x == &y);\
	_x > _y ? _x : _y;})
#endif

#ifndef swap
#define swap(x,y) {x=x^y;y=x^y;x=x^y;}
#endif

struct astar_link {
	struct astar_point *p;
	int g;
};

struct astar_point {
	int x,y;
	int g,h;
	int b,vc,vo;
	struct astar_point *p;
	struct astar_link link[8];
};

struct astar {
	int width,height;
	struct astar_point **pt;
	struct heap *openlist;
	struct astar_point *path;
	int vc,vo;
};

ASTAR_API int
astar_memsize(void) {
	return sizeof(struct astar);
}

static int
heap_cmp(void *v1, void *v2) {
	struct astar_point *p1 = (struct astar_point *)v1;
	struct astar_point *p2 = (struct astar_point *)v2;
	return (p1->g + p1->h) < (p2->g + p2->h);
}

ASTAR_API void
astar_init(struct astar *astar, int width, int height) {
	int x,y;
	astar->width = width;
	astar->height = height;
	astar->vc = astar->vo = 0;
	astar->path = 0;
	astar->pt = (struct astar_point **)malloc(sizeof(struct astar_point *)*width);
	for (x = 0; x < width; x++) {
		astar->pt[x] = (struct astar_point *)malloc(sizeof(struct astar_point)*height);
		memset(astar->pt[x], 0, sizeof(struct astar_point)*height);
		for (y = 0; y < height; y++) {
			astar->pt[x][y].x = x;
			astar->pt[x][y].y = y;
		}
	}
	astar->openlist = heap_new(16, heap_cmp);
}

ASTAR_API void
astar_unit(struct astar *astar) {
	if (astar->width) {
		int x;
		for (x=0; x<astar->width; x++) {
			free(astar->pt[x]);
		}
		free(astar->pt);
		heap_free(astar->openlist);
	}
}

ASTAR_API int
astar_valid_pt(struct astar *astar, int x, int y) {
	return x >= 0 && y >= 0 && x < astar->width && y < astar->height && astar->pt[x][y].b == 0;
}

static int
astar_valid_rd(struct astar *astar, int x, int y) {
	if (!astar_valid_pt(astar, x-1, y-1) || !astar_valid_pt(astar, x, y-1) ||
		!astar_valid_pt(astar, x-1, y) || !astar_valid_pt(astar, x, y)) {
			return 0;
	}
	return 1;
}

static int
_astar_get_h(struct astar_point *cur, struct astar_point *end) {
	int dx,dy;
	dx = abs(cur->x - end->x);
	dy = abs(cur->y - end->y);
	return min(dx,dy)*3+abs(dx-dy)*2;
}

static int
_astar(struct astar *astar, struct astar_point *p, struct astar_point *e) {
	astar->vc++;
	astar->vo++;
	p->g = 0;
	p->p = 0;
	e->p = 0;
	heap_push(astar->openlist, p);
	p->vo = astar->vo;
	while ((p = heap_pop(astar->openlist))) {
		int i;
		if (p->x == e->x && p->y == e->y) {
			return 0;
		}
		p->vc = astar->vc;
		for (i = 0; i < 8; i++) {
			struct astar_point *t;
			int g, h, pg;
			t = p->link[i].p;
			if (!t || t->vc == astar->vc) {
				continue;
			}
			pg = p->link[i].g;
			g = p->g + pg;
			h = _astar_get_h(t, e);
			if (t->vo != astar->vo) {
				t->g = g;
				t->h = h;
				t->p = p;
				heap_push(astar->openlist, t);
				t->vo = astar->vo;
			}
			if (g + h < t->g + t->h) {
				t->g = g;
				t->h = h;
				t->p = p;
				heap_update(astar->openlist, t);
			}
		}
	}
	return -1;
}

static int
_bresenham(struct astar *astar, int sx, int sy, int ex, int ey, int (*f)(struct astar *, int, int)) {
	int dx, dy, x, y, _y, e;
	int d = abs(ey - sy) - abs(ex - sx);
	if(d > 0) {
		swap(sx, sy);
		swap(ex, ey);
	}
	if (sx > ex) {
		swap(sx, ex);
		swap(sy, ey);
	}
	dx = ex - sx;
	dy = ey - sy;
	e = dx >> 1;
	y = sy;
	_y = ((ey > sy) << 1) - 1;
	for (x = sx; x <= ex; x++) {
		if (d > 0) {
			if (!f(astar, y, x)) {
				return 0;
			}
		} else {
			if (!f(astar, x, y)) {
				return 0;
			}
		}
		e -= abs(dy);
		if (e < 0) {
			y += _y;
			e += dx;
		}
	}
	return 1;
}

ASTAR_API int
astar_cross(struct astar *astar, int sx, int sy, int ex, int ey, int (*f)(struct astar *, int, int)) {
	if (sx == ex && sy == ey) {
		return 1;
	}
	if (!f) {
		f = astar_valid_rd;
	}
	return _bresenham(astar, sx, sy, ex, ey, f);
}

static int
_floyd(struct astar *astar) {
	int n = 1;
	struct astar_point *p = astar->path;
	struct astar_point *p2 = p->p, *p3;
	if (p->p && p->p->p) {
		int dx,dy;
		p2 = p->p;
		p3 = p2->p;
		dx = p->x - p2->x;
		dy = p->y - p2->y;
		while (p3) {
			int tx = p2->x-p3->x;
			int ty = p2->y-p3->y;
			if (tx == dx && ty == dy) {
				p->p = p3;
			} else {
				dx = tx;
				dy = ty;
				p = p2;
			}
			p2 = p->p;
			p3 = p2->p;
		}
	}
	while (astar->path != p2 && astar->path->p != p2) {
		p = astar->path;
		while (p->p != p2) {
			if (astar_cross(astar, p->x, p->y, p2->x, p2->y, 0)) {
				break;
			}
			p = p->p;
			n ++;
		}
		p2 = p;
	}
	return n;
}

ASTAR_API int
astar_find(struct astar *astar, int sx, int sy, int ex, int ey, int opt) {
    if (sx < 0 || sy < 0 || sx >= astar->width || sy >= astar->height) {
        return 0;
    }
    if (ex < 0 || ey < 0 || ex >= astar->width || ey >= astar->height) {
        return 0;
    }
	struct astar_point *p = &astar->pt[sx][sy];
	struct astar_point *e = &astar->pt[ex][ey];
	astar->path = 0;
	if (p->x == e->x && p->y == e->y) {
		return 0;
	}
	heap_clear(astar->openlist);
	if (astar_cross(astar, sx, sy, ex, ey, 0)) {
		e->p = &astar->pt[sx][sy];
		e->p->p = 0;
	} else {
		if (0 != _astar(astar, p, e)) {
			return 0;
		}
	}
	int c = 0;
	while (e) {
		struct astar_point *n = e->p;
		if (!astar_valid_pt(astar, e->x, e->y)) {
			e->p = 0;
			astar->path = 0;
		} else {
			e->p = astar->path;
			astar->path = e;
			c ++;
		}
		e = n;
	}
	if (!astar->path) return 0;
	if (opt) {
		return _floyd(astar);
	}
	return c;
}

ASTAR_API void
astar_block(struct astar *astar, int x, int y, int g, int block) {
	astar->pt[x][y].b = block;
	astar->pt[x][y].g = g;
}

static void
_astar_init_link(struct astar *astar, struct astar_point *p) {
	int sx,sy,ex,ey,x,y,g=0,i=0,c;
	sx = max(0, p->x-1);
	ex = min(astar->width-1, p->x+1);
	sy = max(0, p->y-1);
	ey = min(astar->height-1, p->y+1);
	for (x=sx; x<=ex; x++) {
		for (y=sy; y<=ey; y++) {
			struct astar_point *t = &astar->pt[x][y];
			if (t == p) {
				continue;
			}
			c = t->g?t->g:1;
			if (!astar_valid_pt(astar, x, y)) {
				c = astar->width*astar->height*3;
			}
			if (x != p->x && y != p->y)	{
				if (!astar_valid_pt(astar, p->x, y) || !astar_valid_pt(astar, x, p->y)) {
					c = astar->width*astar->height*3;
				}
				g = 3;
			} else {
				g = 2;
			}
			p->link[i].g = g*c;
			p->link[i].p = t;
			i++;
		}
	}
}

ASTAR_API void
astar_link(struct astar *astar) {
	int i,j;
	for (i=0; i<astar->width; i++) {
		for (j=0; j<astar->height; j++) {
			_astar_init_link(astar, &astar->pt[i][j]);
		}
	}
}

ASTAR_API int
astar_next(struct astar *astar, int *px, int *py) {
	if (astar->path) {
		*px = astar->path->x;
		*py = astar->path->y;
		astar->path = astar->path->p;
		return 0;
	}
	return -1;
}

#endif // ASTAR_IMPLEMENTATION
