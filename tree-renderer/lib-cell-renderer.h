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

#define LIB_CELL_RENDERER(obj) G_TYPE_CHECK_INSTANCE_CAST(obj, lib_cell_renderer_get_type(), LibCellRenderer)
#define LIB_CELL_RENDERER_CLASS(klass) G_TYPE_CHECK_CLASS_CAST(klass, lib_cell_renderer_get_type(), LibCellRendererClass)
#define IS_LIB_CELL_RENDERER(obj) G_TYPE_CHECK_INSTANCE_TYPE(obj, layer_element_get_type())
#define TYPE_LIB_CELL_RENDERER (lib_cell_renderer_get_type())

typedef struct _LibCellRenderer {
        /* Inheritance */
        GtkCellRendererText super;
        /* Custom Elements */
} LibCellRenderer;

typedef struct _LibCellRendererClass {
        GtkCellRendererTextClass parent;
} LibCellRendererClass;


GType lib_cell_renderer_get_type(void);
GtkCellRenderer *lib_cell_renderer_new(void);



#endif /* __LIB_CELL_RENDERER_H__ */
