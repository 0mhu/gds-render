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
 * @brief tree_sel_search_func
 * @param model Tree model
 * @param column Column id
 * @param key Search key
 * @param iter Iterator
 * @param search_data User data. In this case always NULL
 * @return returns false (!) if matches
 */
static gboolean tree_sel_search_func(GtkTreeModel *model, gint column, const gchar *key, GtkTreeIter *iter, gpointer *search_data)
{
	(void)search_data;
	(void)column;
	struct gds_cell *cell;
	struct gds_library *lib;
	GtkTreePath *path;
	GtkTreeIter lib_iter;
	char *lib_str = NULL, *cell_str = NULL, *div_str = NULL, *key_copy;
	gboolean result = TRUE;

	gtk_tree_model_get(model, iter, CELL_SEL_CELL, &cell, -1);
	path = gtk_tree_model_get_path(model, iter);

	/* Libraries not selectable */
	if (!cell)
		return TRUE;

	if (!gtk_tree_path_up(path)) {
		gtk_tree_path_free(path);
		printf("Cell without parent library found during search! Somethings really wrong!!!\n");
		return TRUE;
	}

	/* Find name of parent library */
	gtk_tree_model_get_iter(model, &lib_iter, path);
	gtk_tree_model_get(model, &lib_iter, CELL_SEL_LIBRARY, &lib, -1);
	gtk_tree_path_free(path);

	if (!lib) {
		printf("Parent library invalid!\n");
		return TRUE;
	}

	/* Search for Library/Cell division operator */
	key_copy = strdup(key);
	if (!key_copy)
		goto abort_search;

	if ((div_str = strstr(key_copy, ":"))) {
		lib_str = key_copy;
		*div_str = 0x00;
		cell_str = div_str + 1;

		if (!strncmp(lib_str, lib->name, sizeof(lib->name))) {
			if (!strncmp(cell_str, cell->name, sizeof(cell->name)))
				result = FALSE;
		}

	} else {
		result = (strncmp(key, cell->name, sizeof(cell->name)) ? TRUE : FALSE);
	}

abort_search:
	return result;
}

/**
 * @brief Setup a GtkTreeView with the necessary columns
 * @param view Tree view to set up
 * @param search_entry Entry field for search
 * @return Tree stores for storing data inside the GtkTreeView
 */
struct tree_stores *setup_cell_selector(GtkTreeView* view, GtkEntry *search_entry)
{
	static struct tree_stores stores;

	GtkTreeStore *cell_store;

	GtkCellRenderer *render_dates;
	GtkCellRenderer *render_cell;
	GtkCellRenderer *render_lib;
	GtkTreeViewColumn *column;

	cell_store = gtk_tree_store_new(CELL_SEL_COLUMN_COUNT, G_TYPE_POINTER, G_TYPE_POINTER, GDK_TYPE_RGBA, G_TYPE_STRING, G_TYPE_STRING);
	gtk_tree_view_set_model(view, GTK_TREE_MODEL(cell_store));

	render_dates = gtk_cell_renderer_text_new();
	render_cell = lib_cell_renderer_new();
	render_lib = lib_cell_renderer_new();

	column = gtk_tree_view_column_new_with_attributes("Library", render_lib, "gds-lib", CELL_SEL_LIBRARY, NULL);
	gtk_tree_view_append_column(view, column);

	/* Cell color: #3D9801 */
	column = gtk_tree_view_column_new_with_attributes("Cell", render_cell, "gds-cell", CELL_SEL_CELL, "foreground-rgba", CELL_SEL_CELL_COLOR, NULL);
	gtk_tree_view_append_column(view, column);

	column = gtk_tree_view_column_new_with_attributes("Mod. Date", render_dates, "text", CELL_SEL_MODDATE, NULL);
	gtk_tree_view_append_column(view, column);

	column = gtk_tree_view_column_new_with_attributes("Acc. Date", render_dates, "text", CELL_SEL_ACCESSDATE, NULL);
	gtk_tree_view_append_column(view, column);

	/* Callback for selection
	 * This prevents selecting a library */
	gtk_tree_selection_set_select_function(gtk_tree_view_get_selection(view), tree_sel_func, NULL, NULL);

	/* Searching */
	if (search_entry) {
		gtk_tree_view_set_enable_search(view, TRUE);
		gtk_tree_view_set_search_column(view, CELL_SEL_CELL);
		gtk_tree_view_set_search_equal_func(view, (GtkTreeViewSearchEqualFunc)tree_sel_search_func, NULL, NULL);
		gtk_tree_view_set_search_entry(view, search_entry);
	}

	stores.base_store = cell_store;

	return &stores;
}
/** @} */
