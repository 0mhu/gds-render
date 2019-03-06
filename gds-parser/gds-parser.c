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
 * @file gds-parser.c
 * @brief Implementation of the GDS-Parser
 * @author Mario Hüttel <mario.huettel@gmx.net>
 *
 * What's missing? - A lot:
 * Support for 4 Byte real
 * Support for pathtypes
 * Support for datatypes (only layer so far)
 * etc...
 */

/**
 * @addtogroup GDS-Utilities
 * @{
 */


#include "gds-parser.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <cairo.h>

/**
 * @brief Default units assumed for library.
 * @note This value is usually overwritten with the value defined in the library.
 */
#define GDS_DEFAULT_UNITS (10E-9)

#define GDS_ERROR(fmt, ...) printf("[PARSE_ERROR] " fmt "\n", ##__VA_ARGS__) /**< @brief Print GDS error*/
#define GDS_WARN(fmt, ...) printf("[PARSE_WARNING] " fmt "\n", ##__VA_ARGS__) /**< @brief Print GDS warning */

#if GDS_PRINT_DEBUG_INFOS
	#define GDS_INF(fmt, ...) printf(fmt, ##__VA_ARGS__) /**< @brief standard printf. But can be disabled in code */
#else
	#define GDS_INF(fmt, ...)
#endif
enum gds_record {
	INVALID = 0x0000,
	HEADER = 0x0002,
	BGNLIB = 0x0102,
	LIBNAME = 0x0206,
	UNITS = 0x0305,
	ENDLIB = 0x0400,
	BGNSTR = 0x0502,
	STRNAME = 0x0606,
	ENDSTR = 0x0700,
	BOUNDARY = 0x0800,
	PATH = 0x0900,
	SREF = 0x0A00,
	ENDEL = 0x1100,
	XY = 0x1003,
	MAG = 0x1B05,
	ANGLE = 0x1C05,
	SNAME = 0x1206,
	STRANS = 0x1A01,
	BOX = 0x2D00,
	LAYER = 0x0D02,
	WIDTH = 0x0F03,
	PATHTYPE = 0x2102
};

/**
 * @brief Name cell reference
 * @param cell_inst Cell reference
 * @param bytes Length of name
 * @param data Name
 * @return 0 if successful
 */
static int name_cell_ref(struct gds_cell_instance *cell_inst,
			 unsigned int bytes, char *data)
{
	int len;

	if (cell_inst == NULL) {
		GDS_ERROR("Naming cell ref with no opened cell ref");
		return -1;
	}
	data[bytes] = 0; // Append '0'
	len = (int)strlen(data);
	if (len > CELL_NAME_MAX-1) {
		GDS_ERROR("Cell name '%s' too long: %d\n", data, len);
		return -1;
	}

	/* else: */
	strcpy(cell_inst->ref_name, data);
	GDS_INF("\tCell referenced: %s\n", cell_inst->ref_name);

	return 0;
}

/**
 * @brief Convert GDS 8-byte real to double
 * @param data 8 Byte GDS real
 * @return result
 */
static double gds_convert_double(const char *data)
{
	bool sign_bit;
	int i;
	double ret_val;
	char current_byte;
	int bit = 0;
	int exponent;

	sign_bit = ((data[0] & 0x80) ? true : false);

	/* Check for real 0 */
	for (i = 0; i < 8; i++) {
		if (data[i] != 0)
			break;
		if (i == 7) {
			/* 7 bytes all 0 */
			return 0.0;
		}
	}

	/* Value is other than 0 */
	ret_val = 0.0;
	for (i = 8; i < 64; i++) {
		current_byte = data[i/8];
		bit = i % 8;
		/* isolate bit */
		if ((current_byte & (0x80 >> bit)))
			ret_val += pow(2, ((double)(-i+7)));

	}

	/* Parse exponent and sign bit */
	exponent = (int)(data[0] & 0x7F);
	exponent -= 64;
	ret_val *= pow(16, exponent) * (sign_bit == true ? -1 : 1);

	return ret_val;
}

