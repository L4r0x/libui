// 21 april 2016

/*
This file assumes that you have included <windows.h> and "ui.h" beforehand. It provides API-specific functions for interfacing with foreign controls in Windows.
*/

#ifndef __LIBUI_UI_WINDOWS_H__
#define __LIBUI_UI_WINDOWS_H__

#include <windows.h>

#include "ui.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct uiWindowsSizing uiWindowsSizing;
typedef struct uiWindowsControl uiWindowsControl;

struct uiWindowsControlFunctions {
	uiControlFunctions c;
	void (*SyncEnableState)(uiWindowsControl *, int);
	void (*MinimumSize)(uiWindowsControl *, int *, int *);
	void (*MinimumSizeChanged)(uiWindowsControl *);
};

struct uiWindowsControl {
	uiControl c;
	uiControl *parent;
	BOOL enabled;
	BOOL visible;
};

#define uiWindowsControl(this) ((uiWindowsControl *)(this))
// Default functions
_UI_EXTERN void uiWindowsControlDefaultDestroy(uiControl *c);
_UI_EXTERN uiControl *uiWindowsControlDefaultParent(uiControl *c);
_UI_EXTERN void uiWindowsControlDefaultSetParent(uiControl *c, uiControl *parent);
_UI_EXTERN int uiWindowsControlDefaultToplevel(uiControl *c);
_UI_EXTERN int uiWindowsControlDefaultVisible(uiControl *c);
_UI_EXTERN void uiWindowsControlDefaultShow(uiControl *c);
_UI_EXTERN void uiWindowsControlDefaultHide(uiControl *c);
_UI_EXTERN int uiWindowsControlDefaultEnabled(uiControl *c);
_UI_EXTERN void uiWindowsControlDefaultEnable(uiControl *c);
_UI_EXTERN void uiWindowsControlDefaultDisable(uiControl *c);
// Windows specific default functions
_UI_EXTERN void uiWindowsControlDefaultSyncEnableState(uiWindowsControl *, int);
_UI_EXTERN void uiWindowsControlDefaultSetParentHWND(uiWindowsControl *, HWND);
_UI_EXTERN void uiWindowsControlDefaultMinimumSize(uiWindowsControl *, int *, int *);
_UI_EXTERN void uiWindowsControlDefaultMinimumSizeChanged(uiWindowsControl *);
_UI_EXTERN void uiWindowsControlDefaultLayoutRect(uiWindowsControl *, RECT *);
_UI_EXTERN void uiWindowsControlDefaultAssignControlIDZOrder(uiWindowsControl *, LONG_PTR *, HWND *);
_UI_EXTERN void uiWindowsControlDefaultChildVisibilityChanged(uiWindowsControl *);
// Calls the specified handler on the given control
_UI_EXTERN void uiWindowsControlSyncEnableState(uiWindowsControl *, int);
_UI_EXTERN void uiWindowsControlSetParentHWND(uiWindowsControl *, HWND);
_UI_EXTERN void uiWindowsControlMinimumSize(uiWindowsControl *, int *, int *);
_UI_EXTERN void uiWindowsControlMinimumSizeChanged(uiWindowsControl *);
_UI_EXTERN void uiWindowsControlLayoutRect(uiWindowsControl *, RECT *);
_UI_EXTERN void uiWindowsControlAssignControlIDZOrder(uiWindowsControl *, LONG_PTR *, HWND *);
_UI_EXTERN void uiWindowsControlChildVisibilityChanged(uiWindowsControl *);

#define uiWindowsControlDefaultHandle(type)     \
	static uintptr_t type##Handle(uiControl *c) \
	{                                           \
		return (uintptr_t)(type(c)->hwnd);      \
	}

#define uiWindowsControlFunctionsDefault(type)           \
	static uiWindowsControlFunctions type##Functions = { \
		{                                                \
			uiWindowsControlDefaultDestroy,              \
			type##Handle,                                \
			uiWindowsControlDefaultParent,               \
			uiWindowsControlDefaultSetParent,            \
			uiWindowsControlDefaultToplevel,             \
			uiWindowsControlDefaultVisible,              \
			uiWindowsControlDefaultShow,                 \
			uiWindowsControlDefaultHide,                 \
			uiWindowsControlDefaultEnabled,              \
			uiWindowsControlDefaultEnable,               \
			uiWindowsControlDefaultDisable,              \
		},                                               \
		type##SyncEnableState,                           \
		type##MinimumSize,                               \
		type##MinimumSizeChanged,                        \
	};

#define uiWindowsControlFunctionsDefaultExceptDestroy(type) \
	static uiWindowsControlFunctions type##Functions = {    \
		{                                                   \
			type##Destroy,                                  \
			type##Handle,                                   \
			uiWindowsControlDefaultParent,                  \
			uiWindowsControlDefaultSetParent,               \
			uiWindowsControlDefaultToplevel,                \
			uiWindowsControlDefaultVisible,                 \
			uiWindowsControlDefaultShow,                    \
			uiWindowsControlDefaultHide,                    \
			uiWindowsControlDefaultEnabled,                 \
			uiWindowsControlDefaultEnable,                  \
			uiWindowsControlDefaultDisable,                 \
		},                                                  \
		type##SyncEnableState,                              \
		type##MinimumSize,                                  \
		type##MinimumSizeChanged,                           \
	};

