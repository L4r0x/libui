// 22 april 2015
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include "../ui.h"

// main.c
extern void die(const char *, ...);
extern uiBox *mainBox;
extern uiTab *mainTab;
extern uiBox *(*newhbox)(void);
extern uiBox *(*newvbox)(void);

// spaced.c
extern void setSpaced(int);
extern void querySpaced(char[12]);
extern uiWindow *newWindow(const char *title, int width, int height, int hasMenubar);
extern uiBox *newHorizontalBox(void);
extern uiBox *newVerticalBox(void);
extern uiTab *newTab(void);
extern uiGroup *newGroup(const char *);
extern uiGrid *newGrid(void);

// menus.c
extern uiMenuItem *shouldQuitItem;
extern void initMenus(void);

// page1.c
extern uiBox *page1;
extern void makePage1(uiWindow *);

// page2.c
extern uiGroup *page2group;
extern uiBox *makePage2(void);

// page3.c
extern uiBox *makePage3(void);

// page4.c
extern uiBox *makePage4(void);

// page5.c
extern uiBox *makePage5(uiWindow *);

// removed draw area and form

// page11.c
extern uiBox *makePage11(void);

// page12.c
extern uiBox *makePage12(void);

// page13.c
extern uiBox *makePage13(void);

// page14.c
extern uiTab *makePage14(void);

// page15.c
extern uiBox *makePage15(uiWindow *);

// page16.c
extern uiBox *makePage16(void);
extern void freePage16(void);

// images.c
extern void appendImageNamed(uiImage *img, const char *name);
