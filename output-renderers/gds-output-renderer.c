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

struct renderer_params {
		struct gds_cell *cell;
		double scale;
};

typedef struct {
	gchar *output_file;
	LayerSettings *layer_settings;
	GMutex settings_lock;
	gboolean mutex_init_status;
	GTask *task;
	struct renderer_params async_params;
	gpointer padding[11];
} GdsOutputRendererPrivate;

enum {
	PROP_OUTPUT_FILE = 1,
	PROP_LAYER_SETTINGS,
	N_PROPERTIES
};

G_DEFINE_TYPE_WITH_PRIVATE(GdsOutputRenderer, gds_output_renderer, G_TYPE_OBJECT)

enum gds_output_renderer_signal_ids {ASYNC_FINISHED = 0, ASYNC_PROGRESS_UPDATE, GDS_OUTPUT_RENDERER_SIGNAL_COUNT};
static guint gds_output_renderer_signals[GDS_OUTPUT_RENDERER_SIGNAL_COUNT];

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

	if (priv->mutex_init_status) {
		/* Try locking the mutex, to test if it's free */
		g_mutex_lock(&priv->settings_lock);
		g_mutex_unlock(&priv->settings_lock);
		g_mutex_clear(&priv->settings_lock);
		priv->mutex_init_status = FALSE;
	}

	g_clear_object(&priv->task);

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
		g_mutex_lock(&priv->settings_lock);
		if (priv->output_file)
			g_free(priv->output_file);
		priv->output_file = g_strdup(g_value_get_string(value));
		g_mutex_unlock(&priv->settings_lock);
		break;
	case PROP_LAYER_SETTINGS:
		g_mutex_lock(&priv->settings_lock);
		g_clear_object(&priv->layer_settings);
		priv->layer_settings = g_value_get_object(value);
		g_object_ref(priv->layer_settings);
		g_mutex_unlock(&priv->settings_lock);
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

	/* Setup output signals */
	gds_output_renderer_signals[ASYNC_FINISHED] =
			g_signal_newv("async-finished", GDS_RENDER_TYPE_OUTPUT_RENDERER,
				      G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE,
				      NULL,
				      NULL,
				      NULL,
				      NULL,
				      G_TYPE_NONE,
				      0,
				      NULL);
	gds_output_renderer_signals[ASYNC_PROGRESS_UPDATE] =
			g_signal_newv("progress-update", GDS_RENDER_TYPE_OUTPUT_RENDERER,
				      G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE,
				      NULL,
				      NULL,
				      NULL,
				      NULL,
				      G_TYPE_NONE,
				      0,
				      NULL);
}

void gds_output_renderer_init(GdsOutputRenderer *self)
{
	GdsOutputRendererPrivate *priv;

	priv = gds_output_renderer_get_instance_private(self);

	priv->layer_settings = NULL;
	priv->output_file = NULL;
	priv->task = NULL;
	priv->mutex_init_status = TRUE;
	g_mutex_init(&priv->settings_lock);

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

LayerSettings *gds_output_renderer_get_and_ref_layer_settings(GdsOutputRenderer *renderer)
{
	LayerSettings *ret = NULL;
	GdsOutputRendererPrivate *priv;

	priv = gds_output_renderer_get_instance_private(renderer);

	/* Acquire settings lock */
	g_mutex_lock(&priv->settings_lock);

	g_object_get(renderer, "layer-settings", &ret, NULL);
	/* Reference it, so it is not cleared by another thread overwriting the property */
	g_object_ref(ret);

	/* It is now safe to clear the lock */
	g_mutex_unlock(&priv->settings_lock);

	return ret;
}

void gds_output_renderer_set_layer_settings(GdsOutputRenderer *renderer, LayerSettings *settings)
{
	g_return_if_fail(GDS_RENDER_IS_LAYER_SETTINGS(settings));

	g_object_set(renderer, "layer-settings", settings, NULL);
}

int gds_output_renderer_render_output(GdsOutputRenderer *renderer, struct gds_cell *cell, double scale)
{
	int ret;
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

	ret = klass->render_output(renderer, cell, scale);

	return ret;
}

static void gds_output_renderer_async_wrapper(GTask *task,
					     gpointer source_object,
					     gpointer task_data,
					     GCancellable *cancellable)
{
	GdsOutputRenderer *renderer;
	GdsOutputRendererPrivate *priv;
	int ret;

	renderer = GDS_RENDER_OUTPUT_RENDERER(source_object);
	priv = gds_output_renderer_get_instance_private(renderer);
	if (!priv) {
		ret = -1000;
		goto ret_from_task;
	}
	if(!priv->mutex_init_status) {
		ret = -1001;
		goto ret_from_task;
	}

	ret = gds_output_renderer_render_output(renderer, priv->async_params.cell, priv->async_params.scale);

ret_from_task:
	g_task_return_int(task, ret);
}

static void gds_output_renderer_async_finished(GObject *src_obj, GAsyncResult *res, gpointer user_data)
{
	GdsOutputRendererPrivate *priv;
	(void)user_data;
	(void)res; /* Will hopefully be destroyed later */

	priv = gds_output_renderer_get_instance_private(GDS_RENDER_OUTPUT_RENDERER(src_obj));

	g_signal_emit(src_obj, gds_output_renderer_signals[ASYNC_FINISHED], 0);
	g_clear_object(&priv->task);
}

int gds_output_renderer_render_output_async(GdsOutputRenderer *renderer, struct gds_cell *cell, double scale)
{
	GdsOutputRendererPrivate *priv;
	int ret = -1;

	priv = gds_output_renderer_get_instance_private(renderer);
	if (priv->task) {
		g_warning("renderer already started asynchronously");
		return -2000;
	}

	priv->task = g_task_new(renderer, NULL, gds_output_renderer_async_finished, NULL);
	g_mutex_lock(&priv->settings_lock);
	priv->async_params.cell = cell;
	priv->async_params.scale = scale;
	g_mutex_unlock(&priv->settings_lock);

	g_task_run_in_thread(priv->task, gds_output_renderer_async_wrapper);

	return ret;
}

/** @} */