/**
 * @brief Convert GDS INT32 to int
 * @param data Buffer containing the int
 * @return result
 */
static signed int gds_convert_signed_int(const char *data)
{
	int ret;

	if (!data) {
		GDS_ERROR("This should not happen");
		return 0;
	}

	ret =	(signed int)(((((int)data[0]) & 0xFF) << 24) |
			((((int)data[1]) & 0xFF) << 16) |
			(((int)(data[2]) & 0xFF) <<  8) |
			(((int)(data[3]) & 0xFF) <<  0));
	return ret;
}

/**
 * @brief Convert GDS INT16 to int16
 * @param data Buffer containing the INT16
 * @return result
 */
static int16_t gds_convert_signed_int16(const char *data)
{
	if (!data) {
		GDS_ERROR("This should not happen");
		return 0;
	}
	return (int16_t)((((int16_t)(data[0]) & 0xFF) <<  8) |
			(((int16_t)(data[1]) & 0xFF) <<  0));
}

/**
 * @brief Convert GDS UINT16 String to uint16
 * @param data Buffer containing the uint16
 * @return result
 */
static uint16_t gds_convert_unsigend_int16(const char *data)
{
	if (!data) {
		GDS_ERROR("This should not happen");
		return 0;
	}
	return (uint16_t)((((uint16_t)(data[0]) & 0xFF) <<  8) |
			(((uint16_t)(data[1]) & 0xFF) <<  0));
}

/**
 * @brief Append library to list
 * @param curr_list List containing gds_library elements. May be NULL.
 * @param library_ptr Return of newly created library.
 * @return Newly created list pointer
 */
static GList *append_library(GList *curr_list, struct gds_library **library_ptr)
{
	struct gds_library *lib;

	lib = (struct gds_library *)malloc(sizeof(struct gds_library));
	if (lib) {
		lib->cells = NULL;
		lib->name[0] = 0;
		lib->unit_in_meters = GDS_DEFAULT_UNITS; // Default. Will be overwritten
		lib->cell_names = NULL;
	} else
		return NULL;
	if (library_ptr)
		*library_ptr = lib;

	return g_list_append(curr_list, lib);
}

/**
 * @brief Append graphics to list
 * @param curr_list List containing gds_graphics elements. May be NULL
 * @param type Type of graphics
 * @param graphics_ptr newly created graphic is written here
 * @return new list pointer
 */
static GList *append_graphics(GList *curr_list, enum graphics_type type,
			      struct gds_graphics **graphics_ptr)
{
	struct gds_graphics *gfx;

	gfx = (struct gds_graphics *)malloc(sizeof(struct gds_graphics));
	if (gfx) {
		gfx->datatype = 0;
		gfx->layer = 0;
		gfx->vertices = NULL;
		gfx->width_absolute = 0;
		gfx->gfx_type = type;
		gfx->path_render_type = PATH_FLUSH;
	} else
		return NULL;

	if (graphics_ptr)
		*graphics_ptr = gfx;

	return g_list_append(curr_list, gfx);
}

/**
 * @brief Appends vertext List
 * @param curr_list List containing gds_point elements. May be NULL.
 * @param x x-coordinate of new point
 * @param y y-coordinate of new point
 * @return new Pointer to List.
 */
static GList *append_vertex(GList *curr_list, int x, int y)
{
	struct gds_point *vertex;

	vertex = (struct gds_point *)malloc(sizeof(struct gds_point));
	if (vertex) {
		vertex->x = x;
		vertex->y = y;
	} else
		return NULL;
	return g_list_append(curr_list, vertex);
}

/**
 * @brief append_cell Append a gds_cell to a list
 *
 * Usage similar to append_cell_ref().
 * @param curr_list List containing gds_cell elements. May be NULL
 * @param cell_ptr newly created cell
 * @return new pointer to list
 */
