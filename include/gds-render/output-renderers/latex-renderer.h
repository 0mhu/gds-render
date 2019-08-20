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
 * @file latex-output.h
 * @brief LaTeX output renderer
 * @author Mario Hüttel <mario.huettel@gmx.net>
 */

/**
 * @addtogroup LaTeX-Renderer
 * @{
 */

#ifndef _LATEX_OUTPUT_H_
#define _LATEX_OUTPUT_H_

#include <gds-render/output-renderers/gds-output-renderer.h>
#include <gds-render/gds-utils/gds-types.h>

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(LatexRenderer, latex_renderer, GDS_RENDER, LATEX_RENDERER, GdsOutputRenderer)

#define GDS_RENDER_TYPE_LATEX_RENDERER (latex_renderer_get_type())

/**
 * @brief Buffer for LaTeX Code line in KiB
 */
#define LATEX_LINE_BUFFER_KB (10)

/**
 * @brief Create new LatexRenderer object
 * @return New object
 */
LatexRenderer *latex_renderer_new();

/**
 * @brief Create new LatexRenderer object
 *
 * This function sets the 'pdf-layers' and 'standalone'
 * properties for the newly created object.
 *
 * They can later be changes by modifying the properties again.
 * On top of that, The options can be changed in the resulting
 * LaTeX output file if needed.
 *
 * @param pdf_layers If PDF OCR layers should be enabled
 * @param standalone If output TeX file should be standalone compilable
 * @return New object
 */
LatexRenderer *latex_renderer_new_with_options(gboolean pdf_layers, gboolean standalone);

G_END_DECLS

#endif /* _LATEX_OUTPUT_H_ */

/** @} */
