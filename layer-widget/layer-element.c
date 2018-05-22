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

#include "layer-element.h"

G_DEFINE_TYPE(LayerElement, layer_element, GTK_TYPE_BOX)

static void layer_element_dispose(GObject *obj)
{
	/* destroy parent container. This destroys all widgets inside */
	G_OBJECT_CLASS(layer_element_parent_class)->dispose(obj);
}

static void layer_element_constructed(GObject *obj)
{
	G_OBJECT_CLASS(layer_element_parent_class)->constructed(obj);
}

static void layer_element_class_init(LayerElementClass *klass)
{
	GObjectClass *oclass = G_OBJECT_CLASS(klass);
	oclass->dispose = layer_element_dispose;
	oclass->constructed = layer_element_constructed;
}

static void layer_element_init(LayerElement *self)
{
	GtkBuilder *builder;
	GtkWidget *glade_box;
	builder = gtk_builder_new_from_resource("/layer-widget.glade");
	glade_box = GTK_WIDGET(gtk_builder_get_object(builder, "box"));
	gtk_box_pack_start(GTK_BOX(self), glade_box, TRUE, TRUE, 0);

	/* Get Elements */
	self->priv.color = GTK_COLOR_BUTTON(gtk_builder_get_object(builder, "color"));
	self->priv.export = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "export"));
	self->priv.layer = GTK_LABEL(gtk_builder_get_object(builder, "layer"));
	self->priv.name = GTK_ENTRY(gtk_builder_get_object(builder, "entry"));

	/* Connect signals */
	/* None */

	g_object_unref(builder);
}

GtkWidget *layer_element_new(void)
{
	return GTK_WIDGET(g_object_new(TYPE_LAYER_ELEMENT, NULL));
}

const char *layer_element_get_name(LayerElement *elem)
{
	return gtk_entry_get_text(elem->priv.name);
}

void layer_element_set_name(LayerElement *elem, const char* name)
{
	gtk_entry_set_text(elem->priv.name, name);
}

void layer_element_set_layer(LayerElement *elem, int layer)
{
	GString *string;

	string = g_string_new_len(NULL, 100);
	g_string_printf(string, "Layer: %3d", layer);
	gtk_label_set_text(elem->priv.layer, (const gchar *)string->str);
	elem->priv.layer_num = layer;
	g_string_free(string, TRUE);
}

int layer_element_get_layer(LayerElement *elem)
{
	return elem->priv.layer_num;
}

void layer_element_set_export(LayerElement *elem, gboolean export)
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(elem->priv.export), export);
}

gboolean layer_element_get_export(LayerElement *elem)
{
	return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(elem->priv.export));
}

void layer_element_get_color(LayerElement *elem, GdkRGBA *rgba)
{
	if (!rgba)
		return;
	gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(elem->priv.color), rgba);
}

void layer_element_set_color(LayerElement *elem, GdkRGBA *rgba)
{
	gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(elem->priv.color), rgba);
}
