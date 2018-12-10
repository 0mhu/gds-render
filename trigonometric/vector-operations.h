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
 * @file vector-operations.h
 * @brief Header for 2D Vector operations
 * @author Mario Hüttel <mario.huettel@gmx.net>
 */

/**
 * @addtogroup trigonometric
 * @{
 */


#ifndef _VECTOR_OPERATIONS_H_
#define _VECTOR_OPERATIONS_H_

#include <math.h>

struct vector_2d {
    double x;
    double y;
};

#define DEG2RAD(a) ((a)*M_PI/180.0)

double vector_2d_scalar_multipy(struct vector_2d *a, struct vector_2d *b);
void vector_2d_normalize(struct vector_2d *vec);
void vector_2d_rotate(struct vector_2d *vec, double angle);
struct vector_2d *vector_2d_copy(struct vector_2d *opt_res, struct vector_2d *vec);
struct vector_2d *vector_2d_alloc(void);
void vector_2d_free(struct vector_2d *vec);
void vector_2d_scale(struct vector_2d *vec, double scale);
double vector_2d_abs(struct vector_2d *vec);
double vector_2d_calculate_angle_between(struct vector_2d *a, struct vector_2d *b);
void vector_2d_subtract(struct vector_2d *res, struct vector_2d *a, struct vector_2d *b);
void vector_2d_add(struct vector_2d *res, struct vector_2d *a, struct vector_2d *b);

#endif /* _VECTOR_OPERATIONS_H_ */

/** @} */
