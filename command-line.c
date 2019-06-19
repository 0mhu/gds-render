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
 * @addtogroup cmdline
 * @{
 */

#include <stdio.h>

#include <gds-render/command-line.h>
#include <gds-render/gds-utils/gds-parser.h>
#include <gds-render/layer/mapping-parser.h>
#include <gds-render/layer/layer-info.h>
#include <gds-render/output-renderers/cairo-renderer.h>
#include <gds-render/output-renderers/latex-renderer.h>
#include <gds-render/output-renderers/external-renderer.h>
#include <gds-render/gds-utils/gds-tree-checker.h>

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

void command_line_convert_gds(const char *gds_name, const char *cell_name, const char *output_file_name,  const char *layer_file, const char *so_path,
			       enum command_line_renderer renderer, enum cmd_options options, double scale)
{
	GList *libs = NULL;
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
	GdsOutputRenderer *output_renderer;
	gboolean tex_layers = FALSE, tex_standalone = FALSE;

	/* Check if parameters are valid */
	if (!gds_name || !cell_name || !output_file_name || !layer_file) {
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
		res = mapping_parser_load_line(dstream, &layer_export, &layer_name, &layer, &layer_color);
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


	/* Render */

	if (options & CMD_OPT_LATEX_LAYERS)
		tex_layers = TRUE;
	if (options & CMD_OPT_LATEX_STANDALONE)
		tex_standalone = TRUE;

	switch (renderer) {
	case CMD_CAIRO_SVG:
		output_renderer = GDS_RENDER_OUTPUT_RENDERER(cairo_renderer_new_svg());
		break;
	case CMD_LATEX:
		output_renderer = GDS_RENDER_OUTPUT_RENDERER(latex_renderer_new_with_options(tex_layers, tex_standalone));
		break;
	case CMD_CAIRO_PDF:
		output_renderer = GDS_RENDER_OUTPUT_RENDERER(cairo_renderer_new_pdf());
		break;
	case CMD_EXTERNAL:
		output_renderer = GDS_RENDER_OUTPUT_RENDERER(external_renderer_new_with_so(so_path));
		break;
	case CMD_NONE:
		/* Do nothing */
		output_renderer = NULL;
		break;
	default:
		output_renderer = NULL;
		fprintf(stderr, "Invalid renderer supplied");
		break;

	}

	if (output_renderer) {
		gds_output_renderer_render_output(output_renderer, toplevel_cell, layer_info_list, output_file_name, scale);
		g_object_unref(output_renderer);
	}
	/* Render end */
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
