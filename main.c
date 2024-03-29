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
 * @file main.c
 * @brief main.c
 * @author Mario Hüttel <mario.huettel@gmx.net>
 */

#include <stdio.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <locale.h>

#include <gds-render/gds-render-gui.h>
#include <gds-render/command-line.h>
#include <gds-render/output-renderers/external-renderer.h>
#include <gds-render/version.h>

/**
 * @brief Structure containing The GtkApplication and a list containing the GdsRenderGui objects.
 */
struct application_data {
		GtkApplication *app;
		GList *gui_list;
};

/**
 * @brief Callback for the menu entry 'Quit'
 *
 * Destroys all GUIs contained in the application_data structure
 * provided by \p user_data.
 *
 * The complete suspension of all main windows leads to the termination of the
 * GApplication.
 *
 * @param action unused
 * @param parameter unused
 * @param user_data application_data structure
 */
static void app_quit(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
	struct application_data * const appdata = (struct application_data *)user_data;
	(void)action;
	(void)parameter;
	GList *list_iter;
	GdsRenderGui *gui;

	/* Dispose all GUIs */
	for (list_iter = appdata->gui_list; list_iter != NULL; list_iter = g_list_next(list_iter)) {
		gui = RENDERER_GUI(list_iter->data);
		g_object_unref(gui);
	}

	g_list_free(appdata->gui_list);
	appdata->gui_list = NULL;
}

/**
 * @brief Callback for the 'About' menu entry
 *
 * This function shows the about dialog.
 *
 * @param action GSimpleAction, unused
 * @param parameter Unused.
 * @param user_data Unused
 */
static void app_about(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
	GtkBuilder *builder;
	GtkDialog *dialog;
	GdkPixbuf *logo_buf;
	GError *error = NULL;
	(void)user_data;
	(void)action;
	(void)parameter;
	GString *comment_text;

	comment_text = g_string_new(_("gds-render is a free tool for rendering GDS2 layout files into vector graphics."));
	g_string_append_printf(comment_text, _("\n\nFull git commit: %s"), _app_git_commit);

	builder = gtk_builder_new_from_resource("/gui/about.glade");
	dialog = GTK_DIALOG(gtk_builder_get_object(builder, "about-dialog"));
	gtk_window_set_transient_for(GTK_WINDOW(dialog), NULL);
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), _app_version_string);
	gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog), comment_text->str);

	g_string_free(comment_text, TRUE);

	/* Load icon from resource */
	logo_buf = gdk_pixbuf_new_from_resource_at_scale("/images/logo.svg", 100, 100, TRUE, &error);
	if (logo_buf) {
		/* Set logo */
		gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(dialog), logo_buf);

		/* Pixbuf is now owned by about dialog. Unref */
		g_object_unref(logo_buf);
	} else if (error) {
		fprintf(stderr, _("Logo could not be displayed: %s\n"), error->message);
		g_error_free(error);
	}

	gtk_dialog_run(dialog);

	gtk_widget_destroy(GTK_WIDGET(dialog));
	g_object_unref(builder);
}

/**
 * @brief Contains the application menu entries
 */
static const GActionEntry app_actions[] = {
	{ "quit", app_quit, NULL, NULL, NULL, {0} },
	{ "about", app_about, NULL, NULL, NULL, {0} },
};

/**
 * @brief Called when a GUI main window is closed
 *
 * The GdsRenderGui object associated with the closed main window
 * is removed from the list of open GUIs (\p user_data) and dereferenced.
 *
 * @param gui The GUI instance the closed main window belongs to
 * @param user_data List of GUIs
 */
static void gui_window_closed_callback(GdsRenderGui *gui, gpointer user_data)
{
	GList **gui_list = (GList **)user_data;

	/* Dispose of Gui element */
	*gui_list = g_list_remove(*gui_list, gui);
	g_object_unref(gui);
}

