/*
 * GDSII-Converter example plugin
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
 * @defgroup example-plugin Example Plugin for External Renderer
 * @ingroup plugins
 * This is a template / example for an external renderer plugin
 * @addtogroup example-plugin
 * @{
 */


#include <stdio.h>
#include <glib.h>
#include <gds-render/gds-utils/gds-types.h>
#include <gds-render/output-renderers/external-renderer-interfaces.h>

int EXPORTED_FUNC_DECL(EXTERNAL_LIBRARY_RENDER_FUNCTION)(struct gds_cell *toplevel, GList *layer_info_list, const char *output_file_name, double scale)
{
	if (!toplevel)
		return -1000;

	printf("Rendering %s\n", toplevel->name);
	return 0;
}

int EXPORTED_FUNC_DECL(EXTERNAL_LIBRARY_INIT_FUNCTION)(const char *params, const char *version)
{
	printf("Init with params: %s\ngds-render version: %s\n", params, version);
	return 0;
}

/** @} */
