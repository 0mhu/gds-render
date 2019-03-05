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
 * @brief cell_store_filter_visible_func Decides whether an element of the tree model @p model is visible.
 * @param model Tree model
 * @param iter Current element / iter in Model to check
 * @param data Data. Set to static stores variable
 * @return TRUE if visible, else FALSE
 * @note TODO: Maybe implement Damerau-Levenshtein distance matching
 */
static gboolean cell_store_filter_visible_func(GtkTreeModel *model, GtkTreeIter *iter, gpointer data)
{
	struct tree_stores *stores = (struct tree_stores *)data;
	struct gds_cell *cell;
	struct gds_library *lib;
	gboolean result = FALSE;
	const char *search_string;

	if (!model || !iter || !stores)
		goto exit_filter;

	gtk_tree_model_get(model, iter, CELL_SEL_CELL, &cell, CELL_SEL_LIBRARY, &lib, -1);

	if (lib) {
		result = TRUE;
		goto exit_filter;
	}

	if (!cell)
		goto exit_filter;

	search_string = gtk_entry_get_text(stores->search_entry);

	/* Show all, if field is empty */
	if (!strlen(search_string))
		result = TRUE;

	if (strstr(cell->name, search_string))
		result = TRUE;

	gtk_tree_view_expand_all(stores->base_tree_view);

exit_filter:
	return result;
}

static void change_filter(GtkWidget *entry, gpointer data)
{
	struct tree_stores *stores = (struct tree_stores *)data;
	gtk_tree_model_filter_refilter(stores->filter);
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
	GtkCellRenderer *render_dates;
	GtkCellRenderer *render_cell;
	GtkCellRenderer *render_lib;
	GtkTreeViewColumn *column;

	stores.base_tree_view = view;
	stores.search_entry = search_entry;

	stores.base_store = gtk_tree_store_new(CELL_SEL_COLUMN_COUNT, G_TYPE_POINTER, G_TYPE_POINTER, G_TYPE_UINT, G_TYPE_STRING, G_TYPE_STRING);

	/* Searching */
	if (search_entry) {
		stores.filter = GTK_TREE_MODEL_FILTER(gtk_tree_model_filter_new(GTK_TREE_MODEL(stores.base_store), NULL));
		gtk_tree_model_filter_set_visible_func (stores.filter,
								(GtkTreeModelFilterVisibleFunc)cell_store_filter_visible_func,
								 &stores, NULL);
		g_signal_connect(GTK_SEARCH_ENTRY(search_entry), "search-changed", G_CALLBACK(change_filter), &stores);
	}

	gtk_tree_view_set_model(view, GTK_TREE_MODEL(stores.filter));

	render_dates = gtk_cell_renderer_text_new();
	render_cell = lib_cell_renderer_new();
	render_lib = lib_cell_renderer_new();

	column = gtk_tree_view_column_new_with_attributes("Library", render_lib, "gds-lib", CELL_SEL_LIBRARY, NULL);
	gtk_tree_view_append_column(view, column);

	column = gtk_tree_view_column_new_with_attributes("Cell", render_cell, "gds-cell", CELL_SEL_CELL,
							  "error-level", CELL_SEL_CELL_ERROR_STATE, NULL);
	gtk_tree_view_append_column(view, column);

	column = gtk_tree_view_column_new_with_attributes("Mod. Date", render_dates, "text", CELL_SEL_MODDATE, NULL);
	gtk_tree_view_append_column(view, column);

	column = gtk_tree_view_column_new_with_attributes("Acc. Date", render_dates, "text", CELL_SEL_ACCESSDATE, NULL);
	gtk_tree_view_append_column(view, column);

	/* Callback for selection
	 * This prevents selecting a library */
	gtk_tree_selection_set_select_function(gtk_tree_view_get_selection(view), tree_sel_func, NULL, NULL);

	return &stores;
}
/** @} */
