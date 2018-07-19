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

#ifndef __CAIRO_OUTPUT_H__
#define __CAIRO_OUTPUT_H__

#include "../layer-selector.h"
#include "../gds-parser/gds-types.h"

#define MAX_LAYERS (2048)

void cairo_render_cell_to_pdf(struct gds_cell *cell, GList *layer_infos, char *pdf_file, double scale);

#endif /* __CAIRO_OUTPUT_H__ */
