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
 * @brief calculate_cell_bounding_box Calculate bounding box of gds cell
 * @param box Resulting boundig box. Will be uüdated and not overwritten
 * @param cell Toplevel cell
 * @warning Path handling not yet implemented correctly.
 */
void calculate_cell_bounding_box(union bounding_box *box, struct gds_cell *cell);

#endif /* _CELL_GEOMETRICS_H_ */

/** @} */
