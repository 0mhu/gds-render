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

#ifndef __LAYER_SELECTOR_H__
#define __LAYER_SELECTOR_H__

#include <gtk/gtk.h>
#include <glib.h>

struct layer_info
{
	int layer;
	GdkRGBA color;
};

void generate_layer_widgets(GtkListBox *listbox, GList *libs);
void setup_load_mapping_callback(GtkWidget *button, GtkWindow *main_window);
void setup_save_mapping_callback(GtkWidget *button, GtkWindow *main_window);
GList *export_rendered_layer_info();

#endif /* __LAYER_SELECTOR_H__ */
