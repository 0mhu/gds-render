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
 * @file command-line.c
 * @brief Function to render according to command line parameters
 * @author Mario Hüttel <mario.huettel@gmx.net>
 */

/**
 * @addtogroup MainApplication
 * @{
 */

#include <stdio.h>
#include "command-line.h"
#include "gds-parser/gds-parser.h"
#include "mapping-parser.h"
#include "cairo-output/cairo-output.h"
#include "latex-output/latex-output.h"
#include "external-renderer.h"
#include "gds-parser/gds-tree-checker.h"

/**
 * @brief Delete layer_info and free nem element.
 *
 * Like delete_layer_info_struct() but also frees layer_info::name
 * @param info
 * @warning This function must not be used if the layer_info::name field references the internal storage strings if e.g. an entry field
 */
static void delete_layer_info_with_name(struct layer_info *info)
{
	if (info) {
		if (info->name)
			g_free(info->name);
		free(info);
	}
}

void command_line_convert_gds(char *gds_name, char *pdf_name, char *tex_name, gboolean pdf, gboolean tex,
			      char *layer_file, char *cell_name, double scale, gboolean pdf_layers,
			      gboolean pdf_standalone, gboolean svg, char *svg_name, char *so_name, char *so_out_file)
{
	GList *libs = NULL;
	FILE *tex_file;
	int res;
	GFile *file;
	int i;
	GFileInputStream *stream;
	GDataInputStream *dstream;
	gboolean layer_export;
	GdkRGBA layer_color;
	int layer;
	char *layer_name;
	GList *layer_info_list = NULL;
	GList *cell_list;
	struct layer_info *linfo_temp;
	struct gds_library *first_lib;
	struct gds_cell *toplevel_cell = NULL, *temp_cell;


	/* Check if parameters are valid */
	if (!gds_name || (!pdf_name && pdf)  || (!tex_name && tex) || !layer_file || !cell_name) {
		printf("Probably missing argument. Check --help option\n");
		return;
	}

	/* Load GDS */
	clear_lib_list(&libs);
	res = parse_gds_from_file(gds_name, &libs);
	if (res)
		goto ret_destroy_library_list;

	file = g_file_new_for_path(layer_file);
	stream = g_file_read(file, NULL, NULL);

	if (!stream) {
		printf("Layer mapping not readable!\n");
		goto ret_destroy_file;
	}
	dstream = g_data_input_stream_new(G_INPUT_STREAM(stream));
	i = 0;
	do {
		res = load_csv_line(dstream, &layer_export, &layer_name, &layer, &layer_color);
		if (res == 0) {
			if (!layer_export)
				continue;
			linfo_temp = (struct layer_info *)malloc(sizeof(struct layer_info));
			if (!linfo_temp) {
				printf("Out of memory\n");
				goto ret_clear_layer_list;
			}
			linfo_temp->color.alpha = layer_color.alpha;
			linfo_temp->color.red = layer_color.red;
			linfo_temp->color.green = layer_color.green;
			linfo_temp->color.blue = layer_color.blue;
			linfo_temp->name = layer_name;
			linfo_temp->stacked_position = i++;
			linfo_temp->layer = layer;
			layer_info_list = g_list_append(layer_info_list, (gpointer)linfo_temp);
		}
	} while(res >= 0);


	/* find_cell in first library. */
	if (!libs)
		goto ret_clear_layer_list;

	first_lib = (struct gds_library *)libs->data;
	if (!first_lib) {
		fprintf(stderr, "No library in library list. This should not happen.\n");
		goto ret_clear_layer_list;
	}

	for (cell_list = first_lib->cells; cell_list != NULL; cell_list = g_list_next(cell_list)) {
		temp_cell = (struct gds_cell *)cell_list->data;
		if (!strcmp(temp_cell->name, cell_name)) {
			toplevel_cell = temp_cell;
			break;
		}
	}

	if (!toplevel_cell) {
		printf("Couldn't find cell in first library!\n");
		goto ret_clear_layer_list;
	}

	/* Check if cell passes vital checks */
	res = gds_tree_check_reference_loops(toplevel_cell->parent_library);
	if (res < 0) {
		fprintf(stderr, "Checking library %s failed.\n", first_lib->name);
		goto ret_clear_layer_list;
	} else if (res > 0) {
		fprintf(stderr, "%d reference loops found.\n", res);

		/* do further checking if the specified cell and/or its subcells are affected */
		if (toplevel_cell->checks.affected_by_reference_loop == 1) {
			fprintf(stderr, "Cell is affected by reference loop. Abort!\n");
			goto ret_clear_layer_list;
		}
	}

	if (toplevel_cell->checks.affected_by_reference_loop == GDS_CELL_CHECK_NOT_RUN)
		fprintf(stderr, "Cell was not checked. This should not happen. Please report this issue. Will continue either way.\n");

	/* Note: unresolved references are not an abort condition.
	 * Deal with it.
	 */

	/* Render outputs */
	if (pdf == TRUE || svg == TRUE) {
		cairo_render_cell_to_vector_file(toplevel_cell, layer_info_list, (pdf == TRUE ? pdf_name : NULL),
						 (svg == TRUE ? svg_name : NULL), scale);
	}

	if (tex == TRUE) {
		tex_file = fopen(tex_name, "w");
		if (!tex_file)
			goto ret_clear_layer_list;
		latex_render_cell_to_code(toplevel_cell, layer_info_list, tex_file, scale, pdf_layers, pdf_standalone);
		fclose(tex_file);
	}

	if (so_name && so_out_file) {
		if (strlen(so_name) == 0 || strlen(so_out_file) == 0)
			goto ret_clear_layer_list;

		/* Render output using external renderer */
		printf("Invoking external renderer!\n");
		external_renderer_render_cell(toplevel_cell, layer_info_list, so_out_file, so_name);
		printf("External renderer finished!\n");
	}

ret_clear_layer_list:
	g_list_free_full(layer_info_list, (GDestroyNotify)delete_layer_info_with_name);

	g_object_unref(dstream);
	g_object_unref(stream);
ret_destroy_file:
	g_object_unref(file);
	/* Delete all allocated libraries */
ret_destroy_library_list:
	clear_lib_list(&libs);
}

/** @} */
