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
 * @file gds-statistics.c
 * @brief Statistics of GDS files
 * @author Mario Hüttel <mario.huettel@gmx.net>
 *
 */

#include <gds-render/gds-utils/gds-statistics.h>
#include <stdio.h>

/**
 * @addtogroup GDS-Utilities
 * @{
 */

#define MAX_RECURSION_DEPTH (1024)

static void calculate_vertex_gfx_count_cell(struct gds_cell *cell, unsigned int recursion_depth)
{
	GList *cell_iter;
	struct gds_cell_instance *cell_ref;
	struct gds_cell *sub_cell;

	g_return_if_fail(cell);

	/* Check if cell has already been calculated */
	if (cell->stats.total_gfx_count && cell->stats.total_vertex_count) {
		/* Return. This cell and all of its subcells have been calculated */
		return;
	}

	/* Update with own vertex / GFX count */
	cell->stats.total_vertex_count = cell->stats.vertex_count;
	cell->stats.total_gfx_count = cell->stats.gfx_count;

	/* Do not analyze further, if maximum recursion depth is reached */
	if (!recursion_depth)
		return;

	for (cell_iter = cell->child_cells; cell_iter; cell_iter = g_list_next(cell_iter)) {
		/* Scan all subcells recursively, if there are any */

		cell_ref = (struct gds_cell_instance *)cell_iter->data;
		sub_cell = (struct gds_cell *)cell_ref->cell_ref;

		calculate_vertex_gfx_count_cell(sub_cell, recursion_depth - 1);

		/* Increment count */
		cell->stats.total_vertex_count += sub_cell->stats.total_vertex_count;
		cell->stats.total_gfx_count += sub_cell->stats.total_vertex_count;
	}

}


void gds_statistics_calc_cummulative_counts_in_lib(struct gds_library *lib)
{
	GList *cell_iter;
	struct gds_cell *cell;

	g_return_if_fail(lib);

	for (cell_iter = lib->cells; cell_iter; cell_iter = g_list_next(cell_iter)) {
		cell = (struct gds_cell *)cell_iter->data;
		calculate_vertex_gfx_count_cell(cell, MAX_RECURSION_DEPTH);
		lib->stats.vertex_count += cell->stats.vertex_count;
		lib->stats.cell_count++;
		lib->stats.gfx_count += cell->stats.gfx_count;
		lib->stats.reference_count += cell->stats.reference_count;
	}
}


/** @} */
