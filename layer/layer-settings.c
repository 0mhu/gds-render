/*
 * GDSII-Converter
 * Copyright (C) 2019  Mario Hüttel <mario.huettel@gmx.net>
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
 * @file layer-info.c
 * @brief Implementation of the LayerSettings class
 * @author Mario Hüttel <mario.huettel@gmx.net>
 */

#include <gds-render/layer/layer-settings.h>
#include <stdlib.h>

struct _LayerSettings {
	GObject parent;
	GList *layer_infos;
	gpointer padding[12];
};

G_DEFINE_TYPE(LayerSettings, layer_settings, G_TYPE_OBJECT)

static void layer_settings_init(LayerSettings *self)
{
	self->layer_infos = NULL;
}

static void layer_info_delete_with_name(struct layer_info *const info)
{
	if (!info)
		return;

	if (info->name)
		free(info->name);
	free(info);
}

static void layer_settings_dispose(GObject *obj)
{
	LayerSettings *self;

	self = GDS_RENDER_LAYER_SETTINGS(obj);

	if (self->layer_infos) {
		g_list_free_full(self->layer_infos, (GDestroyNotify)layer_info_delete_with_name);
		self->layer_infos = NULL;
	}

	G_OBJECT_CLASS(layer_settings_parent_class)->dispose(obj);
}

static void layer_settings_class_init(LayerSettingsClass *klass)
{
	GObjectClass *oclass;

	oclass = G_OBJECT_CLASS(klass);
	oclass->dispose = layer_settings_dispose;

	return;
}

/**
 * @brief Copy layer_info struct
 *
 * This function copies a layer info struct.
 * Be aware, that it does not only copy the pointer to the
 * layer name, but instead duplicates the string.
 *
 * @param info Info to copy
 * @return new layer_info struct
 */
static struct layer_info *layer_info_copy(const struct layer_info * const info)
{
	struct layer_info *copy;

	if (!info)
		return 0;

	copy = (struct layer_info *)malloc(sizeof(struct layer_info));
	if (!copy)
		return 0;

	/* Copy data */
	memcpy(copy, info, sizeof(struct layer_info));
	/* Duplicate string */
	if (info->name)
		copy->name = strdup(info->name);

	return copy;
}

LayerSettings *layer_settings_new()
{
	return g_object_new(GDS_RENDER_TYPE_LAYER_SETTINGS, NULL);
}

int layer_settings_append_layer_info(LayerSettings *settings, struct layer_info *info)
{
	struct layer_info *info_copy;

	g_return_val_if_fail(GDS_RENDER_IS_LAYER_SETTINGS(settings), -1);
	if (!info)
		return -2;

	/* Copy layer info */
	info_copy = layer_info_copy(info);

	/* Append to list */
	settings->layer_infos = g_list_append(settings->layer_infos, info_copy);

	return (settings->layer_infos ? 0 : -3);
}

void layer_settings_clear(LayerSettings *settings)
{
	g_return_if_fail(GDS_RENDER_IS_LAYER_SETTINGS(settings));

	/* Clear list and delete layer_info structs including the name field */
	g_list_free_full(settings->layer_infos, (GDestroyNotify)layer_info_delete_with_name);
	settings->layer_infos = NULL;
}

int layer_settings_remove_layer(LayerSettings *settings, int layer)
{
	GList *list_iter;
	GList *found = NULL;
	struct layer_info *inf;

	g_return_val_if_fail(GDS_RENDER_IS_LAYER_SETTINGS(settings), -1);

	/* Find in list */
	for (list_iter = settings->layer_infos; list_iter; list_iter = list_iter->next) {
		inf = (struct layer_info *)list_iter->data;

		if (!inf)
			continue;
		if (inf->layer == layer)
			found = list_iter;
	}

	if (found)  {
		/* Free the layer_info struct */
		layer_info_delete_with_name((struct layer_info *)found->data);
		/* Delete the list element */
		settings->layer_infos = g_list_delete_link(settings->layer_infos, found);
		return 0;
	}

	return -2;
}

GList *layer_settings_get_layer_info_list(LayerSettings *settings)
{
	g_return_val_if_fail(GDS_RENDER_IS_LAYER_SETTINGS(settings), NULL);
	return settings->layer_infos;
}

/**
 * @brief Generate a layer mapping CSV line for a given layer_info struct
 * @param string Buffer to write to
 * @param linfo Layer information
 */
