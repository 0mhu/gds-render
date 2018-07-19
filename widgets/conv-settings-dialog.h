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

#ifndef __CONV_SETTINGS_DIALOG_H__
#define __CONV_SETTINGS_DIALOG_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

enum output_renderer {RENDERER_LATEX_TIKZ, RENDERER_CAIROGRAPHICS};

G_DECLARE_FINAL_TYPE(RendererSettingsDialog, renderer_settings_dialog, RENDERER, SETTINGS_DIALOG, GtkDialog)

RendererSettingsDialog *renderer_settings_dialog_new(GtkWindow *parent);

#define RENDERER_TYPE_SETTINGS_DIALOG (renderer_settings_dialog_get_type())

struct render_settings {
	double scale;
	enum output_renderer renderer;
	gboolean tex_pdf_layers;
	gboolean tex_standalone;
};

G_END_DECLS


void renderer_settings_dialog_set_settings(RendererSettingsDialog *dialog, struct render_settings *settings);
void renderer_settings_dialog_get_settings(RendererSettingsDialog *dialog, struct render_settings *settings);

#endif /* __CONV_SETTINGS_DIALOG_H__ */
