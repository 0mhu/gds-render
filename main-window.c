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
 * @file main-window.c
 * @brief Handling of GUI
 * @author Mario Hüttel <mario.huettel@gmx.net>
 */

/** @addtogroup MainApplication
 * @{
 */

#include "main-window.h"
#include <stdio.h>
#include "gds-parser/gds-parser.h"
#include <gtk/gtk.h>
#include "layer-selector/layer-selector.h"
#include "layer-selector/layer-selector-dnd.h"
#include "tree-renderer/tree-store.h"
#include "latex-output/latex-output.h"
#include "widgets/conv-settings-dialog.h"
#include "cairo-output/cairo-output.h"
#include "trigonometric/cell-trigonometrics.h"
#include "version/version.h"
#include "tree-renderer/lib-cell-renderer.h"
#include "gds-parser/gds-tree-checker.h"
/**
 * @brief User data supplied to callback function of the open button
 */
struct open_button_data {
	GtkWindow *main_window;
	GList **list_ptr;
	GtkTreeStore *cell_store;
	GtkListBox *layer_box;
	GtkSearchEntry *search_entry;
};

/**
 * @brief User data supplied to callback function of the convert button
 */
struct convert_button_data {
	GtkTreeView *tree_view;
	GtkWindow *main_window;
};

/**
 * @brief Window close event of main window
 *
 * Closes the main window. This leads to the termination of the whole application
 * @param window main window
 * @param user not used
 * @return TRUE. This indicates that the event has been fully handled
 */
static gboolean on_window_close(gpointer window, gpointer user)
{
	gtk_widget_destroy(GTK_WIDGET(window));
	return TRUE;
}

/**
 * @brief generate string from gds_time_field
 * @param date Date to convert
 * @return String with date
 */
static GString *generate_string_from_date(struct gds_time_field *date)
{
	GString *str;

	str = g_string_new_len(NULL, 50);
	g_string_printf(str, "%02u.%02u.%u - %02u:%02u",
			(unsigned int)date->day,
			(unsigned int)date->month,
			(unsigned int)date->year,
			(unsigned int)date->hour,
			(unsigned int)date->minute);
	return str;
}

/**
 * @brief Callback function of Load GDS button
 * @param button
 * @param user Necessary Data
 */