#define uiWindowsControlFunctions(type)                  \
	static uiWindowsControlFunctions type##Functions = { \
		{                                                \
			type##Destroy,                               \
			type##Handle,                                \
			type##Parent,                                \
			type##SetParent,                             \
			type##Toplevel,                              \
			type##Visible,                               \
			type##Show,                                  \
			type##Hide,                                  \
			type##Enabled,                               \
			type##Enable,                                \
			type##Disable,                               \
		},                                               \
		type##SyncEnableState,                           \
		type##MinimumSize,                               \
		type##MinimumSizeChanged,                        \
	};

// TODO document
#define uiWindowsNewControl(type) \
	type(uiWindowsAllocControl(sizeof(type), type##Signature, #type, &type##Functions))

// TODO document
_UI_EXTERN uiWindowsControl *uiWindowsAllocControl(
	size_t n, uint32_t typesig, const char *typenamestr, uiWindowsControlFunctions *functions);

// TODO document
_UI_EXTERN HWND uiWindowsEnsureCreateControlHWND(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, HINSTANCE hInstance, LPVOID lpParam, BOOL useStandardControlFont);

// TODO document
_UI_EXTERN void uiWindowsEnsureDestroyWindow(HWND hwnd);

// TODO document
// TODO document that this should only be used in SetParentHWND() implementations
_UI_EXTERN void uiWindowsEnsureSetParentHWND(HWND hwnd, HWND parent);

// TODO document
_UI_EXTERN void uiWindowsEnsureAssignControlIDZOrder(HWND hwnd, LONG_PTR *controlID, HWND *insertAfter);

// TODO document
_UI_EXTERN void uiWindowsEnsureGetClientRect(HWND hwnd, RECT *r);
_UI_EXTERN void uiWindowsEnsureGetWindowRect(HWND hwnd, RECT *r);

// TODO document
_UI_EXTERN char *uiWindowsWindowText(HWND hwnd);
_UI_EXTERN void uiWindowsSetWindowText(HWND hwnd, const char *text);

// TODO document
_UI_EXTERN int uiWindowsWindowTextWidth(HWND hwnd);

// TODO document
// TODO point out this should only be used in a resize cycle
_UI_EXTERN void uiWindowsEnsureMoveWindowDuringResize(HWND hwnd, int x, int y, int width, int height);

// TODO document
_UI_EXTERN void uiWindowsRegisterWM_COMMANDHandler(HWND hwnd, BOOL (*handler)(uiControl *, HWND, WORD, LRESULT *), uiControl *c);
_UI_EXTERN void uiWindowsUnregisterWM_COMMANDHandler(HWND hwnd);

// TODO document
_UI_EXTERN void uiWindowsRegisterWM_NOTIFYHandler(HWND hwnd, BOOL (*handler)(uiControl *, HWND, NMHDR *, LRESULT *), uiControl *c);
_UI_EXTERN void uiWindowsUnregisterWM_NOTIFYHandler(HWND hwnd);

// TODO document
_UI_EXTERN void uiWindowsRegisterWM_HSCROLLHandler(HWND hwnd, BOOL (*handler)(uiControl *, HWND, WORD, LRESULT *), uiControl *c);
_UI_EXTERN void uiWindowsUnregisterWM_HSCROLLHandler(HWND hwnd);

// TODO document
_UI_EXTERN void uiWindowsRegisterReceiveWM_WININICHANGE(HWND hwnd);
_UI_EXTERN void uiWindowsUnregisterReceiveWM_WININICHANGE(HWND hwnd);

// TODO document
typedef struct uiWindowsSizing uiWindowsSizing;
struct uiWindowsSizing {
	int BaseX;
	int BaseY;
	LONG InternalLeading;
};
_UI_EXTERN void uiWindowsGetSizing(HWND hwnd, uiWindowsSizing *sizing);
_UI_EXTERN void uiWindowsSizingDlgUnitsToPixels(uiWindowsSizing *sizing, int *x, int *y);
_UI_EXTERN void uiWindowsSizingStandardPadding(uiWindowsSizing *sizing, int *x, int *y);

// TODO document
_UI_EXTERN HWND uiWindowsMakeContainer(uiWindowsControl *c, void (*onResize)(uiWindowsControl *));

// TODO document
_UI_EXTERN BOOL uiWindowsControlTooSmall(uiWindowsControl *c);
_UI_EXTERN void uiWindowsControlContinueMinimumSizeChanged(uiWindowsControl *c);

// TODO document
_UI_EXTERN void uiWindowsControlAssignSoleControlIDZOrder(uiWindowsControl *);

// TODO document
_UI_EXTERN BOOL uiWindowsShouldStopSyncEnableState(uiWindowsControl *c, int enabled);

// TODO document
_UI_EXTERN void uiWindowsControlNotifyVisibilityChanged(uiWindowsControl *c);

#ifdef __cplusplus
}
#endif

#endif
