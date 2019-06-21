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
#include <gds-render/layer/layer-info.h>

typedef struct {
	gchar *output_file;
	LayerSettings *layer_settings;
	gpointer padding[11];
} GdsOutputRendererPrivate;

enum {
	PROP_OUTPUT_FILE = 1,
	PROP_LAYER_SETTINGS,
	N_PROPERTIES
};

G_DEFINE_TYPE_WITH_PRIVATE(GdsOutputRenderer, gds_output_renderer, G_TYPE_OBJECT)

static int gds_output_renderer_render_dummy(GdsOutputRenderer *renderer,
						struct gds_cell *cell,
						double scale)
{
	(void)renderer;
	(void)cell;
	(void)scale;

	g_warning("Output renderer does not define a render_output function!");
	return 0;
}

static void gds_output_renderer_dispose(GObject *self_obj)
{
	GdsOutputRenderer *renderer = GDS_RENDER_OUTPUT_RENDERER(self_obj);
	GdsOutputRendererPrivate *priv;

	priv = gds_output_renderer_get_instance_private(renderer);
	if (priv->output_file)
		g_free(priv->output_file);

	g_clear_object(&priv->layer_settings);

	/* Chain up to parent class */
	G_OBJECT_CLASS(gds_output_renderer_parent_class)->dispose(self_obj);
}

static void gds_output_renderer_get_property(GObject *obj, guint property_id, GValue *value, GParamSpec *pspec)
{
	GdsOutputRenderer *self = GDS_RENDER_OUTPUT_RENDERER(obj);
	GdsOutputRendererPrivate *priv;

	priv = gds_output_renderer_get_instance_private(self);

	switch (property_id) {
	case PROP_OUTPUT_FILE:
		g_value_set_string(value, priv->output_file);
		break;
	case PROP_LAYER_SETTINGS:
		g_value_set_object(value, priv->layer_settings);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, property_id, pspec);
		break;
	}
}

static void gds_output_renderer_set_property(GObject *obj, guint property_id, const GValue *value, GParamSpec *pspec)
{
	GdsOutputRenderer *self = GDS_RENDER_OUTPUT_RENDERER(obj);
	GdsOutputRendererPrivate *priv;

	priv = gds_output_renderer_get_instance_private(self);

	switch (property_id) {
	case PROP_OUTPUT_FILE:
		if (priv->output_file)
			g_free(priv->output_file);
		priv->output_file = g_strdup(g_value_get_string(value));
		break;
	case PROP_LAYER_SETTINGS:
		g_clear_object(&priv->layer_settings);
		priv->layer_settings = g_value_get_object(value);
		g_object_ref(priv->layer_settings);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, property_id, pspec);
		break;
	}
}

static GParamSpec *gds_output_renderer_properties[N_PROPERTIES] = {NULL};

static void gds_output_renderer_class_init(GdsOutputRendererClass *klass)
{
	GObjectClass *oclass = G_OBJECT_CLASS(klass);

	klass->render_output = gds_output_renderer_render_dummy;

	oclass->dispose = gds_output_renderer_dispose;
	oclass->set_property = gds_output_renderer_set_property;
	oclass->get_property = gds_output_renderer_get_property;

	/* Setup properties */
	gds_output_renderer_properties[PROP_OUTPUT_FILE] =
			g_param_spec_string("output-file", "output file", "Output file for renderer",
					    NULL, G_PARAM_READWRITE);
	gds_output_renderer_properties[PROP_LAYER_SETTINGS] =
			g_param_spec_object("layer-settings", "Layer Settings object",
					    "Object containing the layer rendering information",
					    GDS_RENDER_TYPE_LAYER_SETTINGS, G_PARAM_READWRITE);
	g_object_class_install_properties(oclass, N_PROPERTIES, gds_output_renderer_properties);
}

void gds_output_renderer_init(GdsOutputRenderer *self)
{
	GdsOutputRendererPrivate *priv;

	priv = gds_output_renderer_get_instance_private(self);

	priv->output_file = NULL;
	return;
}

GdsOutputRenderer *gds_output_renderer_new()
{
	return GDS_RENDER_OUTPUT_RENDERER(g_object_new(GDS_RENDER_TYPE_OUTPUT_RENDERER, NULL));
}

GdsOutputRenderer *gds_output_renderer_new_with_props(const char *output_file, LayerSettings *layer_settings)
{
	return GDS_RENDER_OUTPUT_RENDERER(g_object_new(GDS_RENDER_TYPE_OUTPUT_RENDERER,
						       "layer-settings", layer_settings,
						       "output-file", output_file,
						       NULL));
}

void gds_output_renderer_set_output_file(GdsOutputRenderer *renderer, const gchar *file_name)
{
	g_return_if_fail(GDS_RENDER_IS_OUTPUT_RENDERER(renderer));

	/* Check if the filename is actually filled */
	if (!file_name || !file_name[0])
		return;
	g_object_set(renderer, "output-file", file_name, NULL);
}

const char *gds_output_renderer_get_output_file(GdsOutputRenderer *renderer)
{
	const char *file = NULL;

	g_object_get(renderer, "output-file", &file, NULL);
	return file;
}

LayerSettings *gds_output_renderer_get_layer_settings(GdsOutputRenderer *renderer)
{
	LayerSettings *ret = NULL;

	g_object_get(renderer, "layer-settings", &ret, NULL);
	return ret;
}

void gds_output_renderer_set_layer_settings(GdsOutputRenderer *renderer, LayerSettings *settings)
{
	g_return_if_fail(GDS_RENDER_IS_LAYER_SETTINGS(settings));

	g_object_set(renderer, "layer_settings", settings, NULL);
}

int gds_output_renderer_render_output(GdsOutputRenderer *renderer, struct gds_cell *cell, double scale)
{
	GdsOutputRendererClass *klass;
	GdsOutputRendererPrivate *priv = gds_output_renderer_get_instance_private(renderer);

	if (GDS_RENDER_IS_OUTPUT_RENDERER(renderer) == FALSE) {
		g_error("Output Renderer not valid.");
		return GDS_OUTPUT_RENDERER_GEN_ERR;
	}

	if (!priv->output_file || !priv->output_file[0]) {
		g_error("No/invalid output file set.");
		return GDS_OUTPUT_RENDERER_GEN_ERR;
	}

	if (!priv->layer_settings) {
		g_error("No layer specification supplied.");
		return GDS_OUTPUT_RENDERER_GEN_ERR;
	}

	if (!cell) {
		g_error("Output renderer called without cell to render.");
		return GDS_OUTPUT_RENDERER_PARAM_ERR;
	}

	klass = GDS_RENDER_OUTPUT_RENDERER_GET_CLASS(renderer);
	if (klass->render_output == NULL) {
		g_critical("Output Renderer: Rendering function broken. This is a bug.");
		return GDS_OUTPUT_RENDERER_GEN_ERR;
	}

	return klass->render_output(renderer, cell, scale);
}


/** @} */
