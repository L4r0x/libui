#include "uipriv.h"
#include "table.h"

int uiprivTableModelNumColumns(uiTableModel *model)
{
	uiTableModelHandler *handler = uiprivTableModelHandler(model);
	void *data = uiprivTableModelData(model);
	return (*(handler->NumColumns))(model, data);
}

uiTableValueType uiprivTableModelColumnType(uiTableModel *model, int column)
{
	uiTableModelHandler *handler = uiprivTableModelHandler(model);
	void *data = uiprivTableModelData(model);
	return (*(handler->ColumnType))(model, data, column);
}

int uiprivTableModelNumRows(uiTableModel *model)
{
	uiTableModelHandler *handler = uiprivTableModelHandler(model);
	void *data = uiprivTableModelData(model);
	return (*(handler->NumRows))(model, data);
}

uiTableValue *uiprivTableModelCellValue(uiTableModel *model, int row, int column)
{
	uiTableModelHandler *handler = uiprivTableModelHandler(model);
	void *data = uiprivTableModelData(model);
	return (*(handler->CellValue))(model, data, row, column);
}

void uiprivTableModelSetCellValue(uiTableModel *model, int row, int column, const uiTableValue *value)
{
	uiTableModelHandler *handler = uiprivTableModelHandler(model);
	void *data = uiprivTableModelData(model);
	(*(handler->SetCellValue))(model, data, row, column, value);
}

const uiTableTextColumnOptionalParams uiprivDefaultTextColumnOptionalParams = {
	.ColorModelColumn = -1,
};

int uiprivTableModelCellEditable(uiTableModel *model, int row, int column)
{
	switch (column) {
	case uiTableModelColumnNeverEditable:
		return 0;
	case uiTableModelColumnAlwaysEditable:
		return 1;
	}
	uiTableValue *value = uiprivTableModelCellValue(model, row, column);
	int editable = uiTableValueInt(value);
	uiFreeTableValue(value);
	return editable;
}

int uiprivTableModelColorIfProvided(
	uiTableModel *model, int row, int column, double *r, double *g, double *b, double *a)
{
	if (column == -1)
		return 0;
	uiTableValue *value = uiprivTableModelCellValue(model, row, column);
	if (value == NULL)
		return 0;
	uiTableValueColor(value, r, g, b, a);
	uiFreeTableValue(value);
	return 1;
}
