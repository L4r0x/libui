#include "table.hpp"
#include "uipriv_windows.hpp"

uiTableModel *uiNewTableModel(uiTableModelHandler *mh, void *tableData)
{
	uiTableModel *model = uiprivNew(uiTableModel);
	model->mh = mh;
	model->data = tableData;
	model->tables = new std::vector<uiTable *>;
	return model;
}

void uiFreeTableModel(uiTableModel *model)
{
	delete model->tables;
	uiprivFree(model);
}

void uiTableModelRowInserted(uiTableModel *model, int newIndex)
{
	LVITEMW item;
	ZeroMemory(&item, sizeof(LVITEMW));
	item.mask = 0;
	item.iItem = newIndex;
	item.iSubItem = 0;
	for (auto table : *(model->tables)) {
		if (ListView_InsertItem(table->hwnd, &item) == -1)
			logLastError(L"error calling LVM_INSERTITEM in uiTableModelRowInserted()");
	}
}

// TODO compare LVM_UPDATE and LVM_REDRAWITEMS
void uiTableModelRowChanged(uiTableModel *model, int index)
{
	for (auto table : *(model->tables)) {
		if (!ListView_Update(table->hwnd, index))
			logLastError(L"error calling LVM_UPDATE in uiTableModelRowChanged()");
	}
}

void uiTableModelRowDeleted(uiTableModel *model, int oldIndex)
{
	for (auto table : *(model->tables)) {
		if (!ListView_DeleteItem(table->hwnd, oldIndex))
			logLastError(L"error calling LVM_DELETEITEM in uiTableModelRowDeleted()");
	}
}

uiTableModelHandler *uiprivTableModelHandler(uiTableModel *model)
{
	return model->mh;
}

void *uiprivTableModelData(uiTableModel *model)
{
	return model->data;
}

// TODO explain all this
static LRESULT CALLBACK tableSubProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIDSubclass, DWORD_PTR dwRefData)
{
	uiTable *table = (uiTable *)dwRefData;
	NMHDR *nmhdr = (NMHDR *)lParam;

	bool finishEdit = false;
	bool abortEdit = false;
	switch (uMsg) {
	case WM_TIMER: {
		if (wParam == (WPARAM)(&(table->inDoubleClickTimer))) {
			table->inDoubleClickTimer = FALSE;
			// TODO check errors
			KillTimer(hwnd, wParam);
			return 0;
		}
		if (wParam != (WPARAM)table)
			break;
		// TODO only increment and update if visible?
		for (auto &i : *(table->indeterminatePositions)) {
			i.second++;
			// TODO check errors
			ListView_Update(hwnd, i.first.first);
		}
		return 0;
	}
	case WM_LBUTTONDOWN: {
		table->inLButtonDown = TRUE;
		LRESULT lResult = DefSubclassProc(hwnd, uMsg, wParam, lParam);
		table->inLButtonDown = FALSE;
		return lResult;
	}
	case WM_COMMAND: {
		if (HIWORD(wParam) == EN_UPDATE) {
			// the real list view resizes the edit control on this notification specifically
			HRESULT hr = uiprivTableResizeWhileEditing(table);
			if (hr != S_OK) {
				// TODO
			}
			break;
		}
		// the real list view accepts changes in this case
		if (HIWORD(wParam) == EN_KILLFOCUS)
			finishEdit = true;
		break;
	}
	case WM_NOTIFY: {
		// list view accepts changes on column resize, but does not provide such notifications :/
		HWND header = ListView_GetHeader(table->hwnd);
		if (nmhdr->hwndFrom == header) {
			NMHEADERW *nm = (NMHEADERW *)nmhdr;

			switch (nmhdr->code) {
			case HDN_ITEMCHANGED:
				if ((nm->pitem->mask & HDI_WIDTH) == 0)
					break;
				// fall through
			case HDN_DIVIDERDBLCLICK:
			case HDN_TRACK:
			case HDN_ENDTRACK:
			case NM_KILLFOCUS:
				finishEdit = true;
			case NM_DBLCLK:
				printf("Item column double click\n");
				break;
			}
		}
		break;
	}
	case LVM_CANCELEDITLABEL: {
		finishEdit = true;
		// TODO properly imitate notifiactions
		break;
	}
	// TODO finish edit on WM_WINDOWPOSCHANGING and WM_SIZE?
	// for the next three: this item is about to go away; don't bother keeping changes
	case LVM_SETITEMCOUNT: {
		if (wParam <= table->editedItem)
			abortEdit = true;
		break;
	}
	case LVM_DELETEITEM: {
		if (wParam == table->editedItem)
			abortEdit = true;
		break;
	}
	case LVM_DELETEALLITEMS: {
		abortEdit = true;
		break;
	}
	case WM_NCDESTROY: {
		if (RemoveWindowSubclass(hwnd, tableSubProc, uIDSubclass) == FALSE)
			logLastError(L"RemoveWindowSubclass()");
		// fall through
	}
	}
	if (finishEdit) {
		HRESULT hr = uiprivTableFinishEditingText(table);
		if (hr != S_OK) {
			// TODO
		}
	} else if (abortEdit) {
		HRESULT hr = uiprivTableAbortEditingText(table);
		if (hr != S_OK) {
			// TODO
		}
	}
	return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}

