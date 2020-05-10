// 17 june 2018
#include "table.hpp"
#include "uipriv_windows.hpp"

// TODOs
// - clicking on the same item restarts editing instead of cancels it

// this is not how the real list view positions and sizes the edit control, but this is a) close enough b) a lot easier to follow c) something I can actually get working d) something I'm slightly more comfortable including in libui
static HRESULT resizeEdit(uiTable *t, WCHAR *wstr, int iItem, int iSubItem)
{

	uiprivTableMetrics *metrics;
	HRESULT hr = uiprivTableGetMetrics(t, iItem, iSubItem, &metrics);
	if (hr != S_OK)
		return hr;
	RECT r = metrics->realTextRect;
	uiprivFree(metrics);

	// TODO check errors for all these
	HDC dc = GetDC(t->hwnd); // use the list view DC since we're using its coordinate space
	HFONT prevFont = (HFONT)SelectObject(dc, hMessageFont);
	TEXTMETRICW tm;
	GetTextMetricsW(dc, &tm);
	SIZE textSize;
	GetTextExtentPoint32W(dc, wstr, wcslen(wstr), &textSize);
	SelectObject(dc, prevFont);
	ReleaseDC(t->hwnd, dc);

	RECT editRect;
	SendMessageW(t->edit, EM_GETRECT, 0, (LPARAM)(&editRect));
	r.left -= editRect.left;
	// find the top of the text
	r.top += ((r.bottom - r.top) - tm.tmHeight) / 2;
	// and move THAT by the right offset
	r.top -= editRect.top;
	r.right = r.left + textSize.cx;
	// the real listview does this to add some extra space at the end
	// TODO this still isn't enough space
	r.right += 4 * GetSystemMetrics(SM_CXEDGE) + GetSystemMetrics(SM_CYEDGE);
	// and make the bottom equally positioned to the top
	r.bottom = r.top + editRect.top + tm.tmHeight + editRect.top;

	// make sure the edit box doesn't stretch outside the listview
	// the list view just does this, which is dumb for when the list view wouldn't be visible at all, but given that it doesn't scroll the edit into view either...
	// TODO check errors
	RECT clientRect;
	GetClientRect(t->hwnd, &clientRect);
	IntersectRect(&r, &r, &clientRect);

	// TODO check error or use the right function
	SetWindowPos(t->edit, NULL,
		r.left, r.top,
		r.right - r.left, r.bottom - r.top,
		SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER);
	return S_OK;
}

// the real list view intercepts these keys to control editing
static LRESULT CALLBACK editSubProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIDSubclass, DWORD_PTR dwRefData)
{
	uiTable *t = (uiTable *)dwRefData;
	HRESULT hr;

	switch (uMsg) {
	case WM_KEYDOWN:
		switch (wParam) {
		// TODO handle VK_TAB and VK_SHIFT+VK_TAB
		case VK_RETURN:
			hr = uiprivTableFinishEditingText(t);
			if (hr != S_OK) {
				logLastError(L"uiprivTableFinishEditingText()");
			}
			return 0;
		case VK_ESCAPE:
			hr = uiprivTableAbortEditingText(t);
			if (hr != S_OK) {
				logLastError(L"uiprivTableFinishEditingText()");
			}
			return 0;
		}
		break;
	// the real list view also forces these flags
	case WM_GETDLGCODE:
		return DLGC_HASSETSEL | DLGC_WANTALLKEYS;
	case WM_NCDESTROY:
		if (RemoveWindowSubclass(hwnd, editSubProc, uIDSubclass) == FALSE)
			logLastError(L"RemoveWindowSubclass()");
		break;
	}
	return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}

