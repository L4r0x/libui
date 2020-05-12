// 8 december 2015
#import "uipriv_darwin.h"

// NSTextView has no intrinsic content size by default, which wreaks havoc on a pure-Auto Layout system
// we'll have to take over to get it to work
// see also http://stackoverflow.com/questions/24210153/nstextview-not-properly-resizing-with-auto-layout and http://stackoverflow.com/questions/11237622/using-autolayout-with-expanding-nstextviews
@interface intrinsicSizeTextView : NSTextView {
	uiMultilineEntry *libui_e;
}
- (id)initWithFrame:(NSRect)r e:(uiMultilineEntry *)e;
@end

struct uiMultilineEntry {
	uiDarwinControl c;
	NSScrollView *sv;
	intrinsicSizeTextView *tv;
	uiprivScrollViewData *d;
	void (*onChanged)(uiMultilineEntry *, void *);
	void *onChangedData;
	BOOL changing;
};

@implementation intrinsicSizeTextView

- (id)initWithFrame:(NSRect)r e:(uiMultilineEntry *)e
{
	self = [super initWithFrame:r];
	if (self)
		self->libui_e = e;
	return self;
}

- (NSSize)intrinsicContentSize
{
	NSTextContainer *textContainer = [self textContainer];
	NSLayoutManager *layoutManager = [self layoutManager];
	[layoutManager ensureLayoutForTextContainer:textContainer];
	NSRect rect = [layoutManager usedRectForTextContainer:textContainer];
	return rect.size;
}

- (void)didChangeText
{
	[super didChangeText];
	[self invalidateIntrinsicContentSize];
	if (!self->libui_e->changing)
		(*(self->libui_e->onChanged))(self->libui_e, self->libui_e->onChangedData);
}

@end

uiDarwinControlAllDefaultsExceptDestroy(uiMultilineEntry, sv)

static void uiMultilineEntryDestroy(uiControl *c)
{
	uiMultilineEntry *e = uiMultilineEntry(c);

	uiprivScrollViewFreeData(e->sv, e->d);
	[e->tv release];
	[e->sv release];
	uiFreeControl(uiControl(e));
}

static void defaultOnChanged(uiMultilineEntry *e, void *data)
{
	// do nothing
}

char *uiMultilineEntryText(uiMultilineEntry *e)
{
	return uiDarwinNSStringToText([e->tv string]);
}

void uiMultilineEntrySetText(uiMultilineEntry *e, const char *text)
{
	[[e->tv textStorage] replaceCharactersInRange:NSMakeRange(0, [[e->tv string] length])
									   withString:uiprivToNSString(text)];
	// must be called explicitly according to the documentation of shouldChangeTextInRange:replacementString:
	e->changing = YES;
	[e->tv didChangeText];
	e->changing = NO;
}

// TODO scroll to end?
void uiMultilineEntryAppend(uiMultilineEntry *e, const char *text)
{
	[[e->tv textStorage] replaceCharactersInRange:NSMakeRange([[e->tv string] length], 0)
									   withString:uiprivToNSString(text)];
	e->changing = YES;
	[e->tv didChangeText];
	e->changing = NO;
}

void uiMultilineEntryOnChanged(uiMultilineEntry *e, void (*f)(uiMultilineEntry *e, void *data), void *data)
{
	e->onChanged = f;
	e->onChangedData = data;
}

int uiMultilineEntryReadOnly(uiMultilineEntry *e)
{
	return [e->tv isEditable] == NO;
}

void uiMultilineEntrySetReadOnly(uiMultilineEntry *e, int readonly)
{
	BOOL editable = !readonly;
	[e->tv setEditable:editable];
}

static uiMultilineEntry *uiprivNewMultilineEntry(BOOL hscroll)
{
	uiMultilineEntry *entry;
	uiDarwinNewControl(uiMultilineEntry, entry);

	entry->tv = [[intrinsicSizeTextView alloc] initWithFrame:NSZeroRect e:entry];

	// NSText properties:
	// this is what Interface Builder sets the background color to
	[entry->tv setEditable:YES];
	[entry->tv setSelectable:YES];
	[entry->tv setFieldEditor:NO];
	[entry->tv setRichText:NO];
	[entry->tv setImportsGraphics:NO];
	[entry->tv setUsesFontPanel:NO];
	[entry->tv setRulerVisible:NO];
	[entry->tv setHorizontallyResizable:NO];
	[entry->tv setVerticallyResizable:YES];

	// NSTextView properties:
	[entry->tv setAllowsUndo:YES];
	[entry->tv setAllowsImageEditing:NO];
	[entry->tv setDisplaysLinkToolTips:YES];
	[entry->tv setUsesRuler:NO];
	[entry->tv setUsesInspectorBar:NO];
	[entry->tv setSelectionGranularity:NSSelectByCharacter];
	[entry->tv setContinuousSpellCheckingEnabled:NO];
	[entry->tv setGrammarCheckingEnabled:NO];
	[entry->tv setUsesFindPanel:YES];
	[entry->tv setAutomaticDataDetectionEnabled:NO];
	[entry->tv setUsesFindBar:NO];
	[entry->tv setIncrementalSearchingEnabled:NO];

	// disable auto correction
	// LONGTERM do we really want to do this? make it an option?
	[entry->tv setEnabledTextCheckingTypes:0];
	[entry->tv setAutomaticDashSubstitutionEnabled:NO];
	[entry->tv setAutomaticSpellingCorrectionEnabled:NO];
	[entry->tv setAutomaticTextReplacementEnabled:NO];
	[entry->tv setAutomaticQuoteSubstitutionEnabled:NO];
	[entry->tv setSmartInsertDeleteEnabled:NO];

	// NSTextContainer properties:
	[[entry->tv textContainer] setWidthTracksTextView:YES];
	[[entry->tv textContainer] setHeightTracksTextView:NO];

	// NSLayoutManager properties:
	[[entry->tv layoutManager] setAllowsNonContiguousLayout:YES];

	// see https://developer.apple.com/library/mac/documentation/Cocoa/Conceptual/TextUILayer/Tasks/TextInScrollView.html
	// notice we don't use the Auto Layout code; see scrollview.m for more details
	[entry->tv setMaxSize:NSMakeSize(CGFLOAT_MAX, CGFLOAT_MAX)];
	[entry->tv setVerticallyResizable:YES];
	[entry->tv setHorizontallyResizable:hscroll];
	if (hscroll) {
		[entry->tv setAutoresizingMask:(NSViewWidthSizable | NSViewHeightSizable)];
		[[entry->tv textContainer] setWidthTracksTextView:NO];
	} else {
		[entry->tv setAutoresizingMask:NSViewWidthSizable];
		[[entry->tv textContainer] setWidthTracksTextView:YES];
	}
	[[entry->tv textContainer] setContainerSize:NSMakeSize(CGFLOAT_MAX, CGFLOAT_MAX)];

	uiprivScrollViewCreateParams params = {
		.DocumentView = entry->tv,
		.BackgroundColor = nil,
		.DrawsBackground = YES,
		.Bordered = YES,
		.HScroll = hscroll,
		.VScroll = YES,
	};
	entry->sv = uiprivMkScrollView(&params, &(entry->d));

	uiMultilineEntryOnChanged(entry, defaultOnChanged, NULL);

	return entry;
}

uiMultilineEntry *uiNewMultilineEntry(void)
{
	return uiprivNewMultilineEntry(NO);
}

uiMultilineEntry *uiNewNonWrappingMultilineEntry(void)
{
	return uiprivNewMultilineEntry(YES);
}