// further reading:
// - https://msdn.microsoft.com/en-us/library/ye4z8x58.aspx
static HRESULT handleLVIF_TEXT(uiTable *table, NMLVDISPINFOW *nm, uiprivTableColumnParams *params)
{
	if ((nm->item.mask & LVIF_TEXT) == 0)
		return S_OK;

	int strcol = -1;
	if (params->textModelColumn != -1)
		strcol = params->textModelColumn;
	else if (params->buttonModelColumn != -1)
		strcol = params->buttonModelColumn;
	if (strcol != -1) {
		uiTableValue *value = uiprivTableModelCellValue(table->model, nm->item.iItem, strcol);
		WCHAR *wstr = toUTF16(uiTableValueString(value));
		uiFreeTableValue(value);
		wcsncpy(nm->item.pszText, wstr, nm->item.cchTextMax);
		uiprivFree(wstr);
		return S_OK;
	}

	return S_OK;
}

static HRESULT handleLVIF_IMAGE(uiTable *table, NMLVDISPINFOW *nm, uiprivTableColumnParams *params)
{
	if (nm->item.iSubItem == 0 && params->imageModelColumn == -1 && params->checkboxModelColumn == -1) {
		// Having an image list always leaves space for an image
		// on the main item :|
		// Other places on the internet imply that you should be
		// able to do this but that it shouldn't work, but it works
		// perfectly (and pixel-perfectly too) for me, so...
		nm->item.mask |= LVIF_INDENT;
		nm->item.iIndent = -1;
	}
	if ((nm->item.mask & LVIF_IMAGE) == 0)
		return S_OK; // nothing to do here

	// TODO see if the -1 part is correct
	// TODO see if we should use state instead of images for checkbox value
	nm->item.iImage = -1;
	if (params->imageModelColumn != -1 || params->checkboxModelColumn != -1)
		nm->item.iImage = 0;
	return S_OK;
}

static HRESULT uiprivTableHandleLVN_GETDISPINFO(uiTable *table, NMLVDISPINFOW *nm, LRESULT *lResult)
{
	uiprivTableColumnParams *params = (*(table->columns))[nm->item.iSubItem];
	HRESULT hr = handleLVIF_TEXT(table, nm, params);
	if (hr != S_OK)
		return hr;
	hr = handleLVIF_IMAGE(table, nm, params);
	if (hr != S_OK)
		return hr;
	*lResult = 0;
	return S_OK;
}

