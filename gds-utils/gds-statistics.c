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

void gds_statistics_calc_cell(const struct gds_cell *cell, struct gds_cell_statistics *stat)
{
	GList *iter;
	GList *vertex_iter;
	struct gds_graphics *gfx;

	if (!stat) {
		fprintf(stderr, "Internal pointer error.\n");
		return;
	}

	stat->cell = NULL;
	stat->gfx_count = 0;
	stat->reference_count = 0;
	stat->vertex_count = 0;

	if (!cell) {
		fprintf(stderr, "Internal pointer error.\n");
		return;

	}

	stat->cell = cell;

	/* Sum up references */
	for (iter = cell->child_cells; iter; iter = g_list_next(iter))
		stat->reference_count++;

	/* Sum up graphics objects and vertices */
	for (iter = cell->graphic_objs; iter; iter = g_list_next(iter)) {
		gfx = (struct gds_graphics *)iter->data;
		stat->gfx_count++;
		for (vertex_iter = gfx->vertices; vertex_iter; vertex_iter = g_list_next(vertex_iter)) {
			stat->vertex_count++;
		}
	}
}

void gds_statistics_calc_library(GList *library_list, GList **lib_stat_list)
{

	GList *lib_iter;
	GList *cell_iter;
	struct gds_lib_statistics *lib_stats;
	struct gds_cell_statistics *cell_stats;
	struct gds_library *gds_lib;
	struct gds_cell *cell;

	/* Go through each library/cell and generate statistics */
	for (lib_iter = library_list; lib_iter; lib_iter = g_list_next(lib_iter)) {
		gds_lib = (struct gds_library *)lib_iter->data;
		lib_stats = (struct gds_lib_statistics *)malloc(sizeof(struct gds_lib_statistics));
		if (!lib_stats) {
			g_error("Failed allocating memory");
		}

		lib_stats->library = gds_lib;
		lib_stats->gfx_count = 0;
		lib_stats->cell_count = 0;
		lib_stats->reference_count = 0;
		lib_stats->vertex_count = 0;
		lib_stats->cell_statistics = NULL;

		for (cell_iter = gds_lib->cells; cell_iter; cell_iter = g_list_next(cell_iter)) {
			cell = (struct gds_cell *)cell_iter->data;
			cell_stats = (struct gds_cell_statistics *)malloc(sizeof(struct gds_cell_statistics));
			lib_stats->cell_count++;
			gds_statistics_calc_cell(cell, cell_stats);
			lib_stats->gfx_count += cell_stats->gfx_count;
			lib_stats->reference_count += cell_stats->reference_count;
			lib_stats->vertex_count += cell_stats->vertex_count;
			lib_stats->cell_statistics = g_list_append(lib_stats->cell_statistics, cell_stats);
		}

		*lib_stat_list = g_list_append(*lib_stat_list, lib_stats);
	} /* for lib */
}

static void free_stat_object(gpointer stat_obj)
{
	if (stat_obj)
		free(stat_obj);
}

void gds_statistics_free_lib_stat_list(GList **lib_stat_list)
{
	GList *lib_iter;
	struct gds_lib_statistics *lib_stats;

	g_return_if_fail(lib_stat_list);
	g_return_if_fail(*lib_stat_list);

	for (lib_iter = *lib_stat_list; lib_iter; lib_iter = g_list_next(lib_iter)) {
		lib_stats = (struct gds_lib_statistics *) lib_iter->data;
		g_list_free_full(lib_stats->cell_statistics,
				 free_stat_object);
	}
	g_list_free_full(*lib_stat_list, free_stat_object);
	*lib_stat_list = NULL;
}


/** @} */
