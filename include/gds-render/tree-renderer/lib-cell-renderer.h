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

#ifndef __LIB_CELL_RENDERER_H__
#define __LIB_CELL_RENDERER_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(LibCellRenderer, lib_cell_renderer, LIB_CELL, RENDERER, GtkCellRendererText)
#define TYPE_LIB_CELL_RENDERER (lib_cell_renderer_get_type())

#define LIB_CELL_RENDERER_ERROR_WARN (1U<<0)
#define LIB_CELL_RENDERER_ERROR_ERR (1U<<1)

typedef struct _LibCellRenderer {
        /* Inheritance */
        GtkCellRendererText super;
        /* Custom Elements */
} LibCellRenderer;

GType lib_cell_renderer_get_type(void);
GtkCellRenderer *lib_cell_renderer_new(void);

G_END_DECLS

#endif /* __LIB_CELL_RENDERER_H__ */