static void on_load_gds(gpointer button, gpointer user)
{
	GList *cell;
	GtkTreeIter libiter;
	GtkTreeIter celliter;
	GList *lib;
	struct gds_library *gds_lib;
	struct gds_cell *gds_c;
	struct open_button_data *ptr = (struct open_button_data *)user;
	GtkTreeStore *store = ptr->cell_store;
	GtkWidget *open_dialog;
	GtkFileChooser *file_chooser;
	GtkFileFilter *filter;
	GtkStyleContext *button_style;
	gint dialog_result;
	int gds_result;
	char *filename;
	GString *mod_date;
	GString *acc_date;
	unsigned int cell_error_level;

	open_dialog = gtk_file_chooser_dialog_new("Open GDSII File", ptr->main_window,
						  GTK_FILE_CHOOSER_ACTION_OPEN,
						  "Cancel", GTK_RESPONSE_CANCEL,
						  "Open GDSII", GTK_RESPONSE_ACCEPT,
						  NULL);
	file_chooser = GTK_FILE_CHOOSER(open_dialog);
	/* Add GDS II Filter */
	filter = gtk_file_filter_new();
	gtk_file_filter_add_pattern(filter, "*.gds");
	gtk_file_filter_set_name(filter, "GDSII-Files");
	gtk_file_chooser_add_filter(file_chooser, filter);

	dialog_result = gtk_dialog_run(GTK_DIALOG(open_dialog));

	if (dialog_result == GTK_RESPONSE_ACCEPT) {

		/* Get File name */
		filename = gtk_file_chooser_get_filename(file_chooser);

		gtk_tree_store_clear(store);
		clear_lib_list(ptr->list_ptr);

		/* Parse new GDSII file */
		gds_result = parse_gds_from_file(filename, ptr->list_ptr);

		/* Delete file name afterwards */
		g_free(filename);
		if (gds_result)
			goto end_destroy;

		/* remove suggested action from Open button */
		button_style = gtk_widget_get_style_context(GTK_WIDGET(button));
		gtk_style_context_remove_class(button_style, "suggested-action");

		for (lib = *(ptr->list_ptr); lib != NULL; lib = lib->next) {
			gds_lib = (struct gds_library *)lib->data;
			/* Create top level iter */
			gtk_tree_store_append(store, &libiter, NULL);

			/* Convert dates to String */
			mod_date = generate_string_from_date(&gds_lib->mod_time);
			acc_date = generate_string_from_date(&gds_lib->access_time);

			gtk_tree_store_set(store, &libiter,
					   CELL_SEL_LIBRARY, gds_lib,
					   CELL_SEL_MODDATE, mod_date->str,
					   CELL_SEL_ACCESSDATE, acc_date->str,
					   -1);

			/* Check this library. This might take a while */
			(void)gds_tree_check_cell_references(gds_lib);
			(void)gds_tree_check_reference_loops(gds_lib);

			/* Delete GStrings including string data. */
			/* Cell store copies String type data items */
			g_string_free(mod_date, TRUE);
			g_string_free(acc_date, TRUE);

			for (cell = gds_lib->cells; cell != NULL; cell = cell->next) {
				gds_c = (struct gds_cell *)cell->data;
				gtk_tree_store_append(store, &celliter, &libiter);

				/* Convert dates to String */
				mod_date = generate_string_from_date(&gds_c->mod_time);
				acc_date = generate_string_from_date(&gds_c->access_time);

				/* Get the checking results for this cell */
				cell_error_level = 0;
				if (gds_c->checks.unresolved_child_count)
					cell_error_level |= LIB_CELL_RENDERER_ERROR_WARN;

				/* Check if it is completely b0rken */
				if (gds_c->checks.affected_by_reference_loop)
					cell_error_level |= LIB_CELL_RENDERER_ERROR_ERR;

				/* Add cell to tree store model */
				gtk_tree_store_set(store, &celliter,
						   CELL_SEL_CELL, gds_c,
						   CELL_SEL_MODDATE, mod_date->str,
						   CELL_SEL_ACCESSDATE, acc_date->str,
						   CELL_SEL_CELL_ERROR_STATE, cell_error_level,
						   -1);

				/* Delete GStrings including string data. */
				/* Cell store copies String type data items */
				g_string_free(mod_date, TRUE);
				g_string_free(acc_date, TRUE);
			}
		}

		/* Create Layers in Layer Box */
		generate_layer_widgets(ptr->layer_box, *(ptr->list_ptr));
	}

end_destroy:
	/* Destroy dialog and filter */
	gtk_widget_destroy(open_dialog);
}

/**
 * @brief Convert button callback
 * @param button
 * @param user
 */