static GList *append_cell(GList *curr_list, struct gds_cell **cell_ptr)
{
	struct gds_cell *cell;

	cell = (struct gds_cell *)malloc(sizeof(struct gds_cell));
	if (cell) {
		cell->child_cells = NULL;
		cell->graphic_objs = NULL;
		cell->name[0] = 0;
		cell->parent_library = NULL;
		cell->checks.unresolved_child_count = GDS_CELL_CHECK_NOT_RUN;
		cell->checks.affected_by_reference_loop = GDS_CELL_CHECK_NOT_RUN;
	} else
		return NULL;
	/* return cell */
	if (cell_ptr)
		*cell_ptr = cell;

	return g_list_append(curr_list, cell);
}

/**
 * @brief Append a cell reference to the reference GList.
 *
 * Appends a new gds_cell_instance to \p curr_list and returns the new element via \p instance_ptr
 * @param curr_list List of gds_cell_instance elements. May be NULL
 * @param instance_ptr newly created element
 * @return new GList pointer
 */
static GList *append_cell_ref(GList *curr_list, struct gds_cell_instance **instance_ptr)
{
	struct gds_cell_instance *inst;

	inst = (struct gds_cell_instance *)
			malloc(sizeof(struct gds_cell_instance));
	if (inst) {
		inst->cell_ref = NULL;
		inst->ref_name[0] = 0;
		inst->magnification = 1;
		inst->flipped = 0;
		inst->angle = 0;
	} else
		return NULL;

	if (instance_ptr)
		*instance_ptr = inst;

	return g_list_append(curr_list, inst);
}

/**
 * @brief Name a gds_library
 * @param current_library Library to name
 * @param bytes Lenght of name
 * @param data Name
 * @return 0 if successful
 */
static int name_library(struct gds_library *current_library,
			unsigned int bytes, char *data)
{
	int len;

	if (current_library == NULL) {
		GDS_ERROR("Naming cell with no opened library");
		return -1;
	}

	data[bytes] = 0; // Append '0'
	len = (int)strlen(data);
	if (len > CELL_NAME_MAX-1) {
		GDS_ERROR("Library name '%s' too long: %d\n", data, len);
		return -1;
	}

	strcpy(current_library->name, data);
	GDS_INF("Named library: %s\n", current_library->name);

	return 0;
}

/**
 * @brief Names a gds_cell
 * @param cell Cell to name
 * @param bytes Length of name
 * @param data Name
 * @param lib Library in which \p cell is located
 * @return 0 id successful
 */
static int name_cell(struct gds_cell *cell, unsigned int bytes,
		     char *data, struct gds_library *lib)
{
	int len;

	if (cell == NULL) {
		GDS_ERROR("Naming library with no opened library");
		return -1;
	}
	data[bytes] = 0; // Append '0'
	len = (int)strlen(data);
	if (len > CELL_NAME_MAX-1) {
		GDS_ERROR("Cell name '%s' too long: %d\n", data, len);
		return -1;
	}

	strcpy(cell->name, data);
	GDS_INF("Named cell: %s\n", cell->name);

	/* Append cell name to lib's list of names */
	lib->cell_names = g_list_append(lib->cell_names, cell->name);

	return 0;
}

/**
 * @brief Search for cell reference \p gcell_ref in \p glibrary
 *
 * Search cell referenced by \p gcell_ref inside \p glibrary and update gds_cell_instance::cell_ref with found #gds_cell
 * @param gcell_ref gpointer cast of struct gds_cell_instance *
 * @param glibrary gpointer cast of struct gds_library *
 */
static void parse_reference_list(gpointer gcell_ref, gpointer glibrary)
{
	struct gds_cell_instance *inst = (struct gds_cell_instance *)gcell_ref;
	struct gds_library *lib = (struct gds_library *)glibrary;
	GList *cell_item;
	struct gds_cell *cell;

	GDS_INF("\t\t\tReference: %s: ", inst->ref_name);
	/* Find cell */
	for (cell_item = lib->cells; cell_item != NULL;
	     cell_item = cell_item->next) {

		cell = (struct gds_cell *)cell_item->data;
		/* Check if cell is found */
		if (!strcmp(cell->name, inst->ref_name)) {
			GDS_INF("found\n");
			/* update reference link */
			inst->cell_ref = cell;
			return;
		}
	}

	GDS_INF("MISSING!\n");
	GDS_WARN("referenced cell could not be found in library");
}

