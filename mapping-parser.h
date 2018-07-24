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

/**
 * @file mapping-parser.h
 * @brief Function to read a mapping file line and parse it.
 * @author Mario Hüttel <mario.huettel@gmx.net>
 */

#ifndef __MAPPING_PARSER_H__
#define __MAPPING_PARSER_H__

/**
 * @addtogroup MainApplication
 * @{
 */

#include <gtk/gtk.h>

/**
 * @brief Layer information.
 *
 * This structs contains information on how to render a layer
 */
struct layer_info
{
	int layer; /**< @brief Layer number */
	char *name; /**< @brief Layer name */
	int stacked_position; ///< @brief Position of layer in output @warning This parameter is not used by any renderer so far @note Lower is bottom, higher is top
	GdkRGBA color; /**< @brief RGBA color used to render this layer */
};

/**
 * @brief Load a line from \p stream and parse try to parse it as layer information
 * @param stream Input data stream
 * @param export Layer shall be exported
 * @param name Layer name. Free returned pointer after using.
 * @param layer Layer number
 * @param color RGBA color.
 * @return 1 if malformatted line, 0 if parsing was successful and parameters are valid, -1 if file end
 */
int load_csv_line(GDataInputStream *stream, gboolean *export, char **name, int *layer, GdkRGBA *color);

/** @} */

#endif /* __MAPPING_PARSER_H__ */
