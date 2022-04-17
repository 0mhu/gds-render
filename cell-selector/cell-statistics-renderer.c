/*
 * GDSII-Converter
 * Copyright (C) 2022  Mario Hüttel <mario.huettel@gmx.net>
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
 * @file cell-statistics-renderer.c
 * @brief CellStatisticsRenderer GObject Class
 * @author Mario Hüttel <mario.huettel@gmx.net>
 */

/**
 * @addtogroup CellStatisticsRenderer
 * @{
 */

#include <gds-render/cell-selector/cell-statistics-renderer.h>

G_DEFINE_TYPE(CellStatisticsRenderer, cell_statistics_renderer, GTK_TYPE_CELL_RENDERER_TEXT)

enum {
	PROP_CELL_STAT = 1,
	PROP_COUNT
};

static void cell_statistics_renderer_set_property(GObject      *object,
					   guint        param_id,
					   const GValue *value,
					   GParamSpec   *pspec)
{
	GValue val = G_VALUE_INIT;
	GString *string;
	const struct gds_cell_statistics *cell_stat;

	switch (param_id) {
	case PROP_CELL_STAT:
		cell_stat = (const struct gds_cell_statistics *)g_value_get_pointer(value);
		g_value_init(&val, G_TYPE_STRING);
		string = g_string_new_len("", 5);
		if (cell_stat)
			g_string_printf(string, "%zu", cell_stat->vertex_count);
		g_value_set_string(&val, string->str);
		g_object_set_property(object, "text", &val);
		g_value_unset(&val);
		g_string_free(string, TRUE);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
		break;
	}
}

static void cell_statistics_renderer_get_property(GObject      *object,
					   guint        param_id,
					   GValue	*value,
					   GParamSpec   *pspec)
{
	(void)value;

	switch (param_id) {
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
		break;
	}
}

static GParamSpec *properties[PROP_COUNT];

void cell_statistics_renderer_class_init(CellStatisticsRendererClass *klass)
{
	GObjectClass *oclass = G_OBJECT_CLASS(klass);

	oclass->set_property = cell_statistics_renderer_set_property;
	oclass->get_property = cell_statistics_renderer_get_property;

	properties[PROP_CELL_STAT] = g_param_spec_pointer("cell-stat", "cell-stat",
							  "Cell statistics", G_PARAM_WRITABLE);
	g_object_class_install_properties(oclass, PROP_COUNT, properties);
}

void cell_statistics_renderer_init(CellStatisticsRenderer *self)
{
	(void)self;
}

GtkCellRenderer *cell_statistics_renderer_new(void)
{
	return GTK_CELL_RENDERER(g_object_new(TYPE_GDS_RENDER_CELL_STAT_RENDERER, NULL));
}

/** @} */
