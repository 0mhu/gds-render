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

/** @addtogroup GUI
 * @{
 */

#include <stdio.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>

#include <gds-render/gds-render-gui.h>
#include <gds-render/gds-utils/gds-parser.h>
#include <gds-render/gds-utils/gds-tree-checker.h>
#include <gds-render/layer/layer-selector.h>
#include <gds-render/widgets/activity-bar.h>
#include <gds-render/cell-selector/lib-cell-renderer.h>
#include <gds-render/cell-selector/cell-statistics-renderer.h>
#include <gds-render/output-renderers/latex-renderer.h>
#include <gds-render/output-renderers/cairo-renderer.h>
#include <gds-render/widgets/conv-settings-dialog.h>
#include <gds-render/geometric/cell-geometrics.h>
#include <gds-render/version.h>

/** @brief Columns of selection tree view */
enum cell_store_columns {
	CELL_SEL_LIBRARY = 0,
	CELL_SEL_CELL,
	CELL_SEL_CELL_ERROR_STATE, /**< Used for cell color and selectability */
	CELL_SEL_STAT,
	CELL_SEL_COLUMN_COUNT /**< @brief Not a column. Used to determine count of columns */
};

enum gds_render_gui_signal_sig_ids {SIGNAL_WINDOW_CLOSED = 0, SIGNAL_COUNT};

static guint gds_render_gui_signals[SIGNAL_COUNT];

struct gui_button_states {
	gboolean rendering_active;
	gboolean valid_cell_selected;
};

struct _GdsRenderGui {
	/* Parent GObject */
	GObject parent;

	/* Custom fields */
	GtkWindow *main_window;
	GtkWidget *convert_button;
	GtkWidget *open_button;
	GtkWidget *load_layer_button;
	GtkWidget *save_layer_button;
	GtkWidget *select_all_button;
	GtkTreeStore *cell_tree_store;
	GtkTreeModelFilter *cell_filter;
	GtkWidget *cell_search_entry;
	LayerSelector *layer_selector;
	GtkTreeView *cell_tree_view;
	GList *gds_libraries;
	ActivityBar *activity_status_bar;
	struct render_settings render_dialog_settings;
	ColorPalette *palette;
	struct gui_button_states button_state_data;
};

G_DEFINE_TYPE(GdsRenderGui, gds_render_gui, G_TYPE_OBJECT)

/**
 * @brief Main window close event
 * @param window GtkWindow which is closed
 * @param event unused event
 * @param user GdsRenderGui instance
 * @return Status of the event handling. Always true.
 */
static gboolean on_window_close(gpointer window, GdkEvent *event, gpointer user)
{
	GdsRenderGui *self;
	(void)event;

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
 * @brief This function only allows valid cells to be selected
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
	unsigned int error_level;
	gboolean ret = FALSE;
	(void)selection;
	(void)path_currently_selected;
	(void)data;

	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_model_get(model, &iter, CELL_SEL_CELL, &cell, CELL_SEL_CELL_ERROR_STATE, &error_level, -1);

	/* Allow only rows with _valid_ cell to be selected */
	if (cell) {
		/* Cell available. Check if it passed the critical checks */
		if (!(error_level & LIB_CELL_RENDERER_ERROR_ERR))
			ret = TRUE;
	}

	return ret;
}

/**
 * @brief Trigger refiltering of cell filter
 * @param entry Unused widget, that emitted the signal
 * @param data GdsrenderGui self instance
 */
static void cell_tree_view_change_filter(GtkWidget *entry, gpointer data)
{
	GdsRenderGui *self = RENDERER_GUI(data);
	(void)entry;

	gtk_tree_model_filter_refilter(self->cell_filter);
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
	GdsRenderGui *self;
	struct gds_cell *cell;
	struct gds_library *lib;
	gboolean result = FALSE;
	const char *search_string;

	self = RENDERER_GUI(data);
	g_return_val_if_fail(RENDERER_IS_GUI(self), FALSE);

	if (!model || !iter)
		goto exit_filter;

	gtk_tree_model_get(model, iter, CELL_SEL_CELL, &cell, CELL_SEL_LIBRARY, &lib, -1);

    /* Show always, if this is a pure lib entry */
    if (lib && !cell) {
		result = TRUE;
		goto exit_filter;
	}

	if (!cell)
		goto exit_filter;

	search_string = gtk_entry_get_text(GTK_ENTRY(self->cell_search_entry));

	/* Show all, if field is empty */
	if (!strlen(search_string))
		result = TRUE;

	if (strstr(cell->name, search_string))
		result = TRUE;

	gtk_tree_view_expand_all(self->cell_tree_view);

exit_filter:
	return result;
}

