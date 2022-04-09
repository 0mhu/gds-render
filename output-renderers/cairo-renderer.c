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
 * @file cairo-renderer.c
 * @brief Output renderer for Cairo PDF export
 * @author Mario Hüttel <mario.huettel@gmx.net>
 */

/** @addtogroup Cairo-Renderer
 *  @{
 */

#include <math.h>
#include <stdlib.h>
#include <cairo.h>
#include <cairo-pdf.h>
#include <cairo-svg.h>
#include <glib/gi18n.h>

#include <gds-render/output-renderers/cairo-renderer.h>
#include <sys/wait.h>
#include <unistd.h>

struct _CairoRenderer {
	GdsOutputRenderer parent;
	gboolean svg; /**< @brief TRUE: SVG output, FALSE: PDF output */
};

G_DEFINE_TYPE(CairoRenderer, cairo_renderer, GDS_RENDER_TYPE_OUTPUT_RENDERER)

/**
 * @brief The cairo_layer struct
 * Each rendered layer is represented by this struct.
 */
struct cairo_layer {
	cairo_t *cr; /**< @brief cairo context for layer*/
	cairo_surface_t *rec; /**< @brief Recording surface to hold the layer */
	struct layer_info *linfo; /**< @brief Reference to layer information */
};

/**
 * @brief Revert the last transformation on all layers
 * @param layers Pointer to #cairo_layer structures
 */
static void revert_inherited_transform(struct cairo_layer *layers)
{
	int i;

	for (i = 0; i < MAX_LAYERS; i++) {
		if (layers[i].cr == NULL)
			continue;
		cairo_restore(layers[i].cr);
	}
}

/**
 * @brief Applies transformation to all layers
 * @param layers Array of layers
 * @param origin Origin translation
 * @param magnification Scaling
 * @param flipping Mirror image on x-axis before rotating
 * @param rotation Rotation in degrees
 * @param scale Scale the image down by. Only used for sclaing origin coordinates. Not applied to layer.
 */
static void apply_inherited_transform_to_all_layers(struct cairo_layer *layers,
						    const struct gds_point *origin,
						    double magnification,
						    gboolean flipping,
						    double rotation,
						    double scale)
{
	int i;
	cairo_t *temp_layer_cr;

	for (i = 0; i < MAX_LAYERS; i++) {
		temp_layer_cr = layers[i].cr;
		if (temp_layer_cr == NULL)
			continue;

		/* Save the state and apply transformation */
		cairo_save(temp_layer_cr);
		cairo_translate(temp_layer_cr, (double)origin->x/scale, (double)origin->y/scale);
		cairo_rotate(temp_layer_cr, M_PI*rotation/180.0);
		cairo_scale(temp_layer_cr, magnification,
			    (flipping == TRUE ? -magnification : magnification));
	}
}

/**
 * @brief render_cell Render a cell with its sub-cells
 * @param cell Cell to render
 * @param layers Cell will be rendered into these layers
 * @param scale sclae image down by this factor
 */
