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
  * @file cairo-output.c
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

#include <gds-render/output-renderers/cairo-output.h>
#include <sys/wait.h>
#include <unistd.h>

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
 * @param rotation Rotattion in degrees
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
		if ((temp_cell = cell_instance->cell_ref) != NULL) {
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
		if ((cr = layers[gfx->layer].cr) == NULL)
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
		case GRAPHIC_POLYGON:
			cairo_set_line_width(cr, 0.1/scale);
			cairo_close_path(cr);
			cairo_stroke_preserve(cr); // Prevent graphic glitches
			cairo_fill(cr);
			break;
		}

	}

}

void cairo_render_cell_to_vector_file(struct gds_cell *cell, GList *layer_infos, char *pdf_file, char *svg_file, double scale)
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

	if (pdf_file == NULL && svg_file == NULL) {
		/* No output specified */
		return;
	}

	/* Fork to a new child process. This ensures the memory leaks (see issue #16) in Cairo don't
	 * brick everything.
	 *
	 * And by the way: This now bricks all Windows compatibility. Deal with it.
	 */
	process_id = fork();
	if (process_id < 0) {
		/* Well... shit... We have to run it in our process. */
	} else if (process_id > 0) {
		/* Woohoo... Successfully dumped the shitty code to an unknowing victim */
		goto ret_parent;
	}

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


	render_cell(cell, layers, scale);

	/* get size of image and top left coordinate */
	for (info_list = layer_infos; info_list != NULL; info_list = g_list_next(info_list)) {
		linfo = (struct layer_info *)info_list->data;

		if (linfo->layer >= MAX_LAYERS) {
			printf("Layer outside of Spec.\n");
			continue;
		}

		/* Print size */
		cairo_recording_surface_ink_extents(layers[linfo->layer].rec, &rec_x0, &rec_y0,
				&rec_width, &rec_height);
		printf("Size of layer %d%s%s%s: <%lf x %lf> @ (%lf | %lf)\n",
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

	printf("Cell bounding box: (%lf | %lf) -- (%lf | %lf)\n", xmin, ymin, xmax, ymax);

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
			printf("Layer outside of Spec.\n");
			continue;
		}

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
		if(lay->cr) {
			cairo_destroy(lay->cr);
			cairo_surface_destroy(lay->rec);
		}
	}
	free(layers);

	printf("Cairo export finished. It might still be buggy!\n");

	/* If forked, suspend process */
	if (process_id == 0)
		exit(0);

	/* Fork didn't work. Just return here */
	return;
ret_parent:
	waitpid(process_id, NULL, 0);
	return;
}

/** @} */
