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
 * @file layer-selector.c
 * @brief Implementation of the layer selector
 * @author Mario Hüttel <mario.huettel@gmx.net>
 */

/**
 * @addtogroup layer-selector
 * @{
 */

#include <glib.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <gds-render/layer/layer-selector.h>
#include <gds-render/gds-utils/gds-parser.h>
#include <gds-render/widgets/layer-element.h>

struct _LayerSelector {
	/* Parent */
	GObject parent;
	/* Own fields */
	GtkWidget *associated_load_button;
	GtkWidget *associated_save_button;
	GtkWindow *load_parent_window;
	GtkWindow *save_parent_window;
	GtkListBox *list_box;

	GtkTargetEntry dnd_target;

	gpointer dummy[4];
};

G_DEFINE_TYPE(LayerSelector, layer_selector, G_TYPE_OBJECT)

/*
 * Drag and drop code
 * Original code from https://blog.gtk.org/2017/06/01/drag-and-drop-in-lists-revisited/
 */

static void sel_layer_element_drag_begin(GtkWidget *widget, GdkDragContext *context, gpointer data)
{
	GtkWidget *row;
	GtkAllocation alloc;
	cairo_surface_t *surface;
	cairo_t *cr;
	int x, y;
	(void)data;

	row = gtk_widget_get_ancestor(widget, GTK_TYPE_LIST_BOX_ROW);
	gtk_widget_get_allocation(row, &alloc);
	surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, alloc.width, alloc.height);
	cr = cairo_create(surface);

	gtk_style_context_add_class(gtk_widget_get_style_context(row), "drag-icon");
	gtk_widget_draw(row, cr);
	gtk_style_context_remove_class(gtk_widget_get_style_context(row), "drag-icon");

	gtk_widget_translate_coordinates(widget, row, 0, 0, &x, &y);
	cairo_surface_set_device_offset(surface, -x, -y);
	gtk_drag_set_icon_surface(context, surface);

	cairo_destroy(cr);
	cairo_surface_destroy(surface);

	g_object_set_data(G_OBJECT(gtk_widget_get_parent(row)), "drag-row", row);
	gtk_style_context_add_class(gtk_widget_get_style_context(row), "drag-row");
}

static void sel_layer_element_drag_end(GtkWidget *widget, GdkDragContext *context, gpointer data)
{
	GtkWidget *row;
	(void)context;
	(void)data;

	row = gtk_widget_get_ancestor(widget, GTK_TYPE_LIST_BOX_ROW);
	g_object_set_data(G_OBJECT(gtk_widget_get_parent(row)), "drag-row", NULL);
	gtk_style_context_remove_class(gtk_widget_get_style_context(row), "drag-row");
	gtk_style_context_remove_class(gtk_widget_get_style_context(row), "drag-hover");
}

static void sel_layer_element_drag_data_get(GtkWidget *widget, GdkDragContext *context,
					    GtkSelectionData *selection_data,
					    guint info, guint time, gpointer data)
{
	(void)context;
	(void)info;
	(void)time;
	(void)data;
	GdkAtom atom;

	atom = gdk_atom_intern_static_string("GTK_LIST_BOX_ROW");

	gtk_selection_data_set(selection_data, atom,
			       32, (const guchar *)&widget, sizeof(gpointer));
}

static GtkListBoxRow *layer_selector_get_last_row(GtkListBox *list)
{
	int i;
	GtkListBoxRow *row;
	GtkListBoxRow *tmp;

	row = NULL;
	for (i = 0; ; i++) {
		tmp = gtk_list_box_get_row_at_index(list, i);
		if (tmp == NULL)
			break;
		row = tmp;
	}

	return row;
}

static GtkListBoxRow *layer_selector_get_row_before(GtkListBox *list, GtkListBoxRow *row)
{
	int pos;

	pos = gtk_list_box_row_get_index(row);
	return gtk_list_box_get_row_at_index(list, pos - 1);
}

