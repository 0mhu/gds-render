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

#include <stdio.h>

#include <gds-render/gds-utils/gds-tree-checker.h>

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
 * @brief Check if list contains a cell
 * @param list GList to check. May be a null pointer
 * @param cell Cell to check for
 * @return 0 if cell is not in list. 1 if cell is in list
 */
static int gds_tree_check_list_contains_cell(GList *list, struct gds_cell *cell) {
	GList *iter;

	for (iter = list; iter != NULL; iter = g_list_next(iter)) {
		if ((struct gds_cell *)iter->data == cell)
			return 1;
	}

	return 0;
}

/**
 * @brief This function follows down the reference list of a cell and marks each visited subcell and detects loops
 * @param cell_to_check The cell to check for reference loops
 * @param visited_cells Pointer to list head. May be zero.
 * @return 0 if no loops exist; error in processing: <0; loop found: >0
 */
static int gds_tree_check_iterate_ref_and_check(struct gds_cell *cell_to_check, GList **visited_cells)
{
	GList *ref_iter;
	struct gds_cell_instance *ref;
	struct gds_cell *sub_cell;
	int res;

	if (!cell_to_check)
		return -1;

	/* Check if this cell is already contained in visited cells. This indicates a loop */
	if (gds_tree_check_list_contains_cell(*visited_cells, cell_to_check))
		return 1;

	/* Add cell to visited cell list */
	*visited_cells = g_list_append(*visited_cells, (gpointer)cell_to_check);

	/* Mark references and process sub cells */
	for (ref_iter = cell_to_check->child_cells; ref_iter != NULL; ref_iter = g_list_next(ref_iter)) {
		ref = (struct gds_cell_instance *)ref_iter->data;

		if (!ref)
			return -1;

		sub_cell = ref->cell_ref;

		/* If cell is not resolved, ignore. No harm there */
		if (!sub_cell)
			continue;

		res = gds_tree_check_iterate_ref_and_check(sub_cell, visited_cells);
		if (res < 0) {
			/* Error. return. */
			return -3;
		} else if (res > 0) {
			/* Loop in subcell found. Propagate to top */
			return 1;
		}
	}

	/* Remove cell from visted cells */
	*visited_cells = g_list_remove(*visited_cells, cell_to_check);

	/* No error found in this chain */
	return 0;
}

int gds_tree_check_reference_loops(struct gds_library *lib)
{
	int res;
	int loop_count = 0;
	GList *cell_iter;
	struct gds_cell *cell_to_check;
	GList *visited_cells = NULL;


	if (!lib)
		return -1;

	for (cell_iter = lib->cells; cell_iter != NULL; cell_iter = g_list_next(cell_iter)) {
		cell_to_check = (struct gds_cell *)cell_iter->data;

		/* A broken cell reference will be counted fatal in this case */
		if (!cell_to_check)
			return -2;

		/* iterate through references and check if loop exists */
		res = gds_tree_check_iterate_ref_and_check(cell_to_check, &visited_cells);

		if (visited_cells) {
			/* If  cell contains no loop, print error when list not empty.
			 * In case of a loop, it is completely normal that the list is not empty,
			 * due to the instant return from gds_tree_check_iterate_ref_and_check()
			 */
			if (res == 0)
				fprintf(stderr, "Visited cell list should be empty. This is a bug. Please report this.\n");
			g_list_free(visited_cells);
			visited_cells = NULL;
		}

		if (res < 0) {
			/* Error */
			return res;
		} else if (res > 0) {
			/* Loop found: increment loop count and flag cell */
			cell_to_check->checks.affected_by_reference_loop = 1;
			loop_count++;
		} else if (res == 0) {
			/* No error found for this cell */
			cell_to_check->checks.affected_by_reference_loop = 0;
		}

	}


	return loop_count;
}

/** @} */
