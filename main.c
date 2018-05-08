#include <stdio.h>
#include "gdsparse.h"
#include <gtk/gtk.h>

gboolean on_window_close(gpointer window, gpointer user)
{
	gtk_widget_destroy(GTK_WIDGET(window));
	gtk_main_quit();
	return TRUE;
}

void on_load_gds(gpointer button, gpointer user)
{
	GList **list_ptr = (GList **)user;

	// TODO: File dialog
	clear_lib_list(list_ptr);
	parse_gds_from_file("/home/mari/Desktop/test.gds", list_ptr);
}

void on_convert_clicked(gpointer button, gpointer user)
{
	printf("convert\n");
}

int main(int argc, char **argv)
{
	GtkBuilder *main_builder;
	GList *gds_libs = NULL;

	gtk_init(&argc, &argv);

	main_builder = gtk_builder_new_from_file("glade/main.glade");
	gtk_builder_connect_signals(main_builder, NULL);

	g_signal_connect(GTK_WIDGET(gtk_builder_get_object(main_builder, "button-load-gds")),
			 "clicked", G_CALLBACK(on_load_gds), (gpointer)&gds_libs);


	gtk_main();

	return 0;
}
