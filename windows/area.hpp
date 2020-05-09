#ifndef __LIBUI_UI_AREA_H__
#define __LIBUI_UI_AREA_H__

#include "uipriv_windows.hpp"

// TODOs
// - things look very wrong on initial draw
// - initial scrolling is not set properly
// - should background be inherited from parent control?

struct uiArea {
	uiWindowsControl c;
	HWND hwnd;
	uiAreaHandler *ah;

	BOOL scrolling;
	int scrollWidth;
	int scrollHeight;
	int hscrollpos;
	int vscrollpos;
	int hwheelCarry;
	int vwheelCarry;

	uiprivClickCounter cc;
	BOOL capturing;

	BOOL inside;
	BOOL tracking;

	ID2D1HwndRenderTarget *rt;
};

// areadraw.cpp
extern BOOL areaDoDraw(uiArea *a, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lResult);
extern void areaDrawOnResize(uiArea *, RECT *);

// areascroll.cpp
extern BOOL areaDoScroll(uiArea *a, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lResult);
extern void areaScrollOnResize(uiArea *, RECT *);
extern void areaUpdateScroll(uiArea *a);

// areaevents.cpp
extern BOOL areaDoEvents(uiArea *a, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lResult);

// areautil.cpp
extern void loadAreaSize(uiArea *a, ID2D1RenderTarget *rt, double *width, double *height);
extern void pixelsToDIP(uiArea *a, double *x, double *y);
extern void dipToPixels(uiArea *a, double *x, double *y);

#endif
