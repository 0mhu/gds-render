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

#include "latex-output.h"

#define WRITEOUT_BUFFER(buff) fwrite((buff)->str, sizeof(char), (buff)->len, tex_file)

static void write_layer_definitions(FILE *tex_file, GList *layer_infos, GString *buffer)
{
	GList *list;
	struct layer_info *lifo;
	char *end_str;

	for (list = layer_infos; list != NULL; list = list->next) {
		lifo = (struct layer_info *)list->data;
		g_string_printf(buffer, "\\pgfdeclarelayer{l%d}\n", lifo->layer);
		WRITEOUT_BUFFER(buffer);
	}

	g_string_printf(buffer, "\\pgfsetlayers{");
	WRITEOUT_BUFFER(buffer);

	for (list = layer_infos; list != NULL; list = list->next) {
		lifo = (struct layer_info *)list->data;

		if (list->next == NULL)
			end_str = ",main}";
		else
			end_str = ",";
		g_string_printf(buffer, "l%d%s", lifo->layer, end_str);
		WRITEOUT_BUFFER(buffer);
	}
	fwrite("\n", sizeof(char), 1, tex_file);
}

static int sorting_func_stack(struct layer_info *info1, struct layer_info *info2)
{
	return info1->stacked_position - info2->stacked_position;
}

static void sort_layers_for_rendering(GList **layer_infos)
{
	GList *list = *layer_infos;
	list = g_list_sort(list, (GCompareFunc)sorting_func_stack);
	*layer_infos = list;
}

void render_cell_to_code(struct gds_cell *cell, GList *layer_infos, FILE *tex_file)
{
	GString *working_line;

	if (!tex_file || !layer_infos || !cell)
		return;

	/* 10 kB Line working buffer should be enough */
	working_line = g_string_new_len(NULL, LATEX_LINE_BUFFER_KB*1024);

	/* Sort layer according to target layer */
	sort_layers_for_rendering(&layer_infos);

	/* Write layer definitions */
	write_layer_definitions(tex_file, layer_infos, working_line);

	/* Open tikz Pictute */
	g_string_printf(working_line, "\\begin{tikzpicture}\n");
	WRITEOUT_BUFFER(working_line);

	/* Generate graphics output */



	g_string_printf(working_line, "\\end{tikzpicture}\n");
	WRITEOUT_BUFFER(working_line);

	fflush(tex_file);
	g_string_free(working_line, TRUE);
}