static GtkListBoxRow *layer_selector_get_row_after(GtkListBox *list, GtkListBoxRow *row)
{
	int pos;

	pos = gtk_list_box_row_get_index(row);
	return gtk_list_box_get_row_at_index(list, pos + 1);
}

static void layer_selector_drag_data_received(GtkWidget *widget, GdkDragContext *context, gint x, gint y,
					     GtkSelectionData *selection_data, guint info, guint32 time,
					     gpointer data)
{
	GtkWidget *row_before, *row_after;
	GtkWidget *row;
	GtkWidget *source;
	int pos;

	/* Handle unused parameters */
	(void)context;
	(void)x;
	(void)y;
	(void)info;
	(void)time;
	(void)data;

	row_before = GTK_WIDGET(g_object_get_data(G_OBJECT(widget), "row-before"));
	row_after = GTK_WIDGET(g_object_get_data(G_OBJECT(widget), "row-after"));

	g_object_set_data(G_OBJECT(widget), "row-before", NULL);
	g_object_set_data(G_OBJECT(widget), "row-after", NULL);

	if (row_before)
		gtk_style_context_remove_class(gtk_widget_get_style_context(row_before), "drag-hover-bottom");
	if (row_after)
		gtk_style_context_remove_class(gtk_widget_get_style_context(row_after), "drag-hover-top");

	row = (gpointer) *((gpointer *)gtk_selection_data_get_data(selection_data));
	source = gtk_widget_get_ancestor(row, GTK_TYPE_LIST_BOX_ROW);

	if (source == row_after)
		return;

	g_object_ref(source);
	gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(source)), source);

	if (row_after)
		pos = gtk_list_box_row_get_index(GTK_LIST_BOX_ROW(row_after));
	else
		pos = gtk_list_box_row_get_index(GTK_LIST_BOX_ROW(row_before)) + 1;

	gtk_list_box_insert(GTK_LIST_BOX(widget), source, pos);
	g_object_unref(source);
}

static gboolean layer_selector_drag_motion(GtkWidget *widget, GdkDragContext *context, int x, int y, guint time)
{
	GtkAllocation alloc;
	GtkWidget *row;
	int hover_row_y;
	int hover_row_height;
	GtkWidget *drag_row;
	GtkWidget *row_before;
	GtkWidget *row_after;
	(void)context;
	(void)x;
	(void)y;
	(void)time;

	row = GTK_WIDGET(gtk_list_box_get_row_at_y(GTK_LIST_BOX(widget), y));

	drag_row = GTK_WIDGET(g_object_get_data(G_OBJECT(widget), "drag-row"));
	row_after = GTK_WIDGET(g_object_get_data(G_OBJECT(widget), "row-after"));
	row_before = GTK_WIDGET(g_object_get_data(G_OBJECT(widget), "row-before"));

	gtk_style_context_remove_class(gtk_widget_get_style_context(drag_row), "drag-hover");
	if (row_before)
		gtk_style_context_remove_class(gtk_widget_get_style_context(row_before), "drag-hover-bottom");
	if (row_after)
		gtk_style_context_remove_class(gtk_widget_get_style_context(row_after), "drag-hover-top");

	if (row) {
		gtk_widget_get_allocation(row, &alloc);
		hover_row_y = alloc.y;
		hover_row_height = alloc.height;

		if (y < hover_row_y + hover_row_height/2) {
			row_after = row;
			row_before = GTK_WIDGET(layer_selector_get_row_before(GTK_LIST_BOX(widget),
									      GTK_LIST_BOX_ROW(row)));
		} else {
			row_before = row;
			row_after = GTK_WIDGET(layer_selector_get_row_after(GTK_LIST_BOX(widget),
									    GTK_LIST_BOX_ROW(row)));
		}
	} else {
		row_before = GTK_WIDGET(layer_selector_get_last_row(GTK_LIST_BOX(widget)));
		row_after = NULL;
	}

	g_object_set_data(G_OBJECT(widget), "row-before", row_before);
	g_object_set_data(G_OBJECT(widget), "row-after", row_after);

	if (drag_row == row_before || drag_row == row_after) {
		gtk_style_context_add_class(gtk_widget_get_style_context(drag_row), "drag-hover");
		return FALSE;
	}

	if (row_before)
		gtk_style_context_add_class(gtk_widget_get_style_context(row_before), "drag-hover-bottom");
	if (row_after)
		gtk_style_context_add_class(gtk_widget_get_style_context(row_after), "drag-hover-top");

	return TRUE;
}

