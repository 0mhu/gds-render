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

/*
 * The drag and drop implementation is adapted from
 * https://gitlab.gnome.org/GNOME/gtk/blob/gtk-3-22/tests/testlist3.c
 *
 * Thanks to the GTK3 people for creating these examples.
 */

/**
 * @file layer-element.c
 * @brief Implementation of the layer element used for configuring layer colors etc.
 * @author Mario Hüttel <mario.huettel@gmx.net>
 */

/**
 * @addtogroup Widgets
 * @{
 */

#include "layer-element.h"

G_DEFINE_TYPE(LayerElement, layer_element, GTK_TYPE_LIST_BOX_ROW)

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

static GtkTargetEntry entries[] = {
	{ "GTK_LIST_BOX_ROW", GTK_TARGET_SAME_APP, 0 }
};

static void layer_element_drag_begin(GtkWidget *widget,
				     GdkDragContext *context,
				     gpointer data)
{
	GtkWidget *row;
	GtkAllocation alloc;
	cairo_surface_t *surface;
	cairo_t *cr;
	int x, y;
	(void)data;

	row = gtk_widget_get_ancestor(widget, GTK_TYPE_LIST_BOX_ROW);
	gtk_widget_get_allocation(row, &alloc);
	surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, alloc.width, alloc.height);
	cr = cairo_create(surface);

	gtk_style_context_add_class (gtk_widget_get_style_context(row), "drag-icon");
	gtk_widget_draw (row, cr);
	gtk_style_context_remove_class(gtk_widget_get_style_context(row), "drag-icon");

	gtk_widget_translate_coordinates (widget, row, 0, 0, &x, &y);
	cairo_surface_set_device_offset (surface, -x, -y);
	gtk_drag_set_icon_surface (context, surface);

	cairo_destroy (cr);
	cairo_surface_destroy (surface);

	g_object_set_data(G_OBJECT(gtk_widget_get_parent(row)), "drag-row", row);
	gtk_style_context_add_class(gtk_widget_get_style_context(row), "drag-row");
}

static void layer_element_drag_end(GtkWidget *widget, GdkDragContext *context, gpointer data)
{
	GtkWidget *row;
	(void)context;
	(void)data;

	row = gtk_widget_get_ancestor(widget, GTK_TYPE_LIST_BOX_ROW);
	g_object_set_data(G_OBJECT(gtk_widget_get_parent(row)), "drag-row", NULL);
	gtk_style_context_remove_class(gtk_widget_get_style_context(row), "drag-row");
	gtk_style_context_remove_class(gtk_widget_get_style_context(row), "drag-hover");
}

static void layer_element_drag_data_get(GtkWidget *widget, GdkDragContext *context, GtkSelectionData *selection_data,
					guint info, guint time, gpointer data)
{
	(void)context;
	(void)info;
	(void)time;
	(void)data;

	gtk_selection_data_set(selection_data, gdk_atom_intern_static_string("GTK_LIST_BOX_ROW"),
			       32, (const guchar *)&widget, sizeof(gpointer));
}

static void layer_element_init(LayerElement *self)
{
	GtkBuilder *builder;
	GtkWidget *glade_box;
	builder = gtk_builder_new_from_resource("/layer-widget.glade");
	glade_box = GTK_WIDGET(gtk_builder_get_object(builder, "box"));
	gtk_container_add(GTK_CONTAINER(self), glade_box);

	/* Get Elements */
	self->priv.color = GTK_COLOR_BUTTON(gtk_builder_get_object(builder, "color"));
	self->priv.export = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "export"));
	self->priv.layer = GTK_LABEL(gtk_builder_get_object(builder, "layer"));
	self->priv.name = GTK_ENTRY(gtk_builder_get_object(builder, "entry"));
	self->priv.event_handle = GTK_EVENT_BOX(gtk_builder_get_object(builder, "event-box"));

	/* Setup drag and drop */
	gtk_style_context_add_class (gtk_widget_get_style_context(GTK_WIDGET(self)), "row");
	gtk_drag_source_set(GTK_WIDGET(self->priv.event_handle), GDK_BUTTON1_MASK, entries, 1, GDK_ACTION_MOVE);
	g_signal_connect(self->priv.event_handle, "drag-begin", G_CALLBACK(layer_element_drag_begin), NULL);
	g_signal_connect(self->priv.event_handle, "drag-data-get", G_CALLBACK(layer_element_drag_data_get), NULL);
	g_signal_connect(self->priv.event_handle, "drag-end", G_CALLBACK(layer_element_drag_end), NULL);

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
	g_string_printf(string, "Layer: %d", layer);
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

/** @} */
