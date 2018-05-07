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

static struct gds_library * append_library(struct gds_library *curr_arr, unsigned int curr_count)
{
	unsigned int size = (curr_count +1) * sizeof(struct gds_library);
	struct gds_library *ptr = (struct gds_library *)realloc(curr_arr, size);
	if (ptr) {
		ptr[curr_count].cells_count = 0;
		ptr[curr_count].name[0] = 0;
	}
	return ptr;
}

static struct gds_graphics * append_graphics(struct gds_graphics *curr_arr, unsigned int curr_count)
{
	unsigned int size = (curr_count +1) * sizeof(struct gds_graphics);
	struct gds_graphics *ptr = (struct gds_graphics *)realloc(curr_arr, size);
	if (ptr != NULL) {
		ptr[curr_count].vertices_count = 0;
		ptr[curr_count].vertices = NULL;
	}
	return ptr;
}

static struct gds_cell * append_cell(struct gds_cell *curr_arr, unsigned int curr_count)
{
	unsigned int size = (curr_count +1) * sizeof(struct gds_cell);
	struct gds_cell *ptr = (struct gds_cell *)realloc(curr_arr, size);

	if (ptr != NULL) {
		ptr[curr_count].child_cells_count = 0;
		ptr[curr_count].graphic_objs_count = 0;
		ptr[curr_count].child_cells = NULL;
		ptr[curr_count].graphic_objs = NULL;
		ptr[curr_count].name[0] = 0;
	}

	return ptr;
}

static gds_cell_instance * append_cell_ref(struct gds_cell_instance* curr_arr, unsigned int curr_count)
{
	unsigned int size = (curr_count + 1) * sizeof(struct gds_cell_instance);
	struct gds_cell_instance *ptr = (struct gds_cell_instance *)realloc(curr_arr, size);
	if (ptr) {
		ptr[curr_count].cell_ref = NULL;
		ptr[curr_count].ref_name[0] = 0;
	}
	return ptr;

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


int parse_gds_from_file(const char *filename, struct gds_library **library_array, unsigned int *count)
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
	struct gds_library *lib_arr = NULL;
	struct gds_cell *cell_arr = NULL;
	unsigned int lib_arr_cnt = 0;
	unsigned int cell_arr_cnt = 0;

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
				lib_arr = append_library(lib_arr, lib_arr_cnt);
				lib_arr_cnt++;
				if (lib_arr == NULL) {
					GDS_ERROR("Allocating memory failed");
					run = -3;
					break;

				}
				printf("Entering Lib\n");
				current_lib = &(lib_arr[lib_arr_cnt-1]);
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
				current_lib->cells = cell_arr;
				current_lib->cells_count = cell_arr_cnt;

				current_lib = NULL;
				printf("Leaving Library\n");
				break;
			case BGNSTR:
				cell_arr = append_cell(cell_arr, cell_arr_cnt);
				cell_arr_cnt++;
				if (cell_arr == NULL) {
					GDS_ERROR("Allocating memory failed");
					run = -3;
					break;

				}
				printf("Entering Cell\n");
				current_cell = &(cell_arr[cell_arr_cnt-1]);
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
				current_cell->graphic_objs = append_graphics(current_cell->graphic_objs, current_cell->graphic_objs_count);
				current_cell->graphic_objs_count++;
				if (current_cell->graphic_objs == NULL) {
					GDS_ERROR("Memory allocation failed");
					run = -4;
					break;
				}
				current_graphics = &(current_cell->graphic_objs[current_cell->graphic_objs_count-1]);
				current_graphics->type = GRAPHIC_POLYGON;
				printf("\tEntering boundary\n");
				break;
			case SREF:
				if (current_cell == NULL) {
					GDS_ERROR("Path outside of cell");
					run = -3;
					break;
				}
				current_cell->child_cells = append_cell_ref(current_cell->child_cells, current_cell->child_cells_count);
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
				current_cell->graphic_objs = append_graphics(current_cell->graphic_objs, current_cell->graphic_objs_count);
				current_cell->graphic_objs_count++;
				if (current_cell->graphic_objs == NULL) {
					GDS_ERROR("Memory allocation failed");
					run = -4;
					break;
				}
				current_graphics = &(current_cell->graphic_objs[current_cell->graphic_objs_count-1]);
				current_graphics->type = GRAPHIC_PATH;
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


	*library_array = lib_arr;
	*count = lib_arr_cnt;
	return run;


}