/**
 * @brief Setup a GtkTreeView with the necessary columns
 * @param self Current GUI object
 */
int gds_render_gui_setup_cell_selector(GdsRenderGui *self)
{
	GtkCellRenderer *render_cell;
	GtkCellRenderer *render_lib;
	GtkCellRenderer *render_vertex_count;
	GtkTreeViewColumn *column;

	self->cell_tree_store = gtk_tree_store_new(CELL_SEL_COLUMN_COUNT, G_TYPE_POINTER,
					 G_TYPE_POINTER, G_TYPE_UINT, G_TYPE_POINTER);

	/* Searching */
	self->cell_filter = GTK_TREE_MODEL_FILTER(
				gtk_tree_model_filter_new(GTK_TREE_MODEL(self->cell_tree_store), NULL));

	gtk_tree_model_filter_set_visible_func(self->cell_filter,
						(GtkTreeModelFilterVisibleFunc)cell_store_filter_visible_func,
						 self, NULL);
	g_signal_connect(GTK_SEARCH_ENTRY(self->cell_search_entry), "search-changed",
			 G_CALLBACK(cell_tree_view_change_filter), self);

	gtk_tree_view_set_model(self->cell_tree_view, GTK_TREE_MODEL(self->cell_filter));

	render_cell = lib_cell_renderer_new();
	render_lib = lib_cell_renderer_new();
	render_vertex_count = cell_statistics_renderer_new();

	column = gtk_tree_view_column_new_with_attributes(_("Library"), render_lib, "gds-lib", CELL_SEL_LIBRARY, NULL);
	gtk_tree_view_append_column(self->cell_tree_view, column);

	column = gtk_tree_view_column_new_with_attributes(_("Cell"), render_cell, "gds-cell", CELL_SEL_CELL,
							  "error-level", CELL_SEL_CELL_ERROR_STATE, NULL);
	gtk_tree_view_append_column(self->cell_tree_view, column);

	column = gtk_tree_view_column_new_with_attributes(_("Vertex | GFX Count"), render_vertex_count, "cell-stat", CELL_SEL_STAT,
							  NULL);
	gtk_tree_view_append_column(self->cell_tree_view, column);

	/* Callback for selection
	 * This prevents selecting a library
	 */
	gtk_tree_selection_set_select_function(gtk_tree_view_get_selection(self->cell_tree_view),
					       tree_sel_func, NULL, NULL);

	return 0;
}

const struct gds_cell_statistics cc =  {
	.vertex_count = 12,
};

/**
 * @brief Callback function of Load GDS button
 * @param button
 * @param user GdsRenderGui instance
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
	unsigned int cell_error_level;

	const struct gds_library_parsing_opts gds_parsing_options = {
		.simplified_polygons = 1,
	};

	self = RENDERER_GUI(user);
	if (!self)
		return;

	open_dialog = gtk_file_chooser_dialog_new(_("Open GDSII File"), self->main_window,
						  GTK_FILE_CHOOSER_ACTION_OPEN,
						  _("Cancel"), GTK_RESPONSE_CANCEL,
						  _("Open GDSII"), GTK_RESPONSE_ACCEPT,
						  NULL);
	file_chooser = GTK_FILE_CHOOSER(open_dialog);

	/* Add GDS II Filter */
	filter = gtk_file_filter_new();
	gtk_file_filter_add_pattern(filter, "*.gds");
	gtk_file_filter_set_name(filter, _("GDSII-Files"));
	gtk_file_chooser_add_filter(file_chooser, filter);

	dialog_result = gtk_dialog_run(GTK_DIALOG(open_dialog));

	if (dialog_result != GTK_RESPONSE_ACCEPT)
		goto end_destroy;

	/* Get File name */
	filename = gtk_file_chooser_get_filename(file_chooser);

	gtk_tree_store_clear(self->cell_tree_store);
	clear_lib_list(&self->gds_libraries);

	/* Parse new GDSII file */
	gds_result = parse_gds_from_file(filename, &self->gds_libraries, &gds_parsing_options);

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

		gtk_tree_store_set(self->cell_tree_store, &libiter,
				   CELL_SEL_LIBRARY, gds_lib,
				   -1);

		/* Check this library. This might take a while */
		(void)gds_tree_check_cell_references(gds_lib);
		(void)gds_tree_check_reference_loops(gds_lib);

		for (cell = gds_lib->cells; cell != NULL; cell = cell->next) {
			gds_c = (struct gds_cell *)cell->data;
			gtk_tree_store_append(self->cell_tree_store, &celliter, &libiter);

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
					   CELL_SEL_CELL_ERROR_STATE, cell_error_level,
					   CELL_SEL_LIBRARY, gds_c->parent_library,
					   CELL_SEL_STAT, &gds_c->stats,
					   -1);
		} /* for cells */
	} /* for libraries */

	/* Create Layers in Layer Box */
	layer_selector_generate_layer_widgets(self->layer_selector, self->gds_libraries);

