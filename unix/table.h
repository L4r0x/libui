#ifndef __LIBUI_TABLE_H__
#define __LIBUI_TABLE_H__

#include "../common/table.h"

#include "uipriv_unix.h"

// tablemodel.c
// Class of definition of out custom table model
typedef struct uiTableModelClass uiTableModelClass;
struct uiTableModelClass {
	GObjectClass parent_class;
};

// Our custom table model wrapper
struct uiTableModel {
	GObject parent_instance;
	uiTableModelHandler mh;
	void *data;
};

extern GType uiTableModel_get_type(void);

// Conversion macros
#define uiTableModelType (uiTableModel_get_type())
#define uiTableModel(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), uiTableModelType, uiTableModel))
#define uiTableModelClass(class) (G_TYPE_CHECK_CLASS_CAST((class), uiTableModelType, uiTableModelClass))

#endif