/**
 * @brief Activation of the GUI
 * @param app The GApplication reference
 * @param user_data Used to store the individual GUI instances.
 */
static void gapp_activate(GApplication *app, gpointer user_data)
{
	GtkWindow *main_window;
	GdsRenderGui *gui;
	struct application_data * const appdata = (struct application_data *)user_data;

	gui = gds_render_gui_new();
	appdata->gui_list = g_list_append(appdata->gui_list, gui);

	g_signal_connect(gui, "window-closed", G_CALLBACK(gui_window_closed_callback), &appdata->gui_list);

	main_window = gds_render_gui_get_main_window(gui);

	gtk_application_add_window(GTK_APPLICATION(app), main_window);
	gtk_widget_show(GTK_WIDGET(main_window));
}

/**
 * @brief Start the graphical interface.
 *
 * This function starts the GUI. If there's already a
 * running instance of this program, a second window will be
 * created in that instance and the second one is terminated.
 *
 * @param argc
 * @param argv
 * @return
 */
static int start_gui(int argc, char **argv)
{
	GtkApplication *gapp;
	GString *application_domain;
	int app_status;
	static struct application_data appdata = {
		.gui_list = NULL
	};
	GMenu *menu;
	GMenu *m_quit;
	GMenu *m_about;

	/*
	 * Generate version dependent application id
	 * This allows running the application in different versions at the same time.
	 */
	application_domain = g_string_new(NULL);
	g_string_printf(application_domain, "de.shimatta.gds_render_%s", _app_git_commit);

	gapp = gtk_application_new(application_domain->str, G_APPLICATION_FLAGS_NONE);
	g_string_free(application_domain, TRUE);

	g_application_register(G_APPLICATION(gapp), NULL, NULL);
	g_signal_connect(gapp, "activate", G_CALLBACK(gapp_activate), &appdata);

	if (g_application_get_is_remote(G_APPLICATION(gapp)) == TRUE) {
		g_application_activate(G_APPLICATION(gapp));
		printf(_("There is already an open instance. Will open second window in that instance.\n"));
		return 0;
	}

	menu = g_menu_new();
	m_quit = g_menu_new();
	m_about = g_menu_new();
	g_menu_append(m_quit, _("Quit"), "app.quit");
	g_menu_append(m_about, _("About"), "app.about");
	g_menu_append_section(menu, NULL, G_MENU_MODEL(m_about));
	g_menu_append_section(menu, NULL, G_MENU_MODEL(m_quit));
	g_action_map_add_action_entries(G_ACTION_MAP(gapp), app_actions,
					G_N_ELEMENTS(app_actions), &appdata);
	gtk_application_set_app_menu(GTK_APPLICATION(gapp), G_MENU_MODEL(menu));

	g_object_unref(m_quit);
	g_object_unref(m_about);
	g_object_unref(menu);

	app_status = g_application_run(G_APPLICATION(gapp), argc, argv);
	g_object_unref(gapp);

	g_list_free(appdata.gui_list);

	return app_status;
}

/**
 * @brief Print the application version string to stdout
 */
static void print_version(void)
{
	printf(_("This is gds-render, version: %s\n\nFor a list of supported commands execute with --help option.\n"),
	       _app_version_string);
}

/**
 * @brief The "entry point" of the application
 * @param argc Number of command line parameters
 * @param argv Command line parameters
 * @return Execution status of the application
 */
