// 11 june 2015
#include "uipriv_unix.h"

struct uiTab {
	uiUnixControl c;
	GtkWidget *widget;
	GPtrArray *pages;
};

uiUnixControlAllDefaultsExceptDestroy(uiTab)

static void uiTabDestroy(uiControl *control)
{
	uiTab *tab = uiTab(control);

	for (guint i = 0; i < tab->pages->len; i++) {
		uiControl *page = g_ptr_array_index(tab->pages, i);
		uiControlSetParent(page, NULL);
		uiControlDestroy(page);
	}
	g_ptr_array_free(tab->pages, TRUE);

	// and free ourselves
	g_object_unref(tab->widget);
	uiFreeControl(uiControl(tab));
}

void uiTabAppend(uiTab *tab, const char *name, uiControl *child)
{
	uiTabInsertAt(tab, name, uiTabNumPages(tab), child);
}

void uiTabInsertAt(uiTab *tab, const char *name, int n, uiControl *child)
{
	GtkWidget *widget = GTK_WIDGET(uiControlHandle(child));
	gtk_widget_show(widget);

	// Child expands and fills the page
	gtk_widget_set_hexpand(widget, TRUE);
	gtk_widget_set_halign(widget, GTK_ALIGN_FILL);
	gtk_widget_set_vexpand(widget, TRUE);
	gtk_widget_set_valign(widget, GTK_ALIGN_FILL);

	// Add widget to noteboot
	g_object_ref(widget); //  Add reference as we destroy it manually.
	gtk_notebook_insert_page(GTK_NOTEBOOK(tab->widget), widget, gtk_label_new(name), n);

	g_ptr_array_insert(tab->pages, n, child);
	uiControlSetParent(child, uiControl(tab));
}

void uiTabDelete(uiTab *tab, int n)
{
	uiTabSetMargined(tab, n, FALSE); // remove margins
	uiControl *page = g_ptr_array_index(tab->pages, n);
	uiControlSetParent(page, NULL);
	g_ptr_array_remove_index(tab->pages, n);

	gtk_notebook_remove_page(GTK_NOTEBOOK(tab->widget), n);
	gtk_widget_queue_draw(tab->widget); // Need to refresh the widget
}

int uiTabNumPages(uiTab *tab)
{
	return gtk_notebook_get_n_pages(GTK_NOTEBOOK(tab->widget));
}

int uiTabMargined(uiTab *tab, int n)
{
	GtkWidget *widget = gtk_notebook_get_nth_page(GTK_NOTEBOOK(tab->widget), n);
	return uiprivChildMargined(widget);
}

void uiTabSetMargined(uiTab *tab, int n, int margined)
{
	GtkWidget *widget = gtk_notebook_get_nth_page(GTK_NOTEBOOK(tab->widget), n);
	uiprivSetControlMargined(widget, margined);
}

uiTab *uiNewTab(void)
{
	uiTab *tab;
	uiUnixNewControl(uiTab, tab);
	tab->widget = gtk_notebook_new();
	tab->pages = g_ptr_array_new();

	gtk_notebook_set_scrollable(GTK_NOTEBOOK(tab->widget), TRUE);

	return tab;
}
