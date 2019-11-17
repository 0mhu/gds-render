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

/**
 * @brief Calculate bounding box of polygon
 * @param vertices List of vertices that describe the polygon
 * @param conv_func Conversion function to convert vertices to vector_2d structs.
 * @param box Box to write to. This box is not updated! All previous data is discarded
 */
void bounding_box_calculate_polygon(GList *vertices, conv_generic_to_vector_2d_t conv_func, union bounding_box *box);

/**
 * @brief Update an exisitng bounding box with another one.
 * @param destination Target box to update
 * @param update Box to update the target with
 */
void bounding_box_update_box(union bounding_box *destination, union bounding_box *update);

/**
 * @brief Prepare an empty bounding box.
 *
 * Updating this specially prepared box, results in a bounding box that is the same size as the update
 *
 * @param box Box to preapre
 */
void bounding_box_prepare_empty(union bounding_box *box);

/**
 * @brief Update bounding box with a point
 * @param destination Bounding box to update
 * @param conv_func Conversion function to convert \p pt to a vector_2d. May be NULL
 * @param pt Point to update bounding box with
 */
void bounding_box_update_point(union bounding_box *destination, conv_generic_to_vector_2d_t conv_func, void *pt);

/**
 * @brief Return all four corner points of a bounding box
 * @param[out] points Array of 4 vector_2d structs that has to be allocated by the caller
 * @param box Bounding box
 */
void bounding_box_get_all_points(struct vector_2d *points, union bounding_box *box);

/**
 * @brief Apply transformations onto bounding box.
 *
 * All corner points \f$ \vec{P_i} \f$ of the bounding box are transformed to output points \f$ \vec{P_o} \f$ by:
 *
 * \f$ \vec{P_o} = s \cdot \begin{pmatrix}\cos\left(\phi\right) & -\sin\left(\phi\right)\\ \sin\left(\phi\right) & \cos\left(\phi\right)\end{pmatrix} \cdot \begin{pmatrix} 1 & 0 \\ 0 & -1^{m} \end{pmatrix} \cdot \vec{P_i} \f$, with:
 *
 * * \f$s\f$: Scale
 * * \f$m\f$: 1, if flipped_at_x is True, else 0
 * * \f$\phi\f$: Rotation angle in radians. The conversion degrees => radians is done internally
 *
 * The result is the bounding box generated around all output points
 *
 * @param scale Scaling factor
 * @param rotation_deg Rotation of bounding box around the origin in degrees (counterclockwise)
 * @param flip_at_x Flip the boundig box on the x axis before rotating.
 * @param box Bounding box the operations should be applied to.
 * @note Keep in mind, that this bounding boxy is actually the bounding box of the rotated boundig box and not the object itself.
 *       It might be too big.
 */
void bounding_box_apply_transform(double scale, double rotation_deg, bool flip_at_x, union bounding_box *box);

/**
 * @brief Calculate the bounding box of a path and update the given bounding box
 * @param vertices Vertices the path is made up of
 * @param thickness Thisckness of the path
 * @param conv_func Conversion function for vertices to vector_2d structs
 * @param box Bounding box to write results in.
 */
void bounding_box_update_with_path(GList *vertices, double thickness, conv_generic_to_vector_2d_t conv_func, union bounding_box *box);

#endif /* _BOUNDING_BOX_H_ */

/** @} */
