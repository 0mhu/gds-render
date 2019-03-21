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
 * @file gds-render-gui.c
 * @brief Handling of GUI
 * @author Mario Hüttel <mario.huettel@gmx.net>
 */

/** @addtogroup MainApplication
 * @{
 */

#include "gds-render-gui.h"
#include <stdio.h>
#include "gds-utils/gds-parser.h"
#include <gtk/gtk.h>
#include "layer/layer-selector.h"
#include "tree-renderer/tree-store.h"
#include "latex-output/latex-output.h"
#include "widgets/conv-settings-dialog.h"
#include "cairo-output/cairo-output.h"
#include "trigonometric/cell-trigonometrics.h"
#include "version/version.h"
#include "tree-renderer/lib-cell-renderer.h"
#include "gds-utils/gds-tree-checker.h"

enum gds_render_gui_signal_sig_ids {SIGNAL_WINDOW_CLOSED = 0, SIGNAL_COUNT};

static guint gds_render_gui_signals[SIGNAL_COUNT];

struct _GdsRenderGui {
	/* Parent GObject */
	GObject parent;

	/* Custom fields */
	GtkWindow *main_window;
	GtkWidget *convert_button;
	GtkTreeStore *cell_tree_store;
	GtkWidget *cell_search_entry;
	LayerSelector *layer_selector;
	GtkTreeView *cell_tree_view;
	GList *gds_libraries;
};

G_DEFINE_TYPE(GdsRenderGui, gds_render_gui, G_TYPE_OBJECT)

/**
 * @brief Window close event of main window
 *
 * Closes the main window. This leads to the termination of the whole application
 * @param window main window
 * @param user not used
 * @return TRUE. This indicates that the event has been fully handled
 */
