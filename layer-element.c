#include "layer-element.h"

G_DEFINE_TYPE (LayerElement, layer_element, GTK_TYPE_BOX)

static void layer_element_class_init(LayerElementClass *klass)
{
	return;
}

static void layer_element_init(LayerElement *self)
{
	self->button = gtk_button_new();
	gtk_box_pack_start(GTK_BOX(self), self->button, TRUE, TRUE, 0);
	gtk_widget_show(self->button);
}

GtkWidget *layer_element_new(void)
{
	return GTK_WIDGET(g_object_new(layer_element_get_type(), NULL));
}
