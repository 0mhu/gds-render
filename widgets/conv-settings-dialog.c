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
 * @file conv-settings-dilaog.c
 * @brief Implementation of the setting dialog
 * @author Mario Hüttel <mario.huettel@gmx.net>
 */

/**
 * @addtogroup Widgets
 * @{
 */

#include "conv-settings-dialog.h"

struct  _RendererSettingsDialog {
		GtkDialog parent;
		/* Private loot */
		GtkWidget *radio_latex;
		GtkWidget *radio_cairo_pdf;
		GtkWidget *radio_cairo_svg;
		GtkWidget *scale;
		GtkWidget *layer_check;
		GtkWidget *standalone_check;
};

G_DEFINE_TYPE(RendererSettingsDialog, renderer_settings_dialog, GTK_TYPE_DIALOG)



static void renderer_settings_dialog_class_init(RendererSettingsDialogClass *klass)
{
	/*  No special code needed. Child cells are destroyed automatically due to reference counter */
	return;
}

static void show_tex_options(RendererSettingsDialog *self)
{
	gtk_widget_show(self->layer_check);
	gtk_widget_show(self->standalone_check);

}

static void hide_tex_options(RendererSettingsDialog *self)
{
	gtk_widget_hide(self->layer_check);
	gtk_widget_hide(self->standalone_check);
}

static void latex_render_callback(GtkToggleButton *radio, RendererSettingsDialog *dialog)
{
	if (gtk_toggle_button_get_active(radio))
		show_tex_options(dialog);
	else
		hide_tex_options(dialog);
}

static void renderer_settings_dialog_init(RendererSettingsDialog *self)
{
	GtkBuilder *builder;
	GtkWidget *box;
	GtkDialog *dialog;


	dialog = &(self->parent);

	builder = gtk_builder_new_from_resource("/dialog.glade");
	box = GTK_WIDGET(gtk_builder_get_object(builder, "dialog-box"));
	self->radio_latex = GTK_WIDGET(gtk_builder_get_object(builder, "latex-radio"));
	self->radio_cairo_pdf = GTK_WIDGET(gtk_builder_get_object(builder, "cairo-pdf-radio"));
	self->radio_cairo_svg = GTK_WIDGET(gtk_builder_get_object(builder, "cairo-svg-radio"));
	self->scale = GTK_WIDGET(gtk_builder_get_object(builder, "dialog-scale"));
	self->standalone_check = GTK_WIDGET(gtk_builder_get_object(builder, "standalone-check"));
	self->layer_check = GTK_WIDGET(gtk_builder_get_object(builder, "layer-check"));

	gtk_dialog_add_buttons(dialog, "Cancel", GTK_RESPONSE_CANCEL, "OK", GTK_RESPONSE_OK, NULL);
	gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(dialog)), box);
	gtk_window_set_title(GTK_WINDOW(self), "Renderer Settings");

	g_signal_connect(self->radio_latex, "toggled", G_CALLBACK(latex_render_callback), (gpointer)self);

	g_object_unref(builder);
}

RendererSettingsDialog *renderer_settings_dialog_new(GtkWindow *parent)
{
	RendererSettingsDialog *res;

	res = RENDERER_SETTINGS_DIALOG(g_object_new(RENDERER_TYPE_SETTINGS_DIALOG, NULL));
	if (res && parent) {
		gtk_window_set_transient_for(GTK_WINDOW(res), parent);
	}
	return res;
}

void renderer_settings_dialog_get_settings(RendererSettingsDialog *dialog, struct render_settings *settings)
{
	GList *radio_buttons;
	GList *temp_button_list;
	GtkToggleButton *temp_button = NULL;

	if (!settings || !dialog)
		return;
	settings->scale = gtk_range_get_value(GTK_RANGE(dialog->scale));

	/* Get active radio button selection */
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dialog->radio_latex)) == TRUE) {
		settings->renderer = RENDERER_LATEX_TIKZ;
	} else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dialog->radio_cairo_pdf)) == TRUE) {
		settings->renderer = RENDERER_CAIROGRAPHICS_PDF;
	} else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dialog->radio_cairo_svg)) == TRUE) {
		settings->renderer = RENDERER_CAIROGRAPHICS_SVG;
	}

	settings->tex_pdf_layers = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dialog->layer_check));
	settings->tex_standalone = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dialog->standalone_check));
}

void renderer_settings_dialog_set_settings(RendererSettingsDialog *dialog, struct render_settings *settings)
{
	if (!settings || !dialog)
		return;

	gtk_range_set_value(GTK_RANGE(dialog->scale), settings->scale);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dialog->layer_check), settings->tex_pdf_layers);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dialog->standalone_check), settings->tex_standalone);

	switch (settings->renderer) {
	case RENDERER_LATEX_TIKZ:
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dialog->radio_latex), TRUE);
		show_tex_options(dialog);
		break;
	case RENDERER_CAIROGRAPHICS_PDF:
		hide_tex_options(dialog);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dialog->radio_cairo_pdf), TRUE);
		break;
	case RENDERER_CAIROGRAPHICS_SVG:
		hide_tex_options(dialog);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dialog->radio_cairo_svg), TRUE);
		break;


	}
}

/** @} */
