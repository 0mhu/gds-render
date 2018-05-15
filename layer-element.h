/*
 * GDSII-Converter
 * Copyright (C) 2018  Mario Hüttel <mario.huettel@gmx.net>
 *
 * This file is part of GDSII-Converter.
 *
 * GDSII-Converter is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License.
 *
 * GDSII-Converter is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GDSII-Converter.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __LAYER_ELEMENT_H__
#define __LAYER_ELEMENT_H__

#include <gtk/gtk.h>
// #include <gdk/gdk.h>

#define LAYER_ELEMENT(obj) G_TYPE_CHECK_INSTANCE_CAST(obj, layer_element_get_type(), LayerElement)
#define LAYER_ELEMENT_CLASS(klass) G_TYPE_CHECK_CLASS_CAST(klass, layer_element_get_type(), LayerElementClass)
#define IS_LAYE_RELEMENT(obj) G_TYPE_CHECK_INSTANCE_TYPE(obj, layer_element_get_type())

typedef struct _LayerElement {
    /* Inheritance */
    GtkBox hbox;
    /* Custom Elements */
    GtkWidget *button;
} LayerElement;

typedef struct _LayerElementClass {
    GtkBoxClass parent_class;
} LayerElementClass;


GType layer_element_get_type(void);
GtkWidget *layer_element_new(void);

#endif /* __LAYER_ELEMENT_H__ */