static void layer_selector_drag_leave(GtkWidget *widget, GdkDragContext *context, guint time)
{
	GtkWidget *drag_row;
	GtkWidget *row_before;
	GtkWidget *row_after;
	(void)context;
	(void)time;

	drag_row = GTK_WIDGET(g_object_get_data(G_OBJECT(widget), "drag-row"));
	row_before = GTK_WIDGET(g_object_get_data(G_OBJECT(widget), "row-before"));
	row_after = GTK_WIDGET(g_object_get_data(G_OBJECT(widget), "row-after"));

	gtk_style_context_remove_class(gtk_widget_get_style_context(drag_row), "drag-hover");
	if (row_before)
		gtk_style_context_remove_class(gtk_widget_get_style_context(row_before), "drag-hover-bottom");
	if (row_after)
		gtk_style_context_remove_class(gtk_widget_get_style_context(row_after), "drag-hover-top");

}

static const char *dnd_additional_css =
	".row:not(:first-child) { "
	"  border-top: 1px solid alpha(gray,0.5); "
	"  border-bottom: 1px solid transparent; "
	"}"
	".row:first-child { "
	"  border-top: 1px solid transparent; "
	"  border-bottom: 1px solid transparent; "
	"}"
	".row:last-child { "
	"  border-top: 1px solid alpha(gray,0.5); "
	"  border-bottom: 1px solid alpha(gray,0.5); "
	"}"
	".row.drag-icon { "
	"  background: #282828; "
	"  border: 1px solid blue; "
	"}"
	".row.drag-row { "
	"  color: gray; "
	"  background: alpha(gray,0.2); "
	"}"
	".row.drag-row.drag-hover { "
	"  border-top: 1px solid #4e9a06; "
	"  border-bottom: 1px solid #4e9a06; "
	"}"
	".row.drag-hover image, "
	".row.drag-hover label { "
	"  color: #4e9a06; "
	"}"
	".row.drag-hover-top {"
	"  border-top: 1px solid #4e9a06; "
	"}"
	".row.drag-hover-bottom {"
	"  border-bottom: 1px solid #4e9a06; "
	"}";

static void layer_selector_dispose(GObject *self)
{
	LayerSelector *sel = LAYER_SELECTOR(self);

	g_clear_object(&sel->list_box);
	g_clear_object(&sel->load_parent_window);
	g_clear_object(&sel->save_parent_window);
	g_clear_object(&sel->associated_load_button);
	g_clear_object(&sel->associated_save_button);

	if (sel->dnd_target.target) {
		g_free(sel->dnd_target.target);
		sel->dnd_target.target = NULL;
	}

	/* Chain up to parent's dispose function */
	G_OBJECT_CLASS(layer_selector_parent_class)->dispose(self);
}

static void layer_selector_class_init(LayerSelectorClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	GtkCssProvider *provider;

	/* Implement handles to virtual functions */
	object_class->dispose = layer_selector_dispose;

	/* Setup the CSS provider for the drag and drop animations once */
	provider = gtk_css_provider_new();
	gtk_css_provider_load_from_data(provider, dnd_additional_css, -1, NULL);
	gtk_style_context_add_provider_for_screen(gdk_screen_get_default(), GTK_STYLE_PROVIDER(provider), 800);

	g_object_unref(provider);
}

