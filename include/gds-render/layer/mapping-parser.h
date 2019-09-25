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
 * @file mapping-parser.h
 * @brief Function to read a mapping file line and parse it.
 * @author Mario Hüttel <mario.huettel@gmx.net>
 */

#ifndef __MAPPING_PARSER_H__
#define __MAPPING_PARSER_H__

/**
 * @addtogroup  Mapping-Parser
 * @{
 */

#include <glib.h>

#include <gds-render/widgets/layer-element.h>
#include <gds-render/layer/layer-settings.h>

/**
 * @brief Create Line for LayerMapping file with supplied information
 * @param layer_element information
 * @param line_buffer buffer to write to
 * @param max_len Maximum length that cna be used in \p line_buffer
 */
void mapping_parser_gen_csv_line(LayerElement *layer_element, char *line_buffer, size_t max_len);

/** @} */

#endif /* __MAPPING_PARSER_H__ */
