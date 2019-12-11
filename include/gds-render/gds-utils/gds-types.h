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
 * @file gds-types.h
 * @brief Defines types and macros used by the GDS-Parser
 * @author Mario Hüttel <mario.huettel@gmx.net>
 */

/**
  * @addtogroup GDS-Utilities
  * @{
  */

#ifndef __GDS_TYPES_H__
#define __GDS_TYPES_H__

#include <stdint.h>
#include <glib.h>

#define CELL_NAME_MAX (100) /**< @brief Maximum length of a gds_cell::name or a gds_library::name */

/* Maybe use the macros that ship with the compiler? */
#define MIN(a,b) (((a) < (b)) ? (a) : (b)) /**< @brief Return smaller number */
#define MAX(a,b) (((a) > (b)) ? (a) : (b)) /**< @brief Return bigger number */

/** @brief Defintion of check counter default value
 *  that indicates that the corresponding check has not yet been executed */
enum {GDS_CELL_CHECK_NOT_RUN = -1};

/** @brief Types of graphic objects */
enum graphics_type
{
		    GRAPHIC_PATH = 0, /**< @brief Path. Esentially a line */
		    GRAPHIC_POLYGON = 1, /**< @brief An arbitrary polygon */
		    GRAPHIC_BOX = 2 /**< @brief A rectangle. @warning Implementation in renderers might be buggy!*/
};

/**
 * @brief Defines the line caps of a path
 */
enum path_type {PATH_FLUSH = 0, PATH_ROUNDED = 1, PATH_SQUARED = 2}; /**< Path line caps */

/**
 * @brief A point in the 2D plane. Sometimes referred to as vertex
 */
struct gds_point {
	int x;
	int y;
};

/**
 * @brief Stores the result of the cell checks.
 */
struct gds_cell_checks {
	int unresolved_child_count; /**< @brief Number of unresolved cell instances inside this cell. Default: @ref GDS_CELL_CHECK_NOT_RUN */
	int affected_by_reference_loop; /**< @brief 1 if the cell is affected by a reference loop and therefore not renderable. Default: @ref GDS_CELL_CHECK_NOT_RUN*/
	/**
	 * @brief For the internal use of the checker.
	 * @warning Do not use this structure and its contents!
	 */
	struct _check_internals {
		int marker;
	} _internal;
};

/**
 * @brief Date information for cells and libraries
 */
struct gds_time_field {
	uint16_t year;
	uint16_t month;
	uint16_t day;
	uint16_t hour;
	uint16_t minute;
	uint16_t second;
};

/**
 * @brief A GDS graphics object
 */
struct gds_graphics {
	enum graphics_type gfx_type; /**< \brief Type of graphic */
	GList *vertices; /**< @brief List of #gds_point */
	enum path_type path_render_type; /**< @brief Line cap */
	int width_absolute; /**< @brief Width. Not used for objects other than paths */
	int16_t layer; /**< @brief Layer the graphic object is on */
	uint16_t datatype;
};

/**
 * @brief This represents an instanc of a cell inside another cell
 */
struct gds_cell_instance {
	char ref_name[CELL_NAME_MAX]; /**< @brief Name of referenced cell */
	struct gds_cell *cell_ref; /**< @brief Referenced gds_cell structure */
	struct gds_point origin; /**< @brief Origin */
	int flipped; /**< @brief Mirrored on x-axis before rotation */
	double angle; /**< @brief Angle of rotation (counter clockwise) in degrees */
	double magnification; /**< @brief magnification */
};

/**
 * @brief A Cell inside a gds_library
 */
struct gds_cell {
	char name[CELL_NAME_MAX];
	struct gds_time_field mod_time;
	struct gds_time_field access_time;
	GList *child_cells; /**< @brief List of #gds_cell_instance elements */
	GList *graphic_objs; /**< @brief List of #gds_graphics */
	struct gds_library *parent_library; /**< @brief Pointer to parent library */
	struct gds_cell_checks checks; /**< @brief Checking results */
};

/**
 * @brief GDS Toplevel library
 */
struct gds_library {
	char name[CELL_NAME_MAX];
	struct gds_time_field mod_time;
	struct gds_time_field access_time;
	double unit_in_meters;  /**< Length of a database unit in meters */
	GList *cells; /**< List of #gds_cell that contains all cells in this library*/
	GList *cell_names /**< List of strings that contains all cell names */;
};

/** @} */

#endif /* __GDS_TYPES_H__ */