static void layer_selector_setup_dnd(LayerSelector *self)
{
	gtk_drag_dest_set(GTK_WIDGET(self->list_box), GTK_DEST_DEFAULT_MOTION | GTK_DEST_DEFAULT_DROP,
			  &self->dnd_target, 1, GDK_ACTION_MOVE);
	g_signal_connect(self->list_box, "drag-data-received", G_CALLBACK(layer_selector_drag_data_received), NULL);
	g_signal_connect(self->list_box, "drag-motion", G_CALLBACK(layer_selector_drag_motion), NULL);
	g_signal_connect(self->list_box, "drag-leave", G_CALLBACK(layer_selector_drag_leave), NULL);
}

/* Drag and drop end */

static void layer_selector_init(LayerSelector *self)
{
	self->load_parent_window = NULL;
	self->save_parent_window = NULL;
	self->associated_load_button = NULL;
	self->associated_save_button = NULL;

	self->dnd_target.target = g_strdup_printf("LAYER_SELECTOR_DND_%p", self);
	self->dnd_target.info = 0;
	self->dnd_target.flags = GTK_TARGET_SAME_APP;
}

LayerSelector *layer_selector_new(GtkListBox *list_box)
{
	LayerSelector *selector;

	if (GTK_IS_LIST_BOX(list_box) == FALSE)
		return NULL;

	selector = LAYER_SELECTOR(g_object_new(TYPE_LAYER_SELECTOR, NULL));
	selector->list_box = list_box;
	layer_selector_setup_dnd(selector);
	g_object_ref(G_OBJECT(list_box));

	return selector;
}

LayerSettings *layer_selector_export_rendered_layer_info(LayerSelector *selector)
{
	LayerSettings *layer_settings;
	struct layer_info linfo;
	GList *row_list;
	GList *iterator;
	LayerElement *le;
	int i;

	layer_settings = layer_settings_new();
	if (!layer_settings)
		return NULL;

	row_list = gtk_container_get_children(GTK_CONTAINER(selector->list_box));

	for (i = 0, iterator = row_list; iterator != NULL; iterator = g_list_next(iterator), i++) {
		le = LAYER_ELEMENT(iterator->data);

		/* Get name from layer element. This must not be freed */
		linfo.name = (char *)layer_element_get_name(le);

		layer_element_get_color(le, &linfo.color);
		linfo.render = (layer_element_get_export(le) ? 1 : 0);
		linfo.stacked_position = i;
		linfo.layer = layer_element_get_layer(le);

		/* This function copies the entire layer info struct including the name string.
		 * Therefore, using the same layer_info struct over and over is safe.
		 */
		layer_settings_append_layer_info(layer_settings, &linfo);
	}

	return layer_settings;
}

static void layer_selector_clear_widgets(LayerSelector *self)
{
	GList *list;
	GList *temp;

	list = gtk_container_get_children(GTK_CONTAINER(self->list_box));
	for (temp = list; temp != NULL; temp = temp->next)
		gtk_container_remove(GTK_CONTAINER(self->list_box), GTK_WIDGET(temp->data));

	/* Widgets are already destroyed when removed from box because they are only referenced inside the container */

	g_list_free(list);

	/* Deactivate buttons */
	if (self->associated_load_button)
		gtk_widget_set_sensitive(self->associated_load_button, FALSE);
	if (self->associated_save_button)
		gtk_widget_set_sensitive(self->associated_save_button, FALSE);
}

/**
 * @brief Check if a specific layer element with the given layer number is present in the layer selector
 * @param self LayerSelector instance
 * @param layer Layer number to check for
 * @return TRUE if layer is present, else FALSE
 */
static gboolean layer_selector_check_if_layer_widget_exists(LayerSelector *self, int layer)
{
	GList *list;
	GList *temp;
	LayerElement *widget;
	gboolean ret = FALSE;

	list = gtk_container_get_children(GTK_CONTAINER(self->list_box));

	for (temp = list; temp != NULL; temp = temp->next) {
		widget = LAYER_ELEMENT(temp->data);
		if (layer_element_get_layer(widget) == layer) {
			ret = TRUE;
			break;
		}
	}

	g_list_free(list);

	return ret;
}

/**
 * @brief Setup the necessary drag and drop callbacks of layer elements.
 * @param self LayerSelector instance. Used to get the DnD target entry.
 * @param element LayerElement instance to set the callbacks
 */
