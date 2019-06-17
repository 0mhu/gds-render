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
 * @addtogroup external-renderer
 * @{
 */

#include <dlfcn.h>
#include <stdio.h>

#include <gds-render/output-renderers/external-renderer.h>

int external_renderer_render_cell(struct gds_cell *toplevel_cell, GList *layer_info_list,
				  char *output_file, char *so_path)
{
	int (*so_render_func)(struct gds_cell *, GList *, char *) = NULL;
	void *so_handle = NULL;
	char *error_msg;
	int ret = 0;

	/* Check parameter sanity */
	if (!output_file || !so_path || !toplevel_cell || !layer_info_list)
		return -3000;

	/* Load shared object */
	so_handle = dlopen(so_path, RTLD_LAZY);
	if (!so_handle) {
		printf("Could not load external library '%s'\nDetailed error is:\n%s\n", so_path, dlerror());
		return -2000;
	}

	/* Load symbol from library */
	so_render_func = (int (*)(struct gds_cell *, GList *, char *))dlsym(so_handle, EXTERNAL_LIBRARY_FUNCTION);
	error_msg = dlerror();
	if (error_msg != NULL) {
		printf("Rendering function not found in library:\n%s\n", error_msg);
		goto ret_close_so_handle;
	}

	/* Execute */
	if (so_render_func)
		so_render_func(toplevel_cell, layer_info_list, output_file);

ret_close_so_handle:
	dlclose(so_handle);
	return ret;
}

/** @} */
