#include "gdsparse.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define GDS_ERROR(fmt, ...) printf("[PARSE_ERROR] " fmt "\n", #__VA_ARGS__)
#define GDS_WARN(fmt, ...) printf("[PARSE_WARNING] " fmt "\n", #__VA_ARGS__)
enum parsing_state {PARSING_LENGTH = 0, PARSING_TYPE, PARSING_DAT};
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
	ENDEL = 0x1100
};

static GList * append_library(GList *curr_list)
{
	struct gds_library *lib;

	lib = (struct gds_library *)malloc(sizeof(struct gds_library));
	if(lib) {
		lib->cells = NULL;
		lib->name[0] = 0;
		lib->unit_to_meters = 1; // Default. Will be overwritten
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

static int name_cell(struct gds_cell *cell, unsigned int bytes, char* data) {
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
	}
	return 0;

}


int parse_gds_from_file(const char *filename, GList **library_list)
{
	char workbuff[1024];
	int read;
	int run = 1;
	FILE *gds_file = NULL;
	uint16_t rec_data_length;
	enum record rec_type;
	enum parsing_state state = PARSING_LENGTH;
	struct gds_library *current_lib = NULL;
	struct gds_cell *current_cell = NULL;
	struct gds_graphics *current_graphics = NULL;
	struct gds_cell_instance *current_s_reference = NULL;
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
		switch (state) {
		case PARSING_LENGTH:
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
			state = PARSING_TYPE;
			break;
		case PARSING_TYPE:
			read = fread(workbuff, sizeof(char), 2, gds_file);
			if (read != 2) {
				run = -2;
				GDS_ERROR("Unexpected end of file");
				break;
			}
			rec_type = (uint16_t)((((uint16_t)(workbuff[0])) << 8) |
					(uint16_t)(workbuff[1]));
			state = PARSING_DAT;

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
				break;

			}
			break;

		case PARSING_DAT:
			read = fread(workbuff, sizeof(char), rec_data_length, gds_file);
			state = PARSING_LENGTH;

			if (read != rec_data_length) {
				GDS_ERROR("Could not read enough data");
				run = -5;
				break;
			}
			/* No Data -> No Processing */
			if (!read) break;

			switch (rec_type) {
			case LIBNAME:
				name_library(current_lib, read, workbuff);
				break;
			case STRNAME:
				name_cell(current_cell, read, workbuff);
				break;

			}
			break;

		}
	}

	fclose(gds_file);


	/* Iterate and find references to cells */


	*library_list = lib_list;
	return run;


}