int main(int argc, char **argv)
{
	int i;
	GError *error = NULL;
	GOptionContext *context;
	gchar *gds_name;
	gchar **output_paths = NULL;
	gchar *mappingname = NULL;
	gchar *cellname = NULL;
	gchar **renderer_args = NULL;
	gboolean version = FALSE, pdf_standalone = FALSE, pdf_layers = FALSE;
	gboolean analyze = FALSE;
	gchar *format = NULL;
	int scale = 1000;
	int app_status = 0;
	struct external_renderer_params so_render_params;

	so_render_params.so_path = NULL;
	so_render_params.cli_params = NULL;

	bindtextdomain(GETTEXT_PACKAGE, LOCALEDATADIR "/locale");
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);

	GOptionEntry entries[] = {
		{"version", 'v', 0, G_OPTION_ARG_NONE, &version, _("Print version"), NULL},
		{"analyze", 'A', 0, G_OPTION_ARG_NONE, &analyze, _("Anaylze GDS file"), NULL},
		{"format", 'f', 0, G_OPTION_ARG_STRING, &format, _("Output format of analysis result, Default simple"), "[simple | pretty | cellsonly]"},
		{"renderer", 'r', 0, G_OPTION_ARG_STRING_ARRAY, &renderer_args,
			_("Renderer to use. Can be used multiple times."), "pdf|svg|tikz|ext"},
		{"scale", 's', 0, G_OPTION_ARG_INT, &scale, _("Divide output coordinates by <SCALE>"), "<SCALE>" },
		{"output-file", 'o', 0, G_OPTION_ARG_FILENAME_ARRAY, &output_paths,
			_("Output file path. Can be used multiple times."), "PATH" },
		{"mapping", 'm', 0, G_OPTION_ARG_FILENAME, &mappingname, _("Path for Layer Mapping File"), "PATH" },
		{"cell", 'c', 0, G_OPTION_ARG_STRING, &cellname, _("Cell to render"), "NAME" },
		{"tex-standalone", 'a', 0, G_OPTION_ARG_NONE, &pdf_standalone, _("Create standalone TeX"), NULL },
		{"tex-layers", 'l', 0, G_OPTION_ARG_NONE, &pdf_layers, _("Create PDF Layers (OCG)"), NULL },
		{"custom-render-lib", 'P', 0, G_OPTION_ARG_FILENAME, &so_render_params.so_path,
			_("Path to a custom shared object, that implements the necessary rendering functions"), "PATH"},
		{"render-lib-params", 'W', 0, G_OPTION_ARG_STRING, &so_render_params.cli_params,
			_("Argument string passed to render lib"), NULL},
		{NULL, 0, 0, 0, NULL, NULL, NULL}
	};

	context = g_option_context_new(_(" FILE - Convert GDS file <FILE> to graphic"));
	g_option_context_add_main_entries(context, entries, NULL);
	g_option_context_add_group(context, gtk_get_option_group(TRUE));

	if (!g_option_context_parse(context, &argc, &argv, &error)) {
		g_print(_("Option parsing failed: %s\n"), error->message);
		exit(1);
	}

	g_option_context_free(context);

	if (version) {
		print_version();
		goto ret_status;
	}

	if (argc >= 2) {
		if (scale < 1) {
			printf(_("Scale < 1 not allowed. Setting to 1\n"));
			scale = 1;
		}

		/* Get gds name */
		gds_name = argv[1];

		/* Print out additional arguments as ignored */
		for (i = 2; i < argc; i++)
			printf(_("Ignored argument: %s"), argv[i]);

		if (analyze) {
			app_status = command_line_analyze_lib(format, gds_name);
		} else {
			app_status =
				command_line_convert_gds(gds_name, cellname, renderer_args, output_paths, mappingname,
							 &so_render_params, pdf_standalone, pdf_layers, scale);
		}
	} else {
		app_status = start_gui(argc, argv);
	}

ret_status:
	/* If necessary, free command line parameters.
	 * This is only really necessary for automated mem-leak testing.
	 * Omitting these frees would be perfectly fine.
	 */
	if (output_paths)
		g_strfreev(output_paths);
	if (renderer_args)
		g_strfreev(renderer_args);
	if (mappingname)
		g_free(mappingname);
	if (cellname)
		free(cellname);
	if (so_render_params.so_path)
		free(so_render_params.so_path);
	if (so_render_params.cli_params)
		g_free(so_render_params.cli_params);

	return app_status;
}
