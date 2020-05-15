// 18 april 2015
#include "uipriv_unix.h"

void uiprivSetMargined(GtkContainer *c, int margined)
{
	if (margined)
		gtk_container_set_border_width(c, uiprivGTKXMargin);
	else
		gtk_container_set_border_width(c, 0);
}

void uiprivSetControlMargined(GtkWidget *widget, int margined)
{
	if (margined) {
		gtk_widget_set_margin_top(widget, uiprivGTKYMargin);
		gtk_widget_set_margin_bottom(widget, uiprivGTKYMargin);
		gtk_widget_set_margin_left(widget, uiprivGTKXMargin);
		gtk_widget_set_margin_right(widget, uiprivGTKXMargin);

	} else {
		gtk_widget_set_margin_top(widget, 0);
		gtk_widget_set_margin_bottom(widget, 0);
		gtk_widget_set_margin_left(widget, 0);
		gtk_widget_set_margin_right(widget, 0);
	}
}

int uiprivChildMargined(GtkWidget *widget)
{
	return gtk_widget_get_margin_top(widget) > 0;
}