end_destroy:
	/* Destroy dialog and filter */
	gtk_widget_destroy(open_dialog);
}

static void process_button_state_changes(GdsRenderGui *self)
{
	gboolean convert_button_state = FALSE;
	gboolean open_gds_button_state = FALSE;

	/* Calculate states */
	if (!self->button_state_data.rendering_active) {
		open_gds_button_state = TRUE;
		if (self->button_state_data.valid_cell_selected)
			convert_button_state = TRUE;
	}

	/* Apply states */
	gtk_widget_set_sensitive(self->convert_button, convert_button_state);
	gtk_widget_set_sensitive(self->open_button, open_gds_button_state);
}

/**
 * @brief Callback for auto coloring button
 * @param button
 * @param user
 */
static void on_auto_color_clicked(gpointer button, gpointer user)
{
	GdsRenderGui *self;
	(void)button;

	self = RENDERER_GUI(user);
	layer_selector_auto_color_layers(self->layer_selector, self->palette, 1.0);
}

static void async_rendering_finished_callback(GdsOutputRenderer *renderer, gpointer gui)
{
	GdsRenderGui *self;

	self = RENDERER_GUI(gui);

	self->button_state_data.rendering_active = FALSE;
	process_button_state_changes(self);
	activity_bar_set_ready(self->activity_status_bar);

	g_object_unref(renderer);
}

static void async_rendering_status_update_callback(GdsOutputRenderer *renderer,
						   const char *status_message,
						   gpointer data)
{
	GdsRenderGui *gui;
	(void)renderer;

	gui = RENDERER_GUI(data);

	activity_bar_set_busy(gui->activity_status_bar, status_message);
}

/**
 * @brief Convert button callback
 * @param button
 * @param user
 */
static void on_convert_clicked(gpointer button, gpointer user)
{
	(void)button;
	GdsRenderGui *self;
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	GtkTreeModel *model;
	struct gds_cell *cell_to_render;
	GtkWidget *dialog;
	RendererSettingsDialog *settings;
	GtkFileFilter *filter;
	gint res;
	char *file_name;
	union bounding_box cell_box;
	unsigned int height, width;
	struct render_settings *sett;
	LayerSettings *layer_settings;
	GdsOutputRenderer *render_engine;

	self = RENDERER_GUI(user);

	if (!self)
		return;

	/* Abort if rendering is already active */
	if (self->button_state_data.rendering_active == TRUE)
		return;

	sett = &self->render_dialog_settings;

	/* Get selected cell */
	selection = gtk_tree_view_get_selection(self->cell_tree_view);
	if (gtk_tree_selection_get_selected(selection, &model, &iter) == FALSE)
		return;

	gtk_tree_model_get(model, &iter, CELL_SEL_CELL, &cell_to_render, -1);

	if (!cell_to_render)
		return;

	/* Get layers that are rendered */
	layer_settings = layer_selector_export_rendered_layer_info(self->layer_selector);

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
	renderer_settings_dialog_set_settings(settings, sett);
	renderer_settings_dialog_set_database_unit_scale(settings, cell_to_render->parent_library->unit_in_meters);
	renderer_settings_dialog_set_cell_height(settings, height);
	renderer_settings_dialog_set_cell_width(settings, width);
	g_object_set(G_OBJECT(settings), "cell-name", cell_to_render->name, NULL);

	res = gtk_dialog_run(GTK_DIALOG(settings));
	if (res == GTK_RESPONSE_OK) {
		renderer_settings_dialog_get_settings(settings, sett);
		gtk_widget_destroy(GTK_WIDGET(settings));
	} else {
		gtk_widget_destroy(GTK_WIDGET(settings));
		goto ret_layer_destroy;
	}

	/* save file dialog */
	dialog = gtk_file_chooser_dialog_new((sett->renderer == RENDERER_LATEX_TIKZ
					      ? "Save LaTeX File" : "Save PDF"),
					     GTK_WINDOW(self->main_window), GTK_FILE_CHOOSER_ACTION_SAVE,
					     "Cancel", GTK_RESPONSE_CANCEL, "Save", GTK_RESPONSE_ACCEPT, NULL);
	/* Set file filter according to settings */
	filter = gtk_file_filter_new();
	switch (sett->renderer) {
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

		switch (sett->renderer) {
		case RENDERER_LATEX_TIKZ:
			render_engine =
				GDS_RENDER_OUTPUT_RENDERER(latex_renderer_new_with_options(sett->tex_pdf_layers,
											   sett->tex_standalone));
			break;
		case RENDERER_CAIROGRAPHICS_SVG:
			render_engine = GDS_RENDER_OUTPUT_RENDERER(cairo_renderer_new_svg());
			break;
		case RENDERER_CAIROGRAPHICS_PDF:
			render_engine = GDS_RENDER_OUTPUT_RENDERER(cairo_renderer_new_pdf());
			break;
		default:
			/* Abort rendering */
			render_engine = NULL;
			break;
		}

		if (render_engine) {
			gds_output_renderer_set_output_file(render_engine, file_name);
			gds_output_renderer_set_layer_settings(render_engine, layer_settings);
			/* Prevent user from overwriting library or triggering additional conversion */
			self->button_state_data.rendering_active = TRUE;
			process_button_state_changes(self);

			g_signal_connect(render_engine, "async-finished", G_CALLBACK(async_rendering_finished_callback),
					 self);

			activity_bar_set_busy(self->activity_status_bar, _("Rendering cell..."));

			g_signal_connect(render_engine, "progress-changed",
					 G_CALLBACK(async_rendering_status_update_callback), self);
			gds_output_renderer_render_output_async(render_engine, cell_to_render, sett->scale);
		}
		g_free(file_name);
	} else {
		gtk_widget_destroy(dialog);
	}
ret_layer_destroy:
	g_object_unref(layer_settings);
}

