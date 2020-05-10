// 6 april 2015

// TODO add a uiVerifyControlType() function that can be used by control implementations to verify controls

// TODOs
// - make getters that return whether something exists accept a NULL pointer to discard the value (and thus only return that the thing exists?)
// - const-correct everything
// - normalize documentation between typedefs and structs

#ifndef __LIBUI_UI_H__
#define __LIBUI_UI_H__

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// this macro is generated by cmake
#ifdef libui_EXPORTS
#ifdef _WIN32
#define _UI_EXTERN __declspec(dllexport) extern
#else
#define _UI_EXTERN __attribute__((visibility("default"))) extern
#endif
#else
// TODO add __declspec(dllimport) on windows, but only if not static
#define _UI_EXTERN extern
#endif

// C++ is really really really really really really dumb about enums, so screw that and just make them anonymous
// This has the advantage of being ABI-able should we ever need an ABI...
#define _UI_ENUM(s)         \
	typedef unsigned int s; \
	enum

// This constant is provided because M_PI is nonstandard.
// This comes from Go's math.Pi, which in turn comes from http://oeis.org/A000796.
#define uiPi 3.14159265358979323846264338327950288419716939937510582097494459

// TODO uiBool?

// uiForEach represents the return value from one of libui's various ForEach functions.
_UI_ENUM(uiForEach){
	uiForEachContinue,
	uiForEachStop,
};

typedef struct uiInitOptions uiInitOptions;

struct uiInitOptions {
	size_t Size;
};

_UI_EXTERN const char *uiInit(uiInitOptions *options);
_UI_EXTERN void uiUninit(void);
_UI_EXTERN void uiFreeInitError(const char *err);

_UI_EXTERN void uiMain(void);
_UI_EXTERN void uiMainSteps(void);
_UI_EXTERN int uiMainStep(int wait);
_UI_EXTERN void uiQuit(void);

// after uiQuit or if uiShouldQuit returns nonzero, uiQueueMain's effect is
// undefined
_UI_EXTERN void uiQueueMain(void (*f)(void *data), void *data);

// TODO standardize the looping behavior return type, either with some enum or something, and the test expressions throughout the code
// TODO figure out what to do about looping and the exact point that the timer is rescheduled so we can document it; see https://github.com/andlabs/libui/pull/277
// TODO (also in the above link) document that this cannot be called from any thread, unlike uiQueueMain()
// TODO document that the minimum exact timing, either accuracy (timer burst, etc.) or granularity (15ms on Windows, etc.), is OS-defined
// TODO also figure out how long until the initial tick is registered on all platforms to document
// TODO also add a comment about how useful this could be in bindings, depending on the language being bound to
_UI_EXTERN void uiTimer(int milliseconds, int (*f)(void *data), void *data);

_UI_EXTERN void uiOnShouldQuit(int (*f)(void *data), void *data);

_UI_EXTERN void uiFreeText(char *text);

typedef struct uiControl uiControl;

struct uiControl {
	uint32_t Signature;
	uint32_t OSSignature;
	uint32_t TypeSignature;
	void (*Destroy)(uiControl *);
	uintptr_t (*Handle)(uiControl *);
	uiControl *(*Parent)(uiControl *);
	void (*SetParent)(uiControl *, uiControl *);
	int (*Toplevel)(uiControl *);
	int (*Visible)(uiControl *);
	void (*Show)(uiControl *);
	void (*Hide)(uiControl *);
	int (*Enabled)(uiControl *);
	void (*Enable)(uiControl *);
	void (*Disable)(uiControl *);
};
// TOOD add argument names to all arguments
#define uiControl(this) ((uiControl *)(this))
_UI_EXTERN void uiControlDestroy(uiControl *);
_UI_EXTERN uintptr_t uiControlHandle(uiControl *);
_UI_EXTERN uiControl *uiControlParent(uiControl *);
_UI_EXTERN void uiControlSetParent(uiControl *, uiControl *);
_UI_EXTERN int uiControlToplevel(uiControl *);
_UI_EXTERN int uiControlVisible(uiControl *);
_UI_EXTERN void uiControlShow(uiControl *);
_UI_EXTERN void uiControlHide(uiControl *);
_UI_EXTERN int uiControlEnabled(uiControl *);
_UI_EXTERN void uiControlEnable(uiControl *);
_UI_EXTERN void uiControlDisable(uiControl *);

