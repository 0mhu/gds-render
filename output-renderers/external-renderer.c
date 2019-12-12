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
#include <sys/wait.h>
#include <glib/gi18n.h>

#include <gds-render/output-renderers/external-renderer.h>
#include <gds-render/version.h>

#define FORCE_FORK 0U /**< @brief if != 0, then forking is forced regardless of the shared object's settings */

struct _ExternalRenderer {
	GdsOutputRenderer parent;
	char *shared_object_path;
	char *cli_param_string;
};

enum {
	PROP_SO_PATH = 1, /**< @brief Shared object path property */
	PROP_PARAM_STRING, /** @brief Shared object renderer parameter string from CLI */
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
 * @param params Parameters passed to EXTERNAL_LIBRARY_INIT_FUNCTION
 * @return 0 if successful
 */
static int external_renderer_render_cell(struct gds_cell *toplevel_cell, GList *layer_info_list,
				   const char *output_file, double scale,  const char *so_path, const char *params)
{
	int (*so_render_func)(struct gds_cell *, GList *, const char *, double) = NULL;
	int (*so_init_func)(const char *, const char *) = NULL;
	void *so_handle = NULL;
	char *error_msg;
	int forking_req;
	int ret = 0;
	pid_t fork_pid = 0;
	int forked_status;

	if (!so_path) {
		fprintf(stderr, _("Path to shared object not set!\n"));
		return -3000;
	}

	/* Check parameter sanity */
	if (!output_file || !toplevel_cell || !layer_info_list)
		return -3000;

	/* Load shared object */
	so_handle = dlopen(so_path, RTLD_LAZY);
	if (!so_handle) {
		fprintf(stderr, _("Could not load external library '%s'\nDetailed error is:\n%s\n"), so_path, dlerror());
		return -2000;
	}

	/* Load rendering symbol from library */
	so_render_func = (int (*)(struct gds_cell *, GList *, const char *, double))
				dlsym(so_handle, xstr(EXTERNAL_LIBRARY_RENDER_FUNCTION));
	error_msg = dlerror();
	if (error_msg != NULL) {
		fprintf(stderr, _("Rendering function not found in library:\n%s\n"), error_msg);
		goto ret_close_so_handle;
	}

	/* Load the init function */
	so_init_func = (int (*)(const char *, const char *))dlsym(so_handle, xstr(EXTERNAL_LIBRARY_INIT_FUNCTION));
	error_msg = dlerror();
	if (error_msg != NULL) {
		fprintf(stderr, _("Init function not found in library:\n%s\n"), error_msg);
		goto ret_close_so_handle;
	}

	/* Check if forking is requested */
	if (dlsym(so_handle, xstr(EXTERNAL_LIBRARY_FORK_REQUEST)))
		forking_req = 1;
	else if (FORCE_FORK)
		forking_req = 1;
	else
		forking_req = 0;

	/* Execute */

	g_message(_("Calling external renderer."));

	if (forking_req)
		fork_pid = fork();
	if (fork_pid != 0)
		goto end_forked;

	ret = so_init_func(params, _app_version_string);
	if (!ret)
		ret = so_render_func(toplevel_cell, layer_info_list, output_file, scale);

	/* If we are in a separate process, terminate here */
	if (forking_req)
		exit(ret);

	/* The forked paths end here */
end_forked:
	if (forking_req) {
		waitpid(fork_pid, &forked_status, 0);
		ret = WEXITSTATUS(forked_status);
	}

	g_message(_("External renderer finished."));

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

	ret = external_renderer_render_cell(cell, layer_infos, output_file, scale, ext_renderer->shared_object_path,
					    ext_renderer->cli_param_string);
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
	case PROP_PARAM_STRING:
		g_value_set_string(value, self->cli_param_string);
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
	case PROP_PARAM_STRING:
		if (self->cli_param_string)
			g_free(self->cli_param_string);
		self->cli_param_string = g_value_dup_string(value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, property_id, pspec);
		break;
	}
}

static void external_renderer_dispose(GObject *self_obj)
{
	ExternalRenderer *self;

	self = GDS_RENDER_EXTERNAL_RENDERER(self_obj);

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
			g_param_spec_string(N_("shared-object-path"),
					    N_("Shared object file path"),
					    N_("Path to the shared object to search rendering function in."),
					    NULL,
					    G_PARAM_READWRITE);
	external_renderer_properties[PROP_PARAM_STRING] =
			g_param_spec_string(N_("param-string"),
					    N_("Shared object renderer parameter string"),
					    N_("Command line arguments passed to the external shared object renderer"),
					    NULL,
					    G_PARAM_READWRITE);
	g_object_class_install_properties(oclass, N_PROPERTIES, external_renderer_properties);
}

static void external_renderer_init(ExternalRenderer *self)
{
	self->shared_object_path = NULL;
	self->cli_param_string = NULL;
}

ExternalRenderer *external_renderer_new()
{
	return g_object_new(GDS_RENDER_TYPE_EXTERNAL_RENDERER, NULL);
}

ExternalRenderer *external_renderer_new_with_so_and_param(const char *so_path, const char *param_string)
{
	return g_object_new(GDS_RENDER_TYPE_EXTERNAL_RENDERER, N_("shared-object-path"), so_path,
				N_("param-string"), param_string, NULL);
}

/** @} */
