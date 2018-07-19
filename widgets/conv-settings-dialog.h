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

G_DECLARE_FINAL_TYPE(RendererSettingsDialog, renderer_settings_dialog, RENDERER, SETTINGS_DIALOG, GtkDialog)

GtkWidget *renderer_settings_dialog_new(void);

struct  _RendererSettingsDialog {
        GtkDialog parent;
};

#define RENDERER_TYPE_SETTINGS_DIALOG (renderer_settings_dialog_get_type())

G_END_DECLS

#endif /* __CONV_SETTINGS_DIALOG_H__ */
