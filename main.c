#include <stdio.h>
#include "gdsparse.h"
#include <gtk/gtk.h>

enum cell_store_columns {
LIBRARY,
CELL,
STORE_COLUMN
};


struct open_button_data {
	GList **list_ptr;
	GtkTreeStore *cell_store;
};




gboolean on_window_close(gpointer window, gpointer user)
{
	gtk_widget_destroy(GTK_WIDGET(window));
	gtk_main_quit();
	return TRUE;
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

	gtk_tree_store_clear(store);

	clear_lib_list(ptr->list_ptr);
	parse_gds_from_file("/home/mari/Desktop/test.gds", ptr->list_ptr);

	for (lib = *(ptr->list_ptr); lib != NULL; lib = lib->next) {
		gds_lib = (struct gds_library *)lib->data;
		/* Create top level iter */
		gtk_tree_store_append (store, &libiter, NULL);
		gtk_tree_store_set (store, &libiter, LIBRARY, gds_lib->name, -1);
		for (cell = gds_lib->cells; cell != NULL; cell = cell->next) {
			gds_c = (struct gds_cell *)cell->data;
			gtk_tree_store_append (store, &celliter, &libiter);
			gtk_tree_store_set (store, &celliter, CELL, gds_c->name, -1);
		}
	}

}

void on_convert_clicked(gpointer button, gpointer user)
{
	printf("convert\n");
}

static GtkTreeStore * setup_cell_selector(GtkTreeView* view)
{
	GtkTreeStore *cell_store;

	GtkCellRenderer *render;
	GtkCellRenderer *render2;
	GtkTreeViewColumn *column;

	cell_store = gtk_tree_store_new(STORE_COLUMN, G_TYPE_STRING, G_TYPE_STRING);
	gtk_tree_view_set_model(view, GTK_TREE_MODEL(cell_store));

	render = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Library", render, "text", LIBRARY, NULL);
	gtk_tree_view_append_column(view, column);
	//g_object_unref(column);
	column = gtk_tree_view_column_new_with_attributes("Cell", render, "text", CELL, NULL);
	gtk_tree_view_append_column(view, column);

	return cell_store;
}

int main(int argc, char **argv)
{
	GtkBuilder *main_builder;
	GList *gds_libs = NULL;
	GtkTreeView *cell_tree;
	GtkTreeStore *cell_store;
	struct open_button_data open_data;

	gtk_init(&argc, &argv);

	main_builder = gtk_builder_new_from_file("glade/main.glade");
	gtk_builder_connect_signals(main_builder, NULL);



	cell_tree = (GtkTreeView *)gtk_builder_get_object(main_builder, "cell-tree");
	cell_store = setup_cell_selector(cell_tree);


	open_data.cell_store = cell_store;
	open_data.list_ptr = &gds_libs;
	g_signal_connect(GTK_WIDGET(gtk_builder_get_object(main_builder, "button-load-gds")),
			 "clicked", G_CALLBACK(on_load_gds), (gpointer)&open_data);


	gtk_main();

	return 0;
}
