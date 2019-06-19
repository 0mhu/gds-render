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
 * @addtogroup cmdline
 * @{
 */

#ifndef _COMMAND_LINE_H_
#define _COMMAND_LINE_H_

#include <glib.h>

enum command_line_renderer {
	CMD_NONE = 0,
	CMD_EXTERNAL,
	CMD_CAIRO_SVG,
	CMD_CAIRO_PDF,
	CMD_LATEX,
};

enum cmd_options {
	CMD_OPT_NONE = 0U,
	CMD_OPT_LATEX_STANDALONE = (1U<<0),
	CMD_OPT_LATEX_LAYERS = (1U<<1),
};

/**
 * @brief render output file according to command line parameters
 * @param gds_name Name of GDS file
 * @param cell_name Name of cell to render
 * @param output_file_name Output file name
 * @param so_file Shared object file to search external rendering function
 * @param renderer Type of output renderer
 * @param options Additional options for output renderer
 * @param scale Scale value
 */
void command_line_convert_gds(const char *gds_name, const char *cell_name, const char *output_file_name, const char *layer_file,
			      const char *so_file, enum command_line_renderer renderer, enum cmd_options options, double scale);

#endif /* _COMMAND_LINE_H_ */

/** @} */
