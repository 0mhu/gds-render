#ifndef __GDS_TYPES_H__
#define __GDS_TYPES_H__

#include <stdint.h>
#include <glib.h>

#define CELL_NAME_MAX (100)

enum graphics_type {GRAPHIC_PATH = 0, GRAPHIC_POLYGON = 1};
enum path_type {PATH_FLUSH = 0, PATH_ROUNDED = 1, PATH_SQUARED = 2};

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
		enum graphics_type gfx_type;
		GList *vertices;
		enum path_type path_render_type;
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
		struct gds_time_field mod_time;
		struct gds_time_field access_time;
		double unit_to_meters;
		GList *cells;
		GList *cell_names;
};

#endif /* __GDS_TYPES_H__ */
