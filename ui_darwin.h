/*
This file provides API-specific functions for interfacing with foreign controls on Mac OS X.
*/

#ifndef __LIBUI_UI_DARWIN_H__
#define __LIBUI_UI_DARWIN_H__

#include <Cocoa/Cocoa.h>

#include "ui.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct uiDarwinControl uiDarwinControl;
typedef struct uiDarwinControlFunctions uiDarwinControlFunctions;

struct uiDarwinControlFunctions {
	uiControlFunctions c;
	void (*SyncEnableState)(uiDarwinControl *, int);
	void (*SetSuperview)(uiDarwinControl *, NSView *);
	BOOL (*HugsTrailingEdge)(uiDarwinControl *);
	BOOL (*HugsBottom)(uiDarwinControl *);
	void (*ChildEdgeHuggingChanged)(uiDarwinControl *);
	NSLayoutPriority (*HuggingPriority)(uiDarwinControl *, NSLayoutConstraintOrientation);
	void (*SetHuggingPriority)(uiDarwinControl *, NSLayoutPriority, NSLayoutConstraintOrientation);
	void (*ChildVisibilityChanged)(uiDarwinControl *);
};

struct uiDarwinControl {
	uiControl c;
	uiControl *parent;
	BOOL enabled;
	BOOL visible;
};

#define uiDarwinControl(this) ((uiDarwinControl *) (this))
// Default implementations of the ui control functions
_UI_EXTERN void uiDarwinControlDefaultDestroy(uiControl *c);
_UI_EXTERN uiControl *uiDarwinControlDefaultParent(uiControl *c);
_UI_EXTERN void uiDarwinControlDefaultSetParent(uiControl *c, uiControl *parent);
_UI_EXTERN int uiDarwinControlDefaultToplevel(uiControl *c);
_UI_EXTERN int uiDarwinControlDefaultVisible(uiControl *c);
_UI_EXTERN void uiDarwinControlDefaultShow(uiControl *c);
_UI_EXTERN void uiDarwinControlDefaultHide(uiControl *c);
_UI_EXTERN int uiDarwinControlDefaultEnabled(uiControl *c);
_UI_EXTERN void uiDarwinControlDefaultEnable(uiControl *c);
_UI_EXTERN void uiDarwinControlDefaultDisable(uiControl *c);
// Cocoa specific functions
_UI_EXTERN void uiDarwinControlDefaultSyncEnableState(uiDarwinControl *c, int enabled);
_UI_EXTERN void uiDarwinControlDefaultSetSuperview(uiDarwinControl *c, NSView *superview);
_UI_EXTERN BOOL uiDarwinControlDefaultHugsTrailingEdge(uiDarwinControl *c);
_UI_EXTERN BOOL uiDarwinControlDefaultHugsBottom(uiDarwinControl *c);
_UI_EXTERN void uiDarwinControlDefaultChildEdgeHuggingChanged(uiDarwinControl *c);
NSLayoutPriority uiDarwinControlDefaultHuggingPriority(
	uiDarwinControl *c, NSLayoutConstraintOrientation orientation);
_UI_EXTERN void uiDarwinControlDefaultSetHuggingPriority(
	uiDarwinControl *c, NSLayoutPriority priority, NSLayoutConstraintOrientation orientation);
_UI_EXTERN void uiDarwinControlDefaultChildVisibilityChanged(uiDarwinControl *c);
// Calls the specified handler on the given control
_UI_EXTERN void uiDarwinControlSyncEnableState(uiDarwinControl *c, int enabled);
_UI_EXTERN void uiDarwinControlSetSuperview(uiDarwinControl *c, NSView *superview);
_UI_EXTERN BOOL uiDarwinControlHugsTrailingEdge(uiDarwinControl *c);
_UI_EXTERN BOOL uiDarwinControlHugsBottom(uiDarwinControl *c);
_UI_EXTERN void uiDarwinControlChildEdgeHuggingChanged(uiDarwinControl *c);
_UI_EXTERN NSLayoutPriority uiDarwinControlHuggingPriority(
	uiDarwinControl *c, NSLayoutConstraintOrientation orientation);
_UI_EXTERN void uiDarwinControlSetHuggingPriority(
	uiDarwinControl *c, NSLayoutPriority priority, NSLayoutConstraintOrientation orientation);
_UI_EXTERN void uiDarwinControlChildVisibilityChanged(uiDarwinControl *c);

