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

	gpointer dummy[4];
};

G_DEFINE_TYPE(ColorPalette, color_palette, G_TYPE_OBJECT)

/**
 * @brief color_palette_fill_with_resource
 * @param palette
 * @param resource_name
 * @return 0 if successful
 */
static int color_palette_fill_with_resource(ColorPalette *palette, char *resource_name)
{
	GBytes *data;
	const char *char_array;
	gsize byte_count;

	if (!palette || !resource_name)
		return -1;

	data = g_resources_lookup_data(resource_name, 0, NULL);

	if (!data)
		return -2;

	char_array = (const char *)g_bytes_get_data(data, &byte_count);

	if (!char_array || !byte_count)
		goto ret_unref;




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

static void color_palette_class_init(ColorPaletteClass *klass)
{
	/* Nothing to do for now */
	return;
}

static void color_palette_init(ColorPalette *self)
{
	self->color_array = NULL;
	self->color_array_length = 0;
}
