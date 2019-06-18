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
	const char *shared_object_path;
};

G_DEFINE_TYPE(ExternalRenderer, external_renderer, GDS_RENDER_TYPE_OUTPUT_RENDERER)

/**
 * @brief Execute render function in shared object to render the supplied cell
 * @param toplevel_cell Cell to render
 * @param layer_info_list Layer information (Color etc.)
 * @param output_file Destination file
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

	/* Check parameter sanity */
	if (!output_file || !so_path || !toplevel_cell || !layer_info_list)
		return -3000;

	/* Load shared object */
	so_handle = dlopen(so_path, RTLD_LAZY);
	if (!so_handle) {
		g_error("Could not load external library '%s'\nDetailed error is:\n%s", so_path, dlerror());
		return -2000;
	}

	/* Load symbol from library */
	so_render_func = (int (*)(struct gds_cell *, GList *, const char *, double))dlsym(so_handle, EXTERNAL_LIBRARY_FUNCTION);
	error_msg = dlerror();
	if (error_msg != NULL) {
		g_error("Rendering function not found in library:\n%s\n", error_msg);
		goto ret_close_so_handle;
	}

	/* Execute */
	if (so_render_func)
		ret = so_render_func(toplevel_cell, layer_info_list, output_file, scale);

ret_close_so_handle:
	dlclose(so_handle);
	return ret;
}

static int external_renderer_render_output(GdsOutputRenderer *renderer,
					    struct gds_cell *cell,
					    GList *layer_infos,
					    const char *output_file,
					    double scale)
{
	ExternalRenderer *ext_renderer = GDS_RENDER_EXTERNAL_RENDERER(renderer);

	return external_renderer_render_cell(cell, layer_infos, output_file, scale, ext_renderer->shared_object_path);
}

static void external_renderer_class_init(ExternalRendererClass *klass)
{
	GdsOutputRendererClass *inherited_parent_class;

	inherited_parent_class = GDS_RENDER_OUTPUT_RENDERER_CLASS(klass);

	/* Override virtual function */
	inherited_parent_class->render_output = external_renderer_render_output;
}

static void external_renderer_init(ExternalRenderer *self)
{
	self->shared_object_path = NULL;
}



/** @} */
