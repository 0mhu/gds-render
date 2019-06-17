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
 * @file tree-store.h
 * @brief Header file for Tree store implementation
 * @author Mario Hüttel <mario.huettel@gmx.net>
 */

/**
 * @addtogroup GUI
 * @{
 */

#ifndef __TREE_STORE_H__
#define __TREE_STORE_H__

#include <gtk/gtk.h>

/** @brief Columns of selection tree view */
enum cell_store_columns {
        CELL_SEL_LIBRARY = 0,
        CELL_SEL_CELL,
	CELL_SEL_CELL_ERROR_STATE, /**< Used for cell color and selectability */
        CELL_SEL_MODDATE,
        CELL_SEL_ACCESSDATE,
	CELL_SEL_COLUMN_COUNT /**< @brief Not a column. Used to determine count of columns */
};

struct tree_stores {
		GtkTreeView *base_tree_view;
		GtkTreeStore *base_store;
		GtkTreeModelFilter *filter;
		GtkEntry *search_entry;
};

struct tree_stores *setup_cell_selector(GtkTreeView* view, GtkEntry *search_entry);

#endif /* __TREE_STORE_H__ */

/** @} */