/**
 * @brief Scans cell references inside cell
 This function searches all the references in \p gcell and updates the gds_cell_instance::cell_ref field in each instance
 * @param gcell pointer cast of #gds_cell *
 * @param library Library where the cell references are searched in
 */
static void scan_cell_reference_dependencies(gpointer gcell, gpointer library)
{
	struct gds_cell *cell = (struct gds_cell *)gcell;

	GDS_INF("\tScanning cell: %s\n", cell->name);

	/* Scan all library references */
	g_list_foreach(cell->child_cells, parse_reference_list, library);

}

/**
 * @brief Scans library's cell references
 *
 * This function searches all the references between cells and updates the gds_cell_instance::cell_ref field in each instance
 * @param library_list_item List containing #gds_library elements
 * @param user not used
 */
static void scan_library_references(gpointer library_list_item, gpointer user)
{
	struct gds_library *lib = (struct gds_library *)library_list_item;
	(void)user;

	GDS_INF("Scanning Library: %s\n", lib->name);
	g_list_foreach(lib->cells, scan_cell_reference_dependencies, lib);
}

/**
 * @brief gds_parse_date
 * @param buffer Buffer that contains the GDS Date field
 * @param length Length of \p buffer
 * @param mod_date Modification Date
 * @param access_date Last Access Date
 */
static void gds_parse_date(const char *buffer, int length, struct gds_time_field *mod_date, struct gds_time_field *access_date)
{

	struct gds_time_field *temp_date;

	if (!access_date || !mod_date) {
		GDS_WARN("Date structures invalid");
		return;
	}

	if (length != (2*6*2)) {
		GDS_WARN("Could not parse date field! Not the specified length");
		return;
	}

	for (temp_date = mod_date; 1; temp_date = access_date) {
		temp_date->year = gds_convert_unsigend_int16(buffer);
		buffer += 2;
		temp_date->month = gds_convert_unsigend_int16(buffer);
		buffer += 2;
		temp_date->day = gds_convert_unsigend_int16(buffer);
		buffer += 2;
		temp_date->hour = gds_convert_unsigend_int16(buffer);
		buffer += 2;
		temp_date->minute = gds_convert_unsigend_int16(buffer);
		buffer += 2;
		temp_date->second = gds_convert_unsigend_int16(buffer);
		buffer += 2;

		if (temp_date == access_date)
			break;
	}
}

