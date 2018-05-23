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

#include "layer-selector.h"
#include "gds-parser/gds-parser.h"
#include "layer-widget/layer-element.h"
#include <glib.h>
#include <string.h>
#include <stdio.h>

static GtkWidget *global_load_button;
static GtkWidget *global_save_button;
static GtkListBox *global_list_box;

static void delete_layer_widget(GtkWidget *widget)
{

	gtk_widget_destroy(widget);
}


void delete_layer_info_struct(struct layer_info *info)
{
	if (info)
		free(info);
}

/**
 * @brief export_rendered_layer_info
 * @return new list with all info elements needed to render cells
 */
GList *export_rendered_layer_info()
{
	GList *info_list = NULL;
	LayerElement *le;
	struct layer_info *linfo;
	GList *row_list;
	GList *temp;
	int i;

	row_list = gtk_container_get_children(GTK_CONTAINER(global_list_box));

	/* Iterate through  widgets and add layers that shall be exported */
	for (i = 0, temp = row_list; temp != NULL; temp = temp->next, i++) {

		le = LAYER_ELEMENT(temp->data);

		if (layer_element_get_export(le) == TRUE) {
			/* Allocate new info and fill with info */
			linfo = (struct layer_info *)malloc(sizeof(struct layer_info));
			layer_element_get_color(le, &linfo->color);
			linfo->layer = layer_element_get_layer(le);
			linfo->stacked_position = i;

			/* Append to list */
			info_list = g_list_append(info_list, (gpointer)linfo);
		}
	}

	return info_list;
}

void clear_list_box_widgets(GtkListBox *box)
{
	GList *list;
	GList *temp;

	list = gtk_container_get_children(GTK_CONTAINER(box));
	for (temp = list; temp != NULL; temp = temp->next) {
		gtk_container_remove(GTK_CONTAINER(box), GTK_WIDGET(temp->data));
	}
	/* Widgets are already destroyed when removed from box because they are only referenced inside the container */

	g_list_free(list);

	/* Deactivate buttons */
	gtk_widget_set_sensitive(global_load_button, FALSE);
	gtk_widget_set_sensitive(global_save_button, FALSE);
}

