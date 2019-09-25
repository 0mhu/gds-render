/*
 *
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
 * @file mapping-parser.c
 * @brief Function to read a mapping file line and parse it.
 * @author Mario Hüttel <mario.huettel@gmx.net>
 */

/**
 * @addtogroup Mapping-Parser
 * @{
 */

#include <gds-render/layer/mapping-parser.h>

void mapping_parser_gen_csv_line(LayerElement *layer_element, char *line_buffer, size_t max_len)
{
	int i;
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
	g_string_printf(string, "%d:%lf:%lf:%lf:%lf:%d:%s\n",
			layer, color.red, color.green,
			color.blue, color.alpha, (export == TRUE ? 1 : 0), name);
	/* Fix broken locale settings */
	for (i = 0; string->str[i]; i++) {
		if (string->str[i] == ',')
			string->str[i] = '.';
	}

	for (i = 0; string->str[i]; i++) {
		if (string->str[i] == ':')
			string->str[i] = ',';
	}

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

/** @} */

