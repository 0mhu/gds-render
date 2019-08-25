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
 * @file color-palette.c
 * @brief Class representing a color palette
 * @author Mario Hüttel <mario.huettel@gmx.net>
 */

#include <gds-render/layer/color-palette.h>

struct _ColorPalette {
	/* Inheritance */
	GObject parent;

	/* Custom fields */
	/** @brief The internal array to store the colors */
	GdkRGBA *color_array;
	/** @brief The length of the _ColorPalette::color_array array */
	unsigned int color_array_length;

	/* Dummy bytes to ensure ABI compatibility in future versions */
	gpointer dummy[4];
};

G_DEFINE_TYPE(ColorPalette, color_palette, G_TYPE_OBJECT)

/**
 * @brief Return the number of non empty lines in array
 *
 * This function returns the number of non empty lines in an
 * array. The scanning is either terminated by the given length
 * or if a \0 terminator is found.
 *
 * @param[in] data Array to count lines in
 * @param[in] length Length of \p data
 * @return < 0: Error, >=0: Lines
 */
static int count_non_empty_lines_in_array(const char *data, size_t length)
{
	unsigned int idx;
	int non_empty_lines = 0;
	char last_char = '\n';

	if (!data)
		return -1;

	/* Count each '\n' as a new line if it is not directly preceded by another '\n' */
	for (idx = 0; idx < length && data[idx]; idx++) {
		if (data[idx] == '\n' && last_char != '\n')
			non_empty_lines++;
		last_char = data[idx];
	}

	/* Count the last line in case the data does not end with a '\n' */
	if (data[idx-1] != '\n')
		non_empty_lines++;

	return non_empty_lines;
}

/**
 * @brief color_palette_fill_with_resource
 * @param palette
 * @param resource_name
 * @return 0 if successful
 */
static int color_palette_fill_with_resource(ColorPalette *palette, char *resource_name)
{
	GBytes *data;
	char line[10];
	int line_idx;
	unsigned int color_idx;
	int idx;
	const char *char_array;
	gsize byte_count;
	int lines;
	GRegex *regex;
	GMatchInfo *mi;
	char *match;

	if (!palette || !resource_name)
		return -1;

	data = g_resources_lookup_data(resource_name, 0, NULL);

	if (!data)
		return -2;

	char_array = (const char *)g_bytes_get_data(data, &byte_count);

	if (!char_array || !byte_count)
		goto ret_unref;

	/* Get maximum lenght of color palette, assuming all entries are valid */
	lines = count_non_empty_lines_in_array(char_array, byte_count);

	if (lines <= 0)
		goto ret_unref;

	palette->color_array = (GdkRGBA *)malloc(sizeof(GdkRGBA) * (unsigned int)lines);

	/* Setup regex for hexadecimal RGB colors like 'A0CB3F' */
	regex = g_regex_new("^(?<red>[0-9A-Fa-f][0-9A-Fa-f])(?<green>[0-9A-Fa-f][0-9A-Fa-f])(?<blue>[0-9A-Fa-f][0-9A-Fa-f])$",
			    0, 0, NULL);

	/* Reset line */
	line_idx = 0;
	line[0] = '\0';

	/* Set color index */
	color_idx = 0;

	/* interate over lines and match */
	for (idx = 0 ; idx < byte_count; idx++) {
		/* Fillup line. */
		line[line_idx] = char_array[idx];


		/* If end of line/string is reached, process */
		if (line[line_idx] == '\n' || line[line_idx] == '\0') {
			line[line_idx] = '\0';

			/* Match the line */
			g_regex_match(regex, line, 0, &mi);
			if (g_match_info_matches(mi) && color_idx < lines) {
				match = g_match_info_fetch_named(mi, "red");
				palette->color_array[color_idx].red = (double)g_ascii_strtoll(match, NULL, 16) / 255.0;
				g_free(match);
				match = g_match_info_fetch_named(mi, "green");
				palette->color_array[color_idx].green = (double)g_ascii_strtoll(match, NULL, 16) / 255.0;
				g_free(match);
				match = g_match_info_fetch_named(mi, "blue");
				palette->color_array[color_idx].blue = (double)g_ascii_strtoll(match, NULL, 16) / 255.0;
				g_free(match);

				/* Only RGB supported so far. Fix alpha channel to 1.0 */
				palette->color_array[color_idx].alpha = 1.0;

				color_idx++;
			}

			g_match_info_free(mi);

			/* End of string */
			if (char_array[idx] == '\0')
				break;

			line_idx = 0;
			continue;
		}

		/* increment line index. If end is reached write all bytes  to the line end
		 * line is longer than required for parsing. This ensures, that everything works as expected
		 */
		line_idx += (line_idx < sizeof(line)-1 ? 1 : 0);
	}

	/* Data read; Shrink array in case of invalid lines */
	palette->color_array = realloc(palette->color_array, (size_t)color_idx * sizeof(GdkRGBA));
	palette->color_array_length = color_idx;

	g_regex_unref(regex);
ret_unref:
	g_bytes_unref(data);

	return 0;
}

ColorPalette *color_palette_new_from_resource(char *resource_name)
{
	ColorPalette *palette;

	palette = GDS_RENDER_COLOR_PALETTE(g_object_new(TYPE_GDS_RENDER_COLOR_PALETTE, NULL));
	if (palette)
		(void)color_palette_fill_with_resource(palette, resource_name);

	return palette;
}

GdkRGBA *color_palette_get_color(ColorPalette *palette, GdkRGBA *color, unsigned int index)
{
	GdkRGBA *c = NULL;

	if (!palette)
		goto ret_c;

	if (index >= palette->color_array_length)
		goto ret_c;

	if (color)
		c = color;
	else
		c = (GdkRGBA *)malloc(sizeof(GdkRGBA));

	/* Copy color */
	c->red = palette->color_array[index].red;
	c->green = palette->color_array[index].green;
	c->blue = palette->color_array[index].blue;
	c->alpha = palette->color_array[index].alpha;
ret_c:
	return c;
}

unsigned int color_palette_get_color_count(ColorPalette *palette)
{
	unsigned int return_val = 0;

	if (palette)
		return_val = palette->color_array_length;

	return return_val;
}

static void color_palette_dispose(GObject *gobj)
{
	ColorPalette *palette;

	palette = GDS_RENDER_COLOR_PALETTE(gobj);
	if (palette->color_array)
	{
		palette->color_array_length = 0;
		free(palette->color_array);
	}

	/* Chain up to parent class */
	G_OBJECT_CLASS(color_palette_parent_class)->dispose(gobj);
}

static void color_palette_class_init(ColorPaletteClass *klass)
{
	GObjectClass *gclass;

	gclass = G_OBJECT_CLASS(klass);
	gclass->dispose = color_palette_dispose;
}

static void color_palette_init(ColorPalette *self)
{
	self->color_array = NULL;
	self->color_array_length = 0;
}
