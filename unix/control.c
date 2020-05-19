#include "uipriv_unix.h"

uiUnixControl *uiUnixAllocControl(size_t n, uint32_t typesig, const char *typenamestr, uiControlFunctions *functions)
{
	uiControl *control = uiAllocControl(n, typesig, typenamestr, functions);
	return uiUnixControl(control);
}