// TODO properly integrate compound statements
static BOOL onWM_NOTIFY(uiControl *c, HWND hwnd, NMHDR *nmhdr, LRESULT *lResult)
{
	uiTable *table = uiTable(c);

	switch (nmhdr->code) {
	case LVN_GETDISPINFO:
		return uiprivTableHandleLVN_GETDISPINFO(table, (NMLVDISPINFOW *)nmhdr, lResult) == S_OK;
	case NM_CUSTOMDRAW:
		return uiprivTableHandleNM_CUSTOMDRAW(table, (NMLVCUSTOMDRAW *)nmhdr, lResult) == S_OK;
	case NM_CLICK:
		return uiprivTableHandleNM_CLICK(table, (NMITEMACTIVATE *)nmhdr, lResult) == S_OK;
	case LVN_ITEMCHANGED: {
		NMLISTVIEW *nm = (NMLISTVIEW *)nmhdr;

		// TODO clean up these if cases
		if (!table->inLButtonDown && table->edit == NULL)
			return FALSE;
		UINT oldSelected = nm->uOldState & LVIS_SELECTED;
		UINT newSelected = nm->uNewState & LVIS_SELECTED;
		if (table->inLButtonDown && oldSelected == 0 && newSelected != 0) {
			table->inDoubleClickTimer = TRUE;
			// TODO check error
			SetTimer(table->hwnd, (UINT_PTR)(&(table->inDoubleClickTimer)),
				GetDoubleClickTime(), NULL);
			*lResult = 0;
			return TRUE;
		}
		// the nm->iItem == -1 case is because that is used if "the change has been applied to all items in the list view"
		if (table->edit != NULL && oldSelected != 0 && newSelected == 0 && (table->editedItem == nm->iItem || nm->iItem == -1)) {
			// TODO see if the real list view accepts or rejects changes here; Windows Explorer accepts
			HRESULT hr = uiprivTableFinishEditingText(table);
			if (hr != S_OK) {
				// TODO
				return FALSE;
			}
			*lResult = 0;
			return TRUE;
		}
		return FALSE;
	}
	// the real list view accepts changes when scrolling or clicking column headers
	case LVN_BEGINSCROLL:
	case LVN_COLUMNCLICK:
		if (uiprivTableFinishEditingText(table) != S_OK) {
			return FALSE;
		}
		*lResult = 0;
		return TRUE;
	}
	return FALSE;
}

static void uiTableDestroy(uiControl *c)
{
	uiTable *table = uiTable(c);

	HRESULT hr = uiprivTableAbortEditingText(table);
	if (hr != S_OK) {
		// TODO
	}
	uiWindowsUnregisterWM_NOTIFYHandler(table->hwnd);
	uiWindowsEnsureDestroyWindow(table->hwnd);
	// detach table from model

	uiTableModel *model = table->model;
	std::vector<uiTable *>::iterator it = std::find(model->tables->begin(), model->tables->end(), table);
	if (it != model->tables->end()) {
		model->tables->erase(it);
	}
	// free the columns
	for (auto col : *(table->columns)) {
		uiprivFree(col);
	}
	delete table->columns;
	// t->imagelist will be automatically destroyed
	delete table->indeterminatePositions;
	uiFreeControl(uiControl(table));
}

uiWindowsControlAllDefaultsExceptDestroy(uiTable)

// suggested listview sizing from http://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing:
// "columns widths that avoid truncated data x an integral number of items"
// Don't think that'll cut it when some cells have overlong data (eg
// stupidly long URLs). So for now, just hardcode a minimum.
// TODO Investigate using LVM_GETHEADER/HDM_LAYOUT here
// TODO investigate using LVM_APPROXIMATEVIEWRECT here
#define tableMinWidth 107		/* in line with other controls */
#define tableMinHeight (14 * 3) /* header + 2 lines (roughly) */

static void uiTableMinimumSize(uiWindowsControl *c, int *width, int *height)
{
	uiTable *table = uiTable(c);
	int x = tableMinWidth;
	int y = tableMinHeight;
	uiWindowsSizing sizing;
	uiWindowsGetSizing(table->hwnd, &sizing);
	uiWindowsSizingDlgUnitsToPixels(&sizing, &x, &y);
	*width = x;
	*height = y;
}

static uiprivTableColumnParams *appendColumn(uiTable *table, const char *name)
{
	LVCOLUMNW lvc;
	ZeroMemory(&lvc, sizeof(LVCOLUMNW));
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = 120; // Width of the column in pixels

	WCHAR *wstr = toUTF16(name);
	lvc.pszText = wstr;

	if (ListView_InsertColumn(table->hwnd, table->columns->size(), &lvc) == -1)
		logLastError(L"error calling LVM_INSERTCOLUMNW in appendColumn()");
	uiprivFree(wstr);

	uiprivTableColumnParams *params = uiprivNew(uiprivTableColumnParams);
	params->textModelColumn = -1;
	params->textEditableModelColumn = -1;
	params->textParams = uiprivDefaultTextColumnOptionalParams;
	params->imageModelColumn = -1;
	params->checkboxModelColumn = -1;
	params->checkboxEditableModelColumn = -1;
	params->progressBarModelColumn = -1;
	params->buttonModelColumn = -1;
	table->columns->push_back(params);
	return params;
}

