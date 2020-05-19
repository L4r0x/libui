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
#define uiUnixControl(this) ((uiUnixControl *)(this))

void uiUnixControlDestroy(uiControl *c);
uiControl *uiUnixControlParent(uiControl *c);
void uiUnixControlSetParent(uiControl *c, uiControl *parent);
int uiUnixControlToplevel(uiControl *c);
int uiUnixControlVisible(uiControl *c);
void uiUnixControlShow(uiControl *c);
void uiUnixControlHide(uiControl *c);
int uiUnixControlEnabled(uiControl *c);
void uiUnixControlEnable(uiControl *c);
void uiUnixControlDisable(uiControl *c);

#define uiUnixControlDefaultHandle(type)        \
	static uintptr_t type##Handle(uiControl *c) \
	{                                           \
		return (uintptr_t)(type(c)->widget);    \
	}

#define uiUnixControlFunctionsDefaultExceptDestroy(type) \
	static uiControlFunctions type##Functions = {        \
		.Destroy = type##Destroy,                        \
		.Handle = type##Handle,                          \
		.Parent = uiUnixControlParent,                   \
		.SetParent = uiUnixControlSetParent,             \
		.Toplevel = uiUnixControlToplevel,               \
		.Visible = uiUnixControlVisible,                 \
		.Show = uiUnixControlShow,                       \
		.Hide = uiUnixControlHide,                       \
		.Enabled = uiUnixControlEnabled,                 \
		.Enable = uiUnixControlEnable,                   \
		.Disable = uiUnixControlDisable,                 \
	};

#define uiUnixControlFunctionsDefault(type)       \
	static uiControlFunctions type##Functions = { \
		.Destroy = uiUnixControlDestroy,          \
		.Handle = type##Handle,                   \
		.Parent = uiUnixControlParent,            \
		.SetParent = uiUnixControlSetParent,      \
		.Toplevel = uiUnixControlToplevel,        \
		.Visible = uiUnixControlVisible,          \
		.Show = uiUnixControlShow,                \
		.Hide = uiUnixControlHide,                \
		.Enabled = uiUnixControlEnabled,          \
		.Enable = uiUnixControlEnable,            \
		.Disable = uiUnixControlDisable,          \
	};

#define uiUnixControlFunctions(type)              \
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

// Macro for allocating new controls of a specific type
#define uiUnixNewControl(type) \
	type(uiUnixAllocControl(sizeof(type), type##Signature, #type, &type##Functions))

// Allocate new controls of a specific type
_UI_EXTERN uiUnixControl *uiUnixAllocControl(
	size_t n, uint32_t typesig, const char *typenamestr, uiControlFunctions *functions);

// Produces a copy of the string suitable for being freed by uiFreeText().
_UI_EXTERN char *uiUnixStrdupText(const char *string);

#ifdef __cplusplus
}
#endif

#endif
