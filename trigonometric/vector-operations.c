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
 * @file vector-operations.c
 * @brief 2D Vector operations
 * @author Mario Hüttel <mario.huettel@gmx.net>
 */

#include "vector-operations.h"
#include <math.h>
#include <stdlib.h>

double vector_2d_scalar_multipy(struct vector_2d *a, struct vector_2d *b)
{
	if (a && b)
		return (a->x * b->x) + (a->y * b->y);
	else
		return 0.0;
}

void vector_2d_normaize(struct vector_2d *vec)
{
	double len;
	if (!vec)
		return;
	len = sqrt(pow(vec->x,2)+pow(vec->y,2));
	vec->x = vec->x/len;
	vec->y = vec->y/len;
}

void vector_2d_rotate(struct vector_2d *vec, double angle)
{
	double sin_val, cos_val;
	struct vector_2d temp;

	sin_val = sin(angle);
	cos_val = cos(angle);

	vecor_2d_copy(&temp, vec);

	/* Apply rotation matrix */
	vec->x = (cos_val * temp.x) - (sin_val * temp.y);
	vec->y = (sin_val * temp.x) + (cos_val * temp.y);
}

struct vector_2d *vecor_2d_copy(struct vector_2d *opt_res, struct vector_2d *vec)
{
	struct vector_2d *res;

	if (!vec)
		return NULL;
	if (opt_res) {
		opt_res->x = vec->x;
		opt_res->y = vec->y;
		return opt_res;
	} else {
		res = vector_2d_alloc();
		if (res) {
			res->x = vec->x;
			res->y = vec->y;
		}
		return res;
	}
}

struct vector_2d *vector_2d_alloc(void)
{
	return (struct vector_2d *)malloc(sizeof(struct vector_2d));
}

void vector_2d_free(struct vector_2d *vec)
{
	if (vec) {
		free(vec);
	}
}

void vector_2d_scale(struct vector_2d *vec, double scale)
{
	if (!vec)
		return;

	vec->x *= scale;
	vec->y *= scale;
}

double vector_2d_abs(struct vector_2d *vec)
{
	double len = 0.0;
	if (vec) {
		len = sqrt(pow(vec->x,2)+pow(vec->y,2));
	}
	return len;
}
