/*
 * GDSII converter
 * Copyright (C) 2018  Mario Hüttel <mario.huettel@gmx.net>
 *
 * This file is part of GDSII-Converter.
 *
 * GDSII-Converter is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License.
 *
 * GDSII-converter is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * What's missing? - A lot:
 * Support for Boxes
 * Support for 4 Byte real
 * Support for pathtypes
 * Support for datatypes (only layer so far)
 * etc...
 */


#include "gdsparse.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#define GDS_ERROR(fmt, ...) printf("[PARSE_ERROR] " fmt "\n",## __VA_ARGS__)
#define GDS_WARN(fmt, ...) printf("[PARSE_WARNING] " fmt "\n",## __VA_ARGS__)

enum record {
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
};

static int name_cell_ref(struct gds_cell_instance *cell_inst, unsigned int bytes, char* data)
{
	int len;

	if (cell_inst == NULL)
	{
		GDS_ERROR("Naming cell ref with no opened cell ref");
		return -1;
	}
	data[bytes] = 0; // Append '0'
	len = strlen(data);
	if (len > CELL_NAME_MAX-1) {
		GDS_ERROR("Cell name '%s' too long: %d\n", data, len);
		return -1;
	} else {
		strcpy(cell_inst->ref_name, data);
		printf("\tCell referenced: %s\n", cell_inst->ref_name);
	}
	return 0;

}

static double gds_convert_double(char *data)
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

static signed int gds_convert_signed_int(char *data)
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

static int16_t gds_convert_signed_int16(char *data)
{
	if (!data) {
		GDS_ERROR("This should not happen");
		return 0;
	}
	return (int16_t)((((int16_t)(data[0]) & 0xFF) <<  8) |
			(((int16_t)(data[1]) & 0xFF) <<  0));
}


static GList * append_library(GList *curr_list)
{
	struct gds_library *lib;

	lib = (struct gds_library *)malloc(sizeof(struct gds_library));
	if(lib) {
		lib->cells = NULL;
		lib->name[0] = 0;
		lib->unit_to_meters = 1; // Default. Will be overwritten
		lib->cell_names = NULL;
	} else
		return NULL;
	return g_list_append(curr_list, lib);
}

static GList * append_graphics(GList *curr_list, enum graphics_type type)
{
	struct gds_graphics *gfx;

	gfx = (struct gds_graphics *)malloc(sizeof(struct gds_graphics));
	if (gfx) {
		gfx->datatype = 0;
		gfx->layer = 0;
		gfx->vertices = NULL;
		gfx->width_absolute = 0;
		gfx->type = type;
	} else
		return NULL;
	return g_list_append(curr_list, gfx);

}

static GList * append_vertex(GList *curr_list, int x, int y)
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

static GList * append_cell(GList *curr_list)
{
	struct gds_cell *cell;

	cell = (struct gds_cell *)malloc(sizeof(struct gds_cell));
	if (cell) {
		cell->child_cells = NULL;
		cell->graphic_objs = NULL;
		cell->name[0] = 0;
	} else
		return NULL;

	return g_list_append(curr_list, cell);
}

static GList * append_cell_ref(GList *curr_list)
{
	struct gds_cell_instance *inst;

	inst = (struct gds_cell_instance *)malloc(sizeof(struct gds_cell_instance));
	if (inst) {
		inst->cell_ref = NULL;
		inst->ref_name[0] = 0;
		inst->magnification = 1;
		inst->flipped = 0;
		inst->angle = 0;
	} else
		return NULL;

	return g_list_append(curr_list, inst);
}

static int name_library(struct gds_library *current_library, unsigned int bytes, char* data) {
	int len;

	if (current_library == NULL)
	{
		GDS_ERROR("Naming cell with no opened library");
		return -1;
	}

	data[bytes] = 0; // Append '0'
	len = strlen(data);
	if (len > CELL_NAME_MAX-1) {
		GDS_ERROR("Library name '%s' too long: %d\n", data, len);
		return -1;
	} else {
		strcpy(current_library->name, data);
		printf("Named library: %s\n", current_library->name);
	}
	return 0;

}