/**
 * @brief cell_tree_view_activated Callback for 'double click' on cell selector element
 * @param tree_view The tree view the event occured in
 * @param path path to the selected row
 * @param column The clicked column
 * @param user pointer to GdsRenderGui object
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
		self->button_state_data.valid_cell_selected = TRUE;
	} else {
		self->button_state_data.valid_cell_selected = FALSE;
	}

	process_button_state_changes(self);
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
	g_clear_object(&self->cell_filter);
	g_clear_object(&self->cell_search_entry);
	g_clear_object(&self->activity_status_bar);
	g_clear_object(&self->palette);
	g_clear_object(&self->load_layer_button);
	g_clear_object(&self->save_layer_button);
	g_clear_object(&self->open_button);
	g_clear_object(&self->select_all_button);

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

/**
 * @brief Callback for the 'select all layers'-button
 * @param button Button that triggered the event
 * @param user_data the GdsrenderGui object containing the main-window the button is placed in
 */
static void on_select_all_layers_clicked(GtkWidget *button, gpointer user_data)
{
	GdsRenderGui *gui;
	(void)button;

	gui = RENDERER_GUI(user_data);
	layer_selector_select_all_layers(gui->layer_selector, TRUE);
}

static gboolean auto_naming_ask_for_override(GdsRenderGui *gui)
{
	GtkDialog *dialog;
	gint dialog_result;
	gboolean overwrite = FALSE;

	g_return_val_if_fail(RENDERER_IS_GUI(gui), FALSE);

	/* Ask for overwrite */
	dialog = GTK_DIALOG(gtk_message_dialog_new(gui->main_window, GTK_DIALOG_USE_HEADER_BAR, GTK_MESSAGE_QUESTION,
						   GTK_BUTTONS_YES_NO, "Overwrite existing layer names?"));
	dialog_result = gtk_dialog_run(dialog);
	switch (dialog_result) {
	case GTK_RESPONSE_YES:
		overwrite = TRUE;
		break;
	case GTK_RESPONSE_NO: /* Expected fallthrough */
	default:
		overwrite = FALSE;
		break;
	}
	gtk_widget_destroy(GTK_WIDGET(dialog));

	return overwrite;
}