_UI_EXTERN uiControl *uiAllocControl(size_t n, uint32_t OSsig, uint32_t typesig, const char *typenamestr);
_UI_EXTERN void uiFreeControl(uiControl *);

// TODO make sure all controls have these
_UI_EXTERN void uiControlVerifySetParent(uiControl *, uiControl *);
_UI_EXTERN int uiControlEnabledToUser(uiControl *);

_UI_EXTERN void uiUserBugCannotSetParentOnToplevel(const char *type);

typedef struct uiWindow uiWindow;
#define uiWindow(this) ((uiWindow *)(this))
_UI_EXTERN char *uiWindowTitle(uiWindow *w);
_UI_EXTERN void uiWindowSetTitle(uiWindow *w, const char *title);
_UI_EXTERN void uiWindowContentSize(uiWindow *w, int *width, int *height);
_UI_EXTERN void uiWindowSetContentSize(uiWindow *w, int width, int height);
_UI_EXTERN int uiWindowFullscreen(uiWindow *w);
_UI_EXTERN void uiWindowSetFullscreen(uiWindow *w, int fullscreen);
_UI_EXTERN void uiWindowOnContentSizeChanged(uiWindow *w, void (*f)(uiWindow *, void *), void *data);
_UI_EXTERN void uiWindowOnClosing(uiWindow *w, int (*f)(uiWindow *w, void *data), void *data);
_UI_EXTERN int uiWindowBorderless(uiWindow *w);
_UI_EXTERN void uiWindowSetBorderless(uiWindow *w, int borderless);
_UI_EXTERN void uiWindowSetChild(uiWindow *w, uiControl *child);
_UI_EXTERN int uiWindowMargined(uiWindow *w);
_UI_EXTERN void uiWindowSetMargined(uiWindow *w, int margined);
_UI_EXTERN uiWindow *uiNewWindow(const char *title, int width, int height, int hasMenubar);

typedef struct uiButton uiButton;
#define uiButton(this) ((uiButton *)(this))
_UI_EXTERN char *uiButtonText(uiButton *b);
_UI_EXTERN void uiButtonSetText(uiButton *b, const char *text);
_UI_EXTERN void uiButtonOnClicked(uiButton *b, void (*f)(uiButton *b, void *data), void *data);
_UI_EXTERN uiButton *uiNewButton(const char *text);

typedef struct uiBox uiBox;
#define uiBox(this) ((uiBox *)(this))
_UI_EXTERN void uiBoxAppend(uiBox *b, uiControl *child, int stretchy);
_UI_EXTERN void uiBoxDelete(uiBox *b, int index);
_UI_EXTERN int uiBoxPadded(uiBox *b);
_UI_EXTERN void uiBoxSetPadded(uiBox *b, int padded);
_UI_EXTERN uiBox *uiNewHorizontalBox(void);
_UI_EXTERN uiBox *uiNewVerticalBox(void);

typedef struct uiCheckbox uiCheckbox;
#define uiCheckbox(this) ((uiCheckbox *)(this))
_UI_EXTERN char *uiCheckboxText(uiCheckbox *c);
_UI_EXTERN void uiCheckboxSetText(uiCheckbox *c, const char *text);
_UI_EXTERN void uiCheckboxOnToggled(uiCheckbox *c, void (*f)(uiCheckbox *c, void *data), void *data);
_UI_EXTERN int uiCheckboxChecked(uiCheckbox *c);
_UI_EXTERN void uiCheckboxSetChecked(uiCheckbox *c, int checked);
_UI_EXTERN uiCheckbox *uiNewCheckbox(const char *text);

typedef struct uiEntry uiEntry;
#define uiEntry(this) ((uiEntry *)(this))
_UI_EXTERN char *uiEntryText(uiEntry *e);
_UI_EXTERN void uiEntrySetText(uiEntry *e, const char *text);
_UI_EXTERN void uiEntryOnChanged(uiEntry *e, void (*f)(uiEntry *e, void *data), void *data);
_UI_EXTERN int uiEntryReadOnly(uiEntry *e);
_UI_EXTERN void uiEntrySetReadOnly(uiEntry *e, int readonly);
_UI_EXTERN uiEntry *uiNewEntry(void);
_UI_EXTERN uiEntry *uiNewPasswordEntry(void);
_UI_EXTERN uiEntry *uiNewSearchEntry(void);

