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
 * @addtogroup MainApplication
 * @{
 */

#include "layer-selector.h"
#include "layer-info.h"
#include "../gds-parser/gds-parser.h"
#include "../widgets/layer-element.h"
#include "../mapping-parser.h"
#include <glib.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

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


/* Drag and drop code 
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

	gtk_style_context_add_class (gtk_widget_get_style_context(row), "drag-icon");
	gtk_widget_draw (row, cr);
	gtk_style_context_remove_class(gtk_widget_get_style_context(row), "drag-icon");

	gtk_widget_translate_coordinates (widget, row, 0, 0, &x, &y);
	cairo_surface_set_device_offset (surface, -x, -y);
	gtk_drag_set_icon_surface (context, surface);

	cairo_destroy (cr);
	cairo_surface_destroy (surface);

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

static void sel_layer_element_drag_data_get(GtkWidget *widget, GdkDragContext *context, GtkSelectionData *selection_data,
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

static GtkListBoxRow *layer_selector_get_last_row (GtkListBox *list)
{
	int i;
	GtkListBoxRow *row;

	row = NULL;
	for (i = 0; ; i++) {
		GtkListBoxRow *tmp;
		tmp = gtk_list_box_get_row_at_index(list, i);
		if (tmp == NULL)
			break;
		row = tmp;
	}

	return row;
}

static GtkListBoxRow *layer_selector_get_row_before (GtkListBox *list, GtkListBoxRow *row)
{
	int pos;

	pos = gtk_list_box_row_get_index (row);
	return gtk_list_box_get_row_at_index (list, pos - 1);
}

static GtkListBoxRow *layer_selector_get_row_after (GtkListBox *list, GtkListBoxRow *row)
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
			row_before = GTK_WIDGET(layer_selector_get_row_before(GTK_LIST_BOX(widget), GTK_LIST_BOX_ROW(row)));
		} else {
			row_before = row;
			row_after = GTK_WIDGET(layer_selector_get_row_after(GTK_LIST_BOX(widget), GTK_LIST_BOX_ROW(row)));
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
	gtk_drag_dest_set(GTK_WIDGET(self->list_box), GTK_DEST_DEFAULT_MOTION | GTK_DEST_DEFAULT_DROP, &self->dnd_target, 1, GDK_ACTION_MOVE);
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

GList *layer_selector_export_rendered_layer_info(LayerSelector *selector)
{
	GList *info_list = NULL;
	LayerElement *le;
	struct layer_info *linfo;
	GList *row_list;
	GList *temp;
	int i;

	if (!selector)
		return NULL;

	row_list = gtk_container_get_children(GTK_CONTAINER(selector->list_box));

	/* Iterate through  widgets and add layers that shall be exported */
	for (i = 0, temp = row_list; temp != NULL; temp = temp->next, i++) {

		le = LAYER_ELEMENT(temp->data);

		if (layer_element_get_export(le) == TRUE) {
			/* Allocate new info and fill with info */
			linfo = (struct layer_info *)malloc(sizeof(struct layer_info));
			layer_element_get_color(le, &linfo->color);
			linfo->layer = layer_element_get_layer(le);
			linfo->stacked_position = i;
			linfo->name = (char *)layer_element_get_name(le);

			/* Append to list */
			info_list = g_list_append(info_list, (gpointer)linfo);
		}
	}

	return info_list;
}

static void layer_selector_clear_widgets(LayerSelector *self)
{
	GList *list;
	GList *temp;

	list = gtk_container_get_children(GTK_CONTAINER(self->list_box));
	for (temp = list; temp != NULL; temp = temp->next) {
		gtk_container_remove(GTK_CONTAINER(self->list_box), GTK_WIDGET(temp->data));
	}
	/* Widgets are already destroyed when removed from box because they are only referenced inside the container */

	g_list_free(list);

	/* Deactivate buttons */
	if (self->associated_load_button)
		gtk_widget_set_sensitive(self->associated_load_button, FALSE);
	if (self->associated_save_button)
		gtk_widget_set_sensitive(self->associated_save_button, FALSE);
}

