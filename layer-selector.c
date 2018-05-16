#include "layer-selector.h"
#include "gdsparse.h"


static void layer_list_remove_element(struct layer_info *inf)
{
	if (inf)
		free(inf);
}

void get_layer_info(GList **info_list, GtkListBox *box)
{
	GList *local_list = NULL;

	/* Clear info Glist */
	if (*info_list != NULL) {
		g_list_free_full(*info_list, (GDestroyNotify)layer_list_remove_element);
		*info_list = NULL;
	}



	*info_list = local_list;
}

static GList *layer_widgets = NULL;

static void delete_layer_widget(GtkWidget *widget)
{

	gtk_widget_destroy(widget);
}

void clear_list_box_widgets(GtkListBox *box)
{
	GList *list;

	list = gtk_container_get_children(GTK_CONTAINER(box));
	for (;list != NULL; list = list->next) {
		gtk_container_remove(GTK_CONTAINER(box), GTK_WIDGET(list->data));
	}
	/* Widgets are already destroyed when removed from box */
	g_list_free(layer_widgets);
	layer_widgets = NULL;
}

static gboolean check_if_layer_widget_exists(int layer) {
	GList *list;
	LayerElement *widget;
	gboolean ret = FALSE;

	for (list = layer_widgets; list != NULL; list = list->next) {
		widget = (LayerElement *)list->data;
		if (layer_element_get_layer(widget) == layer) {
			ret = TRUE;
			break;
		}
	}
	return ret;
}

static void analyze_cell_layers(GtkListBox *listbox, struct gds_cell *cell)
{
	GList *graphics;
	struct gds_graphics *gfx;
	int layer;
	GtkWidget *le;

	for (graphics = cell->graphic_objs; graphics != NULL; graphics = graphics->next) {
		gfx = (struct gds_graphics *)graphics->data;
		layer = (int)gfx->layer;
		if (check_if_layer_widget_exists(layer) == FALSE) {
			le = layer_element_new();
			layer_element_set_layer(LAYER_ELEMENT(le), layer);
			gtk_list_box_insert(listbox, le, -1);
			gtk_widget_show(le);
			layer_widgets = g_list_append(layer_widgets, le);
			printf("added new layer\n");
		}
	}
}


gint sort_func(GtkListBoxRow *row1, GtkListBoxRow *row2, gpointer unused)
{
	GList *children1, *children2;
	LayerElement *le1, *le2;
	gint ret;

	children1 = gtk_container_get_children(GTK_CONTAINER(row1));
	children2 = gtk_container_get_children(GTK_CONTAINER(row2));
	le1 = LAYER_ELEMENT(children1->data);
	le2 = LAYER_ELEMENT(children2->data);

	ret = layer_element_get_layer(le1) - layer_element_get_layer(le2);

	g_list_free(children1);
	g_list_free(children2);

	return ret;
}

void generate_layer_widgets(GtkListBox *listbox, GList *libs)
{
	GList *cell_list = NULL;
	struct gds_library *lib;
	printf("foo?\n");
	clear_list_box_widgets(listbox);

	gtk_list_box_set_sort_func(listbox, sort_func, NULL, NULL);
	printf("layers deleted\n");

	for (; libs != NULL; libs = libs->next) {
		lib = (struct gds_library *)libs->data;
		for (cell_list = lib->cells; cell_list != NULL; cell_list = cell_list->next) {
			analyze_cell_layers(listbox, (struct gds_cell *)cell_list->data);
		} /* For Cell List */
	} /* For libs */

	/* Force sort */
	gtk_list_box_invalidate_sort(listbox);
}
