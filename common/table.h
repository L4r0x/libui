// 23 june 2018

#ifndef __LIBUI_UI_COMMON_TABLE_H__
#define __LIBUI_UI_COMMON_TABLE_H__

#include "../ui.h"

#ifdef __cplusplus
extern "C" {
#endif

// platform specific
extern uiTableModelHandler *uiprivTableModelHandler(uiTableModel *m);
extern void *uiprivTableModelData(uiTableModel *m);

// tablemodel.c
extern int uiprivTableModelNumColumns(uiTableModel *m);
extern uiTableValueType uiprivTableModelColumnType(uiTableModel *m, int column);
extern int uiprivTableModelNumRows(uiTableModel *m);
extern uiTableValue *uiprivTableModelCellValue(uiTableModel *m, int row, int column);
extern void uiprivTableModelSetCellValue(uiTableModel *m, int row, int column, const uiTableValue *value);
extern const uiTableTextColumnOptionalParams uiprivDefaultTextColumnOptionalParams;
extern int uiprivTableModelCellEditable(uiTableModel *m, int row, int column);
extern int uiprivTableModelColorIfProvided(uiTableModel *m, int row, int column, double *r, double *g, double *b, double *a);

#ifdef __cplusplus
}
#endif

#endif
