#include "uipriv_unix.h"

struct uiBox {
	uiUnixControl c;
	GtkWidget *widget;
	GPtrArray *children;
	int vertical;
	// ensures all stretchy controls have the same size
	GtkSizeGroup *stretchygroup;
};

uiUnixControlAllDefaultsExceptDestroy(uiBox)

static void uiBoxDestroy(uiControl *control)
{
	uiBox *box = uiBox(control);
	// free size group
	g_object_unref(box->stretchygroup);
	// free children
	for (guint i = 0; i < box->children->len; i++) {
		uiControl *child = g_ptr_array_index(box->children, i);
		uiControlSetParent(child, NULL);
		uiControlDestroy(child);
	}
	g_ptr_array_free(box->children, TRUE);
	// and then ourselves
	g_object_unref(box->widget);
	uiFreeControl(uiControl(box));
}

void uiBoxAppend(uiBox *box, uiControl *child, int stretchy)
{
	GtkWidget *widget = GTK_WIDGET(uiControlHandle(child));
	if (!uiUnixControl(child)->explicitlyHidden)
		gtk_widget_show(widget);
	gtk_widget_set_hexpand(widget, FALSE);
	gtk_widget_set_halign(widget, GTK_ALIGN_FILL);
	gtk_widget_set_vexpand(widget, FALSE);
	gtk_widget_set_valign(widget, GTK_ALIGN_FILL);

	g_object_ref(widget); // Add reference as we destroy it manually.
	gtk_box_pack_start(GTK_BOX(box->widget), widget, FALSE, TRUE, 0);

	if (stretchy) {
		gtk_size_group_add_widget(box->stretchygroup, widget);
		if (box->vertical) {
			gtk_widget_set_vexpand(widget, TRUE);
		} else {
			gtk_widget_set_hexpand(widget, TRUE);
		}
	}

	uiControlSetParent(child, uiControl(box));
	g_ptr_array_add(box->children, child);
}

void uiBoxDelete(uiBox *box, int index)
{
	uiControl *child = g_ptr_array_index(box->children, index);
	uiControlSetParent(child, NULL);
	g_ptr_array_remove_index(box->children, index);

	GtkWidget *widget = GTK_WIDGET(uiControlHandle(child));
	gtk_container_remove(GTK_CONTAINER(box->widget), widget);
	gtk_size_group_remove_widget(box->stretchygroup, widget);
}

int uiBoxPadded(uiBox *box)
{
	return gtk_box_get_spacing(GTK_BOX(box->widget)) > 0;
}

void uiBoxSetPadded(uiBox *box, int padded)
{
	if (padded) {
		if (box->vertical) {
			gtk_box_set_spacing(GTK_BOX(box->widget), uiprivGTKYPadding);
		} else {
			gtk_box_set_spacing(GTK_BOX(box->widget), uiprivGTKXPadding);
		}
	} else {
		gtk_box_set_spacing(GTK_BOX(box->widget), 0);
	}
}

static uiBox *uiprivNewBox(GtkOrientation orientation)
{
	uiBox *box;
	uiUnixNewControl(uiBox, box);
	box->widget = gtk_box_new(orientation, 0);
	box->vertical = orientation == GTK_ORIENTATION_VERTICAL;

	if (box->vertical) {
		box->stretchygroup = gtk_size_group_new(GTK_SIZE_GROUP_VERTICAL);
	} else {
		box->stretchygroup = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	}

	box->children = g_ptr_array_new();

	return box;
}

uiBox *uiNewHorizontalBox(void)
{
	return uiprivNewBox(GTK_ORIENTATION_HORIZONTAL);
}

uiBox *uiNewVerticalBox(void)
{
	return uiprivNewBox(GTK_ORIENTATION_VERTICAL);
}
