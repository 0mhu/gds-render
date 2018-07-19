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

#include "conv-settings-dialog.h"

G_DEFINE_TYPE(RendererSettingsDialog, renderer_settings_dialog, GTK_TYPE_DIALOG)

static void renderer_settings_dialog_class_init(RendererSettingsDialogClass *klass)
{
	/*  No special code needed. Child cells are destroyed automatically due to reference counter */
	return;
}

static void renderer_settings_dialog_init(RendererSettingsDialog *self)
{
	GtkBuilder *builder;
	GtkBox *box;
	GtkDialog *dialog;

	dialog = &(self->parent);

	builder = gtk_builder_new_from_resource("/dialog.glade");
	box = GTK_BOX(gtk_builder_get_object(builder, "dialog-box"));
	gtk_dialog_add_buttons(dialog, "Cancel", GTK_RESPONSE_CANCEL, "OK", GTK_RESPONSE_OK, NULL);
	gtk_container_add(GTK_CONTAINER(dialog), box);
	g_object_unref(builder);
}

GtkWidget *renderer_settings_dialog_new(void)
{
	return (GtkWidget *) g_object_new(RENDERER_TYPE_SETTINGS_DIALOG, NULL);
}
