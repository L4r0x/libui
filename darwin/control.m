#include "ui_darwin.h"
#import "uipriv_darwin.h"

void uiDarwinControlSyncEnableState(uiDarwinControl *c, int state)
{
	(*(((uiDarwinControlFunctions *)c->c.functions)->SyncEnableState))(c, state);
}

void uiDarwinControlSetSuperview(uiDarwinControl *c, NSView *superview)
{
	(*(((uiDarwinControlFunctions *)c->c.functions)->SetSuperview))(c, superview);
}

BOOL uiDarwinControlHugsTrailingEdge(uiDarwinControl *c)
{
	return (*(((uiDarwinControlFunctions *)c->c.functions)->HugsTrailingEdge))(c);
}

BOOL uiDarwinControlHugsBottom(uiDarwinControl *c)
{
	return (*(((uiDarwinControlFunctions *)c->c.functions)->HugsBottom))(c);
}

void uiDarwinControlChildEdgeHuggingChanged(uiDarwinControl *c)
{
	(*(((uiDarwinControlFunctions *)c->c.functions)->ChildEdgeHuggingChanged))(c);
}

NSLayoutPriority uiDarwinControlHuggingPriority(uiDarwinControl *c, NSLayoutConstraintOrientation orientation)
{
	return (*(((uiDarwinControlFunctions *)c->c.functions)->HuggingPriority))(c, orientation);
}

void uiDarwinControlSetHuggingPriority(uiDarwinControl *c, NSLayoutPriority priority, NSLayoutConstraintOrientation orientation)
{
	(*(((uiDarwinControlFunctions *)c->c.functions)->SetHuggingPriority))(c, priority, orientation);
}

void uiDarwinControlChildVisibilityChanged(uiDarwinControl *c)
{
	(*(((uiDarwinControlFunctions *)c->c.functions)->ChildVisibilityChanged))(c);
}

void uiDarwinSetControlFont(NSControl *c, NSControlSize size)
{
	[c setFont:[NSFont systemFontOfSize:[NSFont systemFontSizeForControlSize:size]]];
}

uiDarwinControl *uiDarwinAllocControl(size_t n, uint32_t typesig, const char *typenamestr, uiDarwinControlFunctions *functions)
{
	uiDarwinControl *control =  uiDarwinControl(uiAllocControl(n, typesig, typenamestr, (uiControlFunctions*) functions));
	control->enabled = YES;
	control->visible = YES;
	return control;
}

BOOL uiDarwinShouldStopSyncEnableState(uiDarwinControl *c, BOOL enabled)
{
	int ce;

	ce = uiControlEnabled(uiControl(c));
	// only stop if we're going from disabled back to enabled; don't stop under any other condition
	// (if we stop when going from enabled to disabled then enabled children of a disabled control won't get disabled at the OS level)
	if (!ce && enabled)
		return YES;
	return NO;
}

void uiDarwinNotifyEdgeHuggingChanged(uiDarwinControl *c)
{
	uiControl *parent;

	parent = uiControlParent(uiControl(c));
	if (parent != NULL)
		uiDarwinControlChildEdgeHuggingChanged(uiDarwinControl(parent));
}

void uiDarwinNotifyVisibilityChanged(uiDarwinControl *c)
{
	uiControl *parent;

	parent = uiControlParent(uiControl(c));
	if (parent != NULL)
		uiDarwinControlChildVisibilityChanged(uiDarwinControl(parent));
}

void uiDarwinControlDefaultDestroy(uiControl *c)
{
	[((NSView *)uiControlHandle(uiControl(c))) release];
	uiFreeControl(c);
}

uiControl *uiDarwinControlDefaultParent(uiControl *c)
{
	return uiDarwinControl(c)->parent;
}

void uiDarwinControlDefaultSetParent(uiControl *c, uiControl *parent)
{
	uiControlVerifySetParent(c, parent);
	uiDarwinControl(c)->parent = parent;
}

int uiDarwinControlDefaultToplevel(uiControl *c)
{
	return 0;
}

int uiDarwinControlDefaultVisible(uiControl *c)
{
	return uiDarwinControl(c)->visible;
}

void uiDarwinControlDefaultShow(uiControl *c)
{
	uiDarwinControl(c)->visible = YES;
	[((NSView *)uiControlHandle(uiControl(c))) setHidden:NO];
	uiDarwinNotifyVisibilityChanged(uiDarwinControl(c));
}

void uiDarwinControlDefaultHide(uiControl *c)
{
	uiDarwinControl(c)->visible = NO;
	[((NSView *)uiControlHandle(uiControl(c))) setHidden:YES];
	uiDarwinNotifyVisibilityChanged(uiDarwinControl(c));
}

int uiDarwinControlDefaultEnabled(uiControl *c)
{
	return uiDarwinControl(c)->enabled;
}

void uiDarwinControlDefaultEnable(uiControl *c)
{
	uiDarwinControl(c)->enabled = YES;
	uiDarwinControlSyncEnableState(uiDarwinControl(c), uiControlEnabledToUser(c));
}

void uiDarwinControlDefaultDisable(uiControl *c)
{
	uiDarwinControl(c)->enabled = NO;
	uiDarwinControlSyncEnableState(uiDarwinControl(c), uiControlEnabledToUser(c));
}

void uiDarwinControlDefaultSyncEnableState(uiDarwinControl *c, int enabled)
{
	if (uiDarwinShouldStopSyncEnableState(c, enabled))
		return;
	if ([((NSView *)uiControlHandle(uiControl(c))) respondsToSelector:@selector(setEnabled:)])
		[((id)(((NSView *)uiControlHandle(uiControl(c))))) setEnabled:enabled]; /* id cast to make compiler happy; thanks mikeash in irc.freenode.net/#macdev */
}

void uiDarwinControlDefaultSetSuperview(uiDarwinControl *c, NSView *superview)
{
	[((NSView *)uiControlHandle(uiControl(c))) setTranslatesAutoresizingMaskIntoConstraints:NO];
	if (superview == nil)
		[((NSView *)uiControlHandle(uiControl(c))) removeFromSuperview];
	else
		[superview addSubview:((NSView *)uiControlHandle(uiControl(c)))];
}

BOOL uiDarwinControlDefaultHugsTrailingEdge(uiDarwinControl *c)
{
	return YES; /* always hug by default */
}

BOOL uiDarwinControlDefaultHugsBottom(uiDarwinControl *c)
{
	return YES; /* always hug by default */
}

void uiDarwinControlDefaultChildEdgeHuggingChanged(uiDarwinControl *c)
{
	/* do nothing */
}

NSLayoutPriority uiDarwinControlDefaultHuggingPriority(uiDarwinControl *c, NSLayoutConstraintOrientation orientation)
{
	return [((NSView *)uiControlHandle(uiControl(c))) contentHuggingPriorityForOrientation:orientation];
}

void uiDarwinControlDefaultSetHuggingPriority(uiDarwinControl *c, NSLayoutPriority priority, NSLayoutConstraintOrientation orientation)
{
	[((NSView *)uiControlHandle(uiControl(c))) setContentHuggingPriority:priority forOrientation:orientation];
}

void uiDarwinControlDefaultChildVisibilityChanged(uiDarwinControl *c)
{
	/* do nothing */
}