static void render_cell(struct gds_cell *cell, struct cairo_layer *layers, double scale)
{
	GList *instance_list;
	struct gds_cell *temp_cell;
	struct gds_cell_instance *cell_instance;
	GList *gfx_list;
	struct gds_graphics *gfx;
	GList *vertex_list;
	struct gds_point *vertex;
	cairo_t *cr;

	/* Render child cells */
	for (instance_list = cell->child_cells; instance_list != NULL; instance_list = instance_list->next) {
		cell_instance = (struct gds_cell_instance *)instance_list->data;
		temp_cell = cell_instance->cell_ref;
		if (temp_cell != NULL) {
			apply_inherited_transform_to_all_layers(layers,
								&cell_instance->origin,
								cell_instance->magnification,
								cell_instance->flipped,
								cell_instance->angle,
								scale);
			render_cell(temp_cell, layers, scale);
			revert_inherited_transform(layers);
		}
	}

	/* Render graphics */
	for (gfx_list = cell->graphic_objs; gfx_list != NULL; gfx_list = gfx_list->next) {
		gfx = (struct gds_graphics *)gfx_list->data;

		/* Get layer renderer */
		if (gfx->layer >= MAX_LAYERS)
			continue;

		cr = layers[gfx->layer].cr;
		if (cr == NULL)
			continue;

		/* Apply settings */
		cairo_set_line_width(cr, (gfx->width_absolute ? gfx->width_absolute/scale : 1));

		switch (gfx->path_render_type) {
		case PATH_FLUSH:
			cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT);
			break;
		case PATH_ROUNDED:
			cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
			break;
		case PATH_SQUARED:
			cairo_set_line_cap(cr, CAIRO_LINE_CAP_SQUARE);
			break;
		}

		/* Add vertices */
		for (vertex_list = gfx->vertices; vertex_list != NULL; vertex_list = vertex_list->next) {
			vertex = (struct gds_point *)vertex_list->data;

			/* If first point -> move to, else line to */
			if (vertex_list->prev == NULL)
				cairo_move_to(cr, vertex->x/scale, vertex->y/scale);
			else
				cairo_line_to(cr, vertex->x/scale, vertex->y/scale);
		}

		/* Create graphics object */
		switch (gfx->gfx_type) {
		case GRAPHIC_PATH:
			cairo_stroke(cr);
			break;
		case GRAPHIC_BOX:
			/* Expected fallthrough */
		case GRAPHIC_POLYGON:
			cairo_set_line_width(cr, 0.1/scale);
			cairo_close_path(cr);
			cairo_stroke_preserve(cr); // Prevent graphic glitches
			cairo_fill(cr);
			break;
		}
	} /* for gfx list */
}

/**
 * @brief Read a line from a file descriptor
 *
 * In case of a broken pipe / closed writing end, it will terminate
 *
 * @param fd File descriptor to read from
 * @param buff Buffer to write data in
 * @param buff_size Buffer size
 * @return length of read data
 */
static int read_line_from_fd(int fd, char *buff, size_t buff_size)
{
	ssize_t cnt;
	char c;
	unsigned int buff_cnt = 0;

	while ((cnt = read(fd, &c, 1)) == 1) {
		if (buff_cnt < (buff_size-1)) {
			buff[buff_cnt++] = c;
			if (c == '\n')
				break;
		} else {
			break;
		}
	}

	buff[buff_cnt] = 0;
	return (int)buff_cnt;
}

/**
 * @brief Render \p cell to a PDF file specified by \p pdf_file
 * @param renderer The current renderer this function is running from
 * @param cell Toplevel cell to @ref Cairo-Renderer
 * @param layer_infos List of layer information. Specifies color and layer stacking
 * @param pdf_file PDF output file. Set to NULL if no PDF file has to be generated
 * @param svg_file SVG output file. Set to NULL if no SVG file has to be generated
 * @param scale Scale the output image down by \p scale
 * @return Error
 */
