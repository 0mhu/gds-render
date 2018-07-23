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

void command_line_convert_gds(char *gds_name, char *pdf_name, char *tex_name, gboolean pdf, gboolean tex, char *layer_file)
{
	GList *libs = NULL;
	int res;
	GFile *file;
	GFileInputStream *stream;
	GDataInputStream *dstream;

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



	g_object_unref(dstream);
	g_object_unref(stream);
destroy_file:
	g_object_unref(file);


}
