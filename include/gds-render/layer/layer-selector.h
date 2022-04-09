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
 * @file layer-selector.h
 * @brief Implementation of the Layer selection list
 * @author Mario Hüttel <mario.huettel@gmx.net>
 */

/**
 * @addtogroup layer-selector
 * @{
 */

#ifndef __LAYER_SELECTOR_H__
#define __LAYER_SELECTOR_H__

#include <gtk/gtk.h>
#include <glib.h>
#include <gds-render/layer/color-palette.h>
#include <gds-render/layer/layer-settings.h>

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(LayerSelector, layer_selector, LAYER, SELECTOR, GObject);

#define TYPE_LAYER_SELECTOR (layer_selector_get_type())

/**
 * @brief Defines how to sort the layer selector list box.
 */
enum layer_selector_sort_algo {LAYER_SELECTOR_SORT_DOWN = 0, LAYER_SELECTOR_SORT_UP};

/**
 * @brief layer_selector_new
 * @param list_box The associated list box, the content is displayed in
 * @return Newly created layer selector
 */
LayerSelector *layer_selector_new(GtkListBox *list_box);

/**
 * @brief Generate layer widgets in in the LayerSelector instance
 * @note This clears all previously inserted elements
 * @param selector LayerSelector instance
 * @param libs The libraries to add
 */
void layer_selector_generate_layer_widgets(LayerSelector *selector, GList *libs);

/**
 * @brief Supply button for loading the layer mapping
 * @param selector LayerSelector instance
 * @param button Load button. Will be referenced
 * @param main_window Parent window for dialogs. Will be referenced
 */
void layer_selector_set_load_mapping_button(LayerSelector *selector, GtkWidget *button, GtkWindow *main_window);

/**
 * @brief Supply button for saving the layer mapping
 * @param selector LayerSelector instance
 * @param button Save button. Will be refeneced
 * @param main_window Parent window for dialogs. Will be referenced
 */
void layer_selector_set_save_mapping_button(LayerSelector *selector, GtkWidget *button, GtkWindow *main_window);

/**
 * @brief Get a list of all layers that shall be exported when rendering the cells
 * @param selector Layer selector instance
 * @return LayerSettings containing the layer information
 */
LayerSettings *layer_selector_export_rendered_layer_info(LayerSelector *selector);

/**
 * @brief Force the layer selector list to be sorted according to \p sort_function
 * @param selector LayerSelector instance
 * @param sort_function The sorting method (up or down sorting)
 */
void layer_selector_force_sort(LayerSelector *selector, enum layer_selector_sort_algo sort_function);

/**
 * @brief Set 'export' value of all layers in the LayerSelector to the supplied select value
 * @param layer_selector LayerSelector object
 * @param select
 */
void layer_selector_select_all_layers(LayerSelector *layer_selector, gboolean select);

/**
 * @brief Apply colors from palette to all layers. Aditionally set alpha
 * @param layer_selector LayerSelector object
 * @param palette Color palette to use
 * @param global_alpha Additional alpha value that is applied to all layers. Must be > 0
 */
void layer_selector_auto_color_layers(LayerSelector *layer_selector, ColorPalette *palette, double global_alpha);

/**
 * @brief Auto name all layers in the layer selector.
 *
 * This functions sets the name of the layer equal to its number.
 * The \p overwrite parameter specifies if already set layer names are overwritten.
 *
 * @param layer_selector LayerSelector
 * @param overwrite Overwrite existing layer names
 */
void layer_selector_auto_name_layers(LayerSelector *layer_selector, gboolean overwrite);

/**
 * @brief Check if the given layer selector contains layer elements.
 *
 * This function checks whether there are elements present.
 * If an invalid object pointer \p layer_selector is passed,
 * the function returns FALSE
 *
 * @param[in] layer_selector Selector to check
 * @return True, if there is at least one layer present inside the selector
 */
gboolean layer_selector_contains_elements(LayerSelector *layer_selector);

/**
 * @brief Get number of layer elements that are named
 * @param[in] layer_selector Layer selector
 * @return Number of layers with a name != NULL or != ""
 */
size_t layer_selector_num_of_named_elements(LayerSelector *layer_selector);

G_END_DECLS

#endif /* __LAYER_SELECTOR_H__ */

/** @} */
