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

#ifndef __MAPPING_PARSER_H__
#define __MAPPING_PARSER_H__

#include <gtk/gtk.h>

struct layer_info
{
        int layer;
        char *name;
        int stacked_position; ///< Lower is bottom, higher is top
        GdkRGBA color;
};

int load_csv_line(GDataInputStream *stream, gboolean *export, char **name, int *layer, GdkRGBA *color);

#endif /* __MAPPING_PARSER_H__ */
