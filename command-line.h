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
 * @file command-line.h
 * @brief Render according to command line parameters
 * @author Mario Hüttel <mario.huettel@gmx.net>
 */

/**
 * @addtogroup MainApplication
 * @{
 */

#ifndef _COMMAND_LINE_H_
#define _COMMAND_LINE_H_
#include <glib.h>

/**
 * @brief Convert GDS according to supplied parameters
 * @param gds_name GDS File path
 * @param pdf_name Cairo-PDF path
 * @param tex_name TeX/TikZ path
 * @param pdf Render Cairo
 * @param tex Render LaTeX
 * @param layer_file Layer mapping file
 * @param cell_name Cell name to render
 * @param scale Scale image down by this value
 * @param pdf_layers TikZ creates OCG layers
 * @param pdf_standalone LaTeX document is standalone7
 * @param svg Render to SVG file
 * @param so_name Path to shared object of custom renderer
 * @param so_out_file Output file path for custom renderer
 * @param svg_name SVG file name
 */
void command_line_convert_gds(char *gds_name, char *pdf_name, char *tex_name, gboolean pdf, gboolean tex,
			      char *layer_file, char *cell_name, double scale, gboolean pdf_layers,
			      gboolean pdf_standalone, gboolean svg, char *svg_name, char *so_name, char *so_out_file);

#endif /* _COMMAND_LINE_H_ */

/** @} */
