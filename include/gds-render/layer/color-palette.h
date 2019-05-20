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
 * @file color-palette.h
 * @brief Class representing a color palette
 * @author Mario Hüttel <mario.huettel@gmx.net>
 */

#ifndef _COLOR_PALETTE_H_
#define _COLOR_PALETTE_H_

#include <glib.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(ColorPalette, color_palette, GDS_RENDER, COLOR_PALETTE, GObject);

#define TYPE_GDS_RENDER_COLOR_PALETTE (color_palette_get_type())

/**
 * @brief Create a new object with from a resource containing the html hex color scheme
 * @param resource_name Name of the resource
 * @return New object
 */
ColorPalette *color_palette_new_from_resource(char *resource_name);

/**
 * @brief Get the n-th color in the palette identified by the index.
 *
 * This function fills the nth color into the supplied \p color.
 * \p color is returned.
 *
 * If \p color is NULL, a new GdkRGBA is created and returned.
 * This element must be freed afterwards.
 *
 * @param palette Color palette
 * @param color GdkRGBA struct to fill data in. May be NULL.
 * @param index Index of color. Starts at 0
 * @return GdkRGBA color. If \p color is NULL, the returned color must be freed afterwards
 */
GdkRGBA *color_palette_get_color(ColorPalette *palette, GdkRGBA *color, unsigned int index);

/**
 * @brief Return amount of stored colors in \p palette
 * @param palette Color palette
 * @return Count of colors
 */
unsigned int color_palette_get_color_count(ColorPalette *palette);

G_END_DECLS

#endif /* _COLOR_PALETTE_H_ */