static void sel_layer_element_setup_dnd_callbacks(LayerSelector *self, LayerElement *element)
{
	struct layer_element_dnd_data dnd_data;

	if (!self || !element)
		return;

	dnd_data.entries = &self->dnd_target;
	dnd_data.entry_count = 1;
	dnd_data.drag_end = sel_layer_element_drag_end;
	dnd_data.drag_begin = sel_layer_element_drag_begin;
	dnd_data.drag_data_get = sel_layer_element_drag_data_get;

	layer_element_set_dnd_callbacks(element, &dnd_data);
}

/**
 * @brief Analyze \p cell layers and append detected layers to layer selector \p self
 * @param self LayerSelector instance
 * @param cell Cell to analyze
 */
static void layer_selector_analyze_cell_layers(LayerSelector *self, struct gds_cell *cell)
{
	GList *graphics;
	struct gds_graphics *gfx;
	int layer;
	GtkWidget *le;

	for (graphics = cell->graphic_objs; graphics != NULL; graphics = graphics->next) {
		gfx = (struct gds_graphics *)graphics->data;
		layer = (int)gfx->layer;
		if (layer_selector_check_if_layer_widget_exists(self, layer) == FALSE) {
			le = layer_element_new();
			sel_layer_element_setup_dnd_callbacks(self, LAYER_ELEMENT(le));
			layer_element_set_layer(LAYER_ELEMENT(le), layer);
			gtk_list_box_insert(self->list_box, le, -1);
			gtk_widget_show(le);
		}
	}
}

/**
 * @brief sort_func Sort callback for list box
 * @param row1
 * @param row2
 * @param unused
 * @note Do not use this function. This is an internal callback
 * @return See sort function documentation of GTK+
 */
static gint layer_selector_sort_func(GtkListBoxRow *row1, GtkListBoxRow *row2, gpointer unused)
{
	LayerElement *le1, *le2;
	gint ret;
	static const enum layer_selector_sort_algo default_sort = LAYER_SELECTOR_SORT_DOWN;
	const enum layer_selector_sort_algo *algo = (const enum layer_selector_sort_algo *)unused;

	/* Assume downward sorting */
	/* TODO: This is nasty. Find a better way */
	if (!algo)
		algo = &default_sort;

	le1 = LAYER_ELEMENT(row1);
	le2 = LAYER_ELEMENT(row2);

	/* Determine sort fow downward sort */
	ret = layer_element_get_layer(le1) - layer_element_get_layer(le2);

	/* Change order if upward sort is requested */
	ret *= (*algo == LAYER_SELECTOR_SORT_DOWN ? 1 : -1);

	return ret;
}

void layer_selector_generate_layer_widgets(LayerSelector *selector, GList *libs)
{
	GList *cell_list = NULL;
	struct gds_library *lib;

	layer_selector_clear_widgets(selector);

	for (; libs != NULL; libs = libs->next) {
		lib = (struct gds_library *)libs->data;
		for (cell_list = lib->cells; cell_list != NULL; cell_list = cell_list->next)
			layer_selector_analyze_cell_layers(selector, (struct gds_cell *)cell_list->data);
	} /* For libs */

	/* Sort the layers */
	layer_selector_force_sort(selector, LAYER_SELECTOR_SORT_DOWN);

	/* Activate Buttons */
	if (selector->associated_load_button)
		gtk_widget_set_sensitive(selector->associated_load_button, TRUE);
	if (selector->associated_save_button)
		gtk_widget_set_sensitive(selector->associated_save_button, TRUE);
}

/**
 * @brief Find LayerElement in list with specified layer number
 * @param el_list List with elements of type LayerElement
 * @param layer Layer number
 * @return Found LayerElement. If nothing is found, NULL.
 */
static LayerElement *layer_selector_find_layer_element_in_list(GList *el_list, int layer)
{
	LayerElement *ret = NULL;

	for (; el_list != NULL; el_list = el_list->next) {
		if (layer_element_get_layer(LAYER_ELEMENT(el_list->data)) == layer) {
			ret = LAYER_ELEMENT(el_list->data);
			break;
		}
	}
	return ret;
}

