// 10 june 2015
#include "uipriv_unix.h"

struct uiButton {
	uiUnixControl c;
	GtkWidget *widget;
	void (*onClicked)(uiButton *, void *);
	void *onClickedData;
};

static void onClicked(GtkButton *button, gpointer data)
{
	uiButton *b = uiButton(data);
	if (b->onClicked) {
		(*(b->onClicked))(b, b->onClickedData);
	}
}

char *uiButtonText(uiButton *b)
{
	return uiUnixStrdupText(gtk_button_get_label(GTK_BUTTON(b->widget)));
}

void uiButtonSetText(uiButton *b, const char *text)
{
	gtk_button_set_label(GTK_BUTTON(b->widget), text);
}

void uiButtonOnClicked(uiButton *b, void (*f)(uiButton *, void *), void *data)
{
	b->onClicked = f;
	b->onClickedData = data;
}

uiUnixControlDefaultHandle(uiButton)
uiUnixControlFunctionsDefault(uiButton)

uiButton *uiNewButton(const char *text)
{
	uiButton *b = uiUnixNewControl(uiButton);
	b->widget = gtk_button_new_with_label(text);
	b->onClicked = NULL;
	b->onClickedData = NULL;

	g_signal_connect(b->widget, "clicked", G_CALLBACK(onClicked), b);

	return b;
}
