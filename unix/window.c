// 11 june 2015
#include "uipriv_unix.h"

// The window consists of an outer box holding the menubar and the content box.
// The content box contains the child control and is used for calculating
// the windows content size.
struct uiWindow {
	uiUnixControl c;
	GtkWidget *widget;
	GtkWidget *box;
	GtkWidget *menubar;
	GtkWidget *content;
	uiControl *child;
	int margined;
	gboolean fullscreen;

	int (*onClosing)(uiWindow *, void *);
	void *onClosingData;
	void (*onContentSizeChanged)(uiWindow *, void *);
	void *onContentSizeChangedData;
};

static gboolean onClosing(GtkWidget *widget, GdkEvent *e, gpointer data)
{
	uiWindow *window = uiWindow(data);

	// manually destroy the window ourselves
	if (window->onClosing && (*(window->onClosing))(window, window->onClosingData))
		uiControlDestroy(uiControl(window));
	// don't continue to the default delete-event handler
	return TRUE;
}

static void onSizeAllocate(GtkWidget *widget, GdkRectangle *allocation, gpointer data)
{
	uiWindow *window = uiWindow(data);

	// TODO deal with spurious size-allocates
	if (window->onContentSizeChanged) {
		(*(window->onContentSizeChanged))(window, window->onContentSizeChangedData);
	}
}

static void uiWindowDestroy(uiControl *control)
{
	uiWindow *window = uiWindow(control);

	// first hide ourselves
	gtk_widget_hide(window->widget);
	// now destroy the child
	if (window->child != NULL) {
		uiControlSetParent(window->child, NULL);
		uiControlDestroy(window->child);
	}
	// now destroy the menus, if any
	if (window->menubar != NULL)
		uiprivFreeMenubar(window->menubar);
	gtk_widget_destroy(window->content);
	gtk_widget_destroy(window->box);
	// and finally free ourselves
	// use gtk_widget_destroy() instead of g_object_unref() because GTK+ has internal references (see #165)
	gtk_widget_destroy(window->widget);
	uiFreeControl(uiControl(window));
}


uiControl *uiWindowParent(uiControl *control)
{
	return NULL;
}

void uiWindowSetParent(uiControl *control, uiControl *parent)
{
	uiUserBugCannotSetParentOnToplevel("uiWindow");
}

static int uiWindowToplevel(uiControl *control)
{
	return 1;
}

static void uiWindowShow(uiControl *control)
{
	uiWindow *window = uiWindow(control);
	// gtk_widget_show() doesn't bring to front or give keyboard focus
	gtk_window_present(GTK_WINDOW(window->widget));
}

char *uiWindowTitle(uiWindow *window)
{
	return uiUnixStrdupText(gtk_window_get_title(GTK_WINDOW(window->widget)));
}

void uiWindowSetTitle(uiWindow *window, const char *title)
{
	gtk_window_set_title(GTK_WINDOW(window->widget), title);
}

void uiWindowContentSize(uiWindow *window, int *width, int *height)
{
	GtkAllocation allocation;

	gtk_widget_get_allocation(window->content, &allocation);
	*width = allocation.width;
	*height = allocation.height;
}

void uiWindowSetContentSize(uiWindow *window, int width, int height)
{
	// we need to resize the child holder widget to the given size
	// we can't resize that without running the event loop
	// but we can do gtk_window_set_size()
	// so how do we deal with the differences in sizes?
	// simple arithmetic, of course!

	// from what I can tell, the return from gtk_widget_get_allocation(w->window)
	// and gtk_window_get_size(w->window) will be the same
	// this is not affected by Wayland and not affected by GTK+ builtin CSD
	// so we can safely juse use them to get the real window size!
	// since we're using gtk_window_resize(), use the latter
	gint winWidth, winHeight;
	gtk_window_get_size(GTK_WINDOW(window->widget), &winWidth, &winHeight);

	// now get the child holder widget's current allocation
	GtkAllocation childAlloc;
	gtk_widget_get_allocation(window->content, &childAlloc);
	// and punch that out of the window size
	winWidth -= childAlloc.width;
	winHeight -= childAlloc.height;

	// now we just need to add the new size back in
	winWidth += width;
	winHeight += height;
	// and set it
	// this will not move the window in my tests, so we're good
	gtk_window_resize(GTK_WINDOW(window->widget), winWidth, winHeight);
}

