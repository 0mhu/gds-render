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
 * @brief LayerSettings class heade file
 * @author Mario Hüttel <mario.huettel@gmx.net>
 */

#ifndef _LAYER_INFO_H_
#define _LAYER_INFO_H_

#include <gtk/gtk.h>

G_BEGIN_DECLS

/**
 * @brief Layer information.
 *
 * This structs contains information on how to render a layer
 * @note You probably don't want to use this struct standalone but in combination
 * with a LayerSettings object.
 */
struct layer_info
{
	int layer; /**< @brief Layer number */
	char *name; /**< @brief Layer name. */
	int stacked_position; ///< @brief Position of layer in output @warning This parameter is not used by any renderer so far @note Lower is bottom, higher is top
	GdkRGBA color; /**< @brief RGBA color used to render this layer */
	int render; /**< @brief true: Render to output */
};

G_DECLARE_FINAL_TYPE(LayerSettings, layer_settings, GDS_RENDER, LAYER_SETTINGS, GObject)

#define GDS_RENDER_TYPE_LAYER_SETTINGS (layer_settings_get_type())

/**
 * @brief Maximum length of a layer mapping CSV line
 */
#define CSV_LINE_MAX_LEN (1024)

/**
 * @brief New LayerSettings object
 * @return New object
 */
LayerSettings *layer_settings_new();

/**
 * @brief layer_settings_append_layer_info
 * @param settings LayerSettings object.
 * @param info Info to append
 * @return Error code. 0 if successful
 * @note \p info is copied internally. You can free this struct afterwards.
 */
int layer_settings_append_layer_info(LayerSettings *settings, struct layer_info *info);

/**
 * @brief Clear all layers in this settings object
 * @param settings LayerSettings object
 */
void layer_settings_clear(LayerSettings *settings);

/**
 * @brief Remove a specific layer number from the layer settings.
 * @param settings LayerSettings object
 * @param layer Layer number
 * @return Error code. 0 if successful
 */
int layer_settings_remove_layer(LayerSettings *settings, int layer);

/**
 * @brief Get a GList with layer_info structs
 *
 * This function returns a GList with all layer_info structs in rendering order
 * (bottom to top) that shall be rendered.
 *
 * @param settings LayerSettings object
 * @return GList with struct layer_info elements.
 */
GList *layer_settings_get_layer_info_list(LayerSettings *settings);

/**
 * @brief Write layer settings to a CSV file
 * @param path
 * @return 0 if successful
 */
int layer_settings_to_csv(LayerSettings *settings, const char *path);

/**
 * @brief Load new layer Settings from CSV
 *
 * This function loads the layer information from a CSV file.
 * All data inside the \p settings is cleared beforehand.
 *
 * @param settings Settings to write to.
 * @param path CSV file path
 * @return 0 if successful
 */
int layer_settings_load_from_csv(LayerSettings *settings, const char *path);

G_END_DECLS

#endif // _LAYER_INFO_H_
