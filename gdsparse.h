/*
 * GDSII-Converter
 * Copyright (C) 2018  Mario Hüttel <mario.huettel@gmx.net>
 *
 * This file is part of GDSII-Converter.
 *
 * GDSII-Converter is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License.
 *
 * GDSII-Converter is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GDSII-Converter.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __GDSPARSE_H__
#define __GDSPARSE_H__

#include <stdint.h>
#include <glib.h>

#define CELL_NAME_MAX (100)

enum graphics_type {GRAPHIC_PATH = 0, GRAPHIC_POLYGON = 1};

struct gds_time_field {
		uint16_t year;
		uint16_t month;
		uint16_t day;
		uint16_t hour;
		uint16_t minute;
		uint16_t second;
};
struct gds_point {
		int x;
		int y;
};

struct gds_graphics {
		enum graphics_type type;
		GList *vertices;
		unsigned int path_width;
		int width_absolute;
		int16_t layer;
		uint16_t datatype;
};

struct gds_cell_instance {
		char ref_name[CELL_NAME_MAX];
		struct gds_cell *cell_ref;
		struct gds_point origin;
		int flipped;
		double angle;
		double magnification;
};

struct gds_cell {
		char name[CELL_NAME_MAX];
		struct gds_time_field mod_time;
		struct gds_time_field access_time;
		GList *child_cells;
		GList *graphic_objs;
};

struct gds_library {
		char name[CELL_NAME_MAX];
		struct gds_time_field time;
		double unit_to_meters;
		GList *cells;
		GList *cell_names;
};


int parse_gds_from_file(const char *filename, GList **library_array);
int clear_lib_list(GList **library_list);

#endif /* __GDSPARSE_H__ */
