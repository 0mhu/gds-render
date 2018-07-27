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
 * @file bounding-box.c
 * @brief Calculation of bounding boxes
 * @author Mario Hüttel <mario.huettel@gmx.net>
 */

#include <stdio.h>
#include "bounding-box.h"

#define MIN(a,b) (((a) < (b)) ? (a) : (b)) /**< @brief Return smaller number */
#define MAX(a,b) (((a) > (b)) ? (a) : (b)) /**< @brief Return bigger number */

void bounding_box_calculate_polygon(GList *vertices, conv_generic_to_vector_2d_t conv_func, union bounding_box *box)
{
	double xmin = DBL_MAX, xmax = DBL_MIN, ymin = DBL_MAX, ymax = DBL_MIN;
	struct vector_2d temp_vec;
	GList *list_item;

	/* Check for errors */
	if (!conv_func || !box || !vertices)
		return;

	for (list_item = vertices; list_item != NULL; list_item = g_list_next(list_item)) {
		/* Convert generic vertex to vector_2d */
		conv_func((void *)list_item->data, &temp_vec);

		/* Update bounding coordinates with vertex */
		xmin = MIN(xmin, temp_vec.x);
		xmax = MAX(xmax, temp_vec.x);
		ymin = MIN(ymin, temp_vec.y);
		ymax = MAX(ymax, temp_vec.y);
	}

	/* Fill bounding box with results */
	box->vectors.lower_left.x = xmin;
	box->vectors.lower_left.y = ymin;
	box->vectors.upper_right.x = xmax;
	box->vectors.upper_right.y = ymax;
}

void bounding_box_update_box(union bounding_box *destination, union bounding_box *update)
{
	if (!destination || !update)
		return;

	destination->vectors.lower_left.x = MIN(destination->vectors.lower_left.x,
						update->vectors.lower_left.x);
	destination->vectors.lower_left.y = MIN(destination->vectors.lower_left.y,
						update->vectors.lower_left.y);
	destination->vectors.upper_right.x = MAX(destination->vectors.upper_right.x,
						update->vectors.upper_right.x);
	destination->vectors.upper_right.y = MAX(destination->vectors.upper_right.y,
						update->vectors.upper_right.y);
}

void bounding_box_prepare_empty(union bounding_box *box)
{
	box->vectors.lower_left.x = DBL_MAX;
	box->vectors.lower_left.y = DBL_MAX;
	box->vectors.upper_right.x = DBL_MIN;
	box->vectors.upper_right.y = DBL_MIN;
}
