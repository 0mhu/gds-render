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
 * @file latex-output.h
 * @brief LaTeX output renderer
 * @author Mario Hüttel <mario.huettel@gmx.net>
 */

#ifndef __LATEX_OUTPUT_H__
#define __LATEX_OUTPUT_H__

/**
 * @addtogroup LaTeX-Renderer
 * @{
 */

#include "../gds-utils/gds-types.h"
#include <glib.h>
#include <stdio.h>
#include "../layer/layer-info.h"

#define LATEX_LINE_BUFFER_KB (10) /**< @brief Buffer for LaTeX Code line in KiB */

/**
 * @brief Render \p cell to LateX/TikZ code
 * @param cell Cell to render
 * @param layer_infos Layer information
 * @param tex_file Already opened file to write data in
 * @param scale Scale image down by this value
 * @param create_pdf_layers Optional content groups used
 * @param standalone_document document can be compiled standalone
 */
void latex_render_cell_to_code(struct gds_cell *cell, GList *layer_infos, FILE *tex_file, double scale,
			       gboolean create_pdf_layers, gboolean standalone_document);

/** @} */

#endif /* __LATEX_OUTPUT_H__ */
