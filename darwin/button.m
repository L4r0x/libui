// 13 august 2015
#import "uipriv_darwin.h"

struct uiButton {
	uiDarwinControl c;
	NSButton *button;
	void (*onClicked)(uiButton *, void *);
	void *onClickedData;
};

@interface buttonDelegateClass : NSObject {
	uiprivMap *buttons;
}
- (IBAction)onClicked:(id)sender;
- (void)registerButton:(uiButton *)b;
- (void)unregisterButton:(uiButton *)b;
@end

@implementation buttonDelegateClass

- (id)init
{
	self = [super init];
	if (self)
		self->buttons = uiprivNewMap();
	return self;
}

- (void)dealloc
{
	uiprivMapDestroy(self->buttons);
	[super dealloc];
}

- (IBAction)onClicked:(id)sender
{
	uiButton *b;

	b = (uiButton *) uiprivMapGet(self->buttons, sender);
	(*(b->onClicked))(b, b->onClickedData);
}

- (void)registerButton:(uiButton *)b
{
	uiprivMapSet(self->buttons, b->button, b);
	[b->button setTarget:self];
	[b->button setAction:@selector(onClicked:)];
}

- (void)unregisterButton:(uiButton *)b
{
	[b->button setTarget:nil];
	uiprivMapDelete(self->buttons, b->button);
}

@end

static buttonDelegateClass *buttonDelegate = nil;

static void uiButtonDestroy(uiControl *c)
{
	uiButton *b = uiButton(c);

	[buttonDelegate unregisterButton:b];
	[b->button release];
	uiFreeControl(uiControl(b));
}

char *uiButtonText(uiButton *b)
{
	return uiDarwinNSStringToText([b->button title]);
}

void uiButtonSetText(uiButton *b, const char *text)
{
	[b->button setTitle:uiprivToNSString(text)];
}

void uiButtonOnClicked(uiButton *b, void (*f)(uiButton *, void *), void *data)
{
	b->onClicked = f;
	b->onClickedData = data;
}

static void defaultOnClicked(uiButton *b, void *data)
{
	// do nothing
}

#define uiButtonSyncEnableState uiDarwinControlDefaultSyncEnableState
#define uiButtonSetSuperview uiDarwinControlDefaultSetSuperview
#define uiButtonHugsTrailingEdge uiDarwinControlDefaultHugsTrailingEdge
#define uiButtonHugsBottom uiDarwinControlDefaultHugsBottom
#define uiButtonChildEdgeHuggingChanged uiDarwinControlDefaultChildEdgeHuggingChanged
#define uiButtonHuggingPriority uiDarwinControlDefaultHuggingPriority
#define uiButtonSetHuggingPriority uiDarwinControlDefaultSetHuggingPriority
#define uiButtonChildVisibilityChanged uiDarwinControlDefaultChildVisibilityChanged
uiDarwinControlDefaultHandle(uiButton, button)
uiDarwinControlFunctionsDefaultExceptDestroy(uiButton)

uiButton *uiNewButton(const char *text)
{
	uiButton *b = uiDarwinNewControl(uiButton);

	b->button = [[NSButton alloc] initWithFrame:NSZeroRect];
	[b->button setTitle:uiprivToNSString(text)];
	[b->button setButtonType:NSMomentaryPushInButton];
	[b->button setBordered:YES];
	[b->button setBezelStyle:NSRoundedBezelStyle];
	uiDarwinSetControlFont(b->button, NSControlSizeRegular);

	if (buttonDelegate == nil) {
		buttonDelegate = [[buttonDelegateClass new] autorelease];
		[uiprivDelegates addObject:buttonDelegate];
	}
	[buttonDelegate registerButton:b];
	uiButtonOnClicked(b, defaultOnClicked, NULL);

	return b;
}
