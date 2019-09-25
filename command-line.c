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
#include <gds-render/layer/layer-settings.h>
#include <gds-render/output-renderers/cairo-renderer.h>
#include <gds-render/output-renderers/latex-renderer.h>
#include <gds-render/output-renderers/external-renderer.h>
#include <gds-render/gds-utils/gds-tree-checker.h>

static int string_array_count(char **string_array)
{
	int count;

	if (!string_array)
		return 0;

	for (count = 0; *string_array; string_array++)
		count++;

	return count;
}

static int create_renderers(char **renderers,
			    char **output_file_names,
			    gboolean tex_layers,
			    gboolean tex_standalone,
			    const char *so_path,
			    GList **renderer_list,
			    LayerSettings *layer_settings)
{
	char **renderer_iter;
	char *current_renderer;
	int idx;
	char *current_out_file;
	int count_render, count_out;
	GdsOutputRenderer *output_renderer;

	if (!renderer_list)
		return -1;

	if (!renderers || !output_file_names) {
		fprintf(stderr, "Please specify renderers and file names\n");
		return -1;
	}

	count_render = string_array_count(renderers);
	count_out = string_array_count(output_file_names);
	if (count_render != count_out) {
		fprintf(stderr, "Count of renderers %d does not match count of output file names %d\n",
			count_render, count_out);
		return -1;
	}

	/* Parse cmd line parameters */
	for (renderer_iter = renderers, idx = 0; *renderer_iter; renderer_iter++, idx++) {
		current_renderer = *renderer_iter;
		current_out_file = output_file_names[idx];

		/* File valid ? */
		if (!current_out_file || !current_out_file[0])
			continue;

		if (!strcmp(current_renderer, "tikz")) {
			output_renderer = GDS_RENDER_OUTPUT_RENDERER(latex_renderer_new_with_options(tex_layers,
												     tex_standalone));
		} else if (!strcmp(current_renderer, "pdf")) {
			output_renderer = GDS_RENDER_OUTPUT_RENDERER(cairo_renderer_new_pdf());
		} else if (!strcmp(current_renderer, "svg")) {
			output_renderer = GDS_RENDER_OUTPUT_RENDERER(cairo_renderer_new_svg());
		} else if (!strcmp(current_renderer, "ext")) {
			if (!so_path) {
				fprintf(stderr, "Please specify shared object for external renderer. Will ignore this renderer.\n");
				continue;
			}
			output_renderer = GDS_RENDER_OUTPUT_RENDERER(external_renderer_new_with_so(so_path));
		} else {
			continue;
		}

		gds_output_renderer_set_output_file(output_renderer, current_out_file);
		gds_output_renderer_set_layer_settings(output_renderer, layer_settings);
		*renderer_list = g_list_append(*renderer_list, output_renderer);
	}

	return 0;
}

static struct gds_cell *find_gds_cell_in_lib(struct gds_library *lib, const char *cell_name)
{
	GList *cell_list;
	struct gds_cell *return_cell = NULL;
	struct gds_cell *temp_cell;

	for (cell_list = lib->cells; cell_list; cell_list = g_list_next(cell_list)) {
		temp_cell = (struct gds_cell *)cell_list->data;
		if (!strncmp(temp_cell->name, cell_name, CELL_NAME_MAX)) {
			return_cell = temp_cell;
			break;
		}
	}
	return return_cell;
}

int command_line_convert_gds(const char *gds_name,
			      const char *cell_name,
			      char **renderers,
			      char **output_file_names,
			      const char *layer_file,
			      const char *so_path,
			      gboolean tex_standalone,
			      gboolean tex_layers,
			      double scale)
{
	int ret = -1;
	GList *libs = NULL;
	int res;
	GList *renderer_list = NULL;
	GList *list_iter;
	struct gds_library *first_lib;
	struct gds_cell *toplevel_cell = NULL;
	LayerSettings *layer_sett;
	GdsOutputRenderer *current_renderer;

	/* Check if parameters are valid */
	if (!gds_name || !cell_name || !output_file_names || !layer_file || !renderers) {
		printf("Probably missing argument. Check --help option\n");
		return -2;
	}

	/* Load layer_settings */
	layer_sett = layer_settings_new();
	layer_settings_load_from_csv(layer_sett, layer_file);

	/* Create renderers */
	if (create_renderers(renderers, output_file_names, tex_layers, tex_standalone,
			     so_path, &renderer_list, layer_sett))
		goto ret_destroy_layer_mapping;


	/* Load GDS */
	clear_lib_list(&libs);
	res = parse_gds_from_file(gds_name, &libs);
	if (res)
		goto ret_destroy_library_list;

	/* find_cell in first library. */
	if (!libs)
		goto ret_clear_renderers;

	first_lib = (struct gds_library *)libs->data;
	if (!first_lib) {
		fprintf(stderr, "No library in library list. This should not happen.\n");
		/* This is safe. Library destruction can handle an empty list element */
		goto ret_destroy_library_list;
	}

	/* Find cell in first library */
	toplevel_cell = find_gds_cell_in_lib(first_lib, cell_name);

	if (!toplevel_cell) {
		printf("Couldn't find cell in first library!\n");
		goto ret_destroy_library_list;
	}

	/* Check if cell passes vital checks */
	res = gds_tree_check_reference_loops(toplevel_cell->parent_library);
	if (res < 0) {
		fprintf(stderr, "Checking library %s failed.\n", first_lib->name);
		goto ret_destroy_library_list;
	} else if (res > 0) {
		fprintf(stderr, "%d reference loops found.\n", res);

		/* do further checking if the specified cell and/or its subcells are affected */
		if (toplevel_cell->checks.affected_by_reference_loop == 1) {
			fprintf(stderr, "Cell is affected by reference loop. Abort!\n");
			goto ret_destroy_library_list;
		}
	}

	if (toplevel_cell->checks.affected_by_reference_loop == GDS_CELL_CHECK_NOT_RUN)
		fprintf(stderr, "Cell was not checked. This should not happen. Please report this issue. Will continue either way.\n");

	/* Note: unresolved references are not an abort condition.
	 * Deal with it.
	 */

	/* Execute all rendererer instances */
	for (list_iter = renderer_list; list_iter; list_iter = list_iter->next) {
		current_renderer = GDS_RENDER_OUTPUT_RENDERER(list_iter->data);
		gds_output_renderer_render_output(current_renderer, toplevel_cell, scale);
	}

ret_destroy_library_list:
	clear_lib_list(&libs);
ret_clear_renderers:
	for (list_iter = renderer_list; list_iter; list_iter = list_iter->next) {
		g_object_unref(list_iter->data);
	}
ret_destroy_layer_mapping:
	g_object_unref(layer_sett);
	return ret;
}

/** @} */
