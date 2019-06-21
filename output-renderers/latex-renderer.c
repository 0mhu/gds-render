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
 * @file latex-output.c
 * @brief LaTeX output renderer
 * @author Mario Hüttel <mario.huettel@gmx.net>
 */

#include <math.h>
#include <stdio.h>
#include <gds-render/output-renderers/latex-renderer.h>
#include <gdk/gdk.h>
#include <gds-render/layer/layer-info.h>
/**
 * @addtogroup LatexRenderer
 * @{
 */

struct _LatexRenderer {
	GdsOutputRenderer parent;
	gboolean tex_standalone;
	gboolean pdf_layers;
};

G_DEFINE_TYPE(LatexRenderer, latex_renderer, GDS_RENDER_TYPE_OUTPUT_RENDERER)

enum {
	PROP_STANDALONE = 1,
	PROP_PDF_LAYERS,
	N_PROPERTIES
};

/**
 * @brief Writes a GString \p buffer to the fixed file tex_file
 * @note This is a convinience macro. Do not use this anywhere else. It might change behavior in futurtre releases
 */
#define WRITEOUT_BUFFER(buff) fwrite((buff)->str, sizeof(char), (buff)->len, tex_file)

/**
 * @brief Write the layer declarration to TeX file
 *
 * This writes the declaration of the layers and the mapping in which order
 * the layers shall be rendered by TikZ. Layers are written in the order they are
 * positioned inside the \p layer_infos list.
 *
 * @param tex_file TeX-File to write to
 * @param layer_infos List containing layer_info structs.
 * @param buffer
 * @note  The field layer_info::stacked_position is ignored. Stack depends on list order.
 */
static void write_layer_definitions(FILE *tex_file, GList *layer_infos, GString *buffer)
{
	GList *list;
	struct layer_info *lifo;

	for (list = layer_infos; list != NULL; list = list->next) {
		lifo = (struct layer_info *)list->data;

		if (!lifo->render)
			continue;

		g_string_printf(buffer, "\\pgfdeclarelayer{l%d}\n\\definecolor{c%d}{rgb}{%lf,%lf,%lf}\n",
				lifo->layer, lifo->layer,
				lifo->color.red, lifo->color.green, lifo->color.blue);
		WRITEOUT_BUFFER(buffer);
	}

	g_string_printf(buffer, "\\pgfsetlayers{");
	WRITEOUT_BUFFER(buffer);

	for (list = layer_infos; list != NULL; list = list->next) {
		lifo = (struct layer_info *)list->data;

		if (!lifo->render)
			continue;

		g_string_printf(buffer, "l%d,", lifo->layer);
		WRITEOUT_BUFFER(buffer);
	}
	fwrite("main}\n", sizeof(char), 1, tex_file);
}

/**
 * @brief Write layer Envirmonment
 *
 * If the requested layer shall be rendered, this code writes the necessary code
 * to open the layer. It also returns the color the layer shall be rendered in.
 *
 * The followingenvironments are generated:
 *
 * @code{.tex}
 * \begin{pgfonlayer}{<layer>}
 * % If pdf layers shall be used also this is enabled:
 * \begin{scope}[ocg={ref=<layer>, status=visible,name={<Layer Name>}}]
 * @endcode
 *
 *
 * If the layer shall not be rendered, FALSE is returned and the color is not filled in and
 * the cod eis not written to the file.
 *
 * @param tex_file TeX file to write to
 * @param color Return of the layer's color
 * @param layer Requested layer number
 * @param linfo Layer information list containing layer_info structs
 * @param buffer Some working buffer
 * @return TRUE, if the layer shall be rendered.
 * @note The opened environments have to be closed afterwards
 */
static gboolean write_layer_env(FILE *tex_file, GdkRGBA *color, int layer, GList *linfo, GString *buffer)
{
	GList *temp;
	struct layer_info *inf;

	for (temp = linfo; temp != NULL; temp = temp->next) {
		inf = (struct layer_info *)temp->data;
		if (inf->layer == layer && inf->render) {
			color->alpha = inf->color.alpha;
			color->red = inf->color.red;
			color->green = inf->color.green;
			color->blue = inf->color.blue;
			g_string_printf(buffer, "\\begin{pgfonlayer}{l%d}\n\\ifcreatepdflayers\n\\begin{scope}[ocg={ref=%d, status=visible,name={%s}}]\n\\fi\n",
					layer, layer, inf->name);
			WRITEOUT_BUFFER(buffer);
			return TRUE;
		}
	}
	return FALSE;
}

