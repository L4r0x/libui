#include "uipriv_unix.h"

struct uiGrid {
	uiUnixControl c;
	GtkWidget *widget;
	GPtrArray *children;
};

uiUnixControlAllDefaultsExceptDestroy(uiGrid)

static void uiGridDestroy(uiControl *control)
{
	uiGrid *grid = uiGrid(control);
	// free children
	for (guint i = 0; i < grid->children->len; i++) {
		uiControl *child = g_ptr_array_index(grid->children, i);
		uiControlSetParent(child, NULL);
		uiControlDestroy(child);
	}
	g_ptr_array_free(grid->children, TRUE);
	// and then ourselves
	g_object_unref(grid->widget);
	uiFreeControl(uiControl(grid));
}

static const GtkAlign gtkAligns[] = {
	[uiAlignFill] = GTK_ALIGN_FILL,
	[uiAlignStart] = GTK_ALIGN_START,
	[uiAlignCenter] = GTK_ALIGN_CENTER,
	[uiAlignEnd] = GTK_ALIGN_END,
};

static const GtkPositionType gtkPositions[] = {
	[uiAtLeading] = GTK_POS_LEFT,
	[uiAtTop] = GTK_POS_TOP,
	[uiAtTrailing] = GTK_POS_RIGHT,
	[uiAtBottom] = GTK_POS_BOTTOM,
};

static GtkWidget *prepare(uiControl *child, int hexpand, uiAlign halign, int vexpand, uiAlign valign)
{
	GtkWidget *widget = GTK_WIDGET(uiControlHandle(child));
	gtk_widget_set_hexpand(widget, hexpand != 0);
	gtk_widget_set_halign(widget, gtkAligns[halign]);
	gtk_widget_set_vexpand(widget, vexpand != 0);
	gtk_widget_set_valign(widget, gtkAligns[valign]);

	if (!uiUnixControl(child)->explicitlyHidden)
		gtk_widget_show(widget);
	return widget;
}

void uiGridAppend(uiGrid *grid, uiControl *child, int left, int top, int xspan, int yspan, int hexpand, uiAlign halign, int vexpand, uiAlign valign)
{
	GtkWidget *widget = prepare(child, hexpand, halign, vexpand, valign);

	g_object_ref(widget); //  Add reference as we destroy it manually.
	gtk_grid_attach(GTK_GRID(grid->widget), widget,
		left, top,
		xspan, yspan);

	g_ptr_array_add(grid->children, child);
	uiControlSetParent(child, uiControl(grid));
}

void uiGridInsertAt(uiGrid *grid, uiControl *child, uiControl *existing, uiAt at, int xspan, int yspan, int hexpand, uiAlign halign, int vexpand, uiAlign valign)
{
	GtkWidget *widget = prepare(child, hexpand, halign, vexpand, valign);

	g_object_ref(widget); //  Add reference as we destroy it manually.
	gtk_grid_attach_next_to(GTK_GRID(grid->widget), widget,
		GTK_WIDGET(uiControlHandle(existing)), gtkPositions[at],
		xspan, yspan);

	g_ptr_array_add(grid->children, child);
	uiControlSetParent(child, uiControl(grid));
}

int uiGridPadded(uiGrid *grid)
{
	return gtk_grid_get_row_spacing(GTK_GRID(grid->widget)) > 0;
}

void uiGridSetPadded(uiGrid *grid, int padded)
{
	if (padded) {
		gtk_grid_set_row_spacing(GTK_GRID(grid->widget), uiprivGTKYPadding);
		gtk_grid_set_column_spacing(GTK_GRID(grid->widget), uiprivGTKXPadding);
	} else {
		gtk_grid_set_row_spacing(GTK_GRID(grid->widget), 0);
		gtk_grid_set_column_spacing(GTK_GRID(grid->widget), 0);
	}
}

uiUnixDefineControlFunctions(uiGrid)

uiGrid *uiNewGrid(void)
{
	uiGrid *grid;
	uiUnixNewControl(uiGrid, grid);
	grid->widget = gtk_grid_new();
	grid->children = g_ptr_array_new();

	return grid;
}
