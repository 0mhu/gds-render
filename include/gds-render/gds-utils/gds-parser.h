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
 * @file gds-parser.h
 * @brief Header file for the GDS-Parser
 * @author Mario Hüttel <mario.huettel@gmx.net>
 */

#ifndef _GDSPARSER_H_
#define _GDSPARSER_H_

/**
 * @addtogroup GDS-Utilities
 * @{
 */

#include <glib.h>

#include <gds-render/gds-utils/gds-types.h>

#define GDS_PRINT_DEBUG_INFOS (0) /**< @brief 1: Print infos, 0: Don't print */

/**
 * @brief Parse a GDS file
 *
 * This function parses a GDS File and creates a list of libraries,
 * which then contain the different cells.
 *
 * The function appends The detected libraries to the \p library_array list.
 * The library array may be empty, meaning *library_list may be NULL.
 *
 * @param[in] filename Path to the GDS file
 * @param[in,out] library_array GList Pointer.
 * @param[in] parsing_options Parsing options.
 * @return 0 if successful
 */
int parse_gds_from_file(const char *filename, GList **library_array,
                        const struct gds_library_parsing_opts *parsing_options);

/**
 * @brief Deletes all libraries including cells, references etc.
 * @param library_list Pointer to a list of #gds_library. Is set to NULL after completion.
 * @return 0
 */
int clear_lib_list(GList **library_list);

/** @} */

#endif /* _GDSPARSER_H_ */
