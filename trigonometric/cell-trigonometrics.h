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
 * @file cell-trigonometrics.h
 * @brief Calculation of gds_cell trigonometrics
 * @author Mario Hüttel <mario.huettel@gmx.net>
 */

/**
 * @addtogroup trigonometric
 * @{
 */

#ifndef _CELL_TRIGONOMETRICS_H_
#define _CELL_TRIGONOMETRICS_H_

#include "bounding-box.h"
#include "../gds-utils/gds-types.h"

/**
 * @brief calculate_cell_bounding_box Calculate bounding box of gds cell
 * @param box Resulting boundig box. Will be uüdated and not overwritten
 * @param cell toplevel cell
 * @warning Path handling not yet implemented correctly
 */
void calculate_cell_bounding_box(union bounding_box *box, struct gds_cell *cell);

#endif /* _CELL_TRIGONOMETRICS_H_ */

/** @} */
