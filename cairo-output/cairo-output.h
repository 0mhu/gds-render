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
 * @file cairo-output.h
 * @brief Header File for Cairo output renderer
 * @author Mario Hüttel <mario.huettel@gmx.net>
 */
#ifndef __CAIRO_OUTPUT_H__
#define __CAIRO_OUTPUT_H__

#include "../layer/layer-info.h"
#include "../gds-parser/gds-types.h"

/** @addtogroup Cairo-Renderer
 *  @{
 */

#define MAX_LAYERS (300) /**< \brief Maximum layer count the output renderer can process. Typically GDS only specifies up to 255 layers.*/

/**
 * @brief Render \p cell to a PDF file specified by \p pdf_file
 * @param cell Toplevel cell to @ref Cairo-Renderer
 * @param layer_infos List of layer information. Specifies color and layer stacking
 * @param pdf_file PDF output file. Set to NULL if no PDF file has to be generated
 * @param svg_file SVG output file. Set to NULL if no SVG file has to be generated
 * @param scale Scale the output image down by \p scale
 */
void cairo_render_cell_to_vector_file(struct gds_cell *cell, GList *layer_infos, char *pdf_file, char *svg_file, double scale);

/** @} */

#endif /* __CAIRO_OUTPUT_H__ */