static int name_cell(struct gds_cell *cell, unsigned int bytes, char* data, struct gds_library* lib) {
	int len;

	if (cell == NULL)
	{
		GDS_ERROR("Naming library with no opened library");
		return -1;
	}
	data[bytes] = 0; // Append '0'
	len = strlen(data);
	if (len > CELL_NAME_MAX-1) {
		GDS_ERROR("Cell name '%s' too long: %d\n", data, len);
		return -1;
	} else {
		strcpy(cell->name, data);
		printf("Named cell: %s\n", cell->name);

		/* Append cell name to lib's list of names */
		lib->cell_names = g_list_append(lib->cell_names, cell->name);

	}


	return 0;

}


void parse_reference_list(gpointer gcell_ref, gpointer glibrary)
{
	struct gds_cell_instance *inst = (struct gds_cell_instance *)gcell_ref;
	struct gds_library *lib = (struct gds_library *)glibrary;
	GList *cell_item;
	struct gds_cell *cell;

	printf("\t\t\tReference: %s: ", inst->ref_name);
	/* Find cell */
	for (cell_item = lib->cells; cell_item != NULL; cell_item = cell_item->next) {
		cell = (struct gds_cell *)cell_item->data;
		/* Check if cell is found */
		if (!strcmp(cell->name, inst->ref_name)) {
			printf("found\n");
			/* update reference link */
			inst->cell_ref = cell;
			return;
		}
	}

	printf("MISSING!\n");
	GDS_WARN("referenced cell could not be found in library");

}


void scan_cell_reference_dependencies(gpointer gcell, gpointer library)
{
	struct gds_cell *cell = (struct gds_cell *)gcell;
	printf("\tScanning cell: %s\n", cell->name);

	/* Scan all library references */
	g_list_foreach(cell->child_cells, parse_reference_list, library);

}


void scan_library_references(gpointer library_list_item, gpointer user)
{
	struct gds_library *lib = (struct gds_library *)library_list_item;

	printf("Scanning Library: %s\n", lib->name);
	g_list_foreach(lib->cells, scan_cell_reference_dependencies, lib);
}


