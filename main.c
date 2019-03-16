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
#include <glib.h>
#include "gds-render-gui.h"
#include "command-line.h"
#include "external-renderer.h"
#include "version/version.h"

struct application_data {
		GtkApplication *app;
		GList *gui_list;
};

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

static void app_about(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
	GtkBuilder *builder;
	GtkDialog *dialog;
	const struct application_data * const appdata = (const struct application_data *)user_data;
	(void)action;
	(void)parameter;

	builder = gtk_builder_new_from_resource("/about.glade");
	dialog = GTK_DIALOG(gtk_builder_get_object(builder, "about-dialog"));
	gtk_window_set_transient_for(GTK_WINDOW(dialog), NULL);
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), _app_version_string);
	gtk_dialog_run(dialog);

	gtk_widget_destroy(GTK_WIDGET(dialog));
	g_object_unref(builder);
}

const static GActionEntry app_actions[] = {
	{"quit", app_quit, NULL, NULL, NULL, {0}},
	{"about", app_about, NULL, NULL, NULL, {0}}
};

static void gui_window_closed_callback(GdsRenderGui *gui, gpointer user_data)
{
	GList **gui_list = (GList **)user_data;

	/* Dispose of Gui element */
	*gui_list = g_list_remove(*gui_list, gui);
	g_object_unref(gui);
}

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

static int start_gui(int argc, char **argv)
{

	GtkApplication *gapp;
	int app_status;
	static struct application_data appdata = {.gui_list = NULL};
	GMenu *menu;
	GMenu *m_quit;
	GMenu *m_about;
	GList *list_iter;
	GdsRenderGui *gui;

	gapp = gtk_application_new("de.shimatta.gds-render", G_APPLICATION_FLAGS_NONE);
	g_application_register(G_APPLICATION(gapp), NULL, NULL);
	g_signal_connect(gapp, "activate", G_CALLBACK(gapp_activate), &appdata);

	if (g_application_get_is_remote(G_APPLICATION(gapp)) == TRUE) {
		g_application_activate(G_APPLICATION(gapp));
		printf("There is already an open instance. Will open second window in said instance.\n");
		return 0;
	}

	menu = g_menu_new();
	m_quit = g_menu_new();
	m_about = g_menu_new();
	g_menu_append(m_quit, "Quit", "app.quit");
	g_menu_append(m_about, "About", "app.about");
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

static void print_version()
{
	printf("This is gds-render, version: %s\n\nFor a list of supported commands execute with --help option.\n",
	       _app_version_string);
}

int main(int argc, char **argv)
{
	int i;
	GError *error = NULL;
	GOptionContext *context;
	gchar *gds_name;
	gchar *basename;
	gchar *pdfname = NULL, *texname = NULL, *mappingname = NULL, *cellname = NULL, *svgname = NULL;
	gboolean tikz = FALSE, pdf = FALSE, pdf_layers = FALSE, pdf_standalone = FALSE, svg = FALSE;
	gboolean version = FALSE;
	gchar *custom_library_path = NULL;
	gchar *custom_library_file_name = NULL;
	int scale = 1000;
	int app_status = 0;

	GOptionEntry entries[] = {
		{"version", 'v', 0, G_OPTION_ARG_NONE, &version, "Print version", NULL},
		{"tikz", 't', 0, G_OPTION_ARG_NONE, &tikz, "Output TikZ code", NULL },
		{"pdf", 'p', 0, G_OPTION_ARG_NONE, &pdf, "Output PDF document", NULL },
		//{"svg", 'S', 0, G_OPTION_ARG_NONE, &svg, "Output SVG image", NULL },
		{"scale", 's', 0, G_OPTION_ARG_INT, &scale, "Divide output coordinates by <SCALE>", "<SCALE>" },
		{"tex-output", 'o', 0, G_OPTION_ARG_FILENAME, &texname, "Optional path for TeX file", "PATH" },
		{"pdf-output", 'O', 0, G_OPTION_ARG_FILENAME, &pdfname, "Optional path for PDF file", "PATH" },
		//{"svg-output", 0, 0, G_OPTION_ARG_FILENAME, &svgname, "Optional path for PDF file", "PATH"},
		{"mapping", 'm', 0, G_OPTION_ARG_FILENAME, &mappingname, "Path for Layer Mapping File", "PATH" },
		{"cell", 'c', 0, G_OPTION_ARG_STRING, &cellname, "Cell to render", "NAME" },
		{"tex-standalone", 'a', 0, G_OPTION_ARG_NONE, &pdf_standalone, "Create standalone PDF", NULL },
		{"tex-layers", 'l', 0, G_OPTION_ARG_NONE, &pdf_layers, "Create PDF Layers (OCG)", NULL },
		{"custom-render-lib", 'P', 0, G_OPTION_ARG_FILENAME, &custom_library_path, "Path to a custom shared object, that implements the " EXTERNAL_LIBRARY_FUNCTION " function", "PATH"},
		{"external-lib-output", 'e', 0, G_OPTION_ARG_FILENAME, &custom_library_file_name, "Output path for external render library", "PATH"},
		{NULL}
	};

	context = g_option_context_new(" FILE - Convert GDS file <FILE> to graphic");
	g_option_context_add_main_entries(context, entries, NULL);
	g_option_context_add_group(context, gtk_get_option_group(TRUE));

	if (!g_option_context_parse(context, &argc, &argv, &error)) {
		g_print("Option parsing failed: %s\n", error->message);
		exit(1);
	}

	if (version) {
		print_version();
		goto ret_status;
	}

	if (argc >= 2) {
		if (scale < 1) {
			printf("Scale < 1 not allowed. Setting to 1\n");
			scale = 1;
		}

		/* No format selected */
		if (!(tikz || pdf || svg))
			tikz = TRUE;

		/* Get gds name */
		gds_name = argv[1];

		/* Print out additional arguments as ignored */
		for (i = 2; i < argc; i++) {
			printf("Ignored argument: %s", argv[i]);
		}

		/* Check if PDF/TeX names are supplied. if not generate */
		basename = g_path_get_basename(gds_name);

		if (!texname)
			texname = g_strdup_printf("./%s.tex", basename);

		if (!pdfname)
			pdfname = g_strdup_printf("./%s.pdf", basename);

		if (!svgname)
			svgname = g_strdup_printf("./%s.svg", basename);

		command_line_convert_gds(gds_name, pdfname, texname, pdf, tikz,
					 mappingname, cellname, (double)scale,
					 pdf_layers, pdf_standalone, svg, svgname,
					 custom_library_path, custom_library_file_name);
		/* Clean up */
		g_free(pdfname);
		g_free(texname);
		g_free(svgname);
		g_free(basename);
		if (mappingname)
			g_free(mappingname);
		if (cellname)
			g_free(cellname);
		app_status = 0;
	} else {
		app_status = start_gui(argc, argv);
	}

ret_status:
	return app_status;
}