static int cairo_renderer_render_cell_to_vector_file(GdsOutputRenderer *renderer,
						     struct gds_cell *cell,
						     GList *layer_infos,
						     const char *pdf_file,
						     const char *svg_file,
						     double scale)
{
	cairo_surface_t *pdf_surface = NULL, *svg_surface = NULL;
	cairo_t *pdf_cr = NULL, *svg_cr = NULL;
	struct layer_info *linfo;
	struct cairo_layer *layers;
	struct cairo_layer *lay;
	GList *info_list;
	int i;
	double rec_x0, rec_y0, rec_width, rec_height;
	double xmin = INT32_MAX, xmax = INT32_MIN, ymin = INT32_MAX, ymax = INT32_MIN;
	pid_t process_id;
	int comm_pipe[2];
	char receive_message[200];

	if (pdf_file == NULL && svg_file == NULL) {
		/* No output specified */
		return -1;
	}

	/* Generate communication pipe for status updates */
	if (pipe(comm_pipe) == -1)
		return -2;

	/* Fork to a new child process. This ensures the memory leaks (see issue #16) in Cairo don't
	 * brick everything.
	 *
	 * And by the way: This now bricks all Windows compatibility. Deal with it.
	 */
	process_id = fork();
	//process_id = -1;
	if (process_id < 0) {
		/* This should not happen */
		fprintf(stderr, _("Fatal error: Cairo Renderer: Could not spawn child process!"));
		exit(-2);
	} else if (process_id > 0) {
		/* Woohoo... Successfully dumped the shitty code to an unknowing victim */
		goto ret_parent;
	}

	/* We are now in a separate process just for rendering the output image.
     * You may print a log message to the activity bar of the gui by writing a line
     * teminated with '\n' to comm_pipe[1]. This will be handled by the parent process.
     * Directly calling the update function 
     *     gds_output_renderer_update_async_progress()
     * does not have any effect because this is a separate process.
     */
  
	/*
     * Close stdin and (stdout and stderr may live on)
     */
	close(0);
	close(comm_pipe[0]);

	layers = (struct cairo_layer *)calloc(MAX_LAYERS, sizeof(struct cairo_layer));

	/* Clear layers */
	for (i = 0; i < MAX_LAYERS; i++) {
		layers[i].cr = NULL;
		layers[i].rec = NULL;
	}

	/* Create recording surface for each layer */
	for (info_list = layer_infos; info_list != NULL; info_list = g_list_next(info_list)) {
		linfo = (struct layer_info *)info_list->data;
		if (linfo->layer < MAX_LAYERS) {
			/* Layer shall not be rendered */
			if (!linfo->render)
				continue;

			lay = &(layers[(unsigned int)linfo->layer]);
			lay->linfo = linfo;
			lay->rec = cairo_recording_surface_create(CAIRO_CONTENT_COLOR_ALPHA,
								  NULL);
			lay->cr = cairo_create(layers[(unsigned int)linfo->layer].rec);
			cairo_scale(lay->cr, 1, -1); // Fix coordinate system
			cairo_set_source_rgb(lay->cr, linfo->color.red, linfo->color.green, linfo->color.blue);
		} else {
			printf("Layer number (%d) too high!\n", linfo->layer);
			goto ret_clear_layers;
		}
	}

	dprintf(comm_pipe[1], "Rendering layers\n");
	render_cell(cell, layers, scale);

	/* get size of image and top left coordinate */
	for (info_list = layer_infos; info_list != NULL; info_list = g_list_next(info_list)) {
		linfo = (struct layer_info *)info_list->data;

		if (linfo->layer >= MAX_LAYERS) {
			printf(_("Layer number too high / outside of spec.\n"));
			continue;
		}

		if (!linfo->render)
			continue;

		/* Print size */
		cairo_recording_surface_ink_extents(layers[linfo->layer].rec, &rec_x0, &rec_y0,
				&rec_width, &rec_height);
		dprintf(comm_pipe[1], _("Size of layer %d%s%s%s: <%lf x %lf> @ (%lf | %lf)\n"),
			linfo->layer,
			(linfo->name && linfo->name[0] ? " (" : ""),
			(linfo->name && linfo->name[0] ? linfo->name : ""),
			(linfo->name && linfo->name[0] ? ")" : ""),
			rec_width, rec_height, rec_x0, rec_y0);

		/* update bounding box */
		xmin = MIN(xmin, rec_x0);
		xmax = MAX(xmax, rec_x0);
		ymin = MIN(ymin, rec_y0);
		ymax = MAX(ymax, rec_y0);
		xmin = MIN(xmin, rec_x0+rec_width);
		xmax = MAX(xmax, rec_x0+rec_width);
		ymin = MIN(ymin, rec_y0+rec_height);
		ymax = MAX(ymax, rec_y0+rec_height);

	}

	/* printf("Cell bounding box: (%lf | %lf) -- (%lf | %lf)\n", xmin, ymin, xmax, ymax); */

	if (pdf_file) {
		pdf_surface = cairo_pdf_surface_create(pdf_file, xmax-xmin, ymax-ymin);
		pdf_cr = cairo_create(pdf_surface);
	}

	if (svg_file) {
		svg_surface = cairo_svg_surface_create(svg_file, xmax-xmin, ymax-ymin);
		svg_cr = cairo_create(svg_surface);
	}

	/* Write layers to PDF */
	for (info_list = layer_infos; info_list != NULL; info_list = g_list_next(info_list)) {
		linfo = (struct layer_info *)info_list->data;

		if (linfo->layer >= MAX_LAYERS) {
			printf(_("Layer outside of spec.\n"));
			continue;
		}

		if (!linfo->render)
			continue;

		dprintf(comm_pipe[1], _("Exporting layer %d to file\n"), linfo->layer);

		if (pdf_file && pdf_cr) {
			cairo_set_source_surface(pdf_cr, layers[linfo->layer].rec, -xmin, -ymin);
			cairo_paint_with_alpha(pdf_cr, linfo->color.alpha);
		}

		if (svg_file && svg_cr) {
			cairo_set_source_surface(svg_cr, layers[linfo->layer].rec, -xmin, -ymin);
			cairo_paint_with_alpha(svg_cr, linfo->color.alpha);
		}
	}

	if (pdf_file) {
		cairo_show_page(pdf_cr);
		cairo_destroy(pdf_cr);
		cairo_surface_destroy(pdf_surface);
	}

	if (svg_file) {
		cairo_show_page(svg_cr);
		cairo_destroy(svg_cr);
		cairo_surface_destroy(svg_surface);
	}

ret_clear_layers:
	for (i = 0; i < MAX_LAYERS; i++) {
		lay = &layers[i];
		if (lay->cr) {
			cairo_destroy(lay->cr);
			cairo_surface_destroy(lay->rec);
		}
	}
	free(layers);

	printf(_("Cairo export finished. It might still be buggy!\n"));

	/* Suspend child process */
	exit(0);

ret_parent:
	close(comm_pipe[1]);

	while (read_line_from_fd(comm_pipe[0], receive_message, sizeof(receive_message)) > 0) {
		/* Strip \n from string and replace with ' ' */
		for (i = 0; receive_message[i] != '\0'; i++) {
			if (receive_message[i] == '\n')
				receive_message[i] = ' ';
		}

		/* Update asyc progress*/
		gds_output_renderer_update_async_progress(renderer, receive_message);
	}

	waitpid(process_id, NULL, 0);

	close(comm_pipe[0]);
	return 0;
}

