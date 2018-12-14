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
 * @file command-line.c
 * @brief Render according to command line parameters
 * @author Mario Hüttel <mario.huettel@gmx.net>
 */

/**
 * @addtogroup MainApplication
 * @{
 */

#ifndef _EXTERNAL_RENDERER_H_
#define _EXTERNAL_RENDERER_H_

#include "gds-parser/gds-types.h"
#include <glib.h>

/**
 * @brief function name expected to be found in external library.
 * @detail The function has to be defined as follows:
 * @code
 * int function_name(gds_cell *toplevel, GList *layer_info_list, char *output_file_name)
 * @endcode
 */
#define EXTERNAL_LIBRARY_FUNCTION "render_cell_to_file"

/**
 * @brief external_renderer_render_cell
 * @param toplevel_cell The toplevel cell to render
 * @param layer_info_list The layer information. Contains #layer_info elements
 * @param output_file Output file
 * @param so_path Path to the shared object file containing #EXTERNAL_LIBRARY_FUNCTION
 * @return 0 on success
 */
int external_renderer_render_cell(struct gds_cell *toplevel_cell, GList *layer_info_list, char *output_file, char *so_path);

#endif /* _EXTERNAL_RENDERER_H_ */

/** @} */
