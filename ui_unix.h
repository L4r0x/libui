// 7 april 2015

/*
This file assumes that you have included <gtk/gtk.h> and "ui.h" beforehand. It provides API-specific functions for interfacing with foreign controls on Unix systems that use GTK+ to provide their UI (currently all except Mac OS X).
*/

#ifndef __LIBUI_UI_UNIX_H__
#define __LIBUI_UI_UNIX_H__

#include <gtk/gtk.h>

#include "ui.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct uiUnixControl uiUnixControl;
struct uiUnixControl {
	uiControl c;
	uiControl *parent;
	gboolean explicitlyHidden;
};
#define uiUnixControl(this) ((uiUnixControl *) (this))

#define uiUnixControlDefaultDestroy(type) \
	static void type ## Destroy(uiControl *c) \
	{ \
		g_object_unref(type(c)->widget); \
		uiFreeControl(c); \
	}
#define uiUnixControlDefaultHandle(type) \
	static uintptr_t type ## Handle(uiControl *c) \
	{ \
		return (uintptr_t) (type(c)->widget); \
	}
#define uiUnixControlDefaultParent(type) \
	static uiControl *type ## Parent(uiControl *c) \
	{ \
		return uiUnixControl(c)->parent; \
	}
#define uiUnixControlDefaultSetParent(type) \
	static void type ## SetParent(uiControl *c, uiControl *parent) \
	{ \
		uiControlVerifySetParent(c, parent); \
		uiUnixControl(c)->parent = parent; \
	}
#define uiUnixControlDefaultToplevel(type) \
	static int type ## Toplevel(uiControl *c) \
	{ \
		return 0; \
	}
#define uiUnixControlDefaultVisible(type) \
	static int type ## Visible(uiControl *c) \
	{ \
		return gtk_widget_get_visible(type(c)->widget); \
	}
#define uiUnixControlDefaultShow(type) \
	static void type ## Show(uiControl *c) \
	{ \
		uiUnixControl(c)->explicitlyHidden=FALSE; \
		gtk_widget_show(type(c)->widget); \
	}
#define uiUnixControlDefaultHide(type) \
	static void type ## Hide(uiControl *c) \
	{ \
		uiUnixControl(c)->explicitlyHidden=TRUE; \
		gtk_widget_hide(type(c)->widget); \
	}
#define uiUnixControlDefaultEnabled(type) \
	static int type ## Enabled(uiControl *c) \
	{ \
		return gtk_widget_get_sensitive(type(c)->widget); \
	}
#define uiUnixControlDefaultEnable(type) \
	static void type ## Enable(uiControl *c) \
	{ \
		gtk_widget_set_sensitive(type(c)->widget, TRUE); \
	}
#define uiUnixControlDefaultDisable(type) \
	static void type ## Disable(uiControl *c) \
	{ \
		gtk_widget_set_sensitive(type(c)->widget, FALSE); \
	}

#define uiUnixControlAllDefaultsExceptDestroy(type) \
	uiUnixControlDefaultHandle(type) \
	uiUnixControlDefaultParent(type) \
	uiUnixControlDefaultSetParent(type) \
	uiUnixControlDefaultToplevel(type) \
	uiUnixControlDefaultVisible(type) \
	uiUnixControlDefaultShow(type) \
	uiUnixControlDefaultHide(type) \
	uiUnixControlDefaultEnabled(type) \
	uiUnixControlDefaultEnable(type) \
	uiUnixControlDefaultDisable(type)

#define uiUnixControlAllDefaults(type) \
	uiUnixControlDefaultDestroy(type) \
	uiUnixControlAllDefaultsExceptDestroy(type)

#define uiUnixDefineControlFunctions(type)        \
	static uiControlFunctions type##Functions = { \
		.Destroy = type##Destroy,                 \
		.Handle = type##Handle,                   \
		.Parent = type##Parent,                   \
		.SetParent = type##SetParent,             \
		.Toplevel = type##Toplevel,               \
		.Visible = type##Visible,                 \
		.Show = type##Show,                       \
		.Hide = type##Hide,                       \
		.Enabled = type##Enabled,                 \
		.Enable = type##Enable,                   \
		.Disable = type##Disable,                 \
	};

// TODO document
#define uiUnixNewControl(type, var) \
	var = type(uiUnixAllocControl(sizeof(type), type##Signature, #type, &type##Functions));

// TODO document
_UI_EXTERN uiUnixControl *uiUnixAllocControl(size_t n, uint32_t typesig, const char *typenamestr, uiControlFunctions *functions);

// uiUnixStrdupText() takes the given string and produces a copy of it suitable for being freed by uiFreeText().
_UI_EXTERN char *uiUnixStrdupText(const char *);

#ifdef __cplusplus
}
#endif

#endif
