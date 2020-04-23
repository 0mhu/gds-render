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
 * @file cell-geometrics.h
 * @brief Calculation of gds_cell geometrics
 * @author Mario Hüttel <mario.huettel@gmx.net>
 */

/**
 * @addtogroup geometric
 * @{
 */

#ifndef _CELL_GEOMETRICS_H_
#define _CELL_GEOMETRICS_H_

#include <gds-render/geometric/bounding-box.h>
#include <gds-render/gds-utils/gds-types.h>

/**
 * @brief Calculate bounding box of a gds cell.
 *
 * This function updates a given bounding box with the dimensions of a
 * gds_cell. Please note that the handling of path miter points is not complete yet.
 * If a path object is the putmost object of your cell at any edge,
 * the resulting bounding box might be the wrong size.
 *
 * @param box Resulting boundig box. Will be updated and not overwritten
 * @param cell Toplevel cell
 * @warning Handling of Path graphic objects not yet implemented correctly.
 */
void calculate_cell_bounding_box(union bounding_box *box, struct gds_cell *cell);

#endif /* _CELL_GEOMETRICS_H_ */

/** @} */
