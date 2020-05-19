#include "uipriv_unix.h"

struct uiGroup {
	uiUnixControl c;
	GtkWidget *widget;
	uiControl *child;
	int margined;
};

static void uiGroupDestroy(uiControl *control)
{
	uiGroup *group = uiGroup(control);
	// free child
	if (group->child != NULL) {
		uiControlSetParent(group->child, NULL);
		uiControlDestroy(group->child);
	}
	// and then ourselves
	g_object_unref(group->widget);
	uiFreeControl(control);
}

char *uiGroupTitle(uiGroup *group)
{
	return uiUnixStrdupText(gtk_frame_get_label(GTK_FRAME(group->widget)));
}

void uiGroupSetTitle(uiGroup *group, const char *text)
{
	gtk_frame_set_label(GTK_FRAME(group->widget), text);
}

void uiGroupSetChild(uiGroup *group, uiControl *child)
{
	if (group->child != NULL) {
		GtkWidget *widget = GTK_WIDGET(uiControlHandle(group->child));
		gtk_container_remove(GTK_CONTAINER(group->widget), widget);
		uiControlSetParent(group->child, NULL);
		uiprivWidgetSetMargined(widget, FALSE);
	}

	group->child = child;
	if (child != NULL) {
		GtkWidget *widget = GTK_WIDGET(uiControlHandle(child));
		if (!uiUnixControl(child)->explicitlyHidden)
			gtk_widget_show(widget);
		gtk_widget_set_hexpand(widget, TRUE);
		gtk_widget_set_halign(widget, GTK_ALIGN_FILL);
		gtk_widget_set_vexpand(widget, TRUE);
		gtk_widget_set_valign(widget, GTK_ALIGN_FILL);
		g_object_ref(widget); // Add reference as we destroy it manually.
		gtk_container_add(GTK_CONTAINER(group->widget), widget);

		uiControlSetParent(group->child, uiControl(group));
		uiGroupSetMargined(group, group->margined);
	}
}

int uiGroupMargined(uiGroup *group)
{
	return group->margined;
}

void uiGroupSetMargined(uiGroup *group, int margined)
{
	group->margined = margined;
	if (group->child != NULL) {
		GtkWidget *widget = GTK_WIDGET(uiControlHandle(group->child));
		uiprivWidgetSetMargined(widget, margined);
	}
}

uiUnixControlDefaultHandle(uiGroup)
uiUnixControlFunctionsDefaultExceptDestroy(uiGroup)

uiGroup *uiNewGroup(const char *text)
{
	uiGroup *group = uiUnixNewControl(uiGroup);

	group->widget = gtk_frame_new(text);

	// with GTK+, groupboxes by default have frames and slightly x-offset regular text
	// they should have no frame and fully left-justified, bold text
	// preserve default y-alignment
	gfloat yalign;
	gtk_frame_get_label_align(GTK_FRAME(group->widget), NULL, &yalign);
	gtk_frame_set_label_align(GTK_FRAME(group->widget), 0, yalign);
	gtk_frame_set_shadow_type(GTK_FRAME(group->widget), GTK_SHADOW_NONE);
	GtkLabel *label = GTK_LABEL(gtk_frame_get_label_widget(GTK_FRAME(group->widget)));
	// this is the boldness level used by GtkPrintUnixDialog
	// (it technically uses "bold" but see pango's pango-enum-types.c for the name conversion; GType is weird)
	PangoAttribute *bold = pango_attr_weight_new(PANGO_WEIGHT_BOLD);
	PangoAttrList *boldlist = pango_attr_list_new();
	pango_attr_list_insert(boldlist, bold);
	gtk_label_set_attributes(label, boldlist);
	pango_attr_list_unref(boldlist);
	return group;
}