static void on_convert_clicked(gpointer button, gpointer user)
{
	(void)button;
	static struct render_settings sett = {
		.scale = 1000.0,
		.renderer = RENDERER_LATEX_TIKZ,
	};
	struct convert_button_data *data = (struct convert_button_data *)user;
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	GtkTreeModel *model;
	GList *layer_list;
	struct gds_cell *cell_to_render;
	FILE *output_file;
	GtkWidget *dialog;
	RendererSettingsDialog *settings;
	GtkFileFilter *filter;
	gint res;
	char *file_name;
	union bounding_box cell_box;
	unsigned int height, width;

	if (!data)
		return;

	/* Get selected cell */
	selection = gtk_tree_view_get_selection(data->tree_view);
	if (gtk_tree_selection_get_selected(selection, &model, &iter) == FALSE)
		return;

	gtk_tree_model_get(model, &iter, CELL_SEL_CELL, &cell_to_render, -1);

	if (!cell_to_render)
		return;

	/* Get layers that are rendered */
	layer_list = export_rendered_layer_info();

	/* Calculate cell size in DB units */
	bounding_box_prepare_empty(&cell_box);
	calculate_cell_bounding_box(&cell_box, cell_to_render);

	/* Calculate size in database units
	 * Note that the results are bound to be positive,
	 * so casting them to unsigned int is asbsolutely valid
	 */
	height = (unsigned int)(cell_box.vectors.upper_right.y - cell_box.vectors.lower_left.y);
	width = (unsigned int)(cell_box.vectors.upper_right.x - cell_box.vectors.lower_left.x);

	/* Show settings dialog */
	settings = renderer_settings_dialog_new(GTK_WINDOW(data->main_window));
	renderer_settings_dialog_set_settings(settings, &sett);
	renderer_settings_dialog_set_database_unit_scale(settings, cell_to_render->parent_library->unit_in_meters);
	renderer_settings_dialog_set_cell_height(settings, height);
	renderer_settings_dialog_set_cell_width(settings, width);

	res = gtk_dialog_run(GTK_DIALOG(settings));
	if (res == GTK_RESPONSE_OK) {
		renderer_settings_dialog_get_settings(settings, &sett);
		gtk_widget_destroy(GTK_WIDGET(settings));
	} else {
		gtk_widget_destroy(GTK_WIDGET(settings));
		goto ret_layer_destroy;
	}

	/* save file dialog */
	dialog = gtk_file_chooser_dialog_new((sett.renderer == RENDERER_LATEX_TIKZ
					      ? "Save LaTeX File" : "Save PDF"),
					     GTK_WINDOW(data->main_window), GTK_FILE_CHOOSER_ACTION_SAVE,
					     "Cancel", GTK_RESPONSE_CANCEL, "Save", GTK_RESPONSE_ACCEPT, NULL);
	/* Set file filter according to settings */
	filter = gtk_file_filter_new();
	switch (sett.renderer) {
	case RENDERER_LATEX_TIKZ:
		gtk_file_filter_add_pattern(filter, "*.tex");
		gtk_file_filter_set_name(filter, "LaTeX-Files");
		break;
	case RENDERER_CAIROGRAPHICS_PDF:
		gtk_file_filter_add_pattern(filter, "*.pdf");
		gtk_file_filter_set_name(filter, "PDF-Files");
		break;
	case RENDERER_CAIROGRAPHICS_SVG:
		gtk_file_filter_add_pattern(filter, "*.svg");
		gtk_file_filter_set_name(filter, "SVG-Files");
		break;
	}

	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);

	res = gtk_dialog_run(GTK_DIALOG(dialog));
	if (res == GTK_RESPONSE_ACCEPT) {
		file_name = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		gtk_widget_destroy(dialog);

		switch (sett.renderer) {
		case RENDERER_LATEX_TIKZ:
			output_file = fopen(file_name, "w");
			latex_render_cell_to_code(cell_to_render, layer_list, output_file, sett.scale,
						  sett.tex_pdf_layers, sett.tex_standalone);
			fclose(output_file);
			break;
		case RENDERER_CAIROGRAPHICS_SVG:
		case RENDERER_CAIROGRAPHICS_PDF:
			cairo_render_cell_to_vector_file(cell_to_render, layer_list,
							 (sett.renderer == RENDERER_CAIROGRAPHICS_PDF
								? file_name
								: NULL),
							 (sett.renderer == RENDERER_CAIROGRAPHICS_SVG
								? file_name
								: NULL),
							 sett.scale);
			break;
		}
		g_free(file_name);

	} else {
		gtk_widget_destroy(dialog);
	}
ret_layer_destroy:
	g_list_free_full(layer_list, (GDestroyNotify)delete_layer_info_struct);
}

/**
 * @brief cell_tree_view_activated
 * @param tree_view Not used
 * @param user convert button data
 */
static void cell_tree_view_activated(gpointer tree_view, GtkTreePath *path,
				     GtkTreeViewColumn *column, gpointer user)
{
	(void)tree_view;
	(void)path;
	(void)column;

	on_convert_clicked(NULL, user);
}


/**
 * @brief Callback for cell-selection change event
 *
 * This function activates/deactivates the convert button depending on whether
 * a cell is selected for conversion or not
 * @param sel
 * @param convert_button
 */
static void cell_selection_changed(GtkTreeSelection *sel, GtkWidget *convert_button)
{
	GtkTreeModel *model = NULL;
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected(sel, &model, &iter)) {
		/* Node selected. Show button */
		gtk_widget_set_sensitive(convert_button, TRUE);
	} else {
		gtk_widget_set_sensitive(convert_button, FALSE);
	}
}

