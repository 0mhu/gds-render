#ifndef __TREE_STORE_H__
#define __TREE_STORE_H__

#include <gtk/gtk.h>

enum cell_store_columns {
        CELL_SEL_LIBRARY = 0,
        CELL_SEL_CELL,
        CELL_SEL_MODDATE,
        CELL_SEL_ACCESSDATE,
        CELL_SEL_COLUMN_COUNT
};

GtkTreeStore *setup_cell_selector(GtkTreeView* view);

#endif /* __TREE_STORE_H__ */
