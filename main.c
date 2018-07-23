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

#include <stdio.h>
#include <gtk/gtk.h>
#include "main-window.h"


static void gapp_activate(GApplication *app, gpointer user_data)
{
	GtkWindow *main_window;

	main_window = create_main_window();
	gtk_application_add_window(GTK_APPLICATION(app), main_window);
}

int main(int argc, char **argv)
{
	GtkApplication *gapp;
	int app_status;

	gapp = gtk_application_new("de.shimatta.gds-render", G_APPLICATION_FLAGS_NONE);
	g_signal_connect (gapp, "activate", G_CALLBACK(gapp_activate), NULL);


	app_status = g_application_run (G_APPLICATION(gapp), argc, argv);
	g_object_unref (gapp);

	return app_status;
}
