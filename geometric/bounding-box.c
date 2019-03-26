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

/**
 * @addtogroup geometric
 * @{
 */

#include <stdio.h>
#include <math.h>

#include <gds-render/geometric/bounding-box.h>

#define MIN(a,b) (((a) < (b)) ? (a) : (b)) /**< @brief Return smaller number */
#define MAX(a,b) (((a) > (b)) ? (a) : (b)) /**< @brief Return bigger number */
#define ABS_DBL(a) ((a) < 0 ? -(a) : (a))

void bounding_box_calculate_polygon(GList *vertices, conv_generic_to_vector_2d_t conv_func, union bounding_box *box)
{
	double xmin = DBL_MAX, xmax = -DBL_MAX, ymin = DBL_MAX, ymax = -DBL_MAX;
	struct vector_2d temp_vec;
	GList *list_item;

	/* Check for errors */
	if (!conv_func || !box || !vertices)
		return;

	for (list_item = vertices; list_item != NULL; list_item = g_list_next(list_item)) {
		/* Convert generic vertex to vector_2d */
		if (conv_func)
			conv_func((void *)list_item->data, &temp_vec);
		else
			vector_2d_copy(&temp_vec, (struct vector_2d *)list_item->data);

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
	box->vectors.upper_right.x = -DBL_MAX;
	box->vectors.upper_right.y = -DBL_MAX;
}

static void calculate_path_miter_points(struct vector_2d *a, struct vector_2d *b, struct vector_2d *c,
					struct vector_2d *m1, struct vector_2d *m2, double width)
{
	double angle, angle_sin, u;
	struct vector_2d ba, bc, u_vec, v_vec, ba_norm;

	if (!a || !b || !c || !m1 || !m2)
		return;

	vector_2d_subtract(&ba, a, b);
	vector_2d_subtract(&bc, c, b);

	angle = vector_2d_calculate_angle_between(&ba, &bc);

	if (ABS_DBL(angle) < 0.05 || ABS_DBL(angle - M_PI) < 0.1) {
		/* Specail cases Don*/
		vector_2d_copy(&ba_norm, &ba);
		vector_2d_rotate(&ba_norm, DEG2RAD(90));
		vector_2d_normalize(&ba_norm);
		vector_2d_scale(&ba_norm, width/2.0);
		vector_2d_add(m1, b, &ba_norm);
		vector_2d_subtract(m2, b, &ba_norm);
		return;
	}
	angle_sin = sin(angle);
	u = width/(2*angle_sin);

	vector_2d_copy(&u_vec, &ba);
	vector_2d_copy(&v_vec, &bc);
	vector_2d_normalize(&u_vec);
	vector_2d_normalize(&v_vec);
	vector_2d_scale(&u_vec, u);
	vector_2d_scale(&v_vec, u);

	vector_2d_copy(m1, b);
	vector_2d_add(m1, m1, &u_vec);
	vector_2d_add(m1, m1, &v_vec);

	vector_2d_copy(m2, b);
	vector_2d_subtract(m2, m2, &u_vec);
	vector_2d_subtract(m2, m2, &v_vec);
}

void bounding_box_calculate_path_box(GList *vertices, double thickness,
					conv_generic_to_vector_2d_t conv_func, union bounding_box *box)
{
	GList *vertex_iterator;
	struct vector_2d pt;

	printf("Warning! Function bounding_box_calculate_path_box not yet implemented correctly!\n");

	if (!vertices || !box)
		return;

	for (vertex_iterator = vertices; vertex_iterator != NULL; vertex_iterator = g_list_next(vertex_iterator)) {

		if (conv_func != NULL)
			conv_func(vertex_iterator->data, &pt);
		else
			(void)vector_2d_copy(&pt, (struct vector_2d *)vertex_iterator->data);

		/* These are approximations.
		 * Used as long as miter point calculation is not fully implemented
		 */
		box->vectors.lower_left.x = MIN(box->vectors.lower_left.x, pt.x - thickness/2);
		box->vectors.lower_left.y = MIN(box->vectors.lower_left.y, pt.y - thickness/2);
		box->vectors.upper_right.x = MAX(box->vectors.upper_right.x, pt.x + thickness/2);
		box->vectors.upper_right.y = MAX(box->vectors.upper_right.y, pt.y + thickness/2);
	}
}

void bounding_box_update_point(union bounding_box *destination, conv_generic_to_vector_2d_t conv_func, void *pt)
{
	struct vector_2d point;

	if (!destination || !pt)
		return;

	if (conv_func)
		conv_func(pt, &point);
	else
		(void)vector_2d_copy(&point, (struct vector_2d *)pt);

	destination->vectors.lower_left.x = MIN(destination->vectors.lower_left.x, point.x);
	destination->vectors.lower_left.y = MIN(destination->vectors.lower_left.y, point.y);
	destination->vectors.upper_right.x = MAX(destination->vectors.upper_right.x, point.x);
	destination->vectors.upper_right.y = MAX(destination->vectors.upper_right.y, point.y);
}

/**
 * @brief Apply transformations onto bounding box.
 * @param scale Scaling factor
 * @param rotation_deg Roation of bounding box around the origin in degrees (counterclockwise)
 * @param flip_at_x Flip the boundig box on the x axis before rotating.
 * @param box Bounding box the operations should be applied to.
 */
void bounding_box_apply_transform(double scale, double rotation_deg, bool flip_at_x, union bounding_box *box)
{
	int i;

	/* Due to linearity, the order of the operations does not matter.
	 * flip must be applied before rotation as defined by the GDS format
	 */
	for (i = 0; i < 2; i++) {
		box->vector_array[i].y *= (flip_at_x ? -1 : 1);
		vector_2d_rotate(&box->vector_array[i], rotation_deg * M_PI / 180);
		vector_2d_scale(&box->vector_array[i], scale);
	}
}

/** @} */