/**
 * @brief Check if specific layer number is present in list box
 * @param layer Layer nu,ber
 * @return TRUE if present
 */
static gboolean layer_selector_check_if_layer_widget_exists(LayerSelector *self, int layer) {
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

static void sel_layer_element_setup_dnd_callbacks(LayerSelector *self, LayerElement *element)
{
	layer_element_set_dnd_callbacks(element, &self->dnd_target, 1,
					sel_layer_element_drag_begin,
					sel_layer_element_drag_data_get,
					sel_layer_element_drag_end);
}

/**
 * @brief Analyze \p cell and append used layers to list box
 * @param listbox listbox to add layer
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
		for (cell_list = lib->cells; cell_list != NULL; cell_list = cell_list->next) {
			layer_selector_analyze_cell_layers(selector, (struct gds_cell *)cell_list->data);
		} /* For Cell List */
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
 * @brief Load file and apply layer definitions to listbox
 * @param file_name CSV Layer Mapping File
 */
static void layer_selector_load_layer_mapping_from_file(LayerSelector *self, gchar *file_name)
{
	GFile *file;
	GFileInputStream *stream;
	GDataInputStream *dstream;
	LayerElement *le;
	char *name;
	gboolean export;
	int layer;
	GdkRGBA color;
	int result;
	GList *rows;
	GList *temp;

	file = g_file_new_for_path(file_name);
	stream = g_file_read(file, NULL, NULL);

	if (!stream)
		goto destroy_file;

	dstream = g_data_input_stream_new(G_INPUT_STREAM(stream));

	rows = gtk_container_get_children(GTK_CONTAINER(self->list_box));

	/* Reference and remove all rows from box */
	for (temp = rows; temp != NULL; temp = temp->next) {
		le = LAYER_ELEMENT(temp->data);
		/* Referencing protets the widget from being deleted when removed */
		g_object_ref(G_OBJECT(le));
		gtk_container_remove(GTK_CONTAINER(self->list_box), GTK_WIDGET(le));
	}

	while((result = load_csv_line(dstream, &export, &name, &layer, &color)) >= 0) {
		/* skip broken line */
		if (result == 1)
			continue;

		/* Add rows in the same order as in file */
		if ((le = layer_selector_find_layer_element_in_list(rows, layer))) {
			gtk_list_box_insert(self->list_box, GTK_WIDGET(le), -1);

			layer_element_set_color(le, &color);
			layer_element_set_export(le, export);
			layer_element_set_name(le, name);
			g_free(name);

			/* Dereference and remove from list */
			g_object_unref(G_OBJECT(le));
			rows = g_list_remove(rows, le);
		}
	}

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

	sel = LAYER_SELECTOR(user_data);

	dialog = gtk_file_chooser_dialog_new("Load Mapping File", GTK_WINDOW(sel->load_parent_window), GTK_FILE_CHOOSER_ACTION_OPEN,
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
 * @brief Save layer mapping of whole list box into file
 * @param file_name layer mapping file
 * @param list_box listbox
 */
static void layer_selector_save_layer_mapping_data(LayerSelector *self, const gchar *file_name)
{
	FILE *file;
	char workbuff[512];
	GList *le_list;
	GList *temp;

	/* Overwrite existing file */
	file = fopen((const char *)file_name, "w");

	le_list = gtk_container_get_children(GTK_CONTAINER(self->list_box));

	/* File format is CSV: <Layer>,<target_pos>,<R>,<G>,<B>,<Alpha>,<Export?>,<Name> */
	for (temp = le_list; temp != NULL; temp = temp->next) {
		/* To be sure it is a valid string */
		workbuff[0] = 0;
		create_csv_line(LAYER_ELEMENT(temp->data), workbuff, sizeof(workbuff));
		fwrite(workbuff, sizeof(char), strlen(workbuff), file);
	}

	g_list_free(le_list);

	/* Save File */
	fflush(file);
	fclose(file);
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

	sel = LAYER_SELECTOR(user_data);

	dialog = gtk_file_chooser_dialog_new("Save Mapping File", GTK_WINDOW(sel->save_parent_window), GTK_FILE_CHOOSER_ACTION_SAVE,
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

/** @} */