static HRESULT openEditControl(uiTable *t, int iItem, int iSubItem, uiprivTableColumnParams *p)
{
	uiTableValue *value;
	WCHAR *wstr;
	HRESULT hr;

	// the real list view accepts changes to the existing item when editing a new item
	hr = uiprivTableFinishEditingText(t);
	if (hr != S_OK)
		return hr;

	// the real list view creates the edit control with the string
	value = uiprivTableModelCellValue(t->model, iItem, p->textModelColumn);
	wstr = toUTF16(uiTableValueString(value));
	uiFreeTableValue(value);
	// TODO copy WS_EX_RTLREADING
	t->edit = CreateWindowExW(0,
		L"EDIT", wstr,
		// these styles are what the normal listview edit uses
		WS_CHILD | WS_CLIPSIBLINGS | WS_BORDER | ES_AUTOHSCROLL,
		// as is this size
		0, 0, 16384, 16384,
		// and this control ID
		t->hwnd, (HMENU)1, hInstance, NULL);
	if (t->edit == NULL) {
		logLastError(L"CreateWindowExW()");
		uiprivFree(wstr);
		return E_FAIL;
	}
	SetWindowFont(t->edit, hMessageFont, TRUE);
	// TODO check errors
	SetWindowSubclass(t->edit, editSubProc, 0, (DWORD_PTR)t);

	hr = resizeEdit(t, wstr, iItem, iSubItem);
	if (hr != S_OK)
		// TODO proper cleanup
		return hr;
	// TODO check errors on these two, if any
	SetFocus(t->edit);
	ShowWindow(t->edit, SW_SHOW);
	Edit_SetSel(t->edit, 0, -1);

	uiprivFree(wstr);
	t->editedItem = iItem;
	t->editedSubitem = iSubItem;
	return S_OK;
}

HRESULT uiprivTableResizeWhileEditing(uiTable *t)
{
	if (t->edit == NULL)
		return S_OK;
	WCHAR *text = windowText(t->edit);
	HRESULT hr = resizeEdit(t, text, t->editedItem, t->editedSubitem);
	uiprivFree(text);
	return hr;
}

HRESULT uiprivTableFinishEditingText(uiTable *t)
{
	uiprivTableColumnParams *p;
	uiTableValue *value;
	char *text;

	if (t->edit == NULL)
		return S_OK;
	text = uiWindowsWindowText(t->edit);
	value = uiNewTableValueString(text);
	uiFreeText(text);
	p = (*(t->columns))[t->editedSubitem];
	uiprivTableModelSetCellValue(t->model, t->editedItem, p->textModelColumn, value);
	uiFreeTableValue(value);
	// always refresh the value in case the model rejected it
	if (SendMessageW(t->hwnd, LVM_UPDATE, (WPARAM)(t->editedItem), 0) == (LRESULT)(-1)) {
		logLastError(L"LVM_UPDATE");
		return E_FAIL;
	}
	return uiprivTableAbortEditingText(t);
}

HRESULT uiprivTableAbortEditingText(uiTable *t)
{
	if (t->edit == NULL)
		return S_OK;
	// set t->edit to NULL now so we don't trigger commits on focus killed
	HWND edit = t->edit;
	t->edit = NULL;

	if (DestroyWindow(edit) == 0) {
		logLastError(L"DestroyWindow()");
		return E_FAIL;
	}
	return S_OK;
}

HRESULT uiprivTableHandleNM_CLICK(uiTable *table, NMITEMACTIVATE *nm, LRESULT *lResult)
{
	LVHITTESTINFO ht;
	ZeroMemory(&ht, sizeof(LVHITTESTINFO));
	ht.pt = nm->ptAction;
	if (ListView_SubItemHitTest(table->hwnd, &ht) == -1) {
		*lResult = 0;
		return S_OK;
	}
	int row = ht.iItem;
	int col = ht.iSubItem;

	uiprivTableColumnParams *params = (*(table->columns))[col];
	if (params->textModelColumn != -1) {
		// Ignore if in double click interval
		if (table->inDoubleClickTimer) {
			*lResult = 0;
			return S_OK;
		}
		if (uiprivTableModelCellEditable(table->model, row, params->textEditableModelColumn)) {
			HRESULT hr = openEditControl(table, row, col, params);
			if (hr != S_OK)
				return hr;
		}
	} else if (params->checkboxModelColumn != -1) {
		if ((ht.flags & LVHT_ONITEMICON) != 0) {
			if (uiprivTableModelCellEditable(table->model, row, params->checkboxEditableModelColumn)) {
				uiTableValue *value = uiprivTableModelCellValue(table->model, row, params->checkboxModelColumn);
				int checked = uiTableValueInt(value);
				uiFreeTableValue(value);
				value = uiNewTableValueInt(!checked);
				uiprivTableModelSetCellValue(table->model, row, params->checkboxModelColumn, value);
				uiFreeTableValue(value);
			}
		}
	} else if (params->buttonModelColumn != -1) {
		if (uiprivTableModelCellEditable(table->model, row, params->buttonClickableModelColumn)) {
			uiprivTableModelSetCellValue(table->model, row, params->buttonModelColumn, NULL);
		}
	}

	// always refresh the value in case the model rejected it
	if (!ListView_Update(table->hwnd, row)) {
		logLastError(L"LVM_UPDATE");
		return E_FAIL;
	}

	*lResult = 0;
	return S_OK;
}
