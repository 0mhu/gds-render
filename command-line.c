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
#include <glib/gi18n.h>
#include <string.h>

#include <gds-render/command-line.h>
#include <gds-render/gds-utils/gds-parser.h>
#include <gds-render/layer/layer-settings.h>
#include <gds-render/output-renderers/cairo-renderer.h>
#include <gds-render/output-renderers/latex-renderer.h>
#include <gds-render/output-renderers/external-renderer.h>
#include <gds-render/gds-utils/gds-tree-checker.h>
#include <gds-render/gds-utils/gds-statistics.h>

#include <fort.h>

#ifndef COUNT_OF
#define COUNT_OF(x) (sizeof((x))/sizeof(0[(x)]))
#endif

enum analysis_format {
	ANA_FORMAT_SIMPLE = 0,
	ANA_FORMAT_CELLS_ONLY,
	ANA_FORMAT_PRETTY,
};

struct analysis_format_cmdarg {
	enum analysis_format format;
	const char *argument;
};



static const struct analysis_format_cmdarg analysis_format_lookup[] = {
	{
		.format = ANA_FORMAT_SIMPLE,
		.argument = "simple",
	},
	{
		.format = ANA_FORMAT_PRETTY,
		.argument = "pretty",
	},
	{
		.format = ANA_FORMAT_CELLS_ONLY,
		.argument = "cellsonly"
	}
};

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
			    const struct external_renderer_params *ext_params,
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
		fprintf(stderr, _("Please specify renderers and file names\n"));
		return -1;
	}

	count_render = string_array_count(renderers);
	count_out = string_array_count(output_file_names);
	if (count_render != count_out) {
		fprintf(stderr, _("Count of renderers %d does not match count of output file names %d\n"),
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
			if (!ext_params->so_path) {
				fprintf(stderr, _("Please specify shared object for external renderer. Will ignore this renderer.\n"));
				continue;
			}
			output_renderer = GDS_RENDER_OUTPUT_RENDERER(
						external_renderer_new_with_so_and_param(ext_params->so_path,
											ext_params->cli_params));
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
			     struct external_renderer_params *ext_param,
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

	const struct gds_library_parsing_opts gds_parsing_options = {
		.simplified_polygons = 1,
	};

	/* Check if parameters are valid */
	if (!gds_name || !cell_name || !output_file_names || !layer_file || !renderers) {
		printf(_("Probably missing argument. Check --help option\n"));
		return -2;
	}

	/* Load layer_settings */
	layer_sett = layer_settings_new();
	layer_settings_load_from_csv(layer_sett, layer_file);

	/* Create renderers */
	if (create_renderers(renderers, output_file_names, tex_layers, tex_standalone,
			     ext_param, &renderer_list, layer_sett))
		goto ret_destroy_layer_mapping;


	/* Load GDS */
	clear_lib_list(&libs);
	res = parse_gds_from_file(gds_name, &libs, &gds_parsing_options);
	if (res)
		goto ret_destroy_library_list;

	/* find_cell in first library. */
	if (!libs)
		goto ret_clear_renderers;

	first_lib = (struct gds_library *)libs->data;
	if (!first_lib) {
		fprintf(stderr, _("No library in library list. This should not happen.\n"));
		/* This is safe. Library destruction can handle an empty list element */
		goto ret_destroy_library_list;
	}

	/* Find cell in first library */
	toplevel_cell = find_gds_cell_in_lib(first_lib, cell_name);

	if (!toplevel_cell) {
		printf(_("Couldn't find cell in first library!\n"));
		goto ret_destroy_library_list;
	}

	/* Check if cell passes vital checks */
	res = gds_tree_check_reference_loops(toplevel_cell->parent_library);
	if (res < 0) {
		fprintf(stderr, _("Checking library %s failed.\n"), first_lib->name);
		goto ret_destroy_library_list;
	} else if (res > 0) {
		fprintf(stderr, _("%d reference loops found.\n"), res);

		/* do further checking if the specified cell and/or its subcells are affected */
		if (toplevel_cell->checks.affected_by_reference_loop == 1) {
			fprintf(stderr, _("Cell is affected by reference loop. Abort!\n"));
			goto ret_destroy_library_list;
		}
	}

	if (toplevel_cell->checks.affected_by_reference_loop == GDS_CELL_CHECK_NOT_RUN)
		fprintf(stderr, _("Cell was not checked. This should not happen. Please report this issue. Will continue either way.\n"));

	/* Note: unresolved references are not an abort condition.
	 * Deal with it.
	 */

	/* Execute all rendererer instances */
	for (list_iter = renderer_list; list_iter; list_iter = list_iter->next) {
		current_renderer = GDS_RENDER_OUTPUT_RENDERER(list_iter->data);
		gds_output_renderer_render_output(current_renderer, toplevel_cell, scale);
	}

	ret = 0;

ret_destroy_library_list:
	clear_lib_list(&libs);
ret_clear_renderers:
	for (list_iter = renderer_list; list_iter; list_iter = list_iter->next)
		g_object_unref(list_iter->data);
ret_destroy_layer_mapping:
	g_object_unref(layer_sett);
	return ret;
}

static void indent_line(int level)
{
	while (level--)
		printf("\t");
}

static int printf_indented(int level, const char *format, ...)
{
	int ret;

	va_list a_list;
	va_start(a_list, format);
	indent_line(level);
	ret = vprintf(format, a_list);
	va_end(a_list);

	return ret;
}

static void print_simple_stat(GList *lib_stat_list)
{
#if 0
	int indentation_level = 0;
	GList *lib_iter;
	GList *cell_iter;
	const struct gds_lib_statistics *lib_stats;
	const struct gds_cell_statistics *cell_stats;

	for (lib_iter = lib_stat_list; lib_iter; lib_iter = g_list_next(lib_iter)) {
		lib_stats = (const struct gds_lib_statistics *)lib_iter->data;
		printf_indented(indentation_level, "Library %s\n", lib_stats->library->name);
		indentation_level++;

		for (cell_iter = lib_stats->cell_statistics; cell_iter; cell_iter = g_list_next(cell_iter)) {
			cell_stats = (const struct gds_cell_statistics *)cell_iter->data;
			printf_indented(indentation_level, "Cell %s\n", cell_stats->cell->name);
			indentation_level++;
			printf_indented(indentation_level, "Reference count: %zu\n", cell_stats->reference_count);
			printf_indented(indentation_level, "Graphics count: %zu\n", cell_stats->gfx_count);
			printf_indented(indentation_level, "Vertex count: %zu\n", cell_stats->vertex_count);
			printf_indented(indentation_level, "Unresolved children: %d\n",
					cell_stats->cell->checks.unresolved_child_count);
			printf_indented(indentation_level, "Reference loop: %s\n",
					cell_stats->cell->checks.affected_by_reference_loop ? "yes" : "no");

			indentation_level--;
		}
		printf_indented(indentation_level, "Cell count: %zu\n", lib_stats->cell_count);
		printf_indented(indentation_level, "Reference count: %zu\n", lib_stats->reference_count);
		printf_indented(indentation_level, "Graphics count: %zu\n", lib_stats->gfx_count);
		printf_indented(indentation_level, "Vertex count: %zu\n", lib_stats->vertex_count);
	}
#endif

}

static void print_table_stat(GList *lib_stat_list)
{
#if 0
	ft_table_t *table;
	GList *lib_stat_iter;
	GList *cell_stat_iter;
	const struct gds_lib_statistics *lib_stats;
	const struct gds_cell_statistics *cell_stats;

	table = ft_create_table();

	ft_set_cell_prop(table, 0, FT_ANY_COLUMN, FT_CPROP_ROW_TYPE, FT_ROW_HEADER);

	ft_set_cell_prop(table, 0, FT_ANY_COLUMN, FT_CPROP_ROW_TYPE, FT_ROW_HEADER);
	ft_write_ln(table, "Library", "Cell", "GFX", "Vertices", "Refs", "Unresolved Refs", "Loops");
	for (lib_stat_iter = lib_stat_list; lib_stat_iter; lib_stat_iter = g_list_next(lib_stat_iter)) {
		lib_stats = (const struct gds_lib_statistics *)lib_stat_iter->data;
		ft_printf_ln(table, "%s|%zu|%zu|%zu|%zu|-|-", lib_stats->library->name, lib_stats->cell_count,
			     lib_stats->gfx_count, lib_stats->vertex_count, lib_stats->reference_count);
		for (cell_stat_iter = lib_stats->cell_statistics; cell_stat_iter;
		     cell_stat_iter = g_list_next(cell_stat_iter)) {
			cell_stats = (const struct gds_cell_statistics *)cell_stat_iter->data;
			ft_printf_ln(table, "%s|%s|%zu|%zu|%zu|%d|%d", lib_stats->library->name, cell_stats->cell->name,
				     cell_stats->gfx_count, cell_stats->vertex_count, cell_stats->reference_count,
				     cell_stats->cell->checks.unresolved_child_count,
				     cell_stats->cell->checks.affected_by_reference_loop);
		}
	}


	printf("%s\n", ft_to_string(table));
	ft_destroy_table(table);
#endif
}

static void print_statistics(enum analysis_format format, GList *lib_stat_list)
{
	switch (format) {
	case ANA_FORMAT_PRETTY:
		print_table_stat(lib_stat_list);
		break;
	default:
		print_simple_stat(lib_stat_list);
		break;
	}
}

static void print_cell_names(GList *lib_list)
{
	GList *lib_iter;
	GList *name_iter;
	struct gds_library *lib;

	for (lib_iter = lib_list; lib_iter; lib_iter = g_list_next(lib_iter)) {
		lib = (struct gds_library *)lib_iter->data;
		for (name_iter = lib->cell_names; name_iter; name_iter = name_iter->next) {
			printf("%s\n", (const char *)name_iter->data);
		}
	}
}

int command_line_analyze_lib(const char *format, const char *gds_name)
{
	enum analysis_format fmt = ANA_FORMAT_SIMPLE;
	size_t idx;
	int found = 0;
	GList *lib_list = NULL;
	const struct gds_library_parsing_opts parsing_opts = {
		.simplified_polygons = 0,
	};
	int res;
	int ret = 0;
	GList *lib_iter;

	g_return_val_if_fail(gds_name, -1002);

	if (format && *format) {
		/* Check format if it is not empty */
		for (idx = 0; idx < COUNT_OF(analysis_format_lookup); idx++) {
			if (!strcmp(analysis_format_lookup[idx].argument, format)) {
				/* Format specifier matches */
				fmt = analysis_format_lookup[idx].format;
				found = 1;
			}
		}

		if (!found) {
			fprintf(stderr, "No format matches %s. Using default.\n", format);
		}
	}

	/* Load the GDS file */
	res = parse_gds_from_file(gds_name, &lib_list, &parsing_opts);
	if (res) {
		fprintf(stderr, "Error parsing GDS file\n");
		(void)clear_lib_list(&lib_list);
		ret = res;
		goto return_val;
	}

	for (lib_iter = lib_list; lib_iter; lib_iter = g_list_next(lib_iter)) {
		res = gds_tree_check_cell_references((struct gds_library *)lib_iter->data);
		if (res < 0) {
			fprintf(stderr, "Error checking cell references. Will continue...\n");
		}
		res = gds_tree_check_reference_loops((struct gds_library *)lib_iter->data);
		if (res < 0) {
			fprintf(stderr, "Error checking cell reference loops. Will continue...\n");
		}
	}

	if (fmt == ANA_FORMAT_CELLS_ONLY) {
		print_cell_names(lib_list);
		goto return_clear_libs;
	}


return_clear_libs:
	clear_lib_list(&lib_list);
return_val:
	return ret;
}

/** @} */