int parse_gds_from_file(const char *filename, GList **library_list)
{
	char *workbuff;
	int read;
	int i;
	int run = 1;
	FILE *gds_file = NULL;
	uint16_t rec_data_length;
	enum gds_record rec_type;
	struct gds_library *current_lib = NULL;
	struct gds_cell *current_cell = NULL;
	struct gds_graphics *current_graphics = NULL;
	struct gds_cell_instance *current_s_reference = NULL;
	int x, y;
	////////////
	GList *lib_list;

	lib_list = *library_list;

	/* Allocate working buffer */
	workbuff = (char *)malloc(sizeof(char)*128*1024);

	if(!workbuff)
		return -100;

	/* open File */
	gds_file = fopen(filename, "rb");
	if (gds_file == NULL) {
		GDS_ERROR("Could not open File %s", filename);
		return -1;
	}

	/* Record parser */
	while (run == 1) {
		rec_type = INVALID;
		read = fread(workbuff, sizeof(char), 2, gds_file);
		if (read != 2 && (current_cell != NULL ||
				  current_graphics != NULL ||
				  current_lib != NULL ||
				  current_s_reference != NULL)) {
			GDS_ERROR("End of File. with openend structs/libs");
			run = -2;
			break;
		} else if (read != 2) {
			/* EOF */
			run = 0;
			break;
		}

		rec_data_length = gds_convert_unsigend_int16(workbuff);

		if (rec_data_length < 4) {
			/* Possible Zero-Padding: */
			run = 0;
			GDS_WARN("Zero Padding detected!");
			if (current_cell != NULL ||
					current_graphics != NULL ||
					current_lib != NULL ||
					current_s_reference != NULL) {
				GDS_ERROR("Not all structures closed");
				run = -2;
			}
			break;
		}
		rec_data_length -= 4;

		read = fread(workbuff, sizeof(char), 2, gds_file);
		if (read != 2) {
			run = -2;
			GDS_ERROR("Unexpected end of file");
			break;
		}
		rec_type = gds_convert_unsigend_int16(workbuff);

		/* if begin: Allocate structures */
		switch (rec_type) {
		case BGNLIB:
			lib_list = append_library(lib_list, &current_lib);
			if (lib_list == NULL) {
				GDS_ERROR("Allocating memory failed");
				run = -3;
				break;

			}
			GDS_INF("Entering Lib\n");
			break;
		case ENDLIB:
			if (current_lib == NULL) {
				run = -4;
				GDS_ERROR("Closing Library with no opened library");
				break;
			}

			/* Check for open Cells */
			if (current_cell != NULL) {
				run = -4;
				GDS_ERROR("Closing Library with opened cells");
				break;
			}
			current_lib = NULL;
			GDS_INF("Leaving Library\n");
			break;
		case BGNSTR:
			if (current_lib == NULL) {
				GDS_ERROR("Defining Cell outside of library!\n");
				run = -4;
				break;
			}
			current_lib->cells = append_cell(current_lib->cells, &current_cell);
			if (current_lib->cells == NULL) {
				GDS_ERROR("Allocating memory failed");
				run = -3;
				break;
			}

			current_cell->parent_library = current_lib;

			GDS_INF("Entering Cell\n");
			break;
		case ENDSTR:
			if (current_cell == NULL) {
				run = -4;
				GDS_ERROR("Closing cell with no opened cell");
				break;
			}
			/* Check for open Elements */
			if (current_graphics != NULL || current_s_reference != NULL) {
				run = -4;
				GDS_ERROR("Closing cell with opened Elements");
				break;
			}
			current_cell = NULL;
			GDS_INF("Leaving Cell\n");
			break;
		case BOX:
		case BOUNDARY:
			if (current_cell == NULL) {
				GDS_ERROR("Boundary/Box outside of cell");
				run = -3;
				break;
			}
			current_cell->graphic_objs = append_graphics(current_cell->graphic_objs,
								     (rec_type == BOUNDARY ? GRAPHIC_POLYGON : GRAPHIC_BOX),
								     &current_graphics);
			if (current_cell->graphic_objs == NULL) {
				GDS_ERROR("Memory allocation failed");
				run = -4;
				break;
			}
			GDS_INF("\tEntering boundary/Box\n");
			break;
		case SREF:
			if (current_cell == NULL) {
				GDS_ERROR("Cell Reference outside of cell");
				run = -3;
				break;
			}
			current_cell->child_cells = append_cell_ref(current_cell->child_cells,
								    &current_s_reference);
			if (current_cell->child_cells == NULL) {
				GDS_ERROR("Memory allocation failed");
				run = -4;
				break;
			}

			GDS_INF("\tEntering reference\n");
			break;
		case PATH:
			if (current_cell == NULL) {
				GDS_ERROR("Path outside of cell");
				run = -3;
				break;
			}
			current_cell->graphic_objs = append_graphics(current_cell->graphic_objs,
								     GRAPHIC_PATH, &current_graphics);
			if (current_cell->graphic_objs == NULL) {
				GDS_ERROR("Memory allocation failed");
				run = -4;
				break;
			}
			GDS_INF("\tEntering Path\n");
			break;
		case ENDEL:
			if (current_graphics != NULL) {

				GDS_INF("\tLeaving %s\n", (current_graphics->gfx_type == GRAPHIC_POLYGON ? "boundary" : "path"));
				current_graphics = NULL;
			}
			if (current_s_reference != NULL) {
				GDS_INF("\tLeaving Reference\n");
				current_s_reference = NULL;
			}
			break;
		case XY:
			if (current_graphics) {

			} else if (current_s_reference) {
				if (rec_data_length != 8) {
					GDS_WARN("Instance has weird coordinates. Rendered output might be screwed!");
				}
			}

			break;

		case MAG:
			break;
		case ANGLE:
			break;
		case STRANS:
			break;
		case WIDTH:
			break;
		case PATHTYPE:
			break;
		case UNITS:
			break;
		default:
			//GDS_WARN("Record: %04x, len: %u", (unsigned int)rec_type, (unsigned int)rec_data_length);
			break;
		} /* switch(rec_type) */


		/* No Data -> No Processing, go back to top */
		if (!rec_data_length || run != 1) continue;

		read = fread(workbuff, sizeof(char), rec_data_length, gds_file);

		if (read != rec_data_length) {
			GDS_ERROR("Could not read enough data: requested: %u, read: %u | Type: 0x%04x\n",
				  (unsigned int)rec_data_length, (unsigned int)read, (unsigned int)rec_type);
			run = -5;
			break;
		}

		switch (rec_type) {

		case HEADER:
		case ENDLIB:
		case ENDSTR:
		case BOUNDARY:
		case PATH:
		case SREF:
		case ENDEL:
		case BOX:
		case INVALID:
			break;

		case UNITS:
			if (!current_lib) {
				GDS_WARN("Units defined outside of library!\n");
				break;
			}

			if (rec_data_length != 16) {
				GDS_WARN("Unit define incomplete. Will assume database unit of %E meters\n", current_lib->unit_in_meters);
				break;
			}

			current_lib->unit_in_meters = gds_convert_double(&workbuff[8]);
			GDS_INF("Length of database unit: %E meters\n", current_lib->unit_in_meters);
			break;
		case BGNLIB:
			/* Parse date record */
			gds_parse_date(workbuff, read, &current_lib->mod_time, &current_lib->access_time);
			break;
		case BGNSTR:
			gds_parse_date(workbuff, read, &current_cell->mod_time, &current_cell->access_time);
			break;
		case LIBNAME:
			name_library(current_lib, (unsigned int)read, workbuff);
			break;
		case STRNAME:
			name_cell(current_cell, (unsigned int)read, workbuff, current_lib);
			break;
		case XY:
			if (current_s_reference) {
				/* Get origin of reference */
				current_s_reference->origin.x = gds_convert_signed_int(workbuff);
				current_s_reference->origin.y = gds_convert_signed_int(&workbuff[4]);
				GDS_INF("\t\tSet origin to: %d/%d\n", current_s_reference->origin.x,
				       current_s_reference->origin.y);
			} else if (current_graphics) {
				for (i = 0; i < read/8; i++) {
					x = gds_convert_signed_int(&workbuff[i*8]);
					y = gds_convert_signed_int(&workbuff[i*8+4]);
					current_graphics->vertices =
							append_vertex(current_graphics->vertices, x, y);
					GDS_INF("\t\tSet coordinate: %d/%d\n", x, y);

				}
			}
			break;
		case STRANS:
			if (!current_s_reference) {
				GDS_ERROR("Transformation defined outside of instance");
				break;
			}
			current_s_reference->flipped = ((workbuff[0] & 0x80) ? 1 : 0);
			break;

		case SNAME:
			if (current_s_reference) {
				name_cell_ref(current_s_reference, (unsigned int)read, workbuff);
			} else {
				GDS_ERROR("reference name set outside of cell reference.\n");
			}
			break;
		case WIDTH:
			if (!current_graphics) {
				GDS_WARN("Width defined outside of path element");
			}
			current_graphics->width_absolute = gds_convert_signed_int(workbuff);
			break;
		case LAYER:
			if (!current_graphics) {
				GDS_WARN("Layer has to be defined inside graphics object. Probably unknown object. Implement it yourself!");
				break;
			}
			current_graphics->layer = gds_convert_signed_int16(workbuff);
			if (current_graphics->layer < 0) {
				GDS_WARN("Layer negative!\n");
			}
			GDS_INF("\t\tAdded layer %d\n", (int)current_graphics->layer);
			break;
		case MAG:
			if (rec_data_length != 8) {
				GDS_WARN("Magnification is not an 8 byte real. Results may be wrong");
			}
			if (current_graphics != NULL && current_s_reference != NULL) {
				GDS_ERROR("Open Graphics and Cell Reference\n\tMissing ENDEL?");
				run = -6;
				break;
			}
			if (current_s_reference != NULL) {
				current_s_reference->magnification = gds_convert_double(workbuff);
				GDS_INF("\t\tMagnification defined: %lf\n", current_s_reference->magnification);
			}
			break;
		case ANGLE:
			if (rec_data_length != 8) {
				GDS_WARN("Angle is not an 8 byte real. Results may be wrong");
			}
			if (current_graphics != NULL && current_s_reference != NULL) {
				GDS_ERROR("Open Graphics and Cell Reference\n\tMissing ENDEL?");
				run = -6;
				break;
			}
			if (current_s_reference != NULL) {
				current_s_reference->angle = gds_convert_double(workbuff);
				GDS_INF("\t\tAngle defined: %lf\n", current_s_reference->angle);
			}
			break;
		case PATHTYPE:
			if (current_graphics == NULL) {
				GDS_WARN("Path type defined outside of path. Ignoring");
				break;
			}
			if (current_graphics->gfx_type == GRAPHIC_PATH) {
				current_graphics->path_render_type = (enum path_type)gds_convert_signed_int16(workbuff);
				GDS_INF("\t\tPathtype: %d\n", current_graphics->path_render_type);
			} else {
				GDS_WARN("Path type defined inside non-path graphics object. Ignoring");
			}
			break;

		}

	} /* while(run == 1) */

	fclose(gds_file);

	if (!run) {
		/* Iterate and find references to cells */
		g_list_foreach(lib_list, scan_library_references, NULL);
	}

	*library_list = lib_list;

	free(workbuff);

	return run;
}

