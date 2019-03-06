/*
 * GDSII-Converter
 * Copyright (C) 2019  Mario Hüttel <mario.huettel@gmx.net>
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
 * @file gds-tree-checker.c
 * @brief Checking functions of a cell tree
 *
 * This file contains cehcking functions for the GDS cell tree.
 * These functions include checks if all child references could be resolved,
 * and if the cell tree contains loops.
 *
 * @author Mario Hüttel <mario.huettel@gmx.net>
 */

/**
 * @addtogroup GDS-Utilities
 * @{
 */

#include "gds-tree-checker.h"
#include <stdio.h>

int gds_tree_check_cell_references(struct gds_library *lib)
{
	GList *cell_iter;
	struct gds_cell *cell;
	GList *instance_iter;
	struct  gds_cell_instance *cell_inst;
	int total_unresolved_count = 0;

	if (!lib)
		return -1;

	/* Iterate over all cells in library */
	for (cell_iter = lib->cells; cell_iter != NULL; cell_iter = g_list_next(cell_iter)) {
		cell = (struct gds_cell *)cell_iter->data;

		/* Check if this list element is broken. This should never happen */
		if (!cell) {
			fprintf(stderr, "Broken cell list item found. Will continue.\n");
			continue;
		}

		/* Reset the unresolved cell reference counter to 0 */
		cell->checks.unresolved_child_count = 0;

		/* Iterate through all child cell references and check if the references are set */
		for (instance_iter = cell->child_cells; instance_iter != NULL;
					instance_iter = g_list_next(instance_iter)) {
			cell_inst = (struct gds_cell_instance *)instance_iter->data;

			/* Check if broken. This should not happen */
			if (!cell_inst) {
				fprintf(stderr, "Broken cell list item found in cell %s. Will continue.\n",
						cell->name);
				continue;
			}

			/* Check if instance is valid; else increment "error" counter of cell */
			if (!cell_inst->cell_ref) {
				total_unresolved_count++;
				cell->checks.unresolved_child_count++;
			}
		}
	}

	return total_unresolved_count;
}

/**
 * @brief This function sets the marker element in the check structure of each cell to zero.
 * @param lib Library to work with
 * @return 0 if successful; negative if a fault (null pointer, ...) occured.
 */
static int gds_tree_check_clear_cell_check_marker(struct gds_library *lib)
{
	GList *cell_iter;
	struct gds_cell *cell;

	if (!lib)
		return -1;

	for (cell_iter = lib->cells; cell_iter != NULL; cell_iter = g_list_next(cell_iter)) {
		cell = (struct gds_cell *)cell_iter->data;

		if (!cell)
			return -2;

		cell->checks._internal.marker = 0;
	}
}

int gds_tree_check_reference_loops(struct gds_library *lib)
{
	return 0;
}

/** @} */