typedef struct uiLabel uiLabel;
#define uiLabel(this) ((uiLabel *)(this))
_UI_EXTERN char *uiLabelText(uiLabel *l);
_UI_EXTERN void uiLabelSetText(uiLabel *l, const char *text);
_UI_EXTERN uiLabel *uiNewLabel(const char *text);

typedef struct uiTab uiTab;
#define uiTab(this) ((uiTab *)(this))
_UI_EXTERN void uiTabAppend(uiTab *t, const char *name, uiControl *c);
_UI_EXTERN void uiTabInsertAt(uiTab *t, const char *name, int before, uiControl *c);
_UI_EXTERN void uiTabDelete(uiTab *t, int index);
_UI_EXTERN int uiTabNumPages(uiTab *t);
_UI_EXTERN int uiTabMargined(uiTab *t, int page);
_UI_EXTERN void uiTabSetMargined(uiTab *t, int page, int margined);
_UI_EXTERN uiTab *uiNewTab(void);

typedef struct uiGroup uiGroup;
#define uiGroup(this) ((uiGroup *)(this))
_UI_EXTERN char *uiGroupTitle(uiGroup *g);
_UI_EXTERN void uiGroupSetTitle(uiGroup *g, const char *title);
_UI_EXTERN void uiGroupSetChild(uiGroup *g, uiControl *c);
_UI_EXTERN int uiGroupMargined(uiGroup *g);
_UI_EXTERN void uiGroupSetMargined(uiGroup *g, int margined);
_UI_EXTERN uiGroup *uiNewGroup(const char *title);

// spinbox/slider rules:
// setting value outside of range will automatically clamp
// initial value is minimum
// complaint if min >= max?

typedef struct uiSpinbox uiSpinbox;
#define uiSpinbox(this) ((uiSpinbox *)(this))
_UI_EXTERN int uiSpinboxValue(uiSpinbox *s);
_UI_EXTERN void uiSpinboxSetValue(uiSpinbox *s, int value);
_UI_EXTERN void uiSpinboxOnChanged(uiSpinbox *s, void (*f)(uiSpinbox *s, void *data), void *data);
_UI_EXTERN uiSpinbox *uiNewSpinbox(int min, int max);

typedef struct uiSlider uiSlider;
#define uiSlider(this) ((uiSlider *)(this))
_UI_EXTERN int uiSliderValue(uiSlider *s);
_UI_EXTERN void uiSliderSetValue(uiSlider *s, int value);
_UI_EXTERN void uiSliderOnChanged(uiSlider *s, void (*f)(uiSlider *s, void *data), void *data);
_UI_EXTERN uiSlider *uiNewSlider(int min, int max);

typedef struct uiProgressBar uiProgressBar;
#define uiProgressBar(this) ((uiProgressBar *)(this))
_UI_EXTERN int uiProgressBarValue(uiProgressBar *p);
_UI_EXTERN void uiProgressBarSetValue(uiProgressBar *p, int n);
_UI_EXTERN uiProgressBar *uiNewProgressBar(void);

typedef struct uiSeparator uiSeparator;
#define uiSeparator(this) ((uiSeparator *)(this))
_UI_EXTERN uiSeparator *uiNewHorizontalSeparator(void);
_UI_EXTERN uiSeparator *uiNewVerticalSeparator(void);

typedef struct uiCombobox uiCombobox;
#define uiCombobox(this) ((uiCombobox *)(this))
_UI_EXTERN void uiComboboxAppend(uiCombobox *c, const char *text);
_UI_EXTERN int uiComboboxSelected(uiCombobox *c);
_UI_EXTERN void uiComboboxSetSelected(uiCombobox *c, int n);
_UI_EXTERN void uiComboboxOnSelected(uiCombobox *c, void (*f)(uiCombobox *c, void *data), void *data);
_UI_EXTERN uiCombobox *uiNewCombobox(void);

