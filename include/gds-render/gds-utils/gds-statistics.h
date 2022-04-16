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
 * @brief Header file for the GDS statistics
 * @author Mario Hüttel <mario.huettel@gmx.net>
 */

#ifndef _GDS_STATISTICS_H_
#define _GDS_STATISTICS_H_

/**
 * @addtogroup GDS-Utilities
 * @{
 */

#include <glib.h>

#include <gds-render/gds-utils/gds-types.h>

struct gds_cell_statistics {
	size_t gfx_count;
	size_t vertex_count;
	size_t reference_count;
	const struct gds_cell *cell;
};

struct gds_lib_statistics {
	size_t gfx_count;
	size_t vertex_count;
	size_t reference_count;
	size_t cell_count;
	GList *cell_statistics;
	const struct gds_library *library;
};

/**
 * @brief Calculate statistics of a single cell
 * @param[in] cell GDS cell
 * @param[out] stat Statistics output
 */
void gds_statistics_calc_cell(const struct gds_cell *cell,
			      struct gds_cell_statistics *stat);

/**
 * @brief Calc statistic information for library
 * @param[in] library_list List containing all libraries
 * @param[in,out] lib_stat_list Statistic list
 */
void gds_statistics_calc_library(GList * library_list,
				 GList ** lib_stat_list);

/**
 * @brief Free library statistics GList
 * @param[in,out] lib_stat_list List to free
 */
void gds_statistics_free_lib_stat_list(GList ** lib_stat_list);

/** @} */

#endif				/* _GDS_STATISTICS_H_ */
