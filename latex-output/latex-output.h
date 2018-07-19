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

#ifndef __LATEX_OUTPUT_H__
#define __LATEX_OUTPUT_H__

#include "../gds-parser/gds-types.h"
#include <glib.h>
#include <stdio.h>
#include "../layer-selector.h"

#define LATEX_LINE_BUFFER_KB (10)

void latex_render_cell_to_code(struct gds_cell *cell, GList *layer_infos, FILE *tex_file, double scale,
			       gboolean create_pdf_layers, gboolean standalone_document);

#endif /* __LATEX_OUTPUT_H__ */
