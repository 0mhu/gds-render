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
#include <gds-render/layer/layer-settings.h>

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
 * @brief Create a new GdsOutputRenderer GObject with its properties
 * @param output_file Output file of the renderer
 * @param layer_settings Layer settings object
 * @return New object
 */
GdsOutputRenderer *gds_output_renderer_new_with_props(const char *output_file, LayerSettings *layer_settings);

/**
 * @brief gds_output_renderer_render_output
 * @param renderer Renderer object
 * @param cell Cell to render
 * @param scale scale value. The output is scaled *down* by this value
 * @return 0 if successful
 */
int gds_output_renderer_render_output(GdsOutputRenderer *renderer,
                                        struct gds_cell *cell,
                                        double scale);

/**
 * @brief Convenience function for setting the "output-file" property
 * @param renderer Renderer object
 * @param file_name Output file path
 */
void gds_output_renderer_set_output_file(GdsOutputRenderer *renderer, const gchar *file_name);

/**
 * @brief Convenience function for getting the "output-file" property
 * @param renderer
 * @return Output file path. This must not be freed
 */
const char *gds_output_renderer_get_output_file(GdsOutputRenderer *renderer);

/**
 * @brief Get layer settings
 *
 * This is a convenience function for getting the
 * "layer-settings" property. This also references it.
 * This is to prevent race conditions with another thread that might
 * alter the layer settings before they are read out.
 *
 * @param renderer Renderer
 * @return Layer settings object
 */
LayerSettings *gds_output_renderer_get_and_ref_layer_settings(GdsOutputRenderer *renderer);

/**
 * @brief Set layer settings
 *
 * This is a convenience function for setting the
 * "layer-settings" property.
 *
 * If another Layer settings has previously been supplied,
 * it is unref'd.
 *
 * @param renderer Renderer
 * @param settings LayerSettings object
 */
void gds_output_renderer_set_layer_settings(GdsOutputRenderer *renderer, LayerSettings *settings);

/**
 * @brief Render output asynchronously
 *
 * This function will render in a separate thread.
 * To wait for the completion of the rendering process.
 *
 * @note A second async thread cannot be spawned.
 *
 * @param renderer Output renderer
 * @param cell Cell to render
 * @param scale Scale
 * @return 0 if successful. In case no thread can be spawned < 0
 */
int gds_output_renderer_render_output_async(GdsOutputRenderer *renderer, struct gds_cell *cell, double scale);

/**
 * @brief This function emits the 'progress-changed' in the thread/context that triggered an asynchronous rendering
 *
 * If the rendering is not asynchronous, this function has no effect.
 *
 * @param renderer GdsOutputrenderer object
 * @param status Status to supply to signal emission
 */
void gds_output_renderer_update_async_progress(GdsOutputRenderer *renderer, const char *status);

G_END_DECLS

#endif /* _GDS_OUTPUT_RENDERER_H_ */

/** @} */
