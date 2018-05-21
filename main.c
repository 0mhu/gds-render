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
#include "gdsparse.h"
#include <gtk/gtk.h>
#include <layer-element.h>
#include "layer-selector.h"
#include "tree-renderer/lib-cell-renderer.h"


enum cell_store_columns {
	LIBRARY,
	CELL,
	MODDATE,
	ACCESSDATE,
	STORE_COLUMN_COUNT
};


struct open_button_data {
	GtkWindow *main_window;
	GList **list_ptr;
	GtkTreeStore *cell_store;
	GtkListBox *layer_box;
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
					    LIBRARY, gds_lib,
					    MODDATE, mod_date->str,
					    ACCESSDATE, acc_date->str,
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
						    CELL, gds_c,
						    MODDATE, mod_date->str,
						    ACCESSDATE, acc_date->str,
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

void on_convert_clicked(gpointer button, gpointer user)
{
	printf("convert\n");
}

static GtkTreeStore * setup_cell_selector(GtkTreeView* view)
{
	GtkTreeStore *cell_store;

	GtkCellRenderer *render_dates;
	GtkCellRenderer *render_cell;
	GtkCellRenderer *render_lib;
	GtkTreeViewColumn *column;
	GdkRGBA cell_text_color;
	GValue val = G_VALUE_INIT;

	cell_store = gtk_tree_store_new(STORE_COLUMN_COUNT, G_TYPE_POINTER, G_TYPE_POINTER, G_TYPE_STRING, G_TYPE_STRING);
	gtk_tree_view_set_model(view, GTK_TREE_MODEL(cell_store));

	render_dates = gtk_cell_renderer_text_new();
	render_cell = lib_cell_renderer_new();
	render_lib = lib_cell_renderer_new();

	/* Set foreground color for cell column */
	cell_text_color.alpha = 1;
	cell_text_color.red = (double)61.0/(double)255.0;
	cell_text_color.green = (double)152.0/(double)255.0;
	cell_text_color.blue = 0.0;

	g_value_init(&val, G_TYPE_BOOLEAN);
	g_value_set_boolean(&val, TRUE);
	g_object_set_property(G_OBJECT(render_cell), "foreground-set", &val);
	g_value_unset(&val);

	g_value_init(&val, GDK_TYPE_RGBA);
	g_value_set_boxed(&val, &cell_text_color);
	g_object_set_property(G_OBJECT(render_cell), "foreground-rgba", &val);
	g_value_unset(&val);




	column = gtk_tree_view_column_new_with_attributes("Library", render_lib, "gds-lib", LIBRARY, NULL);
	gtk_tree_view_append_column(view, column);

	/* Cell color: #3D9801 */
	column = gtk_tree_view_column_new_with_attributes("Cell", render_cell, "gds-cell", CELL, NULL);
	gtk_tree_view_append_column(view, column);

	column = gtk_tree_view_column_new_with_attributes("Mod. Date", render_dates, "text", MODDATE, NULL);
	gtk_tree_view_append_column(view, column);

	column = gtk_tree_view_column_new_with_attributes("Acc. Date", render_dates, "text", ACCESSDATE, NULL);
	gtk_tree_view_append_column(view, column);

	return cell_store;
}

int main(int argc, char **argv)
{
	GtkBuilder *main_builder;
	GList *gds_libs = NULL;
	GtkTreeView *cell_tree;
	GtkTreeStore *cell_store;
	GtkWidget *widget_generic;
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
	widget_generic = GTK_WIDGET(gtk_builder_get_object(main_builder, "convert-button"));
	g_signal_connect(widget_generic, "clicked", G_CALLBACK(on_convert_clicked), layer);

	listbox = GTK_WIDGET(gtk_builder_get_object(main_builder, "layer-list"));
	open_data.layer_box = GTK_LIST_BOX(listbox);

	/* Set buttons fpr layer mapping GUI */
	setup_load_mapping_callback(GTK_WIDGET(gtk_builder_get_object(main_builder, "button-load-mapping")),
				    open_data.main_window);
	setup_save_mapping_callback(GTK_WIDGET(gtk_builder_get_object(main_builder, "button-save-mapping")),
				    open_data.main_window);


	gtk_main();

	return 0;
}
