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
 * @file conv-settings-dialog.c
 * @brief Implementation of the setting dialog
 * @author Mario Hüttel <mario.huettel@gmx.net>
 */

/**
 * @addtogroup Widgets
 * @{
 */

#include <gds-render/widgets/conv-settings-dialog.h>

struct  _RendererSettingsDialog {
		GtkDialog parent;
		/* Private loot */
		GtkWidget *radio_latex;
		GtkWidget *radio_cairo_pdf;
		GtkWidget *radio_cairo_svg;
		GtkWidget *scale;
		GtkWidget *layer_check;
		GtkWidget *standalone_check;
		GtkDrawingArea *shape_drawing;
		GtkLabel *x_label;
		GtkLabel *y_label;

		GtkLabel *x_output_label;
		GtkLabel *y_output_label;

		unsigned int cell_height;
		unsigned int cell_width;
		double unit_in_meters;
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

static gboolean shape_drawer_drawing_callback(GtkWidget *widget, cairo_t *cr, gpointer data)
{
	int width;
	int height;
	GtkStyleContext *style_context;
	GdkRGBA foreground_color;
	RendererSettingsDialog *dialog = (RendererSettingsDialog *)data;
	double usable_width;
	double usable_height;
	double height_scale;
	double width_scale;
	double final_scale_value;

	style_context = gtk_widget_get_style_context(widget);
	width = gtk_widget_get_allocated_width(widget);
	height = gtk_widget_get_allocated_height(widget);

	gtk_render_background(style_context, cr, 0, 0, width, height);

	gtk_style_context_get_color(style_context, gtk_style_context_get_state(style_context),
					&foreground_color);

	gdk_cairo_set_source_rgba(cr, &foreground_color);

	cairo_save(cr);

	/* Tranform coordiante system */
	cairo_scale(cr, 1, -1);
	cairo_translate(cr, (double)width/2.0, -(double)height/2.0);

	/* Define usable drawing area */
	usable_width = (0.95*(double)width) - 15.0;
	usable_height = (0.95*(double)height) - 15.0;

	width_scale = usable_width/(double)dialog->cell_width;
	height_scale = usable_height/(double)dialog->cell_height;

	final_scale_value = (width_scale < height_scale ? width_scale : height_scale);

	cairo_rectangle(cr, -(double)dialog->cell_width*final_scale_value/2.0, -(double)dialog->cell_height*final_scale_value/2.0,
			(double)dialog->cell_width*final_scale_value, (double)dialog->cell_height*final_scale_value);
	cairo_stroke(cr);
	cairo_restore(cr);

	return FALSE;
}

static double convert_number_to_engineering(double input, const char **out_prefix)
{
	const char *selected_prefix = NULL;
	double return_val = 0.0;
	int idx;
	const static char * prefixes[] = {"y", "z", "a", "f", "p", "n", "u", "m", "c", "d", /* < 1 */
				 "", /* 1 */
				 "h", "k", "M", "G", "T", "P", "E", "Z", "Y"}; /* > 1 */
	const static double scale[] = {1E-24, 1E-21, 1E-18, 1E-15, 1E-12, 1E-9, 1E-6, 1E-3, 1E-2, 1E-1,
					1,
					1E2, 1E3, 1E6, 1E9, 1E12, 1E15, 1E18, 1E21, 1E24};
	const int prefix_count = (int)(sizeof(prefixes)/sizeof(char *));

	/* Start with the 2nd smallest prefix */
	for (idx = 1; idx < prefix_count; idx++) {
		if (input < scale[idx]) {
			/* This prefix is bigger than the number. Take the previous one */
			selected_prefix = prefixes[idx-1];
			return_val = input / scale[idx-1];
			break;
		}
	}

	/* Check if prefix was set by loop. Else take the largest in the list */
	if (selected_prefix == NULL) {
		selected_prefix = prefixes[prefix_count-1];
		return_val = input / scale[prefix_count-1];
	}

	if (out_prefix)
		*out_prefix = selected_prefix;

	return return_val;
}

static void renderer_settings_dialog_update_labels(RendererSettingsDialog *self)
{
	char default_buff[100];
	double scale;
	double width_meters;
	double height_meters;
	double width_engineering;
	const char *width_prefix;
	double height_engineering;
	const char *height_prefix;

	if (!self)
		return;

	width_meters = (double)self->cell_width * self->unit_in_meters;
	height_meters = (double)self->cell_height * self->unit_in_meters;

	width_engineering = convert_number_to_engineering(width_meters, &width_prefix);
	height_engineering = convert_number_to_engineering(height_meters, &height_prefix);

	snprintf(default_buff, sizeof(default_buff), "Width: %.3lf %sm", width_engineering, width_prefix);
	gtk_label_set_text(self->x_label, default_buff);
	snprintf(default_buff, sizeof(default_buff), "Height: %.3lf %sm", height_engineering, height_prefix);
	gtk_label_set_text(self->y_label, default_buff);

	scale = gtk_range_get_value(GTK_RANGE(self->scale));

	/* Set the pixel sizes */
	snprintf(default_buff, sizeof(default_buff), "Output Width: %u px",
		 (unsigned int)((double)self->cell_width / scale));
	gtk_label_set_text(self->x_output_label, default_buff);
	snprintf(default_buff, sizeof(default_buff), "Output Height: %u px",
		 (unsigned int)((double)self->cell_height / scale));
	gtk_label_set_text(self->y_output_label, default_buff);
}

static void scale_value_changed(GtkRange *range, gpointer user_data)
{
	(void)range;
	RendererSettingsDialog *dialog;

	dialog = RENDERER_SETTINGS_DIALOG(user_data);
	renderer_settings_dialog_update_labels(dialog);
}

static void renderer_settings_dialog_init(RendererSettingsDialog *self)
{
	GtkBuilder *builder;
	GtkWidget *box;
	GtkDialog *dialog;

	dialog = &self->parent;

	builder = gtk_builder_new_from_resource("/dialog.glade");
	box = GTK_WIDGET(gtk_builder_get_object(builder, "dialog-box"));
	self->radio_latex = GTK_WIDGET(gtk_builder_get_object(builder, "latex-radio"));
	self->radio_cairo_pdf = GTK_WIDGET(gtk_builder_get_object(builder, "cairo-pdf-radio"));
	self->radio_cairo_svg = GTK_WIDGET(gtk_builder_get_object(builder, "cairo-svg-radio"));
	self->scale = GTK_WIDGET(gtk_builder_get_object(builder, "dialog-scale"));
	self->standalone_check = GTK_WIDGET(gtk_builder_get_object(builder, "standalone-check"));
	self->layer_check = GTK_WIDGET(gtk_builder_get_object(builder, "layer-check"));
	self->shape_drawing = GTK_DRAWING_AREA(gtk_builder_get_object(builder, "shape-drawer"));
	self->x_label = GTK_LABEL(gtk_builder_get_object(builder, "x-label"));
	self->y_label = GTK_LABEL(gtk_builder_get_object(builder, "y-label"));
	self->x_output_label = GTK_LABEL(gtk_builder_get_object(builder, "x-output-label"));
	self->y_output_label = GTK_LABEL(gtk_builder_get_object(builder, "y-output-label"));

	gtk_dialog_add_buttons(dialog, "Cancel", GTK_RESPONSE_CANCEL, "OK", GTK_RESPONSE_OK, NULL);
	gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(dialog)), box);
	gtk_window_set_title(GTK_WINDOW(self), "Renderer Settings");

	g_signal_connect(self->radio_latex, "toggled", G_CALLBACK(latex_render_callback), (gpointer)self);
	g_signal_connect(G_OBJECT(self->shape_drawing),
				"draw", G_CALLBACK(shape_drawer_drawing_callback), (gpointer)self);

	g_signal_connect(self->scale, "value-changed", G_CALLBACK(scale_value_changed), (gpointer)self);

	/* Default values */
	self->cell_width = 1;
	self->cell_height = 1;
	self->unit_in_meters = 1E-6;
	renderer_settings_dialog_update_labels(self);

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

void renderer_settings_dialog_set_cell_width(RendererSettingsDialog *dialog, unsigned int width)
{
	if (!dialog)
		return;

	if (width == 0)
		width = 1;

	dialog->cell_width = width;
	renderer_settings_dialog_update_labels(dialog);
}

void renderer_settings_dialog_set_cell_height(RendererSettingsDialog *dialog, unsigned int height)
{
	if (!dialog)
		return;

	if (height == 0)
		height = 1;

	dialog->cell_height = height;
	renderer_settings_dialog_update_labels(dialog);
}

void renderer_settings_dialog_set_database_unit_scale(RendererSettingsDialog *dialog, double unit_in_meters)
{
	if (!dialog)
		return;

	if (unit_in_meters < 0)
		unit_in_meters *= -1;

	dialog->unit_in_meters = unit_in_meters;
	renderer_settings_dialog_update_labels(dialog);
}

/** @} */