typedef struct uiEditableCombobox uiEditableCombobox;
#define uiEditableCombobox(this) ((uiEditableCombobox *)(this))
_UI_EXTERN void uiEditableComboboxAppend(uiEditableCombobox *c, const char *text);
_UI_EXTERN char *uiEditableComboboxText(uiEditableCombobox *c);
_UI_EXTERN void uiEditableComboboxSetText(uiEditableCombobox *c, const char *text);
// TODO what do we call a function that sets the currently selected item and fills the text field with it? editable comboboxes have no consistent concept of selected item
_UI_EXTERN void uiEditableComboboxOnChanged(uiEditableCombobox *c, void (*f)(uiEditableCombobox *c, void *data), void *data);
_UI_EXTERN uiEditableCombobox *uiNewEditableCombobox(void);

typedef struct uiRadioButtons uiRadioButtons;
#define uiRadioButtons(this) ((uiRadioButtons *)(this))
_UI_EXTERN void uiRadioButtonsAppend(uiRadioButtons *r, const char *text);
_UI_EXTERN int uiRadioButtonsSelected(uiRadioButtons *r);
_UI_EXTERN void uiRadioButtonsSetSelected(uiRadioButtons *r, int n);
_UI_EXTERN void uiRadioButtonsOnSelected(uiRadioButtons *r, void (*f)(uiRadioButtons *, void *), void *data);
_UI_EXTERN uiRadioButtons *uiNewRadioButtons(void);

struct tm;
typedef struct uiDateTimePicker uiDateTimePicker;
#define uiDateTimePicker(this) ((uiDateTimePicker *)(this))
// TODO document that tm_wday and tm_yday are undefined, and tm_isdst should be -1
// TODO document that for both sides
// TODO document time zone conversions or lack thereof
// TODO for Time: define what values are returned when a part is missing
_UI_EXTERN void uiDateTimePickerTime(uiDateTimePicker *d, struct tm *time);
_UI_EXTERN void uiDateTimePickerSetTime(uiDateTimePicker *d, const struct tm *time);
_UI_EXTERN void uiDateTimePickerOnChanged(uiDateTimePicker *d, void (*f)(uiDateTimePicker *, void *), void *data);
_UI_EXTERN uiDateTimePicker *uiNewDateTimePicker(void);
_UI_EXTERN uiDateTimePicker *uiNewDatePicker(void);
_UI_EXTERN uiDateTimePicker *uiNewTimePicker(void);

// TODO provide a facility for entering tab stops?
typedef struct uiMultilineEntry uiMultilineEntry;
#define uiMultilineEntry(this) ((uiMultilineEntry *)(this))
_UI_EXTERN char *uiMultilineEntryText(uiMultilineEntry *e);
_UI_EXTERN void uiMultilineEntrySetText(uiMultilineEntry *e, const char *text);
_UI_EXTERN void uiMultilineEntryAppend(uiMultilineEntry *e, const char *text);
_UI_EXTERN void uiMultilineEntryOnChanged(uiMultilineEntry *e, void (*f)(uiMultilineEntry *e, void *data), void *data);
_UI_EXTERN int uiMultilineEntryReadOnly(uiMultilineEntry *e);
_UI_EXTERN void uiMultilineEntrySetReadOnly(uiMultilineEntry *e, int readonly);
_UI_EXTERN uiMultilineEntry *uiNewMultilineEntry(void);
_UI_EXTERN uiMultilineEntry *uiNewNonWrappingMultilineEntry(void);

typedef struct uiMenuItem uiMenuItem;
#define uiMenuItem(this) ((uiMenuItem *)(this))
_UI_EXTERN void uiMenuItemEnable(uiMenuItem *m);
_UI_EXTERN void uiMenuItemDisable(uiMenuItem *m);
_UI_EXTERN void uiMenuItemOnClicked(uiMenuItem *m, void (*f)(uiMenuItem *sender, uiWindow *window, void *data), void *data);
_UI_EXTERN int uiMenuItemChecked(uiMenuItem *m);
_UI_EXTERN void uiMenuItemSetChecked(uiMenuItem *m, int checked);

