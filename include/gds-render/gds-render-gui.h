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
 * @file gds-render-gui.h
 * @brief Header for GdsRenderGui Object
 * @author Mario Hüttel <mario.huettel@gmx.net>
 */

#ifndef _GDS_RENDER_GUI_
#define _GDS_RENDER_GUI_

/**
 * @addtogroup GUI
 * @{
 */

#include <gtk/gtk.h>

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(GdsRenderGui, gds_render_gui, RENDERER, GUI, GObject);

#define RENDERER_TYPE_GUI (gds_render_gui_get_type())

/**
 * @brief Create new GdsRenderGui Object
 * @return New object
 */
GdsRenderGui *gds_render_gui_new();

/**
 * @brief Get main window
 *
 * This function returns the main window of the GUI, which can later be displayed.
 * All handling of hte GUI is taken care of inside the GdsRenderGui Object
 * @return The generated main window
 */
GtkWindow *gds_render_gui_get_main_window(GdsRenderGui *gui);

G_END_DECLS

/** @} */

#endif /* _GDS_RENDER_GUI_ */