static gboolean check_if_layer_widget_exists(int layer) {
	GList *list;
	GList *temp;
	LayerElement *widget;
	gboolean ret = FALSE;

	list = gtk_container_get_children(GTK_CONTAINER(global_list_box));

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

static void analyze_cell_layers(GtkListBox *listbox, struct gds_cell *cell)
{
	GList *graphics;
	struct gds_graphics *gfx;
	int layer;
	GtkWidget *le;

	for (graphics = cell->graphic_objs; graphics != NULL; graphics = graphics->next) {
		gfx = (struct gds_graphics *)graphics->data;
		layer = (int)gfx->layer;
		if (check_if_layer_widget_exists(layer) == FALSE) {
			le = layer_element_new();
			layer_element_set_layer(LAYER_ELEMENT(le), layer);
			gtk_list_box_insert(listbox, le, -1);
			gtk_widget_show(le);
		}
	}
}

gint sort_func(GtkListBoxRow *row1, GtkListBoxRow *row2, gpointer unused)
{
	LayerElement *le1, *le2;
	gint ret;

	le1 = LAYER_ELEMENT(row1);
	le2 = LAYER_ELEMENT(row2);

	ret = layer_element_get_layer(le1) - layer_element_get_layer(le2);

	return ret;
}

void generate_layer_widgets(GtkListBox *listbox, GList *libs)
{
	GList *cell_list = NULL;
	struct gds_library *lib;

	global_list_box = listbox;

	clear_list_box_widgets(listbox);
	gtk_list_box_set_sort_func(listbox, sort_func, NULL, NULL);

	for (; libs != NULL; libs = libs->next) {
		lib = (struct gds_library *)libs->data;
		for (cell_list = lib->cells; cell_list != NULL; cell_list = cell_list->next) {
			analyze_cell_layers(listbox, (struct gds_cell *)cell_list->data);
		} /* For Cell List */
	} /* For libs */

	/* Force sort */
	gtk_list_box_invalidate_sort(listbox);

	/* Disable sort, so user can sort layers */
	gtk_list_box_set_sort_func(listbox, NULL, NULL, NULL);

	/* Activate Buttons */
	gtk_widget_set_sensitive(global_load_button, TRUE);
	gtk_widget_set_sensitive(global_save_button, TRUE);
}

/**
 * @brief load_csv_line
 * @param file
 * @param export
 * @param name
 * @param layer
 * @param color
 * @param opacity
 * @return 0 if succesfull, 1 if line was malformatted or parameters are broken, -1 if file end
 */
static int load_csv_line(GDataInputStream *stream, gboolean *export, char **name, int *layer, GdkRGBA *color)
{
	int ret;
	gsize len;
	gchar *line;
	GRegex *regex;
	GMatchInfo *mi;
	char *match;

	if ((!export) || (!name) || (!layer) || (!color)) {
		ret = 1;
		goto ret_direct;
	}

	regex = g_regex_new("^(?<layer>[0-9]+),(?<r>[0-9\\.]+),(?<g>[0-9\\.]+),(?<b>[0-9\\.]+),(?<a>[0-9\\.]+),(?<export>[01]),(?<name>.*)$", 0, 0, NULL);

	line = g_data_input_stream_read_line(stream, &len, NULL, NULL);
	if (!line) {
		ret = -1;
		goto destroy_regex;
	}

	/* Match line in CSV */
	g_regex_match(regex, line, 0, &mi);
	if (g_match_info_matches(mi)) {
		/* Line is valid */
		match = g_match_info_fetch_named(mi, "layer");
		*layer = (int)g_ascii_strtoll(match, NULL, 10);
		g_free(match);
		match = g_match_info_fetch_named(mi, "r");
		color->red = g_ascii_strtod(match, NULL);
		g_free(match);
		match = g_match_info_fetch_named(mi, "g");
		color->green = g_ascii_strtod(match, NULL);
		g_free(match);
		match = g_match_info_fetch_named(mi, "b");
		color->blue = g_ascii_strtod(match, NULL);
		g_free(match);
		match = g_match_info_fetch_named(mi, "a");
		color->alpha = g_ascii_strtod(match, NULL);
		g_free(match);
		match = g_match_info_fetch_named(mi, "export");
		*export = ((!strcmp(match, "1")) ? TRUE : FALSE);
		g_free(match);
		match = g_match_info_fetch_named(mi, "name");
		*name = match;

		ret = 0;
	} else {
		/* Line is malformatted */
		printf("Could not recognize line in CSV as valid entry: %s\n", line);
		ret = 1;
	}

	g_match_info_free(mi);
	g_free(line);
destroy_regex:
	g_regex_unref(regex);
ret_direct:
	return ret;

}

static LayerElement *find_layer_element_in_list(GList *el_list, int layer)
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

static void load_layer_mapping_from_file(gchar *file_name)
{
	GFile *file;
	GFileInputStream *stream;
	GDataInputStream *dstream;
	LayerElement *le;
	char *name;
	gboolean export;
	int layer;
	int target_layer;
	GdkRGBA color;
	int result;
	GList *rows;
	GList *temp;

	file = g_file_new_for_path(file_name);
	stream = g_file_read(file, NULL, NULL);

	if (!stream)
		goto destroy_file;

	dstream = g_data_input_stream_new(G_INPUT_STREAM(stream));

	rows = gtk_container_get_children(GTK_CONTAINER(global_list_box));

	/* Reference and remove all rows from box */
	for (temp = rows; temp != NULL; temp = temp->next) {
		le = LAYER_ELEMENT(temp->data);
		/* Referencing protets the widget from being deleted when removed */
		g_object_ref(G_OBJECT(le));
		gtk_container_remove(GTK_CONTAINER(global_list_box), GTK_WIDGET(le));
	}

	target_layer = 0;

	while((result = load_csv_line(dstream, &export, &name, &layer, &color)) >= 0) {
		/* skip broken line */
		if (result == 1)
			continue;

		/* Add rows in the same order as in file */
		if (le = find_layer_element_in_list(rows, layer)) {
			gtk_list_box_insert(global_list_box, GTK_WIDGET(le), -1);

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
		gtk_list_box_insert(global_list_box, GTK_WIDGET(le), -1);
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

static void load_mapping_clicked(GtkWidget *button, gpointer user_data)
{
	GtkWidget *dialog;
	gint res;
	gchar *file_name;

	dialog = gtk_file_chooser_dialog_new("Load Mapping File", GTK_WINDOW(user_data), GTK_FILE_CHOOSER_ACTION_OPEN,
					     "Cancel", GTK_RESPONSE_CANCEL, "Load Mapping", GTK_RESPONSE_ACCEPT, NULL);
	res = gtk_dialog_run(GTK_DIALOG(dialog));
	if (res == GTK_RESPONSE_ACCEPT) {
		file_name = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		load_layer_mapping_from_file(file_name);
		g_free(file_name);
	}
	gtk_widget_destroy(dialog);
}

static void create_csv_line(LayerElement *layer_element, char *line_buffer, size_t max_len)
{
	GString *string;
	gboolean export;
	const gchar *name;
	int layer;
	GdkRGBA color;

	string = g_string_new_len(NULL, max_len-1);

	/* Extract values */
	export = layer_element_get_export(layer_element);
	name = (const gchar*)layer_element_get_name(layer_element);
	layer = layer_element_get_layer(layer_element);
	layer_element_get_color(layer_element, &color);

	/* print values to line */
	g_string_printf(string, "%d,%lf,%lf,%lf,%lf,%d,%s\n",
			layer, color.red, color.green,
			color.blue, color.alpha, (export == TRUE ? 1 : 0), name);

	if (string->len > (max_len-1)) {
		printf("Layer Definition too long. Please shorten Layer Name!!\n");
		line_buffer[0] = 0x0;
		return;
	}

	/* copy max_len bytes of string */
	strncpy(line_buffer, (char *)string->str, max_len-1);
	line_buffer[max_len-1] = 0;

	/* Completely remove string */
	g_string_free(string, TRUE);
}

static void save_layer_mapping_data(const gchar *file_name, GtkListBox *list_box)
{
	FILE *file;
	char workbuff[512];
	GList *le_list;
	GList *temp;

	/* Overwrite existing file */
	file = fopen((const char *)file_name, "w");

	le_list = gtk_container_get_children(GTK_CONTAINER(list_box));

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

static void save_mapping_clicked(GtkWidget *button, gpointer user_data)
{
	GtkWidget *dialog;
	gint res;
	gchar *file_name;

	dialog = gtk_file_chooser_dialog_new("Save Mapping File", GTK_WINDOW(user_data), GTK_FILE_CHOOSER_ACTION_SAVE,
					     "Cancel", GTK_RESPONSE_CANCEL, "Save Mapping", GTK_RESPONSE_ACCEPT, NULL);
	res = gtk_dialog_run(GTK_DIALOG(dialog));
	if (res == GTK_RESPONSE_ACCEPT) {
		file_name = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		save_layer_mapping_data(file_name, global_list_box);
		g_free(file_name);
	}
	gtk_widget_destroy(dialog);
}

void setup_load_mapping_callback(GtkWidget *button, GtkWindow *main_window)
{
	g_object_ref(G_OBJECT(button));
	global_load_button = button;
	g_signal_connect(button, "clicked", G_CALLBACK(load_mapping_clicked), main_window);
}

void setup_save_mapping_callback(GtkWidget *button,  GtkWindow *main_window)
{
	g_object_ref(G_OBJECT(button));
	global_save_button = button;
	g_signal_connect(button, "clicked", G_CALLBACK(save_mapping_clicked), main_window);
}
