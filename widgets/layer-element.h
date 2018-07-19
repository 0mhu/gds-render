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

G_BEGIN_DECLS

/* Creates Class structure etc */
G_DECLARE_FINAL_TYPE(LayerElement, layer_element, LAYER, ELEMENT, GtkListBoxRow)

#define TYPE_LAYER_ELEMENT (layer_element_get_type())

typedef struct _LayerElementPriv {
	GtkEntry *name;
	GtkLabel *layer;
	int layer_num;
	GtkEventBox *event_handle;
	GtkColorButton *color;
	GtkCheckButton *export;
} LayerElementPriv;

struct _LayerElement {
	/* Inheritance */
	GtkListBoxRow parent;
	/* Custom Elements */
	LayerElementPriv priv;
};

GtkWidget *layer_element_new(void);

const char *layer_element_get_name(LayerElement *elem);
void layer_element_set_name(LayerElement *elem, const char* name);
void layer_element_set_layer(LayerElement *elem, int layer);
int layer_element_get_layer(LayerElement *elem);
void layer_element_set_export(LayerElement *elem, gboolean export);
gboolean layer_element_get_export(LayerElement *elem);
void layer_element_get_color(LayerElement *elem, GdkRGBA *rgba);
void layer_element_set_color(LayerElement *elem, GdkRGBA *rgba);

G_END_DECLS

#endif /* __LAYER_ELEMENT_H__ */
