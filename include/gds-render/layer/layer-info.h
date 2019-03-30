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
 * @file layer-info.h
 * @brief Helper functions and definition of layer info struct
 * @author Mario Hüttel <mario.huettel@gmx.net>
 */

#ifndef _LAYER_INFO_H_
#define _LAYER_INFO_H_

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
 * @brief Delete a layer_info struct
 * @param info Struct to be deleted.
 * @note The layer_info::name Element has to be freed manually
 */
void layer_info_delete_struct(struct layer_info *info);

#endif // _LAYER_INFO_H_
