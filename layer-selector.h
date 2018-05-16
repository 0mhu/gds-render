#ifndef __LAYER_SELECTOR_H__
#define __LAYER_SELECTOR_H__

#include <gtk/gtk.h>
#include <glib.h>
#include "layer-element.h" 

struct layer_info {
    int id;
    /* This contains both: opacity and Color */
    GdkRGBA color;
};

void generate_layer_widgets(GtkListBox *listbox, GList *libs);
void clear_list_box_widgets();

void setup_load_mapping_callback(GtkWidget *button);
void setup_save_mapping_callback(GtkWidget *button);

void get_layer_info(GList **info_list, GtkListBox *box);


#endif /* __LAYER_SELECTOR_H__ */