typedef struct uiMenu uiMenu;
#define uiMenu(this) ((uiMenu *)(this))
_UI_EXTERN uiMenuItem *uiMenuAppendItem(uiMenu *m, const char *name);
_UI_EXTERN uiMenuItem *uiMenuAppendCheckItem(uiMenu *m, const char *name);
_UI_EXTERN uiMenuItem *uiMenuAppendQuitItem(uiMenu *m);
_UI_EXTERN uiMenuItem *uiMenuAppendPreferencesItem(uiMenu *m);
_UI_EXTERN uiMenuItem *uiMenuAppendAboutItem(uiMenu *m);
_UI_EXTERN void uiMenuAppendSeparator(uiMenu *m);
_UI_EXTERN uiMenu *uiNewMenu(const char *name);

_UI_EXTERN char *uiOpenFile(uiWindow *parent);
_UI_EXTERN char *uiSaveFile(uiWindow *parent);
_UI_EXTERN void uiMsgBox(uiWindow *parent, const char *title, const char *description);
_UI_EXTERN void uiMsgBoxError(uiWindow *parent, const char *title, const char *description);

typedef struct uiColorButton uiColorButton;
#define uiColorButton(this) ((uiColorButton *)(this))
_UI_EXTERN void uiColorButtonColor(uiColorButton *b, double *r, double *g, double *bl, double *a);
_UI_EXTERN void uiColorButtonSetColor(uiColorButton *b, double r, double g, double bl, double a);
_UI_EXTERN void uiColorButtonOnChanged(uiColorButton *b, void (*f)(uiColorButton *, void *), void *data);
_UI_EXTERN uiColorButton *uiNewColorButton(void);

_UI_ENUM(uiAlign){
	uiAlignFill,
	uiAlignStart,
	uiAlignCenter,
	uiAlignEnd,
};

_UI_ENUM(uiAt){
	uiAtLeading,
	uiAtTop,
	uiAtTrailing,
	uiAtBottom,
};

typedef struct uiGrid uiGrid;
#define uiGrid(this) ((uiGrid *)(this))
_UI_EXTERN void uiGridAppend(uiGrid *g, uiControl *c, int left, int top, int xspan, int yspan, int hexpand, uiAlign halign, int vexpand, uiAlign valign);
_UI_EXTERN void uiGridInsertAt(uiGrid *g, uiControl *c, uiControl *existing, uiAt at, int xspan, int yspan, int hexpand, uiAlign halign, int vexpand, uiAlign valign);
_UI_EXTERN int uiGridPadded(uiGrid *g);
_UI_EXTERN void uiGridSetPadded(uiGrid *g, int padded);
_UI_EXTERN uiGrid *uiNewGrid(void);

// uiImage stores an image for display on screen.
//
// Images are built from one or more representations, each with the
// same aspect ratio but a different pixel size. libui automatically
// selects the most appropriate representation for drawing the image
// when it comes time to draw the image; what this means depends
// on the pixel density of the target context. Therefore, one can use
// uiImage to draw higher-detailed images on higher-density
// displays. The typical use cases are either:
//
// 	- have just a single representation, at which point all screens
// 	  use the same image, and thus uiImage acts like a simple
// 	  bitmap image, or
// 	- have two images, one at normal resolution and one at 2x
// 	  resolution; this matches the current expectations of some
// 	  desktop systems at the time of writing (mid-2018)
//
// uiImage is very simple: it only supports premultiplied 32-bit
// RGBA images, and libui does not provide any image file loading
// or image format conversion utilities on top of that.
typedef struct uiImage uiImage;

// @role uiImage constructor
// uiNewImage creates a new uiImage with the given width and
// height. This width and height should be the size in points of the
// image in the device-independent case; typically this is the 1x size.
// TODO for all uiImage functions: use const void * for const correctness
_UI_EXTERN uiImage *uiNewImage(double width, double height);

// @role uiImage destructor
// uiFreeImage frees the given image and all associated resources.
_UI_EXTERN void uiFreeImage(uiImage *i);

