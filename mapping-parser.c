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

#include "mapping-parser.h"

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
int load_csv_line(GDataInputStream *stream, gboolean *export, char **name, int *layer, GdkRGBA *color)
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

