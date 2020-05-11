#include "uipriv_unix.h"

#include "table.h"

// TODO with GDK_SCALE set to 2 the 32x32 images are scaled up to 64x64?

struct uiTable {
	uiUnixControl c;
	// scroll window holding the tree view
	GtkWidget *widget;
	GtkTreeView *treeView;
	// out custom tree view model
	uiTableModel *model;
	// array of table columns
	GPtrArray *columnParams;
	int backgroundColumn;
};

static void applyColor(GtkTreeModel *model, GtkTreeIter *iter, int modelColumn, GtkCellRenderer *renderer, const char *prop, const char *propSet)
{
	GValue value = G_VALUE_INIT;
	gtk_tree_model_get_value(model, iter, modelColumn, &value);
	GdkRGBA *rgba = (GdkRGBA *)g_value_get_boxed(&value);
	if (rgba != NULL)
		g_object_set(renderer, prop, rgba, NULL);
	else
		g_object_set(renderer, propSet, FALSE, NULL);
	g_value_unset(&value);
}

static void setEditable(uiTableModel *model, GtkTreeIter *iter, int modelColumn, GtkCellRenderer *renderer, const char *prop)
{
	// TODO avoid the need for this
	GtkTreePath *path = gtk_tree_model_get_path(GTK_TREE_MODEL(model), iter);
	int row = gtk_tree_path_get_indices(path)[0];
	gboolean editable = uiprivTableModelCellEditable(model, row, modelColumn) != 0;
	g_object_set(renderer, prop, editable, NULL);
}

static void applyBackgroundColor(uiTable *table, GtkTreeModel *model, GtkTreeIter *iter, GtkCellRenderer *renderer)
{
	if (table->backgroundColumn != -1)
		applyColor(model, iter, table->backgroundColumn,
			renderer, "cell-background-rgba", "cell-background-set");
}

static void onEdited(uiTableModel *model, int column, const char *pathstr, const uiTableValue *tvalue, GtkTreeIter *iter)
{
	GtkTreePath *path = gtk_tree_path_new_from_string(pathstr);
	int row = gtk_tree_path_get_indices(path)[0];
	if (iter != NULL)
		gtk_tree_model_get_iter(GTK_TREE_MODEL(model), iter, path);
	gtk_tree_path_free(path);
	uiprivTableModelSetCellValue(model, row, column, tvalue);
}

struct textColumnParams {
	uiTable *table;
	uiTableModel *model;
	int modelColumn;
	int editableColumn;
	uiTableTextColumnOptionalParams params;
};

static void textColumnDataFunc(GtkTreeViewColumn *column, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer data)
{
	struct textColumnParams *p = (struct textColumnParams *)data;
	GValue value = G_VALUE_INIT;
	const gchar *str;

	gtk_tree_model_get_value(model, iter, p->modelColumn, &value);
	str = g_value_get_string(&value);
	g_object_set(renderer, "text", str, NULL);
	g_value_unset(&value);

	setEditable(p->model, iter, p->editableColumn, renderer, "editable");

	if (p->params.ColorModelColumn != -1)
		applyColor(model, iter, p->params.ColorModelColumn,
			renderer, "foreground-rgba", "foreground-set");

	applyBackgroundColor(p->table, model, iter, renderer);
}

static void textColumnEdited(GtkCellRendererText *renderer, gchar *path, gchar *newText, gpointer data)
{
	struct textColumnParams *params = (struct textColumnParams *)data;

	GtkTreeIter iter;
	uiTableValue *tvalue = uiNewTableValueString(newText);
	onEdited(params->model, params->modelColumn, path, tvalue, &iter);
	uiFreeTableValue(tvalue);
	// and update the column TODO copy comment here
	textColumnDataFunc(NULL, GTK_CELL_RENDERER(renderer), GTK_TREE_MODEL(params->model), &iter, data);
}

struct imageColumnParams {
	uiTable *table;
	int modelColumn;
};

static void imageColumnDataFunc(GtkTreeViewColumn *column, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer data)
{
	struct imageColumnParams *params = (struct imageColumnParams *)data;

	//TODO	setImageSize(r);
	GValue value = G_VALUE_INIT;
	gtk_tree_model_get_value(model, iter, params->modelColumn, &value);
	uiImage *img = (uiImage *)g_value_get_pointer(&value);
	g_object_set(renderer, "surface",
		uiprivImageAppropriateSurface(img, GTK_WIDGET(params->table->treeView)),
		NULL);
	g_value_unset(&value);

	applyBackgroundColor(params->table, model, iter, renderer);
}

