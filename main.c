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
#include "gds-parser/gds-parser.h"
#include <gtk/gtk.h>
#include "layer-widget/layer-element.h"
#include "layer-selector.h"
#include "tree-renderer/tree-store.h"
#include "latex-output/latex-output.h"

struct open_button_data {
	GtkWindow *main_window;
	GList **list_ptr;
	GtkTreeStore *cell_store;
	GtkListBox *layer_box;
};

struct convert_button_data {
	GList *layer_info;
	GtkTreeView *tree_view;
};

gboolean on_window_close(gpointer window, gpointer user)
{
	gtk_widget_destroy(GTK_WIDGET(window));
	gtk_main_quit();
	return TRUE;
}

static GString *generate_string_from_date(struct gds_time_field *date)
{
	GString *str;

	str = g_string_new_len(NULL, 50);
	g_string_printf(str, "%02u.%02u.%u - %02u:%02u",
			(unsigned int)date->day,
			(unsigned int)date->month,
			(unsigned int)date->year,
			(unsigned int)date->hour,
			(unsigned int)date->minute);
	return str;
}

void on_load_gds(gpointer button, gpointer user)
{
	GList *cell;
	GtkTreeIter libiter;
	GtkTreeIter celliter;
	GList *lib;
	struct gds_library *gds_lib;
	struct gds_cell *gds_c;
	struct open_button_data *ptr = (struct open_button_data *)user;
	GtkTreeStore *store = ptr->cell_store;
	GtkWidget *open_dialog;
	GtkFileChooser *file_chooser;
	GtkFileFilter *filter;
	GtkStyleContext *button_style;
	gint dialog_result;
	int gds_result;
	char *filename;
	GString *mod_date;
	GString *acc_date;

	open_dialog = gtk_file_chooser_dialog_new("Open GDSII File", ptr->main_window, GTK_FILE_CHOOSER_ACTION_OPEN,
						  "Cancel", GTK_RESPONSE_CANCEL, "Open GDSII", GTK_RESPONSE_ACCEPT, NULL);
	file_chooser = GTK_FILE_CHOOSER(open_dialog);
	/* Add GDS II Filter */
	filter = gtk_file_filter_new();
	gtk_file_filter_add_pattern(filter, "*.gds");
	gtk_file_filter_set_name(filter, "GDSII-Files");
	gtk_file_chooser_add_filter(file_chooser, filter);

	dialog_result = gtk_dialog_run(GTK_DIALOG(open_dialog));

	if (dialog_result == GTK_RESPONSE_ACCEPT) {

		/* Get File name */
		filename = gtk_file_chooser_get_filename(file_chooser);

		gtk_tree_store_clear(store);
		clear_lib_list(ptr->list_ptr);

		/* Parse new GDSII file */
		gds_result = parse_gds_from_file(filename, ptr->list_ptr);

		/* Delete file name afterwards */
		g_free(filename);
		if (gds_result)
			goto end_destroy;

		/* remove suggested action from Open button */
		button_style = gtk_widget_get_style_context(GTK_WIDGET(button));
		gtk_style_context_remove_class(button_style, "suggested-action");

		for (lib = *(ptr->list_ptr); lib != NULL; lib = lib->next) {
			gds_lib = (struct gds_library *)lib->data;
			/* Create top level iter */
			gtk_tree_store_append (store, &libiter, NULL);

			/* Convert dates to String */
			mod_date = generate_string_from_date(&gds_lib->mod_time);
			acc_date = generate_string_from_date(&gds_lib->access_time);

			gtk_tree_store_set (store, &libiter,
					    CELL_SEL_LIBRARY, gds_lib,
					    CELL_SEL_MODDATE, mod_date->str,
					    CELL_SEL_ACCESSDATE, acc_date->str,
					    -1);

			/* Delete GStrings including string data. */
			/* Cell store copies String type data items */
			g_string_free(mod_date, TRUE);
			g_string_free(acc_date, TRUE);

			for (cell = gds_lib->cells; cell != NULL; cell = cell->next) {
				gds_c = (struct gds_cell *)cell->data;
				gtk_tree_store_append (store, &celliter, &libiter);

				/* Convert dates to String */
				mod_date = generate_string_from_date(&gds_c->mod_time);
				acc_date = generate_string_from_date(&gds_c->access_time);

				gtk_tree_store_set (store, &celliter,
						    CELL_SEL_CELL, gds_c,
						    CELL_SEL_MODDATE, mod_date->str,
						    CELL_SEL_ACCESSDATE, acc_date->str,
						    -1);

				/* Delete GStrings including string data. */
				/* Cell store copies String type data items */
				g_string_free(mod_date, TRUE);
				g_string_free(acc_date, TRUE);
			}
		}

		/* Create Layers in Layer Box */
		generate_layer_widgets(ptr->layer_box, *(ptr->list_ptr));
	}

end_destroy:
	/* Destroy dialog and filter */
	gtk_widget_destroy(open_dialog);
}

static void on_convert_clicked(gpointer button, gpointer user)
{
	struct convert_button_data *data = (struct convert_button_data *)user;
	printf("convert\n");
}

/* This function activates/deactivates the convert button depending on whether
 * a cell is selected for conversion or not */
static void cell_selection_changed(GtkTreeSelection *sel, GtkWidget *convert_button)
{
	GtkTreeModel *model = NULL;
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected(sel, &model, &iter)) {
		/* Node selected. Show button */
		gtk_widget_set_sensitive(convert_button, TRUE);
	} else {
		gtk_widget_set_sensitive(convert_button, FALSE);
	}
}

int main(int argc, char **argv)
{
	GtkBuilder *main_builder;
	GList *gds_libs = NULL;
	GtkTreeView *cell_tree;
	GtkTreeStore *cell_store;
	GtkWidget *conv_button;

	GtkWidget *layer;
	GtkWidget *listbox;

	struct open_button_data open_data;

	gtk_init(&argc, &argv);

	main_builder = gtk_builder_new_from_resource("/main.glade");
	gtk_builder_connect_signals(main_builder, NULL);



	cell_tree = (GtkTreeView *)gtk_builder_get_object(main_builder, "cell-tree");
	cell_store = setup_cell_selector(cell_tree);


	open_data.cell_store = cell_store;
	open_data.list_ptr = &gds_libs;
	open_data.main_window = GTK_WINDOW(gtk_builder_get_object(main_builder, "main-window"));
	g_signal_connect(GTK_WIDGET(gtk_builder_get_object(main_builder, "button-load-gds")),
			 "clicked", G_CALLBACK(on_load_gds), (gpointer)&open_data);
	

	/* Connect Convert button */
	conv_button = GTK_WIDGET(gtk_builder_get_object(main_builder, "convert-button"));
	g_signal_connect(conv_button, "clicked", G_CALLBACK(on_convert_clicked), layer);

	listbox = GTK_WIDGET(gtk_builder_get_object(main_builder, "layer-list"));
	open_data.layer_box = GTK_LIST_BOX(listbox);

	/* Set buttons fpr layer mapping GUI */
	setup_load_mapping_callback(GTK_WIDGET(gtk_builder_get_object(main_builder, "button-load-mapping")),
				    open_data.main_window);
	setup_save_mapping_callback(GTK_WIDGET(gtk_builder_get_object(main_builder, "button-save-mapping")),
				    open_data.main_window);

	/* Callback for selection change of cell selector */
	g_signal_connect(G_OBJECT(gtk_tree_view_get_selection(cell_tree)), "changed",
			 G_CALLBACK(cell_selection_changed), conv_button);

	gtk_main();

	return 0;
}
