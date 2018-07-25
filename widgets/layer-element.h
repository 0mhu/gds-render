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
 * @file layer-element.h
 * @brief Omplementation of the layer element used for configuring layer colors etc.
 * @author Mario Hüttel <mario.huettel@gmx.net>
 */

/**
 * @addtogroup Widgets
 * @{
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

/**
 * @brief Create new layer element object
 * @return new object
 */
GtkWidget *layer_element_new(void);

/**
 * @brief get name of the layer
 * @param elem Layer element
 * @return Name. Must not be changed, freed or anything else.
 */
const char *layer_element_get_name(LayerElement *elem);

/**
 * @brief layer_element_set_name
 * @param elem set the name of the layer
 * @param name Name. Can be freed after call to this function
 */
void layer_element_set_name(LayerElement *elem, const char* name);

/**
 * @brief Set layer number for this layer
 * @param elem Layer element
 * @param layer Layer number
 */
void layer_element_set_layer(LayerElement *elem, int layer);

/**
 * @brief Get layer number
 * @param elem Layer Element
 * @return Number of this layer
 */
int layer_element_get_layer(LayerElement *elem);

/**
 * @brief Set export flag for this layer
 * @param elem Layer Element
 * @param export flag
 */
void layer_element_set_export(LayerElement *elem, gboolean export);

/**
 * @brief Get export flag of layer
 * @param elem Layer Element
 * @return
 */
gboolean layer_element_get_export(LayerElement *elem);

/**
 * @brief Get color of layer
 * @param elem Layer Element
 * @param rgba RGBA color
 */
void layer_element_get_color(LayerElement *elem, GdkRGBA *rgba);

/**
 * @brief Set color of layer
 * @param elem Layer Element
 * @param rgba RGBA color
 */
void layer_element_set_color(LayerElement *elem, GdkRGBA *rgba);

G_END_DECLS

#endif /* __LAYER_ELEMENT_H__ */

/** @} */
