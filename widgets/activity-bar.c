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

/*
 * The drag and drop implementation is adapted from
 * https://gitlab.gnome.org/GNOME/gtk/blob/gtk-3-22/tests/testlist3.c
 *
 * Thanks to the GTK3 people for creating these examples.
 */

/**
 * @file activity-bar.c
 * @brief Status bar indicating activity of the program
 * @author Mario Hüttel <mario.huettel@gmx.net>
 */

/**
 * @addtogroup ActivityBar
 * @ingroup Widgets
 * @{
 */

#include <gds-render/widgets/activity-bar.h>

/** @brief Opaque ActivityBar object. Not viewable outside this source file. */
struct _ActivityBar {
	GtkBox super;
	/* Private stuff */
	GtkWidget *spinner;
	GtkWidget *label;
};

G_DEFINE_TYPE(ActivityBar, activity_bar, GTK_TYPE_BOX)

static void activity_bar_dispose(GObject *obj)
{
	ActivityBar *bar;

	bar = ACTIVITY_BAR(obj);

	/* Clear references on owned objects */
	g_clear_object(&bar->label);
	g_clear_object(&bar->spinner);

	/* Chain up */
	G_OBJECT_CLASS(activity_bar_parent_class)->dispose(obj);
}

static void activity_bar_class_init(ActivityBarClass *klass)
{
	GObjectClass *oclass = G_OBJECT_CLASS(klass);

	oclass->dispose = activity_bar_dispose;
}

static void activity_bar_init(ActivityBar *self)
{
	GtkContainer *box = GTK_CONTAINER(self);

	/* Create Widgets */
	self->label = gtk_label_new("");
	self->spinner = gtk_spinner_new();

	/* Add to this widget and show */
	gtk_container_add(box, self->spinner);
	gtk_container_add(box, self->label);
	gtk_widget_show(self->label);
	gtk_widget_show(self->spinner);

	g_object_ref(self->spinner);
	g_object_ref(self->label);
}

ActivityBar *activity_bar_new()
{
	ActivityBar *bar;

	bar = ACTIVITY_BAR(g_object_new(TYPE_ACTIVITY_BAR, "orientation", GTK_ORIENTATION_HORIZONTAL, NULL));
	if (bar)
		activity_bar_set_ready(bar);

	return bar;
}

/* TODO: Complete this once the task list is fully implemented */
void activity_bar_set_ready(ActivityBar *bar)
{
	gtk_label_set_text(GTK_LABEL(bar->label), "Ready");
	gtk_spinner_stop(GTK_SPINNER(bar->spinner));
}

void activity_bar_set_busy(ActivityBar *bar, const char *text)
{
	gtk_label_set_text(GTK_LABEL(bar->label), (text ? text : "Working..."));
	gtk_spinner_start(GTK_SPINNER(bar->spinner));
}


/** @} */