/**
 * @brief Writes a graphics object to the specified tex_file
 *
 * This function opens the layer, writes a graphics object and closes the layer
 *
 * @param tex_file File to write to
 * @param graphics Object to render
 * @param linfo Layer information
 * @param buffer Working buffer
 * @param scale Scale abject down by this value
 */
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

/**
 * @brief Render cell to file
 * @param cell Cell to render
 * @param layer_infos Layer information
 * @param tex_file File to write to
 * @param buffer Working buffer
 * @param scale Scale output down by this value
 */
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

static int latex_render_cell_to_code(struct gds_cell *cell, GList *layer_infos, FILE *tex_file, double scale,
			       gboolean create_pdf_layers, gboolean standalone_document)
{
	GString *working_line;


	if (!tex_file || !layer_infos || !cell)
		return -1;

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

	return 0;
}

static int latex_renderer_render_output(GdsOutputRenderer *renderer,
					  struct gds_cell *cell,
					  double scale)
{
	LatexRenderer *l_renderer = GDS_RENDER_LATEX_RENDERER(renderer);
	FILE *tex_file;
	int ret = -2;
	LayerSettings *settings;
	GList *layer_infos = NULL;
	const char *output_file;

	output_file = gds_output_renderer_get_output_file(renderer);
	settings = gds_output_renderer_get_layer_settings(renderer);

	/* Set layer info list. In case of failure it remains NULL */
	if (settings)
		layer_infos = layer_settings_get_layer_info_list(settings);

	tex_file = fopen(output_file, "w");
	if (tex_file) {
		ret = latex_render_cell_to_code(cell, layer_infos, tex_file, scale,
						l_renderer->pdf_layers, l_renderer->tex_standalone);
		fclose(tex_file);
	} else {
		g_error("Could not open LaTeX outpur file");
	}

	return ret;
}

static void latex_renderer_init(LatexRenderer *self)
{
	self->pdf_layers = FALSE;
	self->tex_standalone = FALSE;
}

static void latex_renderer_get_property(GObject *obj, guint property_id, GValue *value, GParamSpec *pspec)
{
	LatexRenderer *self = GDS_RENDER_LATEX_RENDERER(obj);

	switch (property_id) {
	case PROP_STANDALONE:
		g_value_set_boolean(value, self->tex_standalone);
		break;
	case PROP_PDF_LAYERS:
		g_value_set_boolean(value, self->pdf_layers);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, property_id, pspec);
		break;
	}
}

static void latex_renderer_set_property(GObject *obj, guint property_id, const GValue *value, GParamSpec *pspec)
{
	LatexRenderer *self = GDS_RENDER_LATEX_RENDERER(obj);

	switch (property_id) {
	case PROP_STANDALONE:
		self->tex_standalone = g_value_get_boolean(value);
		break;
	case PROP_PDF_LAYERS:
		self->pdf_layers = g_value_get_boolean(value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, property_id, pspec);
		break;
	}
}

static GParamSpec *latex_renderer_properties[N_PROPERTIES] = {NULL};

static void latex_renderer_class_init(LatexRendererClass *klass)
{
	GdsOutputRendererClass *render_class = GDS_RENDER_OUTPUT_RENDERER_CLASS(klass);
	GObjectClass *oclass = G_OBJECT_CLASS(klass);

	/* Overwrite virtual function */
	render_class->render_output = latex_renderer_render_output;

	/* Property stuff */
	oclass->get_property = latex_renderer_get_property;
	oclass->set_property = latex_renderer_set_property;

	latex_renderer_properties[PROP_STANDALONE] =
			g_param_spec_boolean("standalone",
					     "Standalone TeX file",
					     "Generate a standalone LaTeX file.",
					     FALSE,
					     G_PARAM_READWRITE);
	latex_renderer_properties[PROP_PDF_LAYERS] =
			g_param_spec_boolean("pdf-layers",
					     "PDF OCR layers",
					     "Generate OCR layers",
					     FALSE,
					     G_PARAM_READWRITE);

	g_object_class_install_properties(oclass, N_PROPERTIES, latex_renderer_properties);
}

LatexRenderer *latex_renderer_new()
{
	return GDS_RENDER_LATEX_RENDERER(g_object_new(GDS_RENDER_TYPE_LATEX_RENDERER, NULL));
}

LatexRenderer *latex_renderer_new_with_options(gboolean pdf_layers, gboolean standalone)
{
	GObject *obj;

	obj = g_object_new(GDS_RENDER_TYPE_LATEX_RENDERER, "standalone", standalone, "pdf-layers", pdf_layers, NULL);
	return GDS_RENDER_LATEX_RENDERER(obj);
}

/** @} */
