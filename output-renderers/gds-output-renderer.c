/*
 * GDSII-Converter
 * Copyright (C) 2019  Mario Hüttel <mario.huettel@gmx.net>
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
  * @file gds-output-renderer.c
  * @brief Base GObject class for output renderers
  *
  * All output renderers are derived from this class
  *
  * @author Mario Hüttel <mario.huettel@gmx.net>
  */

/** @addtogroup GdsOutputRenderer
 *  @{
 */

#include <gds-render/output-renderers/gds-output-renderer.h>

typedef struct {
	gpointer padding[12];
} GdsOutputRendererPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(GdsOutputRenderer, gds_output_renderer, G_TYPE_OBJECT)

static int gds_output_renderer_render_dummy(GdsOutputRenderer *renderer,
						struct gds_cell *cell,
						GList *layer_infos,
						const char *output_file,
						double scale)
{
	(void)renderer;
	(void)cell;
	(void)layer_infos;
	(void)output_file;
	(void)scale;

	g_warning("Output renderer does not define a render_output function!");
	return 0;
}

static void gds_output_renderer_class_init(GdsOutputRendererClass *klass)
{
	klass->render_output = gds_output_renderer_render_dummy;
}

void gds_output_renderer_init(GdsOutputRenderer *self)
{
	(void)self;

	return;
}

GdsOutputRenderer *gds_output_renderer_new()
{
	return GDS_RENDER_OUTPUT_RENDERER(g_object_new(GDS_RENDER_TYPE_OUTPUT_RENDERER, NULL));
}

int gds_output_renderer_render_output(GdsOutputRenderer *renderer, struct gds_cell *cell, GList *layer_infos, const char *output_file, double scale)
{
	GdsOutputRendererClass *klass;

	if (GDS_RENDER_IS_OUTPUT_RENDERER(renderer) == FALSE) {
		g_error("Output Renderer not valid.");
		return GDS_OUTPUT_RENDERER_GEN_ERR;
	}

	if (!cell || !layer_infos || !output_file) {
		g_error("Output renderer called with insufficient parameters.");
		return GDS_OUTPUT_RENDERER_PARAM_ERR;
	}

	klass = GDS_RENDER_OUTPUT_RENDERER_GET_CLASS(renderer);
	if (klass->render_output == NULL) {
		g_critical("Output Renderer: Rendering function broken. This is a bug.");
		return GDS_OUTPUT_RENDERER_GEN_ERR;
	}

	return klass->render_output(renderer, cell, layer_infos, output_file, scale);
}


/** @} */