/**
 * @brief Load the layer mapping from a CSV formatted file
 *
 * This function imports the layer specification from a file (see @ref lmf-spec).
 * The layer ordering defined in the file is kept. All layers present in the
 * current loaded library, which are not present in the layer mapping file
 * are appended at the end of the layer selector list.
 *
 * @param self LayerSelector instance
 * @param file_name File name to load from
 */
static void layer_selector_load_layer_mapping_from_file(LayerSelector *self, const gchar *file_name)
{
	GFile *file;
	GFileInputStream *stream;
	GDataInputStream *dstream;
	LayerElement *le;
	GList *rows;
	GList *temp;
	GList *layer_infos;
	int status;
	LayerSettings *layer_settings;
	struct layer_info *linfo;

	file = g_file_new_for_path(file_name);
	stream = g_file_read(file, NULL, NULL);

	if (!stream)
		goto destroy_file;

	dstream = g_data_input_stream_new(G_INPUT_STREAM(stream));

	rows = gtk_container_get_children(GTK_CONTAINER(self->list_box));

	/* Reference and remove all rows from box */
	for (temp = rows; temp != NULL; temp = temp->next) {
		le = LAYER_ELEMENT(temp->data);
		/* Referencing protects the widget from being deleted when removed */
		g_object_ref(G_OBJECT(le));
		gtk_container_remove(GTK_CONTAINER(self->list_box), GTK_WIDGET(le));
	}

	/* Load Layer settings. No need to check pointer, will be checked by load csv func. */
	layer_settings = layer_settings_new();

	status = layer_settings_load_from_csv(layer_settings, file_name);
	if (status)
		goto abort_layer_settings;

	layer_infos = layer_settings_get_layer_info_list(layer_settings);
	if (!layer_infos)
		goto abort_layer_settings;

	/* Loop over all layer infos read from the CSV file */
	for (; layer_infos; layer_infos = g_list_next(layer_infos)) {
		linfo = (struct layer_info *)layer_infos->data;
		le = layer_selector_find_layer_element_in_list(rows, linfo->layer);
		if (!le)
			continue;

		layer_element_set_name(le, linfo->name);
		layer_element_set_export(le, (linfo->render ? TRUE : FALSE));
		layer_element_set_color(le, &linfo->color);
		gtk_container_add(GTK_CONTAINER(self->list_box), GTK_WIDGET(le));
		rows = g_list_remove(rows, le);
	}

abort_layer_settings:
	/* Destroy layer settings. Not needed for adding remaining elements */
	g_object_unref(layer_settings);

	/* Add remaining elements */
	for (temp = rows; temp != NULL; temp = temp->next) {
		le = LAYER_ELEMENT(temp->data);
		/* Referencing protets the widget from being deleted when removed */
		gtk_list_box_insert(self->list_box, GTK_WIDGET(le), -1);
		g_object_unref(G_OBJECT(le));
	}

	/* Delete list */
	g_list_free(rows);

	/* read line */
	g_object_unref(dstream);
	g_object_unref(stream);
destroy_file:
	g_object_unref(file);
}

/**
 * @brief Callback for Load Mapping Button
 * @param button
 * @param user_data
 */
static void layer_selector_load_mapping_clicked(GtkWidget *button, gpointer user_data)
{
	LayerSelector *sel;
	GtkWidget *dialog;
	gint res;
	gchar *file_name;
	(void)button;

	sel = LAYER_SELECTOR(user_data);

	dialog = gtk_file_chooser_dialog_new("Load Mapping File", GTK_WINDOW(sel->load_parent_window),
					     GTK_FILE_CHOOSER_ACTION_OPEN,
					     "Cancel", GTK_RESPONSE_CANCEL, "Load Mapping", GTK_RESPONSE_ACCEPT, NULL);
	res = gtk_dialog_run(GTK_DIALOG(dialog));
	if (res == GTK_RESPONSE_ACCEPT) {
		file_name = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		layer_selector_load_layer_mapping_from_file(sel, file_name);
		g_free(file_name);
	}
	gtk_widget_destroy(dialog);
}