struct checkboxColumnParams {
	uiTable *table;
	uiTableModel *model;
	int modelColumn;
	int editableColumn;
};

static void checkboxColumnDataFunc(GtkTreeViewColumn *column, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer data)
{
	struct checkboxColumnParams *params = (struct checkboxColumnParams *)data;
	GValue value = G_VALUE_INIT;
	gtk_tree_model_get_value(model, iter, params->modelColumn, &value);
	gboolean active = g_value_get_int(&value) != 0;
	g_object_set(renderer, "active", active, NULL);
	g_value_unset(&value);

	setEditable(params->model, iter, params->editableColumn, renderer, "activatable");

	applyBackgroundColor(params->table, model, iter, renderer);
}

static void checkboxColumnToggled(GtkCellRendererToggle *renderer, gchar *pathstr, gpointer data)
{
	struct checkboxColumnParams *params = (struct checkboxColumnParams *)data;

	GtkTreePath *path = gtk_tree_path_new_from_string(pathstr);
	GtkTreeIter iter;
	gtk_tree_model_get_iter(GTK_TREE_MODEL(params->model), &iter, path);
	gtk_tree_path_free(path);

	GValue value = G_VALUE_INIT;
	gtk_tree_model_get_value(GTK_TREE_MODEL(params->model), &iter, params->modelColumn, &value);
	int v = g_value_get_int(&value);
	g_value_unset(&value);

	uiTableValue *tvalue;
	tvalue = uiNewTableValueInt(!v);
	onEdited(params->model, params->modelColumn, pathstr, tvalue, NULL);
	uiFreeTableValue(tvalue);
	// and update the column TODO copy comment here
	// TODO avoid fetching the model data twice
	checkboxColumnDataFunc(NULL, GTK_CELL_RENDERER(renderer), GTK_TREE_MODEL(params->model), &iter, data);
}

struct buttonColumnParams {
	uiTable *table;
	uiTableModel *model;
	int modelColumn;
	int clickableColumn;
};

static void buttonColumnDataFunc(GtkTreeViewColumn *column, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer data)
{
	struct buttonColumnParams *params = (struct buttonColumnParams *)data;

	GValue value = G_VALUE_INIT;
	gtk_tree_model_get_value(model, iter, params->modelColumn, &value);
	const gchar *str = g_value_get_string(&value);
	g_object_set(renderer, "text", str, NULL);
	g_value_unset(&value);

	setEditable(params->model, iter, params->clickableColumn, renderer, "sensitive");

	applyBackgroundColor(params->table, model, iter, renderer);
}

// TODO wrong type here
static void buttonColumnClicked(GtkCellRenderer *r, gchar *pathstr, gpointer data)
{
	struct buttonColumnParams *p = (struct buttonColumnParams *)data;

	onEdited(p->model, p->modelColumn, pathstr, NULL, NULL);
}

static GtkTreeViewColumn *addColumn(uiTable *t, const char *name)
{
	GtkTreeViewColumn *c;

	c = gtk_tree_view_column_new();
	gtk_tree_view_column_set_resizable(c, TRUE);
	gtk_tree_view_column_set_title(c, name);
	gtk_tree_view_append_column(t->treeView, c);
	return c;
}

static void addTextColumn(uiTable *table, GtkTreeViewColumn *column, int textModelColumn, int textEditableModelColumn, uiTableTextColumnOptionalParams *textParams)
{
	struct textColumnParams *params = uiprivNew(struct textColumnParams);
	params->table = table;
	// TODO get rid of these fields AND rename t->model in favor of t->m
	params->model = table->model;
	params->modelColumn = textModelColumn;
	params->editableColumn = textEditableModelColumn;
	if (textParams != NULL)
		params->params = *textParams;
	else
		params->params = uiprivDefaultTextColumnOptionalParams;

	GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, textColumnDataFunc, params, NULL);
	g_signal_connect(renderer, "edited", G_CALLBACK(textColumnEdited), params);
	g_ptr_array_add(table->columnParams, params);
}

// TODO rename modelCOlumn and params everywhere
void uiTableAppendTextColumn(uiTable *table, const char *name, int textModelColumn, int textEditableModelColumn, uiTableTextColumnOptionalParams *textParams)
{
	GtkTreeViewColumn *column = addColumn(table, name);
	addTextColumn(table, column, textModelColumn, textEditableModelColumn, textParams);
}

