/*
 * GDSII-Converter
 * Copyright (C) 2022  Mario Hüttel <mario.huettel@gmx.net>
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
 * @file cell-statistics-renderer.h
 * @brief Header file for the CellStatisticsRenderer GObject Class
 * @author Mario Hüttel <mario.huettel@gmx.net>
 */

/**
 * @addtogroup CellStatisticsRenderer
 * @{
 */

#ifndef _CELL_STATISTICS_RENDERER_H_
#define _CELL_STATISTICS_RENDERER_H_

#include <gtk/gtk.h>
#include <gds-render/gds-utils/gds-statistics.h>

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(CellStatisticsRenderer, cell_statistics_renderer, GDS_RENDER, CELL_STAT_RENDERER, GtkCellRendererText)
#define TYPE_GDS_RENDER_CELL_STAT_RENDERER (cell_statistics_renderer_get_type())

typedef struct _CellStatisticsRenderer {
    GtkCellRendererText super;
} CellStatisticsRenderer;

/**
 * @brief New Cell statistics renderer
 * @return GObject
 */
GtkCellRenderer *cell_statistics_renderer_new(void);

G_END_DECLS

#endif /* _CELL_STATISTICS_RENDERER_H_ */

/** @} */
