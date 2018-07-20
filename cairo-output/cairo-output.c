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

#include "cairo-output.h"
#include <math.h>
#include <stdlib.h>
#include <cairo.h>
#include <cairo-pdf.h>

struct cairo_layer {
	cairo_t *cr;
	cairo_surface_t *rec;
	struct layer_info *linfo;
};

static void revert_inherited_transform(struct cairo_layer *layers)
{
	int i;

	for (i = 0; i < MAX_LAYERS; i++) {
		if (layers[i].cr == NULL)
				continue;
		cairo_restore(layers[i].cr);
	}
}

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
		cairo_set_line_width(cr, (gfx->width_absolute ? gfx->width_absolute : 1));

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
			cairo_close_path(cr);
			cairo_fill(cr);
			break;
		}

	}

}

void cairo_render_cell_to_pdf(struct gds_cell *cell, GList *layer_infos, char *pdf_file, double scale)
{
	cairo_surface_t *surface;
	cairo_t *cr;
	struct layer_info *linfo;
	struct cairo_layer *layers;
	struct cairo_layer *lay;
	GList *info_list;
	int i;

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
			cairo_set_source_rgb(lay->cr, 1, 0, 0);
		} else {
			printf("Layer number (%d) too high!\n", linfo->layer);
			goto ret_clear_layers;
		}
	}


	render_cell(cell, layers, scale);

	/* Todo: Tranfer all layers in order to output file */


ret_clear_layers:
	for (i = 0; i < MAX_LAYERS; i++) {
		lay = &layers[i];
		cairo_destroy(lay->cr);
		cairo_surface_destroy(lay->rec);
	}
	free(layers);

	printf("cairo export not yet implemented!\n");
}