static void addImageColumn(uiTable *table, GtkTreeViewColumn *c, int imageModelColumn)
{
	struct imageColumnParams *param = uiprivNew(struct imageColumnParams);
	param->table = table;
	param->modelColumn = imageModelColumn;

	GtkCellRenderer *renderer = gtk_cell_renderer_pixbuf_new();
	gtk_tree_view_column_pack_start(c, renderer, FALSE);
	gtk_tree_view_column_set_cell_data_func(c, renderer, imageColumnDataFunc, param, NULL);
	g_ptr_array_add(table->columnParams, param);
}

void uiTableAppendImageColumn(uiTable *table, const char *name, int imageModelColumn)
{
	GtkTreeViewColumn *column = addColumn(table, name);
	addImageColumn(table, column, imageModelColumn);
}

void uiTableAppendImageTextColumn(uiTable *table, const char *name, int imageModelColumn, int textModelColumn, int textEditableModelColumn, uiTableTextColumnOptionalParams *textParams)
{
	GtkTreeViewColumn *column = addColumn(table, name);
	addImageColumn(table, column, imageModelColumn);
	addTextColumn(table, column, textModelColumn, textEditableModelColumn, textParams);
}

static void addCheckboxColumn(uiTable *table, GtkTreeViewColumn *column, int checkboxModelColumn, int checkboxEditableModelColumn)
{
	struct checkboxColumnParams *params = uiprivNew(struct checkboxColumnParams);
	params->table = table;
	params->model = table->model;
	params->modelColumn = checkboxModelColumn;
	params->editableColumn = checkboxEditableModelColumn;

	GtkCellRenderer *renderer = gtk_cell_renderer_toggle_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, checkboxColumnDataFunc, params, NULL);
	g_signal_connect(renderer, "toggled", G_CALLBACK(checkboxColumnToggled), params);
	g_ptr_array_add(table->columnParams, params);
}

void uiTableAppendCheckboxColumn(uiTable *table, const char *name, int checkboxModelColumn, int checkboxEditableModelColumn)
{
	GtkTreeViewColumn *column = addColumn(table, name);
	addCheckboxColumn(table, column, checkboxModelColumn, checkboxEditableModelColumn);
}

void uiTableAppendCheckboxTextColumn(uiTable *table, const char *name, int checkboxModelColumn, int checkboxEditableModelColumn, int textModelColumn, int textEditableModelColumn, uiTableTextColumnOptionalParams *textParams)
{
	GtkTreeViewColumn *column = addColumn(table, name);
	addCheckboxColumn(table, column, checkboxModelColumn, checkboxEditableModelColumn);
	addTextColumn(table, column, textModelColumn, textEditableModelColumn, textParams);
}

void uiTableAppendButtonColumn(uiTable *table, const char *name, int buttonModelColumn, int buttonClickableModelColumn)
{
	struct buttonColumnParams *params = uiprivNew(struct buttonColumnParams);
	params->table = table;
	params->model = table->model;
	params->modelColumn = buttonModelColumn;
	params->clickableColumn = buttonClickableModelColumn;

	GtkTreeViewColumn *column = addColumn(table, name);

	GtkCellRenderer *renderer = uiprivNewCellRendererButton();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, buttonColumnDataFunc, params, NULL);
	g_signal_connect(renderer, "clicked", G_CALLBACK(buttonColumnClicked), params);
	g_ptr_array_add(table->columnParams, params);
}

uiUnixControlAllDefaultsExceptDestroy(uiTable)

static void uiTableDestroy(uiControl *control)
{
	uiTable *table = uiTable(control);
	// Free column properties
	for (guint i = 0; i < table->columnParams->len; i++)
		uiprivFree(g_ptr_array_index(table->columnParams, i));
	g_ptr_array_free(table->columnParams, TRUE);
	// Free widgets: the treeView is freed automatically
	g_object_unref(table->widget);
	uiFreeControl(uiControl(table));
}

uiTable *uiNewTable(uiTableParams *p)
{
	uiTable *table;
	uiUnixNewControl(uiTable, table);
	table->model = p->Model;
	table->columnParams = g_ptr_array_new();
	table->backgroundColumn = p->RowBackgroundColorModelColumn;
	table->widget = gtk_scrolled_window_new(NULL, NULL);
	table->treeView = GTK_TREE_VIEW(gtk_tree_view_new_with_model(GTK_TREE_MODEL(table->model)));

	// Attaching the tree view to the scrolled window and setting the correct style
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(table->widget), GTK_SHADOW_IN);
	gtk_container_add(GTK_CONTAINER(table->widget), GTK_WIDGET(table->treeView));

	// Make the tree view visible;
	// Only the scrolled window's visibility is controlled by libui
	gtk_widget_show(GTK_WIDGET(table->treeView));

	return table;
}
