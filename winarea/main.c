// 4 september 2015
#define _GNU_SOURCE
#include "area.h"
#include <math.h>

// #qo LIBS: user32 kernel32 gdi32 msimg32

struct handler {
	uiAreaHandler ah;
};

static HWND area;
static struct handler h;

static void handlerDraw(uiAreaHandler *a, uiArea *area, uiAreaDrawParams *p)
{
	uiDrawStrokeParams sp;

	uiDrawBeginPathRGB(p->Context, 0xFF, 0x00, 0x00);
	uiDrawMoveTo(p->Context, p->ClipX + 5, p->ClipY + 5);
	uiDrawLineTo(p->Context, (p->ClipX + p->ClipWidth) - 5, (p->ClipY + p->ClipHeight) - 5);
	sp.Cap = uiDrawLineCapFlat;
	sp.Join = uiDrawLineJoinMiter;
	sp.Thickness = 1;
	sp.MiterLimit = uiDrawDefaultMiterLimit;
	uiDrawStroke(p->Context, &sp);

	uiDrawBeginPathRGB(p->Context, 0x00, 0x00, 0xC0);
	uiDrawMoveTo(p->Context, p->ClipX, p->ClipY);
	uiDrawLineTo(p->Context, p->ClipX + p->ClipWidth, p->ClipY);
	uiDrawLineTo(p->Context, 50, 150);
	uiDrawLineTo(p->Context, 50, 50);
	uiDrawCloseFigure(p->Context);
	sp.Cap = uiDrawLineCapFlat;
	sp.Join = uiDrawLineJoinRound;
	sp.Thickness = 5;
	uiDrawStroke(p->Context, &sp);

	uiDrawBeginPathRGBA(p->Context, 0x00, 0xC0, 0x00, 0x80);
	uiDrawRectangle(p->Context, 120, 80, 50, 50);
	uiDrawFill(p->Context, uiDrawFillModeWinding);

	uiDrawBeginPathRGB(p->Context, 0x00, 0x80, 0x00);
	uiDrawMoveTo(p->Context, 5, 10);
	uiDrawLineTo(p->Context, 5, 50);
	sp.Cap = uiDrawLineCapFlat;
	sp.Join = uiDrawLineJoinMiter;
	sp.Thickness = 1;
	sp.MiterLimit = uiDrawDefaultMiterLimit;
	uiDrawStroke(p->Context, &sp);

	uiDrawBeginPathRGB(p->Context, 0x80, 0xC0, 0x00);
	uiDrawMoveTo(p->Context, 400, 100);
	uiDrawArcTo(p->Context,
		400, 100,
		50,
		30. * (M_PI / 180.),
		// note the end angle here
		// in GDI, the second angle to AngleArc() is relative to the start, not to 0
		330. * (M_PI / 180.),
		1);
	// TODO add a checkbox for this
	uiDrawLineTo(p->Context, 400, 100);
	uiDrawArcTo(p->Context,
		510, 100,
		50,
		30. * (M_PI / 180.),
		330. * (M_PI / 180.),
		0);
	uiDrawCloseFigure(p->Context);
	sp.Cap = uiDrawLineCapFlat;
	sp.Join = uiDrawLineJoinMiter;
	sp.Thickness = 1;
	sp.MiterLimit = uiDrawDefaultMiterLimit;
	uiDrawStroke(p->Context, &sp);

	uiDrawBeginPathRGB(p->Context, 0x00, 0x80, 0xC0);
	uiDrawMoveTo(p->Context, 300, 300);
	uiDrawBezierTo(p->Context,
		350, 320,
		310, 390,
		435, 372);
	sp.Cap = uiDrawLineCapFlat;
	sp.Join = uiDrawLineJoinMiter;
	sp.Thickness = 1;
	sp.MiterLimit = uiDrawDefaultMiterLimit;
	uiDrawStroke(p->Context, &sp);
}

static uintmax_t handlerHScrollMax(uiAreaHandler *a, uiArea *area)
{
return 0;//TODO	return gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(nhspinb));
}

static uintmax_t handlerVScrollMax(uiAreaHandler *a, uiArea *area)
{
return 0;//TODO	return gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(nvspinb));
}

/* TODO
static void recalcScroll(GtkSpinButton *sb, gpointer data)
{
	areaUpdateScroll(area);
}

static GtkWidget *makeSpinbox(int min)
{
	GtkWidget *sb;

	sb = gtk_spin_button_new_with_range(min, 100000, 1);
	gtk_spin_button_set_digits(GTK_SPIN_BUTTON(sb), 0);
	g_signal_connect(sb, "value-changed", G_CALLBACK(recalcScroll), NULL);
	return sb;
}
*/

static void repos(HWND hwnd)
{
	RECT r;

	GetClientRect(hwnd, &r);
	SetWindowPos(area, NULL, r.left + 12, r.top + 12,
		r.right - r.left - 24, r.bottom - r.top - 24,
		SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER);
}

static LRESULT CALLBACK wndproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	WINDOWPOS *wp = (WINDOWPOS *) lParam;

	switch (uMsg) {
	case WM_WINDOWPOSCHANGED:
		if ((wp->flags & SWP_NOSIZE) != 0)
			break;
		repos(hwnd);
		return 0;
	case WM_CLOSE:
		PostQuitMessage(0);
	}
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

HINSTANCE hInstance;

int main(void)
{
	WNDCLASSW wc;
	HWND mainwin;
	MSG msg;

	hInstance = GetModuleHandle(NULL);

	h.ah.Draw = handlerDraw;
	h.ah.HScrollMax = handlerHScrollMax;
	h.ah.VScrollMax = handlerVScrollMax;

	registerAreaClass();

	ZeroMemory(&wc, sizeof (WNDCLASSW));
	wc.lpszClassName = L"mainwin";
	wc.lpfnWndProc = wndproc;
	wc.hInstance = hInstance;
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	RegisterClassW(&wc);

	mainwin = CreateWindowExW(0,
		L"mainwin", L"mainwin",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, hInstance, NULL);

	area = makeArea(0,
		WS_CHILD | WS_VISIBLE,
		0, 0, 0, 0,
		mainwin,
		(uiAreaHandler *) (&h));

/* TODO
	gtk_grid_attach(GTK_GRID(grid),
		gtk_label_new("H Count"),
		0, 0, 1, 1);
	nhspinb = makeSpinbox(0);
	gtk_grid_attach(GTK_GRID(grid), nhspinb,
		1, 0, 1, 1);

	gtk_grid_attach(GTK_GRID(grid),
		gtk_label_new("V Count"),
		0, 1, 1, 1);
	nvspinb = makeSpinbox(0);
	gtk_grid_attach(GTK_GRID(grid), nvspinb,
		1, 1, 1, 1);
*/

	repos(mainwin);
	ShowWindow(mainwin, SW_SHOWDEFAULT);
	UpdateWindow(mainwin);

	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}
