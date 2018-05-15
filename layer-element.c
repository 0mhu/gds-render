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

#include "layer-element.h"

G_DEFINE_TYPE (LayerElement, layer_element, GTK_TYPE_BOX)

static void layer_element_class_init(LayerElementClass *klass)
{
	return;
}

static void layer_element_init(LayerElement *self)
{
	self->button = gtk_button_new();
	gtk_box_pack_start(GTK_BOX(self), self->button, TRUE, TRUE, 0);
	gtk_widget_show(self->button);
}

GtkWidget *layer_element_new(void)
{
	return GTK_WIDGET(g_object_new(layer_element_get_type(), NULL));
}
