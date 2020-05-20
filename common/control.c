#include "uipriv.h"

void uiControlDestroy(uiControl *c)
{
	(*(c->functions->Destroy))(c);
}

uintptr_t uiControlHandle(uiControl *c)
{
	return (*(c->functions->Handle))(c);
}

uiControl *uiControlParent(uiControl *c)
{
	return (*(c->functions->Parent))(c);
}

void uiControlSetParent(uiControl *c, uiControl *parent)
{
	(*(c->functions->SetParent))(c, parent);
}

int uiControlToplevel(uiControl *c)
{
	return (*(c->functions->Toplevel))(c);
}

int uiControlVisible(uiControl *c)
{
	return (*(c->functions->Visible))(c);
}

void uiControlShow(uiControl *c)
{
	(*(c->functions->Show))(c);
}

void uiControlHide(uiControl *c)
{
	(*(c->functions->Hide))(c);
}

int uiControlEnabled(uiControl *c)
{
	return (*(c->functions->Enabled))(c);
}

void uiControlEnable(uiControl *c)
{
	(*(c->functions->Enable))(c);
}

void uiControlDisable(uiControl *c)
{
	(*(c->functions->Disable))(c);
}

uiControl *uiAllocControl(size_t size, uint32_t typesig, const char *typenamestr, uiControlFunctions *functions)
{
	uiControl *control = (uiControl *) uiprivAlloc(size, typenamestr);
	control->TypeSignature = typesig;
	control->functions = functions;
	return control;
}

void uiFreeControl(uiControl *c)
{
	if (uiControlParent(c) != NULL)
		uiprivUserBug("You cannot destroy a uiControl while it still has a parent. (control: %p)", c);
	uiprivFree(c);
}

void uiControlVerifySetParent(uiControl *c, uiControl *parent)
{
	uiControl *curParent;

	if (uiControlToplevel(c))
		uiprivUserBug("You cannot give a toplevel uiControl a parent. (control: %p)", c);
	curParent = uiControlParent(c);
	if (parent != NULL && curParent != NULL)
		uiprivUserBug("You cannot give a uiControl a parent while it already has one. (control: %p; current parent: %p; new parent: %p)", c, curParent, parent);
	if (parent == NULL && curParent == NULL)
		uiprivImplBug("attempt to double unparent uiControl %p", c);
}

int uiControlEnabledToUser(uiControl *c)
{
	while (c != NULL) {
		if (!uiControlEnabled(c))
			return 0;
		c = uiControlParent(c);
	}
	return 1;
}
