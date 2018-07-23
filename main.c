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

struct application_data {
	GtkApplication *app;
	GtkWindow *main_window;
};


static void app_quit(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
	struct application_data *appdata = (struct application_data *)user_data;
	gtk_widget_destroy(GTK_WIDGET(appdata->main_window));
}

static void app_about(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
	GtkBuilder *builder;
	GtkDialog *dialog;
	struct application_data *appdata = (struct application_data *)user_data;

	builder = gtk_builder_new_from_resource("/about.glade");
	dialog = GTK_DIALOG(gtk_builder_get_object(builder, "about-dialog"));
	gtk_window_set_transient_for(GTK_WINDOW(dialog), appdata->main_window);
	gtk_dialog_run(dialog);

	gtk_widget_destroy(dialog);
	g_object_unref(builder);
}

const GActionEntry app_actions[] = {
	{ "quit", app_quit },
	{ "about", app_about }
};

static void gapp_activate(GApplication *app, gpointer user_data)
{
	GtkWindow *main_window;
	struct application_data *appdata = (struct application_data *)user_data;

	main_window = create_main_window();
	appdata->main_window = main_window;
	gtk_application_add_window(GTK_APPLICATION(app), main_window);
	gtk_widget_show(GTK_WIDGET(main_window));
}

int main(int argc, char **argv)
{
	GtkApplication *gapp;
	int app_status;
	struct application_data appdata;
	GMenu *menu;
	GMenu *m_quit;
	GMenu *m_about;

	gapp = gtk_application_new("de.shimatta.gds-render", G_APPLICATION_FLAGS_NONE);
	g_application_register(G_APPLICATION(gapp), NULL, NULL);
	//g_action_map_add_action_entries(G_ACTION_MAP(gapp), app_actions, G_N_ELEMENTS(app_actions), &appdata);
	g_signal_connect (gapp, "activate", G_CALLBACK(gapp_activate), &appdata);



	menu = g_menu_new();
	m_quit = g_menu_new();
	m_about = g_menu_new();
	g_menu_append(m_quit, "Quit", "app.quit");
	g_menu_append(m_about, "About", "app.about");
	g_menu_append_section(menu, NULL, G_MENU_MODEL(m_about));
	g_menu_append_section(menu, NULL, G_MENU_MODEL(m_quit));
	g_action_map_add_action_entries(G_ACTION_MAP(gapp), app_actions, G_N_ELEMENTS(app_actions), &appdata);
	gtk_application_set_app_menu(GTK_APPLICATION(gapp), G_MENU_MODEL(menu));

	g_object_unref(m_quit);
	g_object_unref(m_about);
	g_object_unref(menu);


	app_status = g_application_run (G_APPLICATION(gapp), argc, argv);
	g_object_unref (gapp);

	return app_status;
}
