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
 * @file activity-bar.h
 * @brief Header file for activity bar widget
 * @author Mario Hüttel <mario.huettel@gmx.net>
 */

/**
 * @addtogroup ActivityBar
 * @ingroup Widgets
 * @{
 */

#ifndef __LAYER_ELEMENT_H__
#define __LAYER_ELEMENT_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

/* Creates Class structure etc */
G_DECLARE_FINAL_TYPE(ActivityBar, activity_bar, ACTIVITY, BAR, GtkBox)

#define TYPE_ACTIVITY_BAR (activity_bar_get_type())

/**
 * @brief Create new Object ActivityBar
 * @return New object. In case of error: NULL.
 */
ActivityBar *activity_bar_new();

/**
 * @brief Deletes all applied tasks and sets bar to "Ready".
 * @param[in] bar AcitivityBar object.
 */
void activity_bar_set_ready(ActivityBar *bar);

/**
 * @brief Enable spinner and set \p text. If text is NULL, 'Working...' is displayed
 * @param bar Activity bar object
 * @param text Text to display, may be NULL

 */
void activity_bar_set_busy(ActivityBar *bar, const char *text);

G_END_DECLS

#endif /* __LAYER_ELEMENT_H__ */

/** @} */