int uiWindowFullscreen(uiWindow *window)
{
	return window->fullscreen;
}

// TODO use window-state-event to track
// TODO does this send an extra size changed?
// TODO what behavior do we want?
void uiWindowSetFullscreen(uiWindow *window, int fullscreen)
{
	window->fullscreen = fullscreen;
	if (window->fullscreen)
		gtk_window_fullscreen(GTK_WINDOW(window->widget));
	else
		gtk_window_unfullscreen(GTK_WINDOW(window->widget));
}

void uiWindowOnContentSizeChanged(uiWindow *window, void (*f)(uiWindow *, void *), void *data)
{
	window->onContentSizeChanged = f;
	window->onContentSizeChangedData = data;
}

void uiWindowOnClosing(uiWindow *window, int (*f)(uiWindow *, void *), void *data)
{
	window->onClosing = f;
	window->onClosingData = data;
}

int uiWindowBorderless(uiWindow *window)
{
	return !gtk_window_get_decorated(GTK_WINDOW(window->widget));
}

void uiWindowSetBorderless(uiWindow *window, int borderless)
{
	gtk_window_set_decorated(GTK_WINDOW(window->widget), !borderless);
}

// TODO save and restore expands and aligns
void uiWindowSetChild(uiWindow *window, uiControl *child)
{
	if (window->child != NULL) {
		GtkWidget *widget = GTK_WIDGET(uiControlHandle(window->child));
		gtk_container_remove(GTK_CONTAINER(window->content), widget);
		uiControlSetParent(window->child, NULL);
		uiprivWidgetSetMargined(widget, FALSE);
	}
	window->child = child;
	if (window->child != NULL) {
		GtkWidget *widget = GTK_WIDGET(uiControlHandle(child));
		if (!uiUnixControl(child)->explicitlyHidden)
			gtk_widget_show(widget);
		gtk_widget_set_hexpand(widget, TRUE);
		gtk_widget_set_halign(widget, GTK_ALIGN_FILL);
		gtk_widget_set_vexpand(widget, TRUE);
		gtk_widget_set_valign(widget, GTK_ALIGN_FILL);
		g_object_ref(widget); // Add reference as we destroy it manually.
		gtk_container_add(GTK_CONTAINER(window->content), widget);

		uiControlSetParent(window->child, uiControl(window));
		uiWindowSetMargined(window, window->margined);
	}
}

int uiWindowMargined(uiWindow *window)
{
	return window->margined;
}

void uiWindowSetMargined(uiWindow *window, int margined)
{
	window->margined = margined;
	GtkWidget *widget = GTK_WIDGET(uiControlHandle(window->child));
	uiprivWidgetSetMargined(widget, margined);
}

#define uiWindowHandle uiUnixControlHandle
#define uiWindowVisible uiUnixControlVisible
#define uiWindowHide uiUnixControlHide
#define uiWindowEnabled uiUnixControlEnabled
#define uiWindowEnable uiUnixControlEnable
#define uiWindowDisable uiUnixControlDisable
uiUnixControlDefaultHandle(uiWindow)
uiUnixControlFunctions(uiWindow)

uiWindow *uiNewWindow(const char *title, int width, int height, int hasMenubar)
{
	uiWindow *window = uiUnixNewControl(uiWindow);
	window->widget = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window->widget), title);
	gtk_window_resize(GTK_WINDOW(window->widget), width, height);

	// The box contains the menubar and the content box
	window->box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add(GTK_CONTAINER(window->widget), window->box);
	gtk_widget_show(window->box);

	if (hasMenubar) {
		window->menubar = uiprivMakeMenubar(uiWindow(window));
		gtk_container_add(GTK_CONTAINER(window->box), window->menubar);
		gtk_widget_show_all(window->menubar);
	}

	// Create the content box which holds the windows child
	window->content = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_container_add(GTK_CONTAINER(window->box), window->content);
	gtk_widget_show(window->content);

	// and connect our events
	uiWindowOnClosing(window, NULL, NULL);
	g_signal_connect(window->widget, "delete-event", G_CALLBACK(onClosing), window);

	uiWindowOnContentSizeChanged(window, NULL, NULL);
	g_signal_connect(window->content, "size-allocate", G_CALLBACK(onSizeAllocate), window);

	return window;
}
