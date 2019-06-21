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

/**
 * @brief Convert GDS according to command line parameters
 * @param gds_name Path to GDS File
 * @param cell_name Cell name
 * @param renderers Renderer ids
 * @param output_file_names Output file names
 * @param layer_file Layer mapping file
 * @param so_path Shared object path
 */
void command_line_convert_gds(const char *gds_name,
			      const char *cell_name,
			      const char * const *renderers,
			      const char * const *output_file_names,
			      const char *layer_file,
			      const char *so_path);

#endif /* _COMMAND_LINE_H_ */

/** @} */
