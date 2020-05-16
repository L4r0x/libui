// 26 june 2016
#include "uipriv_unix.h"
#include "table.h"

static void uiTableModel_gtk_tree_model_interface_init(GtkTreeModelIface *iface);

G_DEFINE_TYPE_WITH_CODE(uiTableModel, uiTableModel, G_TYPE_OBJECT,
	G_IMPLEMENT_INTERFACE(GTK_TYPE_TREE_MODEL, uiTableModel_gtk_tree_model_interface_init))

static void uiTableModel_init(uiTableModel *model)
{
	// nothing to do
}

static void uiTableModel_dispose(GObject *obj)
{
	G_OBJECT_CLASS(uiTableModel_parent_class)->dispose(obj);
}

static void uiTableModel_finalize(GObject *obj)
{
	G_OBJECT_CLASS(uiTableModel_parent_class)->finalize(obj);
}

static GtkTreeModelFlags uiTableModel_get_flags(GtkTreeModel *model)
{
	return GTK_TREE_MODEL_LIST_ONLY;
}

static gint uiTableModel_get_n_columns(GtkTreeModel *model)
{
	return uiprivTableModelNumColumns(uiTableModel(model));
}

static GType uiTableModel_get_column_type(GtkTreeModel *model, gint index)
{
	switch (uiprivTableModelColumnType(uiTableModel(model), index)) {
	case uiTableValueTypeString:
		return G_TYPE_STRING;
	case uiTableValueTypeImage:
		return G_TYPE_POINTER;
	case uiTableValueTypeInt:
		return G_TYPE_INT;
	case uiTableValueTypeColor:
		return GDK_TYPE_RGBA;
	}
	return G_TYPE_INVALID;
}

#define STAMP_GOOD 0x1234
#define STAMP_BAD 0x5678

static gboolean uiTableModel_get_iter(GtkTreeModel *model, GtkTreeIter *iter, GtkTreePath *path)
{
	if (gtk_tree_path_get_depth(path) == 1) {
		gint row = gtk_tree_path_get_indices(path)[0];
		if (row >= 0 && row < uiprivTableModelNumRows(uiTableModel(model))) {
			iter->stamp = STAMP_GOOD;
			iter->user_data = GINT_TO_POINTER(row);
			return TRUE;
		}
	}
	iter->stamp = STAMP_BAD;
	return FALSE;
}

// GtkListStore returns NULL on error; let's do that too
static GtkTreePath *uiTableModel_get_path(GtkTreeModel *mm, GtkTreeIter  *iter)
{
	if (iter->stamp != STAMP_GOOD)
		return NULL;

	gint row = GPOINTER_TO_INT(iter->user_data);
	return gtk_tree_path_new_from_indices(row, -1);
}

// GtkListStore leaves value empty on failure; let's do the same
static void uiTableModel_get_value(GtkTreeModel *mm, GtkTreeIter *iter, gint column, GValue *value)
{
	if (iter->stamp != STAMP_GOOD)
		return;

	uiTableModel *model = uiTableModel(mm);
	gint row = GPOINTER_TO_INT(iter->user_data);
	uiTableValue *tvalue = uiprivTableModelCellValue(model, row, column);
	switch (uiprivTableModelColumnType(model, column)) {
	case uiTableValueTypeString:
		g_value_init(value, G_TYPE_STRING);
		g_value_set_string(value, uiTableValueString(tvalue));
		uiFreeTableValue(tvalue);
		return;
	case uiTableValueTypeImage:
		g_value_init(value, G_TYPE_POINTER);
		g_value_set_pointer(value, uiTableValueImage(tvalue));
		uiFreeTableValue(tvalue);
		return;
	case uiTableValueTypeInt:
		g_value_init(value, G_TYPE_INT);
		g_value_set_int(value, uiTableValueInt(tvalue));
		uiFreeTableValue(tvalue);
		return;
	case uiTableValueTypeColor:
		g_value_init(value, GDK_TYPE_RGBA);
		if (tvalue) {
			GdkRGBA rgba;
			uiTableValueColor(tvalue, &rgba.red, &rgba.green, &rgba.blue, &rgba.alpha);
			uiFreeTableValue(tvalue);
			g_value_set_boxed(value, &rgba);
		} else {
			g_value_set_boxed(value, NULL);
		}
		return;
	}
}

static gboolean uiTableModel_iter_next(GtkTreeModel *model, GtkTreeIter *iter)
{
	if (iter->stamp != STAMP_GOOD)
		return FALSE;

	gint row = GPOINTER_TO_INT(iter->user_data) + 1;
	if (row < uiprivTableModelNumRows(uiTableModel(model))) {
		iter->user_data = GINT_TO_POINTER(row);
		return TRUE;
	}
	iter->stamp = STAMP_BAD;
	return FALSE;
}

