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
 * @file cell-trigonometrics.c
 * @brief Calculation of gds_cell trigonometrics
 * @author Mario Hüttel <mario.huettel@gmx.net>
 */

#include "cell-trigonometrics.h"
#include <math.h>

/**
 * @addtogroup trigonometric
 * @{
 */

static void convert_gds_point_to_2d_vector(struct gds_point *pt, struct vector_2d *vector)
{
	vector->x = pt->x;
	vector->y = pt->y;
}

/**
 * @brief Update the given bounding box with the bounding box of a graphics element.
 * @param box box to update
 * @param gfx Graphics element
 */
static void update_box_with_gfx(union bounding_box *box, struct gds_graphics *gfx)
{
	union bounding_box current_box;

	bounding_box_prepare_empty(&current_box);

	switch (gfx->gfx_type) {
	case GRAPHIC_BOX:
		/* Expected fallthrough */
	case GRAPHIC_POLYGON:
		bounding_box_calculate_polygon(gfx->vertices,
							(conv_generic_to_vector_2d_t)&convert_gds_point_to_2d_vector,
							&current_box);
		break;
	case GRAPHIC_PATH:
		/*
		 * This is not implemented correctly.
		 * Please be aware if paths are the outmost elements of your cell.
		 * You might end up with a completely wrong calculated cell size.
		 */
		bounding_box_calculate_path_box(gfx->vertices, gfx->width_absolute,
							(conv_generic_to_vector_2d_t)&convert_gds_point_to_2d_vector,
							&current_box);
		break;
	default:
		/* Unknown graphics object. */
		/* Print error? Nah.. */
		break;
	}

	/* Update box with results */
	bounding_box_update_box(box, &current_box);
}

void calculate_cell_bounding_box(union bounding_box *box, struct gds_cell *cell)
{
	GList *gfx_list;
	struct gds_graphics *gfx;
	GList *sub_cell_list;
	struct gds_cell_instance *sub_cell;
	union bounding_box temp_box;

	if (!box || !cell)
		return;

	/* Update box with graphic elements */
	for (gfx_list = cell->graphic_objs; gfx_list != NULL; gfx_list = gfx_list->next) {
		gfx = (struct gds_graphics *)gfx_list->data;
		update_box_with_gfx(box, gfx);
	}

	/* Update bounding box with boxes of subcells */
	for (sub_cell_list = cell->child_cells; sub_cell_list != NULL;
						sub_cell_list = sub_cell_list->next) {
		sub_cell = (struct gds_cell_instance *)sub_cell_list->data;
		bounding_box_prepare_empty(&temp_box);
		/* Recursion Woohoo!!  This dies if your GDS is faulty and contains a reference loop */
		calculate_cell_bounding_box(&temp_box, sub_cell->cell_ref);

		/* Apply transformations */
		bounding_box_apply_transform(ABS(sub_cell->magnification), sub_cell->angle,
					     sub_cell->flipped, &temp_box);

		/* Move bounding box to origin */
		temp_box.vectors.lower_left.x += sub_cell->origin.x;
		temp_box.vectors.upper_right.x += sub_cell->origin.x;
		temp_box.vectors.lower_left.y += sub_cell->origin.y;
		temp_box.vectors.upper_right.y += sub_cell->origin.y;

		/* update the parent's box */
		bounding_box_update_box(box, &temp_box);
	}
}

/** @} */
