/*
 * GDSII-Converter
 * Copyright (C) 2019  Mario Hüttel <mario.huettel@gmx.net>
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
 * @file gds-tree-checker.h
 * @brief Checking functions of a cell tree (Header)
 * @author Mario Hüttel <mario.huettel@gmx.net>
 */

/**
 * @addtogroup GDS-Utilities
 * @{
 */

#ifndef _GDS_TREE_CHECKER_H_
#define _GDS_TREE_CHECKER_H_

#include "gds-types.h"

/**
 * @brief gds_tree_check_cell_references checks if all child cell references can be resolved in the given library
 *
 * This function will only mark cells that
 * directly contain unresolved references.
 *
 * If a cell contains a reference to a cell with unresolved references, it is not flagged.
 *
 * @param lib The GDS library to check
 * @return less than 0 if an error occured during processing; 0 if all child cells could be resolved;
 *         greater than zero if the processing was successful but not all cell references could be resolved.
 *         In this case the number of unresolved references is returned
 */
int gds_tree_check_cell_references(struct gds_library *lib);

/**
 * @brief gds_tree_check_reference_loops checks if the given library contains reference loops
 * @param lib GDS library
 * @return negative if an error occured, zero if there are no reference loops, else a positive number representing the number
 *         of affected cells
 */
int gds_tree_check_reference_loops(struct gds_library *lib);

#endif /* _GDS_TREE_CHECKER_H_ */

/** @} */