static void cairo_renderer_init(CairoRenderer *self)
{
	/* PDF default */
	self->svg = FALSE;
}

static int cairo_renderer_render_output(GdsOutputRenderer *renderer,
					struct gds_cell *cell,
					double scale)
{
	CairoRenderer *c_renderer = GDS_RENDER_CAIRO_RENDERER(renderer);
	const char *pdf_file = NULL;
	const char *svg_file = NULL;
	LayerSettings *settings;
	GList *layer_infos = NULL;
	const char *output_file;
	int ret;

	if (!c_renderer)
		return -2000;

	output_file = gds_output_renderer_get_output_file(renderer);
	settings = gds_output_renderer_get_and_ref_layer_settings(renderer);

	/* Set layer info list. In case of failure it remains NULL */
	if (settings)
		layer_infos = layer_settings_get_layer_info_list(settings);

	if (c_renderer->svg == TRUE)
		svg_file = output_file;
	else
		pdf_file = output_file;

	gds_output_renderer_update_async_progress(renderer, _("Rendering Cairo Output..."));
	ret = cairo_renderer_render_cell_to_vector_file(renderer, cell, layer_infos, pdf_file, svg_file, scale);

	if (settings)
		g_object_unref(settings);

	return ret;
}

static void cairo_renderer_class_init(CairoRendererClass *klass)
{
	GdsOutputRendererClass *renderer_class = GDS_RENDER_OUTPUT_RENDERER_CLASS(klass);

	renderer_class->render_output = cairo_renderer_render_output;
}

CairoRenderer *cairo_renderer_new_pdf()
{
	CairoRenderer *renderer;

	renderer = GDS_RENDER_CAIRO_RENDERER(g_object_new(GDS_RENDER_TYPE_CAIRO_RENDERER, NULL));
	renderer->svg = FALSE;

	return renderer;
}

CairoRenderer *cairo_renderer_new_svg()
{
	CairoRenderer *renderer;

	renderer = GDS_RENDER_CAIRO_RENDERER(g_object_new(GDS_RENDER_TYPE_CAIRO_RENDERER, NULL));
	renderer->svg = TRUE;

	return renderer;
}

/** @} */