static void sort_up_callback(GtkWidget *widget, gpointer user)
{
	(void)widget;
	(void)user;

	layer_selector_force_sort(LAYER_SELECTOR_SORT_UP);
}

static void sort_down_callback(GtkWidget *widget, gpointer user)
{
	(void)widget;
	(void)user;

	layer_selector_force_sort(LAYER_SELECTOR_SORT_DOWN);
}

GtkWindow *create_main_window()
{
	GtkBuilder *main_builder;
	GtkTreeView *cell_tree;
	GtkWidget *listbox;
	GtkWidget *conv_button;
	GtkWidget *search_entry;
	GtkHeaderBar *header_bar;
	static GList *gds_libs;
	static struct open_button_data open_data;
	static struct convert_button_data conv_data;
	struct tree_stores *cell_selector_stores;
	GtkWidget *sort_up_button;
	GtkWidget *sort_down_button;

	main_builder = gtk_builder_new_from_resource("/main.glade");
	gtk_builder_connect_signals(main_builder, NULL);

	cell_tree = GTK_TREE_VIEW(gtk_builder_get_object(main_builder, "cell-tree"));
	search_entry = GTK_WIDGET(gtk_builder_get_object(main_builder, "cell-search"));
	open_data.search_entry = GTK_SEARCH_ENTRY(search_entry);
	cell_selector_stores = setup_cell_selector(cell_tree, GTK_ENTRY(search_entry));

	open_data.cell_store = cell_selector_stores->base_store;
	open_data.list_ptr = &gds_libs;
	open_data.main_window = GTK_WINDOW(gtk_builder_get_object(main_builder, "main-window"));
	g_signal_connect(GTK_WIDGET(gtk_builder_get_object(main_builder, "button-load-gds")),
			 "clicked", G_CALLBACK(on_load_gds), (gpointer)&open_data);

	/* Connect delete-event */
	g_signal_connect(GTK_WIDGET(open_data.main_window), "delete-event",
			 G_CALLBACK(on_window_close), NULL);

	/* Connect Convert button */
	conv_data.tree_view = cell_tree;
	conv_data.main_window = open_data.main_window;

	conv_button = GTK_WIDGET(gtk_builder_get_object(main_builder, "convert-button"));
	g_signal_connect(conv_button, "clicked", G_CALLBACK(on_convert_clicked), &conv_data);

	listbox = GTK_WIDGET(gtk_builder_get_object(main_builder, "layer-list"));
	/* Set up the list box sided callbacks for drag and drop */
	layer_selector_list_box_setup_dnd(GTK_LIST_BOX(listbox));

	open_data.layer_box = GTK_LIST_BOX(listbox);

	/* Set buttons fpr layer mapping GUI */
	setup_load_mapping_callback(GTK_WIDGET(gtk_builder_get_object(main_builder, "button-load-mapping")),
				    open_data.main_window);
	setup_save_mapping_callback(GTK_WIDGET(gtk_builder_get_object(main_builder, "button-save-mapping")),
				    open_data.main_window);

	/* Callback for selection change of cell selector */
	g_signal_connect(G_OBJECT(gtk_tree_view_get_selection(cell_tree)), "changed",
			 G_CALLBACK(cell_selection_changed), conv_button);
	g_signal_connect(cell_tree, "row-activated", G_CALLBACK(cell_tree_view_activated), &conv_data);

	/* Set version in main window subtitle */
	header_bar = GTK_HEADER_BAR(gtk_builder_get_object(main_builder, "header-bar"));
	gtk_header_bar_set_subtitle(header_bar, _app_version_string);

	/* Get layer sorting buttons and set callbacks */
	sort_up_button = GTK_WIDGET(gtk_builder_get_object(main_builder, "button-up-sort"));
	sort_down_button = GTK_WIDGET(gtk_builder_get_object(main_builder, "button-down-sort"));

	g_signal_connect(sort_up_button, "clicked", G_CALLBACK(sort_up_callback), NULL);
	g_signal_connect(sort_down_button, "clicked", G_CALLBACK(sort_down_callback), NULL);

	g_object_unref(main_builder);

	return conv_data.main_window;
}

/** @} */
