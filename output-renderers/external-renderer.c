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
 * @file external-renderer.c
 * @brief This file implements the dynamic library loading for the external rendering feature
 * @author Mario Hüttel <mario.huettel@gmx.net>
 */

/**
 * @addtogroup ExternalRenderer
 * @{
 */

#include <dlfcn.h>
#include <stdio.h>

#include <gds-render/output-renderers/external-renderer.h>

struct _ExternalRenderer {
	GdsOutputRenderer parent;
	char *shared_object_path;
};

enum {
	PROP_SO_PATH = 1, /**< @brief Shared object path property */
	N_PROPERTIES /**< @brief Used to get property count */
};

G_DEFINE_TYPE(ExternalRenderer, external_renderer, GDS_RENDER_TYPE_OUTPUT_RENDERER)

/**
 * @brief Execute render function in shared object to render the supplied cell
 * @param toplevel_cell Cell to render
 * @param layer_info_list Layer information (Color etc.)
 * @param output_file Destination file
 * @param scale the scaling value to scale the output cell down by.
 * @param so_path Path to shared object
 * @return 0 if successful
 */
static int external_renderer_render_cell(struct gds_cell *toplevel_cell, GList *layer_info_list,
				   const char *output_file, double scale,  const char *so_path)
{
	int (*so_render_func)(struct gds_cell *, GList *, const char *, double) = NULL;
	void *so_handle = NULL;
	char *error_msg;
	int ret = 0;

	if (!so_path) {
		fprintf(stderr, "Path to shared object not set!\n");
		return -3000;
	}

	/* Check parameter sanity */
	if (!output_file || !toplevel_cell || !layer_info_list)
		return -3000;

	/* Load shared object */
	so_handle = dlopen(so_path, RTLD_LAZY);
	if (!so_handle) {
		fprintf(stderr, "Could not load external library '%s'\nDetailed error is:\n%s\n", so_path, dlerror());
		return -2000;
	}

	/* Load symbol from library */
	so_render_func = (int (*)(struct gds_cell *, GList *, const char *, double))
				dlsym(so_handle, xstr(EXTERNAL_LIBRARY_RENDER_FUNCTION));
	error_msg = dlerror();
	if (error_msg != NULL) {
		fprintf(stderr, "Rendering function not found in library:\n%s\n", error_msg);
		goto ret_close_so_handle;
	}

	/* Execute */
	if (so_render_func) {
		g_message("Calling external renderer.");
		ret = so_render_func(toplevel_cell, layer_info_list, output_file, scale);
		g_message("External renderer finished.");
	}

ret_close_so_handle:
	dlclose(so_handle);
	return ret;
}

static int external_renderer_render_output(GdsOutputRenderer *renderer,
					    struct gds_cell *cell,
					    double scale)
{
	ExternalRenderer *ext_renderer = GDS_RENDER_EXTERNAL_RENDERER(renderer);
	LayerSettings *settings;
	GList *layer_infos = NULL;
	const char *output_file;
	int ret;

	output_file = gds_output_renderer_get_output_file(renderer);
	settings = gds_output_renderer_get_and_ref_layer_settings(renderer);

	/* Set layer info list. In case of failure it remains NULL */
	if (settings)
		layer_infos = layer_settings_get_layer_info_list(settings);

	ret = external_renderer_render_cell(cell, layer_infos, output_file, scale, ext_renderer->shared_object_path);
	if (settings)
		g_object_unref(settings);

	return ret;
}

static void external_renderer_get_property(GObject *obj, guint property_id, GValue *value, GParamSpec *pspec)
{
	ExternalRenderer *self;

	self = GDS_RENDER_EXTERNAL_RENDERER(obj);

	switch (property_id) {
	case PROP_SO_PATH:
		g_value_set_string(value, self->shared_object_path);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, property_id, pspec);
		break;
	}
}

static void external_renderer_set_property(GObject *obj, guint property_id, const GValue *value, GParamSpec *pspec)
{
	ExternalRenderer *self;

	self = GDS_RENDER_EXTERNAL_RENDERER(obj);

	switch (property_id) {
	case PROP_SO_PATH:
		if (self->shared_object_path)
			g_free(self->shared_object_path);
		self->shared_object_path = g_value_dup_string(value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, property_id, pspec);
		break;
	}
}

static void external_renderer_dispose(GObject *self_obj)
{
	ExternalRenderer *self = GDS_RENDER_EXTERNAL_RENDERER(self_obj);

	if (self->shared_object_path) {
		g_free(self->shared_object_path);
		self->shared_object_path = NULL;
	}

	G_OBJECT_CLASS(external_renderer_parent_class)->dispose(self_obj);
}

static GParamSpec *external_renderer_properties[N_PROPERTIES] = {NULL};

static void external_renderer_class_init(ExternalRendererClass *klass)
{
	GdsOutputRendererClass *inherited_parent_class;
	GObjectClass *oclass;

	inherited_parent_class = GDS_RENDER_OUTPUT_RENDERER_CLASS(klass);
	oclass = G_OBJECT_CLASS(klass);

	/* Override virtual function */
	inherited_parent_class->render_output = external_renderer_render_output;

	/* Setup Gobject callbacks */
	oclass->set_property = external_renderer_set_property;
	oclass->get_property = external_renderer_get_property;
	oclass->dispose = external_renderer_dispose;

	/* Setup properties */
	external_renderer_properties[PROP_SO_PATH] =
			g_param_spec_string("shared-object-path",
					    "Shared object file path",
					    "Path to the shared object to search rendering function in.",
					    NULL,
					    G_PARAM_READWRITE);
	g_object_class_install_properties(oclass, N_PROPERTIES, external_renderer_properties);
}

static void external_renderer_init(ExternalRenderer *self)
{
	self->shared_object_path = NULL;
}

ExternalRenderer *external_renderer_new()
{
	return g_object_new(GDS_RENDER_TYPE_EXTERNAL_RENDERER, NULL);
}

ExternalRenderer *external_renderer_new_with_so(const char *so_path)
{
	return g_object_new(GDS_RENDER_TYPE_EXTERNAL_RENDERER, "shared-object-path", so_path, NULL);
}

/** @} */
