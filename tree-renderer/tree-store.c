#include "tree-store.h"
#include "lib-cell-renderer.h"
#include "../gds-parser/gds-types.h"

static gboolean tree_sel_func(GtkTreeSelection *selection,
				GtkTreeModel *model,
				GtkTreePath *path,
				gboolean path_currently_selected,
				gpointer data)
{
	static int cnt = 0;
	GtkTreeIter iter;
	struct gds_cell *cell;
	gchar *p;

	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_model_get(model, &iter, CELL_SEL_CELL, &cell, -1);

	/* Allow only rows with valid cell to be selected */
	if (cell)
		return TRUE;
	else
		return FALSE;
}

GtkTreeStore *setup_cell_selector(GtkTreeView* view)
{
	GtkTreeStore *cell_store;

	GtkCellRenderer *render_dates;
	GtkCellRenderer *render_cell;
	GtkCellRenderer *render_lib;
	GtkTreeViewColumn *column;
	GdkRGBA cell_text_color;
	GValue val = G_VALUE_INIT;

	cell_store = gtk_tree_store_new(CELL_SEL_COLUMN_COUNT, G_TYPE_POINTER, G_TYPE_POINTER, G_TYPE_STRING, G_TYPE_STRING);
	gtk_tree_view_set_model(view, GTK_TREE_MODEL(cell_store));

	render_dates = gtk_cell_renderer_text_new();
	render_cell = lib_cell_renderer_new();
	render_lib = lib_cell_renderer_new();

	/* Set foreground color for cell column */
	cell_text_color.alpha = 1;
	cell_text_color.red = (double)61.0/(double)255.0;
	cell_text_color.green = (double)152.0/(double)255.0;
	cell_text_color.blue = 0.0;

	g_value_init(&val, G_TYPE_BOOLEAN);
	g_value_set_boolean(&val, TRUE);
	g_object_set_property(G_OBJECT(render_cell), "foreground-set", &val);
	g_value_unset(&val);

	g_value_init(&val, GDK_TYPE_RGBA);
	g_value_set_boxed(&val, &cell_text_color);
	g_object_set_property(G_OBJECT(render_cell), "foreground-rgba", &val);
	g_value_unset(&val);




	column = gtk_tree_view_column_new_with_attributes("Library", render_lib, "gds-lib", CELL_SEL_LIBRARY, NULL);
	gtk_tree_view_append_column(view, column);

	/* Cell color: #3D9801 */
	column = gtk_tree_view_column_new_with_attributes("Cell", render_cell, "gds-cell", CELL_SEL_CELL, NULL);
	gtk_tree_view_append_column(view, column);

	column = gtk_tree_view_column_new_with_attributes("Mod. Date", render_dates, "text", CELL_SEL_MODDATE, NULL);
	gtk_tree_view_append_column(view, column);

	column = gtk_tree_view_column_new_with_attributes("Acc. Date", render_dates, "text", CELL_SEL_ACCESSDATE, NULL);
	gtk_tree_view_append_column(view, column);

	/* Callback for selection
	 * This prevents selecting a library */
	gtk_tree_selection_set_select_function(gtk_tree_view_get_selection(view), tree_sel_func, NULL, NULL);

	return cell_store;
}
