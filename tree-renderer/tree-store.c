/*
 * GDSII-Converter
 * Copyright (C) 2018  Mario Hüttel <mario.huettel@gmx.net>
 *
 * This file is part of GDSII-Converter.
 *
 * GDSII-Converter is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * GDSII-Converter is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GDSII-Converter.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file tree-store.h
 * @brief Tree store implementation
 * @author Mario Hüttel <mario.huettel@gmx.net>
 */

/**
 * @addtogroup MainApplication
 * @{
 */

#include "tree-store.h"
#include "lib-cell-renderer.h"
#include "../gds-parser/gds-types.h"

/**
 * @brief this function olny allows cells to be selected
 * @param selection
 * @param model
 * @param path
 * @param path_currently_selected
 * @param data
 * @return TRUE if element is selectable, FALSE if not
 */
static gboolean tree_sel_func(GtkTreeSelection *selection,
				GtkTreeModel *model,
				GtkTreePath *path,
				gboolean path_currently_selected,
				gpointer data)
{
	GtkTreeIter iter;
	struct gds_cell *cell;

	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_model_get(model, &iter, CELL_SEL_CELL, &cell, -1);

	/* Allow only rows with valid cell to be selected */
	if (cell)
		return TRUE;
	else
		return FALSE;
}

/**
 * @brief Setup a GtkTreeView with the necessary columns
 * @param view Tree view to set up
 * @return TreeStore for storing data inside the GtkTreeView
 */
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
/** @} */
