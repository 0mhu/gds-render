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
 * @addtogroup trigonometric
 * @{
 */

#ifndef _BOUNDING_BOX_H_
#define _BOUNDING_BOX_H_
#include <glib.h>
#include "vector-operations.h"

union bounding_box {
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

#endif /* _BOUNDING_BOX_H_ */

/** @} */
