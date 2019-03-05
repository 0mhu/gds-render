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
 * @file gds-tree-checker.c
 * @brief Checking functions of a cell tree
 *
 * This file contains cehcking functions for the GDS cell tree.
 * These functions include checks if all child references could be resolved,
 * and if the cell tree contains loops.
 *
 * @author Mario Hüttel <mario.huettel@gmx.net>
 */

/**
 * @addtogroup GDS-Utilities
 * @{
 */

#include "gds-tree-checker.h"

int gds_tree_check_cell_references(struct gds_library *lib)
{
	return 0;
}

int gds_tree_check_reference_loops(struct gds_library *lib)
{
	return 0;
}

/** @} */
