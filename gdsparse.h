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