static gboolean on_window_close(gpointer window, GdkEvent *event, gpointer user)
{
	GdsRenderGui *self;

	self = RENDERER_GUI(user);
	/* Don't close window in case of error */
	if (!self)
		return TRUE;

	/* Close Window. Leads to termination of the program/the current instance */
	g_clear_object(&self->main_window);
	gtk_widget_destroy(GTK_WIDGET(window));

	/* Delete loaded library data */
	clear_lib_list(&self->gds_libraries);

	g_signal_emit(self, gds_render_gui_signals[SIGNAL_WINDOW_CLOSED], 0);

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
	GdsRenderGui *self;
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

	self = RENDERER_GUI(user);
	if (!self)
		return;

	open_dialog = gtk_file_chooser_dialog_new("Open GDSII File", self->main_window,
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

		gtk_tree_store_clear(self->cell_tree_store);
		clear_lib_list(&self->gds_libraries);

		/* Parse new GDSII file */
		gds_result = parse_gds_from_file(filename, &self->gds_libraries);

		/* Delete file name afterwards */
		g_free(filename);
		if (gds_result)
			goto end_destroy;

		/* remove suggested action from Open button */
		button_style = gtk_widget_get_style_context(GTK_WIDGET(button));
		gtk_style_context_remove_class(button_style, "suggested-action");

		for (lib = self->gds_libraries; lib != NULL; lib = lib->next) {
			gds_lib = (struct gds_library *)lib->data;
			/* Create top level iter */
			gtk_tree_store_append(self->cell_tree_store, &libiter, NULL);

			/* Convert dates to String */
			mod_date = generate_string_from_date(&gds_lib->mod_time);
			acc_date = generate_string_from_date(&gds_lib->access_time);

			gtk_tree_store_set(self->cell_tree_store, &libiter,
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
				gtk_tree_store_append(self->cell_tree_store, &celliter, &libiter);

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
				gtk_tree_store_set(self->cell_tree_store, &celliter,
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
		layer_selector_generate_layer_widgets(self->layer_selector, self->gds_libraries);
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
	GdsRenderGui *self;
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

	self = RENDERER_GUI(user);

	if (!self)
		return;

	/* Get selected cell */
	selection = gtk_tree_view_get_selection(self->cell_tree_view);
	if (gtk_tree_selection_get_selected(selection, &model, &iter) == FALSE)
		return;

	gtk_tree_model_get(model, &iter, CELL_SEL_CELL, &cell_to_render, -1);

	if (!cell_to_render)
		return;

	/* Get layers that are rendered */
	layer_list = layer_selector_export_rendered_layer_info(self->layer_selector);

	/* Calculate cell size in DB units */
	bounding_box_prepare_empty(&cell_box);
	calculate_cell_bounding_box(&cell_box, cell_to_render);

	/* Calculate size in database units
	 * Note that the results are bound to be positive,
	 * so casting them to unsigned int is absolutely valid
	 */
	height = (unsigned int)(cell_box.vectors.upper_right.y - cell_box.vectors.lower_left.y);
	width = (unsigned int)(cell_box.vectors.upper_right.x - cell_box.vectors.lower_left.x);

	/* Show settings dialog */
	settings = renderer_settings_dialog_new(GTK_WINDOW(self->main_window));
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
					     GTK_WINDOW(self->main_window), GTK_FILE_CHOOSER_ACTION_SAVE,
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
	g_list_free_full(layer_list, (GDestroyNotify)layer_info_delete_struct);
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
 * @param self
 */
static void cell_selection_changed(GtkTreeSelection *sel, GdsRenderGui *self)
{
	GtkTreeModel *model = NULL;
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected(sel, &model, &iter)) {
		/* Node selected. Show button */
		gtk_widget_set_sensitive(self->convert_button, TRUE);
	} else {
		gtk_widget_set_sensitive(self->convert_button, FALSE);
	}
}

static void sort_up_callback(GtkWidget *widget, gpointer user)
{
	(void)widget;
	GdsRenderGui *self;

	self = RENDERER_GUI(user);
	if (!self)
		return;
	layer_selector_force_sort(self->layer_selector, LAYER_SELECTOR_SORT_UP);
}

static void sort_down_callback(GtkWidget *widget, gpointer user)
{
	(void)widget;
	GdsRenderGui *self;

	self = RENDERER_GUI(user);
	if (!self)
		return;
	layer_selector_force_sort(self->layer_selector, LAYER_SELECTOR_SORT_DOWN);
}

static void gds_render_gui_dispose(GObject *gobject)
{
	GdsRenderGui *self;

	self = RENDERER_GUI(gobject);

	clear_lib_list(&self->gds_libraries);

	g_clear_object(&self->cell_tree_view);
	g_clear_object(&self->convert_button);
	g_clear_object(&self->layer_selector);
	g_clear_object(&self->cell_tree_store);
	g_clear_object(&self->cell_search_entry);

	if (self->main_window) {
		g_signal_handlers_destroy(self->main_window);
		gtk_widget_destroy(GTK_WIDGET(self->main_window));
		self->main_window = NULL;
	}

	/* Chain up */
	G_OBJECT_CLASS(gds_render_gui_parent_class)->dispose(gobject);
}

static void gds_render_gui_class_init(GdsRenderGuiClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

	gds_render_gui_signals[SIGNAL_WINDOW_CLOSED] =
			g_signal_newv("window-closed", RENDERER_TYPE_GUI,
				      G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE,
				      NULL,
				      NULL,
				      NULL,
				      NULL,
				      G_TYPE_NONE,
				      0,
				      NULL);

	gobject_class->dispose = gds_render_gui_dispose;
}

GtkWindow *gds_render_gui_get_main_window(GdsRenderGui *gui)
{
	return gui->main_window;
}

static void gds_render_gui_init(GdsRenderGui *self)
{
	GtkBuilder *main_builder;
	GtkWidget *listbox;
	GtkHeaderBar *header_bar;
	struct tree_stores *cell_selector_stores;
	GtkWidget *sort_up_button;
	GtkWidget *sort_down_button;

	main_builder = gtk_builder_new_from_resource("/main.glade");
	gtk_builder_connect_signals(main_builder, NULL);

	self->cell_tree_view = GTK_TREE_VIEW(gtk_builder_get_object(main_builder, "cell-tree"));
	self->cell_search_entry = GTK_WIDGET(gtk_builder_get_object(main_builder, "cell-search"));

	cell_selector_stores = setup_cell_selector(self->cell_tree_view, GTK_ENTRY(self->cell_search_entry));

	self->cell_tree_store = cell_selector_stores->base_store;

	self->main_window = GTK_WINDOW(gtk_builder_get_object(main_builder, "main-window"));
	g_signal_connect(GTK_WIDGET(gtk_builder_get_object(main_builder, "button-load-gds")),
			 "clicked", G_CALLBACK(on_load_gds), (gpointer)self);

	self->convert_button = GTK_WIDGET(gtk_builder_get_object(main_builder, "convert-button"));
	g_signal_connect(self->convert_button, "clicked", G_CALLBACK(on_convert_clicked), (gpointer)self);

	listbox = GTK_WIDGET(gtk_builder_get_object(main_builder, "layer-list"));
	/* Create layer selector */
	self->layer_selector = layer_selector_new(GTK_LIST_BOX(listbox));


	/* Callback for selection change of cell selector */
	g_signal_connect(G_OBJECT(gtk_tree_view_get_selection(self->cell_tree_view)), "changed",
			 G_CALLBACK(cell_selection_changed), self);
	g_signal_connect(self->cell_tree_view, "row-activated", G_CALLBACK(cell_tree_view_activated), self);

	/* Set version in main window subtitle */
	header_bar = GTK_HEADER_BAR(gtk_builder_get_object(main_builder, "header-bar"));
	gtk_header_bar_set_subtitle(header_bar, _app_version_string);

	/* Get layer sorting buttons and set callbacks */
	sort_up_button = GTK_WIDGET(gtk_builder_get_object(main_builder, "button-up-sort"));
	sort_down_button = GTK_WIDGET(gtk_builder_get_object(main_builder, "button-down-sort"));

	g_signal_connect(sort_up_button, "clicked", G_CALLBACK(sort_up_callback), self);
	g_signal_connect(sort_down_button, "clicked", G_CALLBACK(sort_down_callback), self);

	/* Set buttons for loading and saving */
	layer_selector_set_load_mapping_button(self->layer_selector,
						GTK_WIDGET(gtk_builder_get_object(main_builder, "button-load-mapping")),
						self->main_window);
	layer_selector_set_save_mapping_button(self->layer_selector, GTK_WIDGET(gtk_builder_get_object(main_builder, "button-save-mapping")),
						self->main_window);

	/* Connect delete-event */
	g_signal_connect(GTK_WIDGET(self->main_window), "delete-event",
			 G_CALLBACK(on_window_close), self);

	g_object_unref(main_builder);

	/* Reference all objects referenced by this object */
	g_object_ref(self->main_window);
	g_object_ref(self->cell_tree_view);
	g_object_ref(self->convert_button);
	g_object_ref(self->layer_selector);
	g_object_ref(self->cell_tree_store);
	g_object_ref(self->cell_search_entry);
}

GdsRenderGui *gds_render_gui_new()
{
	return RENDERER_GUI(g_object_new(RENDERER_TYPE_GUI, NULL));
}

/** @} */
