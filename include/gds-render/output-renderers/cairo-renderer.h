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
 * @file cairo-renderer.h
 * @brief Header File for Cairo output renderer
 * @author Mario Hüttel <mario.huettel@gmx.net>
 */
#ifndef _CAIRO_OUTPUT_H_
#define _CAIRO_OUTPUT_H_

#include <gds-render/gds-utils/gds-types.h>
#include <gds-render/output-renderers/gds-output-renderer.h>
#include <glib-object.h>

G_BEGIN_DECLS

/** @addtogroup Cairo-Renderer
 *  @{
 */

G_DECLARE_FINAL_TYPE(CairoRenderer, cairo_renderer, GDS_RENDER, CAIRO_RENDERER, GdsOutputRenderer)

#define GDS_RENDER_TYPE_CAIRO_RENDERER (cairo_renderer_get_type())

#define MAX_LAYERS (300) /**< \brief Maximum layer count the output renderer can process. Typically GDS only specifies up to 255 layers.*/

/**
 * @brief Create new CairoRenderer for SVG output
 * @return New object
 */
CairoRenderer *cairo_renderer_new_svg();


/**
 * @brief Create new CairoRenderer for PDF output
 * @return New object
 */
CairoRenderer *cairo_renderer_new_pdf();

/** @} */

G_END_DECLS

#endif /* _CAIRO_OUTPUT_H_ */