#define uiDarwinControlDefaultHandle(type, handlefield) \
	static uintptr_t type ## Handle(uiControl *c) \
	{ \
		return (uintptr_t) (type(c)->handlefield); \
	}

#define uiDarwinControlFunctionsDefault(type)           \
	static uiDarwinControlFunctions type##Functions = { \
		{                                               \
			uiDarwinControlDefaultDestroy,              \
			type##Handle,                               \
			uiDarwinControlDefaultParent,               \
			uiDarwinControlDefaultSetParent,            \
			uiDarwinControlDefaultToplevel,             \
			uiDarwinControlDefaultVisible,              \
			uiDarwinControlDefaultShow,                 \
			uiDarwinControlDefaultHide,                 \
			uiDarwinControlDefaultEnabled,              \
			uiDarwinControlDefaultEnable,               \
			uiDarwinControlDefaultDisable,              \
		},                                              \
		type##SyncEnableState,                          \
		type##SetSuperview,                             \
		type##HugsTrailingEdge,                         \
		type##HugsBottom,                               \
		type##ChildEdgeHuggingChanged,                  \
		type##HuggingPriority,                          \
		type##SetHuggingPriority,                       \
		type##ChildVisibilityChanged,                   \
	};

#define uiDarwinControlFunctionsDefaultExceptDestroy(type) \
	static uiDarwinControlFunctions type##Functions = {    \
		{                                                  \
			type##Destroy,                                 \
			type##Handle,                                  \
			uiDarwinControlDefaultParent,                  \
			uiDarwinControlDefaultSetParent,               \
			uiDarwinControlDefaultToplevel,                \
			uiDarwinControlDefaultVisible,                 \
			uiDarwinControlDefaultShow,                    \
			uiDarwinControlDefaultHide,                    \
			uiDarwinControlDefaultEnabled,                 \
			uiDarwinControlDefaultEnable,                  \
			uiDarwinControlDefaultDisable,                 \
		},                                                 \
		type##SyncEnableState,                             \
		type##SetSuperview,                                \
		type##HugsTrailingEdge,                            \
		type##HugsBottom,                                  \
		type##ChildEdgeHuggingChanged,                     \
		type##HuggingPriority,                             \
		type##SetHuggingPriority,                          \
		type##ChildVisibilityChanged,                      \
	};

#define uiDarwinControlFunctions(type)                  \
	static uiDarwinControlFunctions type##Functions = { \
		{                                               \
			type##Destroy,                              \
			type##Handle,                               \
			type##Parent,                               \
			type##SetParent,                            \
			type##Toplevel,                             \
			type##Visible,                              \
			type##Show,                                 \
			type##Hide,                                 \
			type##Enabled,                              \
			type##Enable,                               \
			type##Disable,                              \
		},                                              \
		type##SyncEnableState,                          \
		type##SetSuperview,                             \
		type##HugsTrailingEdge,                         \
		type##HugsBottom,                               \
		type##ChildEdgeHuggingChanged,                  \
		type##HuggingPriority,                          \
		type##SetHuggingPriority,                       \
		type##ChildVisibilityChanged,                   \
	};

// Macro for allocating new controls of a specific type
#define uiDarwinNewControl(type) \
	type(uiDarwinAllocControl(sizeof (type), type ## Signature, #type, &type##Functions))

// Allocate new controls of a specific type
_UI_EXTERN uiDarwinControl *uiDarwinAllocControl(
	size_t n, uint32_t typesig, const char *typenamestr, uiDarwinControlFunctions *functions);

// Use this function as a shorthand for setting control fonts.
_UI_EXTERN void uiDarwinSetControlFont(NSControl *c, NSControlSize size);

// You can use this function from within your control implementations to return text strings that can be freed with uiFreeText().
_UI_EXTERN char *uiDarwinNSStringToText(NSString *);

// TODO document
_UI_EXTERN BOOL uiDarwinShouldStopSyncEnableState(uiDarwinControl *, BOOL);

// TODO document
_UI_EXTERN void uiDarwinNotifyEdgeHuggingChanged(uiDarwinControl *);
_UI_EXTERN void uiDarwinNotifyVisibilityChanged(uiDarwinControl *c);

// TODO document
// TODO document that values should not be cached
_UI_EXTERN CGFloat uiDarwinMarginAmount(void *reserved);
_UI_EXTERN CGFloat uiDarwinPaddingAmount(void *reserved);

#ifdef __cplusplus
}
#endif

#endif