static void auto_naming_clicked(GtkWidget *button, gpointer user_data)
{
	GdsRenderGui *gui;
	gboolean overwrite = FALSE;
	(void)button;

	gui = RENDERER_GUI(user_data);

	/* Don't do anything if the selector is empty. */
	if (!layer_selector_contains_elements(gui->layer_selector))
		return;

	/* Ask, if names shall be overwritten, if they are not empty */
	if (layer_selector_num_of_named_elements(gui->layer_selector) > 0)
		overwrite = auto_naming_ask_for_override(gui);

	layer_selector_auto_name_layers(gui->layer_selector, overwrite);
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
	GtkWidget *sort_up_button;
	GtkWidget *sort_down_button;
	GtkWidget *activity_bar_box;
	GtkWidget *auto_color_button;
	GtkWidget *auto_naming_button;

	main_builder = gtk_builder_new_from_resource("/gui/main.glade");

	self->cell_tree_view = GTK_TREE_VIEW(gtk_builder_get_object(main_builder, "cell-tree"));
	self->cell_search_entry = GTK_WIDGET(gtk_builder_get_object(main_builder, "cell-search"));

	gds_render_gui_setup_cell_selector(self);

	self->main_window = GTK_WINDOW(gtk_builder_get_object(main_builder, "main-window"));
	self->open_button = GTK_WIDGET(gtk_builder_get_object(main_builder, "button-load-gds"));
	g_signal_connect(self->open_button,
			 "clicked", G_CALLBACK(on_load_gds), (gpointer)self);

	self->convert_button = GTK_WIDGET(gtk_builder_get_object(main_builder, "convert-button"));
	g_signal_connect(self->convert_button, "clicked", G_CALLBACK(on_convert_clicked), (gpointer)self);

	listbox = GTK_WIDGET(gtk_builder_get_object(main_builder, "layer-list"));
	/* Create layer selector */
	self->layer_selector = layer_selector_new(GTK_LIST_BOX(listbox));

	activity_bar_box = GTK_WIDGET(gtk_builder_get_object(main_builder, "activity-bar"));

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
	self->load_layer_button = GTK_WIDGET(gtk_builder_get_object(main_builder, "button-load-mapping"));
	self->save_layer_button = GTK_WIDGET(gtk_builder_get_object(main_builder, "button-save-mapping"));
	layer_selector_set_load_mapping_button(self->layer_selector, self->load_layer_button, self->main_window);
	layer_selector_set_save_mapping_button(self->layer_selector, self->save_layer_button, self->main_window);

	/* Connect delete-event */
	g_signal_connect(GTK_WIDGET(self->main_window), "delete-event",
			 G_CALLBACK(on_window_close), self);

	/* Create and apply ActivityBar */
	self->activity_status_bar = activity_bar_new();
	gtk_container_add(GTK_CONTAINER(activity_bar_box), GTK_WIDGET(self->activity_status_bar));
	gtk_widget_show(GTK_WIDGET(self->activity_status_bar));

	/* Create color palette */
	self->palette = color_palette_new_from_resource("/data/color-palette.txt");
	auto_color_button = GTK_WIDGET(gtk_builder_get_object(main_builder, "auto-color-button"));
	g_signal_connect(auto_color_button, "clicked", G_CALLBACK(on_auto_color_clicked), self);


	/* Set default conversion/rendering settings */
	self->render_dialog_settings.scale = 1000;
	self->render_dialog_settings.renderer = RENDERER_LATEX_TIKZ;
	self->render_dialog_settings.tex_pdf_layers = FALSE;
	self->render_dialog_settings.tex_standalone = FALSE;

	/* Get select all button and connect callback */
	self->select_all_button = GTK_WIDGET(gtk_builder_get_object(main_builder, "button-select-all"));
	g_signal_connect(self->select_all_button, "clicked", G_CALLBACK(on_select_all_layers_clicked), self);

	/* Setup auto naming button */
	auto_naming_button = GTK_WIDGET(gtk_builder_get_object(main_builder, "button-auto-name"));
	g_signal_connect(auto_naming_button, "clicked", G_CALLBACK(auto_naming_clicked), self);

	g_object_unref(main_builder);

	/* Setup default button sensibility data */
	self->button_state_data.rendering_active = FALSE;
	self->button_state_data.valid_cell_selected = FALSE;

	/* Reference all objects referenced by this object */
	g_object_ref(self->activity_status_bar);
	g_object_ref(self->main_window);
	g_object_ref(self->cell_tree_view);
	g_object_ref(self->convert_button);
	/* g_object_ref(self->layer_selector); <= This is already referenced by the _new() function */
	g_object_ref(self->cell_search_entry);
	/* g_object_ref(self->palette); */
	g_object_ref(self->open_button);
	g_object_ref(self->load_layer_button);
	g_object_ref(self->save_layer_button);
	g_object_ref(self->select_all_button);
}

GdsRenderGui *gds_render_gui_new()
{
	return RENDERER_GUI(g_object_new(RENDERER_TYPE_GUI, NULL));
}

/** @} */
