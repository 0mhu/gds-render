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

#ifndef __LAYER_ELEMENT_H__
#define __LAYER_ELEMENT_H__

#include <gtk/gtk.h>
// #include <gdk/gdk.h>

#define LAYER_ELEMENT(obj) G_TYPE_CHECK_INSTANCE_CAST(obj, layer_element_get_type(), LayerElement)
#define LAYER_ELEMENT_CLASS(klass) G_TYPE_CHECK_CLASS_CAST(klass, layer_element_get_type(), LayerElementClass)
#define IS_LAYER_ELEMENT(obj) G_TYPE_CHECK_INSTANCE_TYPE(obj, layer_element_get_type())

typedef struct _LayerElementPriv {
	GtkEntry *name;
	GtkLabel *layer;
	int layer_num;
	GtkColorButton *color;
	GtkCheckButton *export;
} LayerElementPriv;

typedef struct _LayerElement {
	/* Inheritance */
	GtkBox hbox;
	/* Custom Elements */
	LayerElementPriv priv;
} LayerElement;

typedef struct _LayerElementClass {
	GtkBoxClass parent_class;
} LayerElementClass;


GType layer_element_get_type(void);
GtkWidget *layer_element_new(void);

const char *layer_element_get_name(LayerElement *elem);
void layer_element_set_name(LayerElement *elem, const char* name);
void layer_element_set_layer(LayerElement *elem, int layer);
int layer_element_get_layer(LayerElement *elem);
void layer_element_set_export(LayerElement *elem, gboolean export);
gboolean layer_element_get_export(LayerElement *elem);
void layer_element_get_color(LayerElement *elem, GdkRGBA *rgba);
void layer_element_set_color(LayerElement *elem, GdkRGBA *rgba);

#endif /* __LAYER_ELEMENT_H__ */