/**
 * @brief Save layer mapping of selector \p self to a file
 * @param self LayerSelector instance
 * @param file_name File name to save to
 */
static void layer_selector_save_layer_mapping_data(LayerSelector *self, const gchar *file_name)
{
	LayerSettings *layer_settings;

	g_return_if_fail(LAYER_IS_SELECTOR(self));
	g_return_if_fail(file_name);

	/* Get layer settings. No need to check return value. to_csv func is safe */
	layer_settings = layer_selector_export_rendered_layer_info(self);
	(void)layer_settings_to_csv(layer_settings, file_name);
}

/**
 * @brief Callback for Save Layer Mapping Button
 * @param button
 * @param user_data
 */
static void layer_selector_save_mapping_clicked(GtkWidget *button, gpointer user_data)
{
	GtkWidget *dialog;
	gint res;
	gchar *file_name;
	LayerSelector *sel;
	(void)button;

	sel = LAYER_SELECTOR(user_data);

	dialog = gtk_file_chooser_dialog_new("Save Mapping File", GTK_WINDOW(sel->save_parent_window),
					     GTK_FILE_CHOOSER_ACTION_SAVE,
					     "Cancel", GTK_RESPONSE_CANCEL, "Save Mapping", GTK_RESPONSE_ACCEPT, NULL);
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);

	res = gtk_dialog_run(GTK_DIALOG(dialog));
	if (res == GTK_RESPONSE_ACCEPT) {
		file_name = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		layer_selector_save_layer_mapping_data(sel, file_name);
		g_free(file_name);
	}
	gtk_widget_destroy(dialog);
}

void layer_selector_set_load_mapping_button(LayerSelector *selector, GtkWidget *button, GtkWindow *main_window)
{
	g_clear_object(&selector->load_parent_window);
	g_clear_object(&selector->associated_load_button);

	g_object_ref(G_OBJECT(button));
	g_object_ref(G_OBJECT(main_window));
	selector->associated_load_button = button;
	selector->load_parent_window = main_window;
	g_signal_connect(button, "clicked", G_CALLBACK(layer_selector_load_mapping_clicked), selector);
}

void layer_selector_set_save_mapping_button(LayerSelector *selector, GtkWidget *button,  GtkWindow *main_window)
{
	g_clear_object(&selector->save_parent_window);
	g_clear_object(&selector->associated_save_button);

	g_object_ref(G_OBJECT(button));
	g_object_ref(G_OBJECT(main_window));
	selector->associated_save_button = button;
	selector->save_parent_window = main_window;
	g_signal_connect(button, "clicked", G_CALLBACK(layer_selector_save_mapping_clicked), selector);
}

void layer_selector_force_sort(LayerSelector *selector, enum layer_selector_sort_algo sort_function)
{
	GtkListBox *box;

	if (!selector)
		return;

	box = selector->list_box;
	if (!box)
		return;

	/* Set sorting function, sort, and disable sorting function */
	gtk_list_box_set_sort_func(box, layer_selector_sort_func, (gpointer)&sort_function, NULL);
	gtk_list_box_invalidate_sort(box);
	gtk_list_box_set_sort_func(box, NULL, NULL, NULL);
}

void layer_selector_select_all_layers(LayerSelector *layer_selector, gboolean select)
{
	GList *le_list;
	GList *iter;
	LayerElement *le;

	g_return_if_fail(LAYER_IS_SELECTOR(layer_selector));
	g_return_if_fail(GTK_IS_LIST_BOX(layer_selector->list_box));

	le_list = gtk_container_get_children(GTK_CONTAINER(layer_selector->list_box));

	for (iter = le_list; iter != NULL; iter = g_list_next(iter)) {
		le = LAYER_ELEMENT(iter->data);
		if (LAYER_IS_ELEMENT(le))
			layer_element_set_export(le, select);
	}

	g_list_free(le_list);
}

