/*
 * GDSII-Converter
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
 * @file layer-selector-dnd.h
 * @brief Header for drag and drop of layer selector
 * @author Mario Hüttel <mario.huettel@gmx.net>
 */

#ifndef _LAYER_SELECTOR_DND_H_
#define _LAYER_SELECTOR_DND_H_

#include <gtk/gtk.h>

void layer_selector_list_box_setup_dnd(GtkListBox *box);

#endif /* _LAYER_SELECTOR_DND_H_ */