// uiImageAppend adds a representation to the uiImage.
// pixels should point to a byte array of premultiplied pixels
// stored in [R G B A] order (so ((uint8_t *) pixels)[0] is the R of the
// first pixel and [3] is the A of the first pixel). pixelWidth and
// pixelHeight is the size *in pixels* of the image, and pixelStride is
// the number *of bytes* per row of the pixels array. Therefore,
// pixels itself must be at least byteStride * pixelHeight bytes long.
// TODO see if we either need the stride or can provide a way to get the OS-preferred stride (in cairo we do)
_UI_EXTERN void uiImageAppend(uiImage *i, void *pixels, int pixelWidth, int pixelHeight, int byteStride);

// uiTableValue stores a value to be passed along uiTable and
// uiTableModel.
//
// You do not create uiTableValues directly; instead, you create a
// uiTableValue of a given type using the specialized constructor
// functions.
//
// uiTableValues are immutable and the uiTableModel and uiTable
// take ownership of the uiTableValue object once returned, copying
// its contents as necessary.
typedef struct uiTableValue uiTableValue;

// @role uiTableValue destructor
// uiFreeTableValue() frees a uiTableValue. You generally do not
// need to call this yourself, as uiTable and uiTableModel do this
// for you. In fact, it is an error to call this function on a uiTableValue
// that has been given to a uiTable or uiTableModel. You can call this,
// however, if you created a uiTableValue that you aren't going to
// use later, or if you called a uiTableModelHandler method directly
// and thus never transferred ownership of the uiTableValue.
_UI_EXTERN void uiFreeTableValue(uiTableValue *v);

// uiTableValueType holds the possible uiTableValue types that may
// be returned by uiTableValueGetType(). Refer to the documentation
// for each type's constructor function for details on each type.
// TODO actually validate these
_UI_ENUM(uiTableValueType){
	uiTableValueTypeString,
	uiTableValueTypeImage,
	uiTableValueTypeInt,
	uiTableValueTypeColor,
};

// uiTableValueGetType() returns the type of v.
// TODO I don't like this name
_UI_EXTERN uiTableValueType uiTableValueGetType(const uiTableValue *v);

// uiNewTableValueString() returns a new uiTableValue that contains
// str. str is copied; you do not need to keep it alive after
// uiNewTableValueString() returns.
_UI_EXTERN uiTableValue *uiNewTableValueString(const char *str);

// uiTableValueString() returns the string stored in v. The returned
// string is owned by v. It is an error to call this on a uiTableValue
// that does not hold a string.
_UI_EXTERN const char *uiTableValueString(const uiTableValue *v);

// uiNewTableValueImage() returns a new uiTableValue that contains
// the given uiImage.
//
// Unlike other similar constructors, uiNewTableValueImage() does
// NOT copy the image. This is because images are comparatively
// larger than the other objects in question. Therefore, you MUST
// keep the image alive as long as the returned uiTableValue is alive.
// As a general rule, if libui calls a uiTableModelHandler method, the
// uiImage is safe to free once any of your code is once again
// executed.
_UI_EXTERN uiTableValue *uiNewTableValueImage(uiImage *img);

// uiTableValueImage() returns the uiImage stored in v. As these
// images are not owned by v, you should not assume anything
// about the lifetime of the image (unless you created the image,
// and thus control its lifetime). It is an error to call this on a
// uiTableValue that does not hold an image.
_UI_EXTERN uiImage *uiTableValueImage(const uiTableValue *v);

// uiNewTableValueInt() returns a uiTableValue that stores the given
// int. This can be used both for boolean values (nonzero is true, as
// in C) or progresses (in which case the valid range is -1..100
// inclusive).
_UI_EXTERN uiTableValue *uiNewTableValueInt(int i);

// uiTableValueInt() returns the int stored in v. It is an error to call
// this on a uiTableValue that does not store an int.
_UI_EXTERN int uiTableValueInt(const uiTableValue *v);

// uiNewTableValueColor() returns a uiTableValue that stores the
// given color.
_UI_EXTERN uiTableValue *uiNewTableValueColor(double r, double g, double b, double a);

// uiTableValueColor() returns the color stored in v. It is an error to
// call this on a uiTableValue that does not store a color.
// TODO define whether all this, for both uiTableValue and uiAttribute, is undefined behavior or a caught error
_UI_EXTERN void uiTableValueColor(const uiTableValue *v, double *r, double *g, double *b, double *a);

