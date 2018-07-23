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

#include "command-line.h"
#include "gds-parser/gds-parser.h"
#include "mapping-parser.h"

static void delete_layer_info_with_name(struct layer_info *info)
{
	if (info) {
		if (info->name)
			g_free(info->name);
		free(info);
	}
}

void command_line_convert_gds(char *gds_name, char *pdf_name, char *tex_name, gboolean pdf, gboolean tex, char *layer_file)
{
	GList *libs = NULL;
	int res;
	GFile *file;
	int i;
	GFileInputStream *stream;
	GDataInputStream *dstream;
	gboolean layer_export;
	GdkRGBA layer_color;
	int layer;
	char *layer_name;
	GList *layer_info_list = NULL;
	struct layer_info *linfo_temp;

	/* Check if parameters are valid */
	if (!gds_name || ! pdf_name || !tex_name || !layer_file) {
		printf("Probably missing argument. Check --help option\n");
		return;
	}

	/* Load GDS */
	clear_lib_list(&libs);
	res = parse_gds_from_file(gds_name, &libs);
	if (res)
		return;

	file = g_file_new_for_path(layer_file);
	stream = g_file_read(file, NULL, NULL);

	if (!stream)
		goto destroy_file;

	dstream = g_data_input_stream_new(G_INPUT_STREAM(stream));
	i = 0;
	do {
		res = load_csv_line(dstream, &layer_export, &layer_name, &layer, &layer_color);
		if (res == 0) {
			linfo_temp = (struct layer_info *)malloc(sizeof(struct layer_info));
			if (!linfo_temp) {
				printf("Out of memory\n");
				goto ret_clear_list;
			}
			linfo_temp->color.alpha = layer_color.alpha;
			linfo_temp->color.red = layer_color.red;
			linfo_temp->color.green = layer_color.green;
			linfo_temp->color.blue = layer_color.blue;
			linfo_temp->name = layer_name;
			linfo_temp->stacked_position = i++;
			linfo_temp->layer = layer;
			layer_info_list = g_list_append(layer_info_list, (gpointer)linfo_temp);
		}
	} while(res >= 0);

ret_clear_list:
	g_list_free_full(layer_info_list, (GDestroyNotify)delete_layer_info_with_name);

	g_object_unref(dstream);
	g_object_unref(stream);
destroy_file:
	g_object_unref(file);


}