int parse_gds_from_file(const char *filename, GList **library_list)
{
	char workbuff[1024];
	int read;
	int i;
	int run = 1;
	FILE *gds_file = NULL;
	uint16_t rec_data_length;
	enum record rec_type;
	struct gds_library *current_lib = NULL;
	struct gds_cell *current_cell = NULL;
	struct gds_graphics *current_graphics = NULL;
	struct gds_cell_instance *current_s_reference = NULL;
	int x, y;
	////////////
	GList *lib_list;

	lib_list = *library_list;

	/* open File */
	gds_file = fopen(filename, "r");
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

		rec_data_length = (uint16_t)((((uint16_t)(workbuff[0])) << 8) |
				(uint16_t)(workbuff[1]));

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
		rec_type = (uint16_t)((((uint16_t)(workbuff[0])) << 8) |
				(uint16_t)(workbuff[1]));

		/* if begin: Allocate structures */
		switch (rec_type) {
		case BGNLIB:
			lib_list = append_library(lib_list);
			if (lib_list == NULL) {
				GDS_ERROR("Allocating memory failed");
				run = -3;
				break;

			}
			printf("Entering Lib\n");
			current_lib = (struct gds_library *)g_list_last(lib_list)->data;
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
			printf("Leaving Library\n");
			break;
		case BGNSTR:
			current_lib->cells = append_cell(current_lib->cells);
			if (current_lib->cells == NULL) {
				GDS_ERROR("Allocating memory failed");
				run = -3;
				break;
			}
			printf("Entering Cell\n");
			current_cell = (struct gds_cell *)g_list_last(current_lib->cells)->data;
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
			printf("Leaving Cell\n");
			break;
			//case BOX:
		case BOUNDARY:
			if (current_cell == NULL) {
				GDS_ERROR("Boundary outside of cell");
				run = -3;
				break;
			}
			current_cell->graphic_objs = append_graphics(current_cell->graphic_objs, GRAPHIC_POLYGON);
			if (current_cell->graphic_objs == NULL) {
				GDS_ERROR("Memory allocation failed");
				run = -4;
				break;
			}
			current_graphics = (struct gds_graphics *) g_list_last(current_cell->graphic_objs)->data;
			printf("\tEntering boundary\n");
			break;
		case SREF:
			if (current_cell == NULL) {
				GDS_ERROR("Path outside of cell");
				run = -3;
				break;
			}
			current_cell->child_cells = append_cell_ref(current_cell->child_cells);
			if (current_cell->child_cells == NULL) {
				GDS_ERROR("Memory allocation failed");
				run = -4;
				break;
			}

			current_s_reference = (struct gds_cell_instance *) g_list_last(current_cell->child_cells)->data;
			printf("\tEntering reference\n");
			break;
		case PATH:
			if (current_cell == NULL) {
				GDS_ERROR("Path outside of cell");
				run = -3;
				break;
			}
			current_cell->graphic_objs = append_graphics(current_cell->graphic_objs, GRAPHIC_PATH);
			if (current_cell->graphic_objs == NULL) {
				GDS_ERROR("Memory allocation failed");
				run = -4;
				break;
			}
			current_graphics = (struct gds_graphics *) g_list_last(current_cell->graphic_objs)->data;
			printf("\tEntering Path\n");
			break;
		case ENDEL:
			if (current_graphics != NULL) {

				printf("\tLeaving %s\n", (current_graphics->type == GRAPHIC_POLYGON ? "boundary" : "path"));
				current_graphics = NULL;
			}
			if (current_s_reference != NULL) {
				printf("\tLeaving Reference\n");
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
		default:
			//GDS_WARN("Record: %04x, len: %u", (unsigned int)rec_type, (unsigned int)rec_data_length);
			break;
		} /* switch(rec_type) */


		/* No Data -> No Processing, go back to top */
		if (!rec_data_length) continue;

		read = fread(workbuff, sizeof(char), rec_data_length, gds_file);

		if (read != rec_data_length) {
			GDS_ERROR("Could not read enough data");
			run = -5;
			break;
		}

		switch (rec_type) {
		case LIBNAME:
			name_library(current_lib, read, workbuff);
			break;
		case STRNAME:
			name_cell(current_cell, read, workbuff, current_lib);
			break;
		case XY:
			if (current_s_reference) {
				/* Get origin of reference */
				current_s_reference->origin.x = gds_convert_signed_int(workbuff);
				current_s_reference->origin.y = gds_convert_signed_int(&workbuff[4]);
				printf("\t\tSet origin to: %d/%d\n", current_s_reference->origin.x,
				       current_s_reference->origin.y);
			} else if (current_graphics) {
				for (i = 0; i < read/8; i++) {
					x = gds_convert_signed_int(&workbuff[i*8]);
					y = gds_convert_signed_int(&workbuff[i*8+4]);
					current_graphics->vertices =
							append_vertex(current_graphics->vertices, x, y);
					printf("\t\tSet coordinate: %d/%d\n", x, y);

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
			name_cell_ref(current_s_reference, read, workbuff);
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
			printf("\t\tAdded layer %d\n", (int)current_graphics->layer);
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
				printf("\t\tMagnification defined: %lf\n", current_s_reference->magnification);
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
				printf("\t\tAngle defined: %lf\n", current_s_reference->angle);
			}
			break;

		}

	} /* while(run == 1) */

	fclose(gds_file);


	/* Iterate and find references to cells */
	g_list_foreach(lib_list, scan_library_references, NULL);

	*library_list = lib_list;
	return run;


}

static void delete_cell_inst_element(struct gds_cell_instance *cell_inst)
{
	free(cell_inst);
}

static void delete_cell_element(struct gds_cell *cell)
{
	g_list_free_full(cell->child_cells, (GDestroyNotify)delete_cell_inst_element);
	free(cell);
}

static void delete_library_element(struct gds_library *lib)
{
	g_list_free(lib->cell_names);
	g_list_free_full(lib->cells, (GDestroyNotify)delete_cell_element);
	free(lib);
}


int clear_lib_list(GList **library_list)
{
	if (*library_list == NULL) return 0;
	g_list_free_full(*library_list, (GDestroyNotify)delete_library_element);
	*library_list = NULL;
	return 0;
}
