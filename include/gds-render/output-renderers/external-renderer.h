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
 * @file external-renderer.h
 * @brief Render according to command line parameters
 * @author Mario Hüttel <mario.huettel@gmx.net>
 */

/**
 * @addtogroup ExternalRenderer
 * @{
 */

#ifndef _EXTERNAL_RENDERER_H_
#define _EXTERNAL_RENDERER_H_

#include <gds-render/output-renderers/gds-output-renderer.h>
#include <gds-render/gds-utils/gds-types.h>

G_BEGIN_DECLS

#define GDS_RENDER_TYPE_EXTERNAL_RENDERER (external_renderer_get_type())

G_DECLARE_FINAL_TYPE(ExternalRenderer, external_renderer, GDS_RENDER, EXTERNAL_RENDERER, GdsOutputRenderer)

/**
 * @brief function name expected to be found in external library.
 * 
 * The function has to be defined as follows:
 * @code
 * int EXTERNAL_LIBRARY_FUNCTION(gds_cell *toplevel, GList *layer_info_list, const char *output_file_name, double scale)
 * @endcode
 */
#define EXTERNAL_LIBRARY_FUNCTION "render_cell_to_file"

/**
 * @brief Create new ExternalRenderer object
 * @return New object
 */
ExternalRenderer *external_renderer_new();

/**
 * @brief Create new ExternalRenderer object with specified shared object path
 * @param so_path Path to shared object, the rendering function is searched in
 * @return New object.
 */
ExternalRenderer *external_renderer_new_with_so(const char *so_path);

G_END_DECLS

#endif /* _EXTERNAL_RENDERER_H_ */

/** @} */
