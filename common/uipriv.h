// 6 april 2015
// note: this file should not include ui.h, as the OS-specific ui_*.h files are included between that one and this one in the OS-specific uipriv_*.h* files

#ifndef __LIBUI_UI_COMMON_UIPRIV_H__
#define __LIBUI_UI_COMMON_UIPRIV_H__

#include <stdarg.h>
#include <string.h>

#include "../ui.h"

#include "controlsigs.h"
#include "utf.h"

#ifdef __cplusplus
extern "C" {
#endif

// OS-specific init.* or main.* files
extern uiInitOptions uiprivOptions;

// OS-specific alloc.* files
extern void *uiprivAlloc(size_t, const char *);
#define uiprivNew(T) ((T *) uiprivAlloc(sizeof (T), #T))
extern void *uiprivRealloc(void *, size_t, const char *);
extern void uiprivFree(void *);

// debug.c and OS-specific debug.* files
// TODO get rid of this mess...
// ugh, __func__ was only introduced in MSVC 2015...
#ifdef _MSC_VER
#define uiprivMacro__func__ __FUNCTION__
#else
#define uiprivMacro__func__ __func__
#endif
extern void uiprivRealBug(const char *file, const char *line, const char *func, const char *prefix, const char *format, va_list ap);
#define uiprivMacro_ns2(s) #s
#define uiprivMacro_ns(s) uiprivMacro_ns2(s)
extern void uiprivDoImplBug(const char *file, const char *line, const char *func, const char *format, ...);
#define uiprivImplBug(...) uiprivDoImplBug(__FILE__, uiprivMacro_ns(__LINE__), uiprivMacro__func__, __VA_ARGS__)
extern void uiprivDoUserBug(const char *file, const char *line, const char *func, const char *format, ...);
#define uiprivUserBug(...) uiprivDoUserBug(__FILE__, uiprivMacro_ns(__LINE__), uiprivMacro__func__, __VA_ARGS__)

// shouldquit.c
extern int uiprivShouldQuit(void);

// OS-specific text.* files
extern int uiprivStricmp(const char *a, const char *b);

#ifdef __cplusplus
}
#endif

#endif
