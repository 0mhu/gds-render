#ifndef __LAYER_ELEMENT_H__
#define __LAYER_ELEMENT_H__

#include <gtk/gtk.h>
// #include <gdk/gdk.h>

#define LAYER_ELEMENT(obj) G_TYPE_CHECK_INSTANCE_CAST(obj, layer_element_get_type(), LayerElement)
#define LAYER_ELEMENT_CLASS(klass) G_TYPE_CHECK_CLASS_CAST(klass, layer_element_get_type(), LayerElementClass)
#define IS_LAYE_RELEMENT(obj) G_TYPE_CHECK_INSTANCE_TYPE(obj, layer_element_get_type())

typedef struct _LayerElement {
    /* Inheritance */
    GtkBox hbox;
    /* Custom Elements */
    GtkWidget *button;
} LayerElement;

typedef struct _LayerElementClass {
    GtkBoxClass parent_class;
} LayerElementClass;


GType layer_element_get_type(void);
GtkWidget *layer_element_new(void);

#endif /* __LAYER_ELEMENT_H__ */
