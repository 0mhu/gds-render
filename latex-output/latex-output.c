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
#include <math.h>

#define WRITEOUT_BUFFER(buff) fwrite((buff)->str, sizeof(char), (buff)->len, tex_file)

static void write_layer_definitions(FILE *tex_file, GList *layer_infos, GString *buffer)
{
	GList *list;
	struct layer_info *lifo;
	char *end_str;

	for (list = layer_infos; list != NULL; list = list->next) {
		lifo = (struct layer_info *)list->data;
		g_string_printf(buffer, "\\pgfdeclarelayer{l%d}\n\\definecolor{c%d}{rgb}{%lf,%lf,%lf}\n",
				lifo->layer, lifo->layer,
				lifo->color.red, lifo->color.green, lifo->color.blue);
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

/**
 * @brief write_layer_env
 * @param tex_file
 * @param layer
 * @param buffer
 * @return TRUE if layer is placeable
 */
static gboolean write_layer_env(FILE *tex_file, GdkRGBA *color, int layer, GList *linfo, GString *buffer)
{
	GList *temp;
	struct layer_info *inf;

	for (temp = linfo; temp != NULL; temp = temp->next) {
		inf = (struct layer_info *)temp->data;
		if (inf->layer == layer) {
			color->alpha = inf->color.alpha;
			color->red = inf->color.red;
			color->green = inf->color.green;
			color->blue = inf->color.blue;
			g_string_printf(buffer, "\\begin{pgfonlayer}{l%d}\n\\ifcreatepdflayers\n\\begin{scope}[ocg={ref=%d, status=visible,name={%s}}]\n\\fi]\n",
					layer, layer, inf->name);
			WRITEOUT_BUFFER(buffer);
			return TRUE;
		}
	}
	return FALSE;
}


static void generate_graphics(FILE *tex_file, GList *graphics, GList *linfo, GString *buffer, double scale)
{
	GList *temp;
	GList *temp_vertex;
	struct gds_graphics *gfx;
	struct gds_point *pt;
	GdkRGBA color;
	static const char *line_caps[] = {"butt", "round", "rect"};

	for (temp = graphics; temp != NULL; temp = temp->next) {
		gfx = (struct gds_graphics *)temp->data;
		if (write_layer_env(tex_file, &color, (int)gfx->layer, linfo, buffer) == TRUE) {

			/* Layer is defined => create graphics */
			if (gfx->gfx_type == GRAPHIC_POLYGON || gfx->gfx_type == GRAPHIC_BOX ) {
				g_string_printf(buffer, "\\draw[line width=0.00001 pt, draw={c%d}, fill={c%d}, fill opacity={%lf}] ",
						gfx->layer, gfx->layer, color.alpha);
				WRITEOUT_BUFFER(buffer);
				/* Append vertices */
				for (temp_vertex = gfx->vertices; temp_vertex != NULL; temp_vertex = temp_vertex->next) {
					pt = (struct gds_point *)temp_vertex->data;
					g_string_printf(buffer, "(%lf pt, %lf pt) -- ", ((double)pt->x)/scale, ((double)pt->y)/scale);
					WRITEOUT_BUFFER(buffer);
				}
				g_string_printf(buffer, "cycle;\n");
				WRITEOUT_BUFFER(buffer);
			} else if (gfx->gfx_type == GRAPHIC_PATH) {

				if (g_list_length(gfx->vertices) < 2) {
					printf("Cannot write path with less than 2 points\n");
					break;
				}

				if (gfx->path_render_type < 0 || gfx->path_render_type > 2) {
					printf("Path type unrecognized. Setting to 'flushed'\n");
					gfx->path_render_type = PATH_FLUSH;
				}

				g_string_printf(buffer, "\\draw[line width=%lf pt, draw={c%d}, opacity={%lf}, cap=%s] ",
						gfx->width_absolute/scale, gfx->layer, color.alpha,
						line_caps[gfx->path_render_type]);
				WRITEOUT_BUFFER(buffer);

				/* Append vertices */
				for (temp_vertex = gfx->vertices; temp_vertex != NULL; temp_vertex = temp_vertex->next) {
					pt = (struct gds_point *)temp_vertex->data;
					g_string_printf(buffer, "(%lf pt, %lf pt)%s",
							((double)pt->x)/scale,
							((double)pt->y)/scale,
							(temp_vertex->next ? " -- " : ""));
					WRITEOUT_BUFFER(buffer);
				}
				g_string_printf(buffer, ";\n");
				WRITEOUT_BUFFER(buffer);
			}

			g_string_printf(buffer, "\\ifcreatepdflayers\n\\end{scope}\n\\fi\n\\end{pgfonlayer}\n");
			WRITEOUT_BUFFER(buffer);
		}

	} /* For graphics */
}


static void render_cell(struct gds_cell *cell, GList *layer_infos, FILE *tex_file, GString *buffer, double scale)
{

	GList *list_child;
	struct gds_cell_instance *inst;

	/* Draw polygons of current cell */
	generate_graphics(tex_file, cell->graphic_objs, layer_infos, buffer, scale);

	/* Draw polygons of childs */
	for (list_child = cell->child_cells; list_child != NULL; list_child = list_child->next) {
		inst = (struct gds_cell_instance *)list_child->data;

		/* Abort if cell has no reference */
		if (!inst->cell_ref)
			continue;

		/* generate translation scope */
		g_string_printf(buffer, "\\begin{scope}[shift={(%lf pt,%lf pt)}]\n",
				((double)inst->origin.x)/scale,((double)inst->origin.y)/scale);
		WRITEOUT_BUFFER(buffer);

		g_string_printf(buffer, "\\begin{scope}[rotate=%lf]\n", inst->angle);
		WRITEOUT_BUFFER(buffer);

		g_string_printf(buffer, "\\begin{scope}[yscale=%lf, xscale=%lf]\n", (inst->flipped ? -1*inst->magnification : inst->magnification),
				inst->magnification);
		WRITEOUT_BUFFER(buffer);

		render_cell(inst->cell_ref, layer_infos, tex_file, buffer, scale);

		g_string_printf(buffer, "\\end{scope}\n");
		WRITEOUT_BUFFER(buffer);

		g_string_printf(buffer, "\\end{scope}\n");
		WRITEOUT_BUFFER(buffer);

		g_string_printf(buffer, "\\end{scope}\n");
		WRITEOUT_BUFFER(buffer);
	}

}

void latex_render_cell_to_code(struct gds_cell *cell, GList *layer_infos, FILE *tex_file, double scale,
			       gboolean create_pdf_layers, gboolean standalone_document)
{
	GString *working_line;


	if (!tex_file || !layer_infos || !cell)
		return;

	/* 10 kB Line working buffer should be enough */
	working_line = g_string_new_len(NULL, LATEX_LINE_BUFFER_KB*1024);

	/* standalone foo */
	g_string_printf(working_line, "\\newif\\iftestmode\n\\testmode%s\n",
			(standalone_document ? "true" : "false"));
	WRITEOUT_BUFFER(working_line);
	g_string_printf(working_line, "\\newif\\ifcreatepdflayers\n\\createpdflayers%s\n",
			(create_pdf_layers ? "true" : "false"));
	WRITEOUT_BUFFER(working_line);
	g_string_printf(working_line, "\\iftestmode\n");
	WRITEOUT_BUFFER(working_line);
	g_string_printf(working_line, "\\documentclass[tikz]{standalone}\n\\usepackage{xcolor}\n\\usetikzlibrary{ocgx}\n\\begin{document}\n");
	WRITEOUT_BUFFER(working_line);
	g_string_printf(working_line, "\\fi\n");
	WRITEOUT_BUFFER(working_line);

	/* Write layer definitions */
	write_layer_definitions(tex_file, layer_infos, working_line);

	/* Open tikz Pictute */
	g_string_printf(working_line, "\\begin{tikzpicture}\n");
	WRITEOUT_BUFFER(working_line);

	/* Generate graphics output */
	render_cell(cell, layer_infos, tex_file, working_line, scale);


	g_string_printf(working_line, "\\end{tikzpicture}\n");
	WRITEOUT_BUFFER(working_line);

	g_string_printf(working_line, "\\iftestmode\n");
	WRITEOUT_BUFFER(working_line);
	g_string_printf(working_line, "\\end{document}\n");
	WRITEOUT_BUFFER(working_line);
	g_string_printf(working_line, "\\fi\n");
	WRITEOUT_BUFFER(working_line);

	fflush(tex_file);
	g_string_free(working_line, TRUE);
}