/**
 * @brief delete_cell_inst_element
 * @param cell_inst
 */
static void delete_cell_inst_element(struct gds_cell_instance *cell_inst)
{
	if (cell_inst)
		free(cell_inst);
}

/**
 * @brief delete_vertex
 * @param vertex
 */
static void delete_vertex(struct gds_point *vertex)
{
	if (vertex)
		free(vertex);
}

/**
 * @brief delete_graphics_obj
 * @param gfx
 */
static void delete_graphics_obj(struct gds_graphics *gfx)
{
	if (!gfx)
		return;

	g_list_free_full(gfx->vertices, (GDestroyNotify)delete_vertex);
	free(gfx);
}

/**
 * @brief delete_cell_element
 * @param cell
 */
static void delete_cell_element(struct gds_cell *cell)
{
	if (!cell)
		return;

	g_list_free_full(cell->child_cells, (GDestroyNotify)delete_cell_inst_element);
	g_list_free_full(cell->graphic_objs, (GDestroyNotify)delete_graphics_obj);
	free(cell);
}

/**
 * @brief delete_library_element
 * @param lib
 */
static void delete_library_element(struct gds_library *lib)
{
	if (!lib)
		return;

	g_list_free(lib->cell_names);
	g_list_free_full(lib->cells, (GDestroyNotify)delete_cell_element);
	free(lib);
}

int clear_lib_list(GList **library_list)
{
	if (!library_list)
		return 0;

	if (*library_list == NULL)
		return 0;

	g_list_free_full(*library_list, (GDestroyNotify)delete_library_element);
	*library_list = NULL;
	return 0;
}

/** @} */
