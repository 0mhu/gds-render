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
 * @file bounding-box.h
 * @brief Header for calculation of bounding boxes
 * @author Mario Hüttel <mario.huettel@gmx.net>
 */

/**
 * @addtogroup geometric
 * @{
 */

#ifndef _BOUNDING_BOX_H_
#define _BOUNDING_BOX_H_

#include <glib.h>
#include <gds-render/geometric/vector-operations.h>
#include <stdbool.h>

union bounding_box {
	/** Coordinate System is (y up | x right) */
	struct _vectors {
		struct vector_2d lower_left;
		struct vector_2d upper_right;
	} vectors;
	struct vector_2d vector_array[2];
};

typedef void (*conv_generic_to_vector_2d_t)(void *, struct vector_2d *);

void bounding_box_calculate_polygon(GList *vertices, conv_generic_to_vector_2d_t conv_func, union bounding_box *box);
void bounding_box_update_box(union bounding_box *destination, union bounding_box *update);
void bounding_box_prepare_empty(union bounding_box *box);
void bounding_box_update_point(union bounding_box *destination, conv_generic_to_vector_2d_t conv_func, void *pt);
void bounding_box_apply_transform(double scale, double rotation_deg, bool flip_at_x, union bounding_box *box);
void bounding_box_calculate_path_box(GList *vertices, double thickness, conv_generic_to_vector_2d_t conv_func, union bounding_box *box);

#endif /* _BOUNDING_BOX_H_ */

/** @} */