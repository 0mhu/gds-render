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

#ifndef __LAYER_SELECTOR_H__
#define __LAYER_SELECTOR_H__

#include <gtk/gtk.h>
#include <glib.h>

/**
 * @brief Defines how to sort the layer selector list box.
 */
enum layer_selector_sort_algo {LAYER_SELECTOR_SORT_DOWN = 0, LAYER_SELECTOR_SORT_UP};

/**
 * @brief Generate layer widgets in \p listbox
 * @note This clears all previously inserted elements
 * @param listbox
 * @param libs The libraries to add
 */
void layer_selector_generate_layer_widgets(GtkListBox *listbox, GList *libs);

/**
 * @brief Supply button for loading the layer mapping
 * @param button
 * @param main_window Parent window for dialogs
 */
void layer_selector_set_load_mapping_button(GtkWidget *button, GtkWindow *main_window);

/**
 * @brief Supply button for saving the layer mapping
 * @param button
 * @param main_window Parent window for dialogs
 */
void layer_selector_set_save_mapping_button(GtkWidget *button, GtkWindow *main_window);

/**
 * @brief get the layer information present in the listbox of the selector
 * @return List with layer_info elements
 */
GList *layer_selector_export_rendered_layer_info(void);

/**
 * @brief Force sorting of the layer selector in a specified way
 *
 * If the layer selector is not yet set up, this function has no effect.
 *
 * @param sort_function Sorting direction
 */
void layer_selector_force_sort(enum layer_selector_sort_algo sort_function);

#endif /* __LAYER_SELECTOR_H__ */
