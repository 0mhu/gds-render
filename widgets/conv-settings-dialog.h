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
 * @file conv-settings-dialog.h
 * @brief Header file for the Conversion Settings Dialog
 * @author Mario.Huettel@gmx.net <mario.huettel@gmx.net>
 */

/**
 * @addtogroup Widgets
 * @{
 */

#ifndef __CONV_SETTINGS_DIALOG_H__
#define __CONV_SETTINGS_DIALOG_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

/** @brief return type of the RedererSettingsDialog */
enum output_renderer {RENDERER_LATEX_TIKZ, RENDERER_CAIROGRAPHICS_PDF, RENDERER_CAIROGRAPHICS_SVG};

G_DECLARE_FINAL_TYPE(RendererSettingsDialog, renderer_settings_dialog, RENDERER, SETTINGS_DIALOG, GtkDialog)

/**
 * @brief Create a new RedererSettingsDialog GObject
 * @param parent Parent window
 * @return Created dialog object
 */
RendererSettingsDialog *renderer_settings_dialog_new(GtkWindow *parent);

#define RENDERER_TYPE_SETTINGS_DIALOG (renderer_settings_dialog_get_type())

/**
 * @brief This struct holds the renderer configuration
 */
struct render_settings {
	double scale; /**< @brief Scale image down by this factor. @note Used to keep image in bound of maximum coordinate limit */
	enum output_renderer renderer; /**< The renderer to use */
	gboolean tex_pdf_layers; /**< Create OCG layers when rendering with TikZ */
	gboolean tex_standalone; /**< Create a standalone compile TeX file */
};

G_END_DECLS

/**
 * @brief Apply settings to dialog
 * @param dialog
 * @param settings
 */
void renderer_settings_dialog_set_settings(RendererSettingsDialog *dialog, struct render_settings *settings);

/**
 * @brief Get the settings configured in the dialog
 * @param dialog
 * @param settings
 */
void renderer_settings_dialog_get_settings(RendererSettingsDialog *dialog, struct render_settings *settings);

/**
 * @brief renderer_settings_dialog_set_cell_width Set width for rendered cell
 * @param dialog
 * @param width Width in database units
 */
void renderer_settings_dialog_set_cell_width(RendererSettingsDialog *dialog, unsigned int width);

/**
 * @brief renderer_settings_dialog_set_cell_height Set height for rendered cell
 * @param dialog
 * @param height Height in database units
 */
void renderer_settings_dialog_set_cell_height(RendererSettingsDialog *dialog, unsigned int height);

/**
 * @brief renderer_settings_dialog_set_database_unit_scale Set database scale
 * @param dialog dialog element
 * @param unit_in_meters Database unit in meters
 */
void renderer_settings_dialog_set_database_unit_scale(RendererSettingsDialog *dialog, double unit_in_meters);

#endif /* __CONV_SETTINGS_DIALOG_H__ */

/** @} */
