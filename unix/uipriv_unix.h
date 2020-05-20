#ifndef __LIBUI_PRIV_UNIX_H__
#define __LIBUI_PRIV_UNIX_H__

#define GLIB_VERSION_MIN_REQUIRED GLIB_VERSION_2_40
#define GLIB_VERSION_MAX_ALLOWED GLIB_VERSION_2_40
#define GDK_VERSION_MIN_REQUIRED GDK_VERSION_3_10
#define GDK_VERSION_MAX_ALLOWED GDK_VERSION_3_10
#include <gtk/gtk.h>

#include "../ui.h"
#include "../ui_unix.h"
#include "../common/uipriv.h"

#define uiprivGTKXMargin 12
#define uiprivGTKYMargin 12
#define uiprivGTKXPadding 12
#define uiprivGTKYPadding 6

// menu.c
extern GtkWidget *uiprivMakeMenubar(uiWindow *);
extern void uiprivFreeMenubar(GtkWidget *);
extern void uiprivUninitMenus(void);

// alloc.c
extern void uiprivInitAlloc(void);
extern void uiprivUninitAlloc(void);

// util.c
void uiprivWidgetSetMargined(GtkWidget *widget, int margined);
int uiprivWidgetMargined(GtkWidget *widget);

// image.c
extern cairo_surface_t *uiprivImageAppropriateSurface(uiImage *i, GtkWidget *w);

// cellrendererbutton.c
extern GtkCellRenderer *uiprivNewCellRendererButton(void);

// future.c
extern void uiprivLoadFutures(void);
extern gboolean uiprivFUTURE_gtk_widget_path_iter_set_object_name(GtkWidgetPath *path, gint pos, const char *name);

#endif