static void layer_settings_gen_csv_line(GString *string, struct layer_info *linfo)
{
	int i;

	g_string_printf(string, "%d:%lf:%lf:%lf:%lf:%d:%s\n",
			linfo->layer, linfo->color.red, linfo->color.green,
			linfo->color.blue, linfo->color.alpha, (linfo->render ? 1 : 0), linfo->name);
	/* Fix broken locale settings */
	for (i = 0; string->str[i]; i++) {
		if (string->str[i] == ',')
			string->str[i] = '.';
	}

	for (i = 0; string->str[i]; i++) {
		if (string->str[i] == ':')
			string->str[i] = ',';
	}
}

int layer_settings_to_csv(LayerSettings *settings, const char *path)
{
	GFile *file;
	GOutputStream *w_fstream;
	GString *string;
	GList *info_iter;
	struct layer_info *linfo;
	int ret = 0;

	file = g_file_new_for_path(path);
	w_fstream = G_OUTPUT_STREAM(g_file_replace(file, NULL, FALSE, G_FILE_CREATE_NONE, NULL, NULL));
	if (!w_fstream) {
		ret = -1;
		goto ret_unref_file;
	}

	/* Allocate new working buffer string. A size bigger than 200 is unexpected, but possible
	 * 200 is a tradeoff between memory usage and preventing the necessity of realloc'ing the string
	 */
	string = g_string_new_len(NULL, 200);
	if (!string) {
		ret = -2;
		goto ret_close_file;
	}

	/* Loop over layers and write CSV lines */
	for (info_iter = settings->layer_infos; info_iter; info_iter = info_iter->next) {
		linfo = (struct layer_info *)info_iter->data;

		layer_settings_gen_csv_line(string, linfo);
		g_output_stream_write(w_fstream, string->str, sizeof(gchar), NULL, NULL);
	}

	/* Delete string */
	g_string_free(string, TRUE);
ret_close_file:
	g_output_stream_flush(w_fstream, NULL, NULL);
	g_output_stream_close(w_fstream, NULL, NULL);
	g_object_unref(w_fstream);
ret_unref_file:
	g_object_unref(file);

	return ret;
}

/**
 * @brief Load a line from \p stream and parse try to parse it as layer information
 * @param stream Input data stream
 * @param linfo Layer info struct to fill
 * @return 1 if malformatted line, 0 if parsing was successful and parameters are valid, -1 if file end
 */
static int layer_settings_load_csv_line_from_stream(GDataInputStream *stream, struct layer_info *linfo)
{
	int ret;
	gsize len;
	gchar *line;
	GRegex *regex;
	GMatchInfo *mi;
	char *match;

	if (!linfo) {
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
		linfo->layer = (int)g_ascii_strtoll(match, NULL, 10);
		g_free(match);
		match = g_match_info_fetch_named(mi, "r");
		linfo->color.red = g_ascii_strtod(match, NULL);
		g_free(match);
		match = g_match_info_fetch_named(mi, "g");
		linfo->color.green = g_ascii_strtod(match, NULL);
		g_free(match);
		match = g_match_info_fetch_named(mi, "b");
		linfo->color.blue = g_ascii_strtod(match, NULL);
		g_free(match);
		match = g_match_info_fetch_named(mi, "a");
		linfo->color.alpha = g_ascii_strtod(match, NULL);
		g_free(match);
		match = g_match_info_fetch_named(mi, "export");
		linfo->render = ((!strcmp(match, "1")) ? 1 : 0);
		g_free(match);
		match = g_match_info_fetch_named(mi, "name");
		linfo->name = match;

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

int layer_settings_load_from_csv(LayerSettings *settings, const char *path)
{
	GFile *file;
	int ret = 0;
	GInputStream *in_stream;
	GDataInputStream *data_stream;
	int parser_ret;
	struct layer_info linfo;

	file = g_file_new_for_path(path);
	in_stream = G_INPUT_STREAM(g_file_read(file, NULL, NULL));
	if (!in_stream) {
		ret = -1;
		goto ret_destroy_file;
	}
	/* Delete old settings */
	layer_settings_clear(settings);

	data_stream = g_data_input_stream_new(in_stream);

	while ((parser_ret = layer_settings_load_csv_line_from_stream(data_stream, &linfo)) >= 0) {
		/* Line broken */
		if (parser_ret == 1)
			continue;

		layer_settings_append_layer_info(settings, &linfo);
		/* Clear name to prevent memory leak */
		if (linfo.name)
			g_free(linfo.name);
	}

	g_object_unref(data_stream);
	g_object_unref(in_stream);
ret_destroy_file:
	g_object_unref(file);

	return ret;
}