static gboolean uiTableModel_iter_previous(GtkTreeModel *model, GtkTreeIter *iter)
{
	if (iter->stamp != STAMP_GOOD)
		return FALSE;

	gint row = GPOINTER_TO_INT(iter->user_data) - 1;
	if (row > 0) {
		iter->user_data = GINT_TO_POINTER(row);
		return TRUE;
	}
	iter->stamp = STAMP_BAD;
	return FALSE;
}

static gboolean uiTableModel_iter_children(GtkTreeModel *model, GtkTreeIter *iter, GtkTreeIter *parent)
{
	return gtk_tree_model_iter_nth_child(model, iter, parent, 0);
}

static gboolean uiTableModel_iter_has_child(GtkTreeModel *model, GtkTreeIter *iter)
{
	return FALSE;
}

static gint uiTableModel_iter_n_children(GtkTreeModel *model, GtkTreeIter *iter)
{
	if (iter != NULL)
		return 0;
	return uiprivTableModelNumRows(uiTableModel(model));
}

static gboolean uiTableModel_iter_nth_child(GtkTreeModel *model, GtkTreeIter *iter, GtkTreeIter *parent, gint n)
{
	if (iter->stamp != STAMP_GOOD)
		return FALSE;

	if (parent == NULL) {
		if (n >= 0 && n < uiprivTableModelNumRows(uiTableModel(model))) {
			iter->stamp = STAMP_GOOD;
			iter->user_data = GINT_TO_POINTER(n);
			return TRUE;
		}
	}
	iter->stamp = STAMP_BAD;
	return FALSE;
}

gboolean uiTableModel_iter_parent(GtkTreeModel *model, GtkTreeIter *iter, GtkTreeIter *child)
{
	iter->stamp = STAMP_BAD;
	return FALSE;
}

static void uiTableModel_class_init(uiTableModelClass *class)
{
	G_OBJECT_CLASS(class)->dispose = uiTableModel_dispose;
	G_OBJECT_CLASS(class)->finalize = uiTableModel_finalize;
}

static void uiTableModel_gtk_tree_model_interface_init(GtkTreeModelIface *iface)
{
	iface->get_flags = uiTableModel_get_flags;
	iface->get_n_columns = uiTableModel_get_n_columns;
	iface->get_column_type = uiTableModel_get_column_type;
	iface->get_iter = uiTableModel_get_iter;
	iface->get_path = uiTableModel_get_path;
	iface->get_value = uiTableModel_get_value;
	iface->iter_next = uiTableModel_iter_next;
	iface->iter_previous = uiTableModel_iter_previous;
	iface->iter_children = uiTableModel_iter_children;
	iface->iter_has_child = uiTableModel_iter_has_child;
	iface->iter_n_children = uiTableModel_iter_n_children;
	iface->iter_nth_child = uiTableModel_iter_nth_child;
	iface->iter_parent = uiTableModel_iter_parent;
	// don't specify ref_node() or unref_node()
}

uiTableModel *uiNewTableModel(uiTableModelHandler mh, void *tableData)
{
	uiTableModel *model = uiTableModel(g_object_new(uiTableModelType, NULL));
	model->mh = mh;
	model->data = tableData;
	return model;
}

void uiFreeTableModel(uiTableModel *model)
{
	g_object_unref(model);
}

void uiTableModelRowInserted(uiTableModel *model, int newIndex)
{
	GtkTreeIter iter = {
		.stamp = STAMP_GOOD,
		.user_data = GINT_TO_POINTER(newIndex),
	};
	GtkTreePath *path = gtk_tree_path_new_from_indices(newIndex, -1);
	gtk_tree_model_row_inserted(GTK_TREE_MODEL(model), path, &iter);
	gtk_tree_path_free(path);
}

void uiTableModelRowChanged(uiTableModel *model, int index)
{
	GtkTreeIter iter = {
		.stamp = STAMP_GOOD,
		.user_data = GINT_TO_POINTER(index),
	};
	GtkTreePath *path = gtk_tree_path_new_from_indices(index, -1);
	gtk_tree_model_row_changed(GTK_TREE_MODEL(model), path, &iter);
	gtk_tree_path_free(path);
}

void uiTableModelRowDeleted(uiTableModel *model, int oldIndex)
{
	GtkTreePath *path = gtk_tree_path_new_from_indices(oldIndex, -1);
	gtk_tree_model_row_deleted(GTK_TREE_MODEL(model), path);
	gtk_tree_path_free(path);
}

uiTableModelHandler *uiprivTableModelHandler(uiTableModel *model)
{
	return &model->mh;
}

extern void *uiprivTableModelData(uiTableModel *model)
{
	return model->data;
}
