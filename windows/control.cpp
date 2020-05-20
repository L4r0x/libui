// 16 august 2015
#include "uipriv_windows.hpp"

void uiWindowsControlDefaultDestroy(uiControl *c)
{
	uiWindowsEnsureDestroyWindow((HWND)uiControlHandle(c));
	uiFreeControl(c);
}

uiControl *uiWindowsControlDefaultParent(uiControl *c)
{
	return uiWindowsControl(c)->parent;
}

void uiWindowsControlDefaultSetParent(uiControl *c, uiControl *parent)
{
	uiControlVerifySetParent(c, parent);
	uiWindowsControl(c)->parent = parent;
}

int uiWindowsControlDefaultToplevel(uiControl *c)
{
	return 0;
}

int uiWindowsControlDefaultVisible(uiControl *c)
{
	return uiWindowsControl(c)->visible;
}

void uiWindowsControlDefaultShow(uiControl *c)
{
	uiWindowsControl(c)->visible = 1;
	ShowWindow((HWND)uiControlHandle(c), SW_SHOW);
	uiWindowsControlNotifyVisibilityChanged(uiWindowsControl(c));
}

void uiWindowsControlDefaultHide(uiControl *c)
{
	uiWindowsControl(c)->visible = 0;
	ShowWindow((HWND)uiControlHandle(c), SW_HIDE);
	uiWindowsControlNotifyVisibilityChanged(uiWindowsControl(c));
}

int uiWindowsControlDefaultEnabled(uiControl *c)
{
	return uiWindowsControl(c)->enabled;
}

void uiWindowsControlDefaultEnable(uiControl *c)
{
	uiWindowsControl(c)->enabled = 1;
	uiWindowsControlSyncEnableState(uiWindowsControl(c), uiControlEnabledToUser(c));
}

void uiWindowsControlDefaultDisable(uiControl *c)
{
	uiWindowsControl(c)->enabled = 0;
	uiWindowsControlSyncEnableState(uiWindowsControl(c), uiControlEnabledToUser(c));
}

void uiWindowsControlDefaultSyncEnableState(uiWindowsControl *c, int enabled)
{
	if (uiWindowsShouldStopSyncEnableState(c, enabled))
		return;
	EnableWindow((HWND)uiControlHandle(uiControl(c)), enabled);
}

// note that there is no uiWindowsControlDefaultMinimumSize(); you MUST define this yourself!
void uiWindowsControlDefaultMinimumSizeChanged(uiWindowsControl *c)
{
	if (uiWindowsControlTooSmall(c)) {
		uiWindowsControlContinueMinimumSizeChanged(c);
		return;
	}
	/* otherwise do nothing; we have no children */
}

void uiWindowsControlSyncEnableState(uiWindowsControl *c, int enabled)
{
	(*(((uiWindowsControlFunctions *)c->c.functions)->SyncEnableState))(c, enabled);
}

void uiWindowsControlMinimumSize(uiWindowsControl *c, int *width, int *height)
{
	(*(((uiWindowsControlFunctions *)c->c.functions)->MinimumSize))(c, width, height);
}

void uiWindowsControlMinimumSizeChanged(uiWindowsControl *c)
{
	(*(((uiWindowsControlFunctions *)c->c.functions)->MinimumSizeChanged))(c);
}

void uiWindowsControlSetParentHWND(uiWindowsControl *c, HWND parent)
{
	uiWindowsEnsureSetParentHWND((HWND)uiControlHandle(uiControl(c)), parent);
}

void uiWindowsControlAssignControlIDZOrder(uiWindowsControl *c, LONG_PTR *controlID, HWND *insertAfter)
{
	uiWindowsEnsureAssignControlIDZOrder((HWND)uiControlHandle(uiControl(c)), controlID, insertAfter);
}

HWND uiWindowsEnsureCreateControlHWND(
	DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle,
	HINSTANCE hInstance, LPVOID lpParam, BOOL useStandardControlFont)
{
	HWND hwnd;

	// don't let using the arrow keys in a uiRadioButtons leave the radio buttons
	if ((dwStyle & WS_TABSTOP) != 0)
		dwStyle |= WS_GROUP;
	hwnd = CreateWindowExW(dwExStyle,
		lpClassName, lpWindowName,
		dwStyle | WS_CHILD | WS_VISIBLE,
		0, 0,
		// use a nonzero initial size just in case some control breaks with a zero initial size
		100, 100,
		utilWindow, NULL, hInstance, lpParam);
	if (hwnd == NULL) {
		logLastError(L"error creating window");
		// TODO return a decoy window
	}
	if (useStandardControlFont)
		SendMessageW(hwnd, WM_SETFONT, (WPARAM)hMessageFont, (LPARAM)TRUE);
	return hwnd;
}

// choose a value distinct from uiWindowSignature

uiWindowsControl *uiWindowsAllocControl(size_t n, uint32_t typesig, const char *typenamestr, uiWindowsControlFunctions *functions)
{
	uiWindowsControl *control = uiWindowsControl(uiAllocControl(n, typesig, typenamestr, (uiControlFunctions *)functions));
	control->visible = 1;
	control->enabled = 1;
	return control;
}

BOOL uiWindowsShouldStopSyncEnableState(uiWindowsControl *c, BOOL enabled)
{
	int ce;

	ce = uiControlEnabled(uiControl(c));
	// only stop if we're going from disabled back to enabled; don't stop under any other condition
	// (if we stop when going from enabled to disabled then enabled children of a disabled control won't get disabled at the OS level)
	if (!ce && enabled)
		return TRUE;
	return FALSE;
}

void uiWindowsControlAssignSoleControlIDZOrder(uiWindowsControl *c)
{
	LONG_PTR controlID;
	HWND insertAfter;

	controlID = 100;
	insertAfter = NULL;
	uiWindowsControlAssignControlIDZOrder(c, &controlID, &insertAfter);
}

BOOL uiWindowsControlTooSmall(uiWindowsControl *c)
{
	RECT r;
	int width, height;

	uiWindowsEnsureGetWindowRect((HWND)uiControlHandle(uiControl(c)), &r);
	uiWindowsControlMinimumSize(c, &width, &height);
	if ((r.right - r.left) < width)
		return TRUE;
	if ((r.bottom - r.top) < height)
		return TRUE;
	return FALSE;
}

void uiWindowsControlContinueMinimumSizeChanged(uiWindowsControl *c)
{
	uiControl *parent;

	parent = uiControlParent(uiControl(c));
	if (parent != NULL)
		uiWindowsControlMinimumSizeChanged(uiWindowsControl(parent));
}

// TODO rename this nad the OS X this and hugging ones to NotifyChild
void uiWindowsControlNotifyVisibilityChanged(uiWindowsControl *c)
{
	// TODO we really need to figure this out; the duplication is a mess
	uiWindowsControlContinueMinimumSizeChanged(c);
}