// uiTableModel is an object that provides the data for a uiTable.
// This data is returned via methods you provide in the
// uiTableModelHandler struct.
//
// uiTableModel represents data using a table, but this table does
// not map directly to uiTable itself. Instead, you can have data
// columns which provide instructions for how to render a given
// uiTable's column — for instance, one model column can be used
// to give certain rows of a uiTable a different background color.
// Row numbers DO match with uiTable row numbers.
//
// Once created, the number and data types of columns of a
// uiTableModel cannot change.
//
// Row and column numbers start at 0. A uiTableModel can be
// associated with more than one uiTable at a time.
typedef struct uiTableModel uiTableModel;

// uiTableModelHandler defines the methods that uiTableModel
// calls when it needs data. Once a uiTableModel is created, these
// methods cannot change.
typedef struct uiTableModelHandler uiTableModelHandler;

// TODO validate ranges; validate types on each getter/setter call (? table columns only?)
struct uiTableModelHandler {
	// NumColumns returns the number of model columns in the
	// uiTableModel. This value must remain constant through the
	// lifetime of the uiTableModel. This method is not guaranteed
	// to be called depending on the system.
	// TODO strongly check column numbers and types on all platforms so these clauses can go away
	int (*NumColumns)(uiTableModel *model, void *tableData);
	// ColumnType returns the value type of the data stored in
	// the given model column of the uiTableModel. The returned
	// values must remain constant through the lifetime of the
	// uiTableModel. This method is not guaranteed to be called
	// depending on the system.
	uiTableValueType (*ColumnType)(uiTableModel *model, void *tableData, int col);
	// NumRows returns the number or rows in the uiTableModel.
	// This value must be non-negative.
	int (*NumRows)(uiTableModel *model, void *tableData);
	// CellValue returns a uiTableValue corresponding to the model
	// cell at (row, column). The type of the returned uiTableValue
	// must match column's value type. Under some circumstances,
	// NULL may be returned; refer to the various methods that add
	// columns to uiTable for details. Once returned, the uiTable
	// that calls CellValue will free the uiTableValue returned.
	uiTableValue *(*CellValue)(uiTableModel *model, void *tableData, int row, int col);
	// SetCellValue changes the model cell value at (row, column)
	// in the uiTableModel. Within this function, either do nothing
	// to keep the current cell value or save the new cell value as
	// appropriate. After SetCellValue is called, the uiTable will
	// itself reload the table cell. Under certain conditions, the
	// uiTableValue passed in can be NULL; refer to the various
	// methods that add columns to uiTable for details. Once
	// returned, the uiTable that called SetCellValue will free the
	// uiTableValue passed in.
	void (*SetCellValue)(uiTableModel *model, void *tableData, int row, int col, const uiTableValue *val);
};

// @role uiTableModel constructor
// uiNewTableModel() creates a new uiTableModel with the given
// handler methods.
// The tableData user pointer is passed directly to the handler methods.
_UI_EXTERN uiTableModel *uiNewTableModel(uiTableModelHandler *mh, void *tableData);

// @role uiTableModel destructor
// uiFreeTableModel() frees the given table model. It is an error to
// free table models currently associated with a uiTable.
_UI_EXTERN void uiFreeTableModel(uiTableModel *m);

// uiTableModelRowInserted() tells any uiTable associated with m
// that a new row has been added to m at index index. You call
// this function when the number of rows in your model has
// changed; after calling it, NumRows() should returm the new row
// count.
_UI_EXTERN void uiTableModelRowInserted(uiTableModel *m, int newIndex);

// uiTableModelRowChanged() tells any uiTable associated with m
// that the data in the row at index has changed. You do not need to
// call this in your SetCellValue() handlers, but you do need to call
// this if your data changes at some other point.
_UI_EXTERN void uiTableModelRowChanged(uiTableModel *m, int index);

// uiTableModelRowDeleted() tells any uiTable associated with m
// that the row at index index has been deleted. You call this
// function when the number of rows in your model has changed;
// after calling it, NumRows() should returm the new row
// count.
// TODO for this and Inserted: make sure the "after" part is right; clarify if it's after returning or after calling
_UI_EXTERN void uiTableModelRowDeleted(uiTableModel *m, int oldIndex);
// TODO reordering/moving

