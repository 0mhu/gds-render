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

/**
 * @addtogroup GDS-Parser
 * @{
 */

#ifndef __GDSPARSE_H__
#define __GDSPARSE_H__

#include <glib.h>
#include "gds-types.h"

#define GDS_PRINT_DEBUG_INFOS (0) /**< @brief 1: Print infos, 0: Don't print */

int parse_gds_from_file(const char *filename, GList **library_array);
/**
 * @brief Deletes all libraries including cells, references etc.
 * @param library_list Pointer to a list of #gds_library. Is set to NULL after completion.
 * @return 0
 */
int clear_lib_list(GList **library_list);

/** @} */

#endif /* __GDSPARSE_H__ */
