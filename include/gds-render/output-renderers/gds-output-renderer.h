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
 * @file gds-output-renderer.h
 * @brief Header for output renderer base class
 * @author Mario Hüttel <mario.huettel@gmx.net>
 */

/**
 * @addtogroup GdsOutputRenderer
 * @{
 */

#ifndef _GDS_OUTPUT_RENDERER_H_
#define _GDS_OUTPUT_RENDERER_H_

#include <gds-render/gds-utils/gds-types.h>
#include <glib-object.h>
#include <glib.h>

G_BEGIN_DECLS

#define GDS_RENDER_TYPE_OUTPUT_RENDERER (gds_output_renderer_get_type())

G_DECLARE_DERIVABLE_TYPE(GdsOutputRenderer, gds_output_renderer, GDS_RENDER, OUTPUT_RENDERER, GObject);

/**
 * @brief Base output renderer class structure.
 * @note This structure is only used for internal inheritance of GObjects. Do not use in code outside of these classes.
 */
struct _GdsOutputRendererClass {
	GObjectClass parent_class;

	/**
	 * @brief Virtual render output function. Overwritten by final class implementation
	 */
	int (*render_output)(GdsOutputRenderer *renderer,
	                        struct gds_cell *cell,
	                        GList *layer_infos,
	                        const char *output_file,
	                        double scale);
	gpointer padding[4];
};

enum {
	GDS_OUTPUT_RENDERER_GEN_ERR = -100, /**< @brief Error set by the _GdsOutputRendererClass::render_output virtual function, if renderer is invalid. */
	GDS_OUTPUT_RENDERER_PARAM_ERR = -200 /**< @brief Error set by the _GdsOutputRendererClass::render_output virtual function, if parameters are faulty. */
};

/**
 * @brief Create a new GdsOutputRenderer GObject.
 * @return New object
 */
GdsOutputRenderer *gds_output_renderer_new();

/**
 * @brief gds_output_renderer_render_output
 * @param renderer Renderer object
 * @param cell Cell to render
 * @param layer_infos List of Layer information (@ref layer_info)
 * @param output_file Output file name
 * @param scale scale value. The output is scaled *down* by this value
 * @return 0 if successful
 */
int gds_output_renderer_render_output(GdsOutputRenderer *renderer,
                                        struct gds_cell *cell,
                                        GList *layer_infos,
                                        const char *output_file,
                                        double scale);

G_END_DECLS

#endif /* _GDS_OUTPUT_RENDERER_H_ */

/** @} */