// uiTableModelColumnNeverEditable and
// uiTableModelColumnAlwaysEditable are the value of an editable
// model column parameter to one of the uiTable create column
// functions; if used, that jparticular uiTable colum is not editable
// by the user and always editable by the user, respectively.
#define uiTableModelColumnNeverEditable (-1)
#define uiTableModelColumnAlwaysEditable (-2)

// uiTableTextColumnOptionalParams are the optional parameters
// that control the appearance of the text column of a uiTable.
typedef struct uiTableTextColumnOptionalParams uiTableTextColumnOptionalParams;

// uiTableParams defines the parameters passed to uiNewTable().
typedef struct uiTableParams uiTableParams;

struct uiTableTextColumnOptionalParams {
	// ColorModelColumn is the model column containing the
	// text color of this uiTable column's text, or -1 to use the
	// default color.
	//
	// If CellValue() for this column for any cell returns NULL, that
	// cell will also use the default text color.
	int ColorModelColumn;
};

struct uiTableParams {
	// Model is the uiTableModel to use for this uiTable.
	// This parameter cannot be NULL.
	uiTableModel *Model;
	// RowBackgroundColorModelColumn is a model column
	// number that defines the background color used for the
	// entire row in the uiTable, or -1 to use the default color for
	// all rows.
	//
	// If CellValue() for this column for any row returns NULL, that
	// row will also use the default background color.
	int RowBackgroundColorModelColumn;
};

// uiTable is a uiControl that shows tabular data, allowing users to
// manipulate rows of such data at a time.
typedef struct uiTable uiTable;
#define uiTable(this) ((uiTable *)(this))

// uiTableAppendTextColumn() appends a text column to t.
// name is displayed in the table header.
// textModelColumn is where the text comes from.
// If a row is editable according to textEditableModelColumn,
// SetCellValue() is called with textModelColumn as the column.
_UI_EXTERN void uiTableAppendTextColumn(uiTable *t,
	const char *name,
	int textModelColumn,
	int textEditableModelColumn,
	uiTableTextColumnOptionalParams *textParams);

// uiTableAppendImageColumn() appends an image column to t.
// Images are drawn at icon size, appropriate to the pixel density
// of the screen showing the uiTable.
_UI_EXTERN void uiTableAppendImageColumn(uiTable *t,
	const char *name,
	int imageModelColumn);

// uiTableAppendImageTextColumn() appends a column to t that
// shows both an image and text.
_UI_EXTERN void uiTableAppendImageTextColumn(uiTable *t,
	const char *name,
	int imageModelColumn,
	int textModelColumn,
	int textEditableModelColumn,
	uiTableTextColumnOptionalParams *textParams);

// uiTableAppendCheckboxColumn appends a column to t that
// contains a checkbox that the user can interact with (assuming the
// checkbox is editable). SetCellValue() will be called with
// checkboxModelColumn as the column in this case.
_UI_EXTERN void uiTableAppendCheckboxColumn(uiTable *t,
	const char *name,
	int checkboxModelColumn,
	int checkboxEditableModelColumn);

// uiTableAppendCheckboxTextColumn() appends a column to t
// that contains both a checkbox and text.
_UI_EXTERN void uiTableAppendCheckboxTextColumn(uiTable *t,
	const char *name,
	int checkboxModelColumn,
	int checkboxEditableModelColumn,
	int textModelColumn,
	int textEditableModelColumn,
	uiTableTextColumnOptionalParams *textParams);

// uiTableAppendButtonColumn() appends a column to t
// that shows a button that the user can click on. When the user
// does click on the button, SetCellValue() is called with a NULL
// value and buttonModelColumn as the column.
// CellValue() on buttonModelColumn should return the text to show
// in the button.
_UI_EXTERN void uiTableAppendButtonColumn(uiTable *t,
	const char *name,
	int buttonModelColumn,
	int buttonClickableModelColumn);

// uiNewTable() creates a new uiTable with the specified parameters.
_UI_EXTERN uiTable *uiNewTable(uiTableParams *params);

#ifdef __cplusplus
}
#endif

#endif
