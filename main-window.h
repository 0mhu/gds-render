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
 * @file main-window.h
 * @brief Header for main-window
 * @author Mario Hüttel <mario.huettel@gmx.net>
 */

#ifndef _MAIN_WINDOW_H_
#define _MAIN_WINDOW_H_

/**
 * @addtogroup MainApplication
 * @{
 */

#include <gtk/gtk.h>

/**
 * @brief Create main window
 *
 * This function creates the main window and sets the necessary callback routines.
 * @return
 */
GtkWindow *create_main_window();

/** @} */

#endif /* _MAIN_WINDOW_H_ */
