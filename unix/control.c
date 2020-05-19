#include "uipriv_unix.h"

uiUnixControl *uiUnixAllocControl(size_t n, uint32_t typesig, const char *typenamestr, uiControlFunctions *functions)
{
	uiControl *control = uiAllocControl(n, typesig, typenamestr, functions);
	return uiUnixControl(control);
}

void uiUnixControlDestroy(uiControl *c)
{
	g_object_unref(GTK_WIDGET(uiControlHandle(c)));
	uiFreeControl(c);
}

uiControl *uiUnixControlParent(uiControl *c)
{
	return uiUnixControl(c)->parent;
}

void uiUnixControlSetParent(uiControl *c, uiControl *parent)
{
	uiControlVerifySetParent(c, parent);
	uiUnixControl(c)->parent = parent;
}

int uiUnixControlToplevel(uiControl *c)
{
	return 0;
}

int uiUnixControlVisible(uiControl *c)
{
	return gtk_widget_get_visible(GTK_WIDGET(uiControlHandle(c)));
}

void uiUnixControlShow(uiControl *c)
{
	uiUnixControl(c)->explicitlyHidden = FALSE;
	gtk_widget_show(GTK_WIDGET(uiControlHandle(c)));
}

void uiUnixControlHide(uiControl *c)
{
	uiUnixControl(c)->explicitlyHidden = TRUE;
	gtk_widget_hide(GTK_WIDGET(uiControlHandle(c)));
}

int uiUnixControlEnabled(uiControl *c)
{
	return gtk_widget_get_sensitive(GTK_WIDGET(uiControlHandle(c)));
}

void uiUnixControlEnable(uiControl *c)
{
	gtk_widget_set_sensitive(GTK_WIDGET(uiControlHandle(c)), TRUE);
}

void uiUnixControlDisable(uiControl *c)
{
	gtk_widget_set_sensitive(GTK_WIDGET(uiControlHandle(c)), FALSE);
}