void uiTableAppendTextColumn(uiTable *t, const char *name, int textModelColumn, int textEditableModelColumn, uiTableTextColumnOptionalParams *textParams)
{
	uiprivTableColumnParams *params = appendColumn(t, name);
	params->textModelColumn = textModelColumn;
	params->textEditableModelColumn = textEditableModelColumn;
	if (textParams != NULL)
		params->textParams = *textParams;
}

void uiTableAppendImageColumn(uiTable *t, const char *name, int imageModelColumn)
{
	uiprivTableColumnParams *params = appendColumn(t, name);
	params->imageModelColumn = imageModelColumn;
}

void uiTableAppendImageTextColumn(uiTable *t, const char *name, int imageModelColumn, int textModelColumn, int textEditableModelColumn, uiTableTextColumnOptionalParams *textParams)
{
	uiprivTableColumnParams *params = appendColumn(t, name);
	params->textModelColumn = textModelColumn;
	params->textEditableModelColumn = textEditableModelColumn;
	if (textParams != NULL)
		params->textParams = *textParams;
	params->imageModelColumn = imageModelColumn;
}

void uiTableAppendCheckboxColumn(uiTable *t, const char *name, int checkboxModelColumn, int checkboxEditableModelColumn)
{
	uiprivTableColumnParams *params = appendColumn(t, name);
	params->checkboxModelColumn = checkboxModelColumn;
	params->checkboxEditableModelColumn = checkboxEditableModelColumn;
}

void uiTableAppendCheckboxTextColumn(uiTable *t, const char *name, int checkboxModelColumn, int checkboxEditableModelColumn, int textModelColumn, int textEditableModelColumn, uiTableTextColumnOptionalParams *textParams)
{
	uiprivTableColumnParams *params = appendColumn(t, name);
	params->textModelColumn = textModelColumn;
	params->textEditableModelColumn = textEditableModelColumn;
	if (textParams != NULL)
		params->textParams = *textParams;
	params->checkboxModelColumn = checkboxModelColumn;
	params->checkboxEditableModelColumn = checkboxEditableModelColumn;
}

void uiTableAppendButtonColumn(uiTable *t, const char *name, int buttonModelColumn, int buttonClickableModelColumn)
{
	uiprivTableColumnParams *params = appendColumn(t, name);
	params->buttonModelColumn = buttonModelColumn;
	params->buttonClickableModelColumn = buttonClickableModelColumn;
}

uiTable *uiNewTable(uiTableParams *params)
{
	uiTable *table;
	uiWindowsNewControl(uiTable, table);

	table->columns = new std::vector<uiprivTableColumnParams *>;
	table->model = params->Model;
	table->backgroundColumn = params->RowBackgroundColorModelColumn;

	// WS_CLIPCHILDREN is here to prevent drawing over the edit box used for editing text
	table->hwnd = uiWindowsEnsureCreateControlHWND(WS_EX_CLIENTEDGE,
		WC_LISTVIEW, L"",
		LVS_REPORT | LVS_OWNERDATA | LVS_SINGLESEL | WS_CLIPCHILDREN | WS_TABSTOP | WS_HSCROLL | WS_VSCROLL,
		hInstance, NULL, TRUE);
	table->model->tables->push_back(table);
	uiWindowsRegisterWM_NOTIFYHandler(table->hwnd, onWM_NOTIFY, uiControl(table));

	// TODO: try LVS_EX_AUTOSIZECOLUMNS
	ListView_SetExtendedListViewStyleEx(table->hwnd,
		LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP | LVS_EX_SUBITEMIMAGES,
		LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP | LVS_EX_SUBITEMIMAGES);

	int n = uiprivTableModelNumRows(table->model);
	if (ListView_SetItemCount(table->hwnd, n) == 0)
		logLastError(L"error calling LVM_SETITEMCOUNT in uiNewTable()");

	if (uiprivUpdateImageListSize(table) != S_OK) {
		logLastError(L"error calling uiprivUpdateImageListSize()");
	}

	table->indeterminatePositions = new std::map<std::pair<int, int>, LONG>;
	if (SetWindowSubclass(table->hwnd, tableSubProc, 0, (DWORD_PTR)table) == FALSE)
		logLastError(L"SetWindowSubclass()");

	return table;
}
