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
 * @file gds-parser.h
 * @brief Header file for the GDS statistics
 * @author Mario Hüttel <mario.huettel@gmx.net>
 */

#ifndef _GDS_STATISTICS_H_
#define _GDS_STATISTICS_H_

/**
 * @addtogroup GDS-Utilities
 * @{
 */

#include <glib.h>

#include <gds-render/gds-utils/gds-types.h>

void gds_statistics_calc_cummulative_counts_in_lib(struct gds_library *lib);

/** @} */

#endif				/* _GDS_STATISTICS_H_ */