void layer_selector_auto_color_layers(LayerSelector *layer_selector, ColorPalette *palette, double global_alpha)
{
	GList *le_list;
	GList *le_list_ptr;
	LayerElement *le;
	unsigned int color_index = 0;
	unsigned int color_count;
	GdkRGBA color;

	g_return_if_fail(GDS_RENDER_IS_COLOR_PALETTE(palette));
	g_return_if_fail(LAYER_IS_SELECTOR(layer_selector));
	g_return_if_fail(global_alpha > 0);
	g_return_if_fail(GTK_IS_LIST_BOX(layer_selector->list_box));

	le_list = gtk_container_get_children(GTK_CONTAINER(layer_selector->list_box));

	/* iterate over layer elements and fill colors */
	color_index = 0;
	color_count = color_palette_get_color_count(palette);
	if (color_count == 0)
		goto ret_free_le_list;

	for (le_list_ptr = le_list; le_list_ptr != NULL; le_list_ptr = le_list_ptr->next) {
		le = LAYER_ELEMENT(le_list_ptr->data);
		if (le) {
			color_palette_get_color(palette, &color, color_index++);
			color.alpha *= global_alpha;
			layer_element_set_color(le, &color);

			if (color_index >= color_count)
				color_index = 0;
		}
	}

ret_free_le_list:
	g_list_free(le_list);
}

void layer_selector_auto_name_layers(LayerSelector *layer_selector, gboolean overwrite)
{
	GList *le_list;
	GList *le_list_ptr;
	LayerElement *le;
	const char *old_layer_name;
	GString *new_layer_name;

	g_return_if_fail(LAYER_IS_SELECTOR(layer_selector));

	new_layer_name = g_string_new_len(NULL, 10);
	le_list = gtk_container_get_children(GTK_CONTAINER(layer_selector->list_box));

	for (le_list_ptr = le_list; le_list_ptr != NULL; le_list_ptr = g_list_next(le_list_ptr)) {
		le = LAYER_ELEMENT(le_list_ptr->data);
		if (!le)
			continue;
		old_layer_name = layer_element_get_name(le);

		/* Check if layer name is empty or may be overwritten */
		if (!old_layer_name || *old_layer_name == '\0' || overwrite) {
			g_string_printf(new_layer_name, "Layer %d", layer_element_get_layer(le));
			layer_element_set_name(le, new_layer_name->str);
		}
	}

	g_string_free(new_layer_name, TRUE);
	g_list_free(le_list);
}

gboolean layer_selector_contains_elements(LayerSelector *layer_selector)
{
	GList *layer_element_list;

	/* Check objects */
	g_return_val_if_fail(LAYER_IS_SELECTOR(layer_selector), FALSE);
	g_return_val_if_fail(GTK_IS_LIST_BOX(layer_selector->list_box), FALSE);

	/* Get a list of the child elements inside the list boy associated with this selector */
	layer_element_list = gtk_container_get_children(GTK_CONTAINER(layer_selector->list_box));

	/* Return TRUE if there is an element in the list, else return FALSE */
	return (layer_element_list ? TRUE : FALSE);
}

size_t layer_selector_num_of_named_elements(LayerSelector *layer_selector)
{
	GList *le_list;
	GList *le_list_ptr;
	LayerElement *le;
	const char *layer_name;
	size_t count = 0U;

	g_return_val_if_fail(LAYER_IS_SELECTOR(layer_selector), 0U);

	le_list = gtk_container_get_children(GTK_CONTAINER(layer_selector->list_box));

	for (le_list_ptr = le_list; le_list_ptr != NULL; le_list_ptr = g_list_next(le_list_ptr)) {
		le = LAYER_ELEMENT(le_list_ptr->data);
		if (!le)
			continue;
		layer_name = layer_element_get_name(le);

		if (layer_name && *layer_name) {
			/* Layer name is not empty. Count it */
			count++;
		}
	}

	return count;
}

/** @} */
