/*
 * GDSII-Converter
 * Copyright (C) 2019  Mario Hüttel <mario.huettel@gmx.net>
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

/*
 * Original Drag and Drop Code taken from:
 * https://gitlab.gnome.org/GNOME/gtk/blob/gtk-3-22/tests/testlist3.c
 */

/**
 * @file layer-selector-dnd.c
 * @brief This file implements the drag and drop functions regarding the list box containing the layer elements
 * @author Mario Hüttel <mario.huettel@gmx.net>
 */

#include "layer-selector-dnd.h"

static GtkTargetEntry entries[] = {
	{ "GTK_LIST_BOX_ROW", GTK_TARGET_SAME_APP, 0 }
};

static GtkListBoxRow *layer_selector_get_last_row (GtkListBox *list)
{
	int i;
	GtkListBoxRow *row;

	row = NULL;
	for (i = 0; ; i++) {
		GtkListBoxRow *tmp;
		tmp = gtk_list_box_get_row_at_index(list, i);
		if (tmp == NULL)
			break;
		row = tmp;
	}

	return row;
}


static GtkListBoxRow *layer_selector_get_row_before (GtkListBox *list, GtkListBoxRow *row)
{
	int pos;

	pos = gtk_list_box_row_get_index (row);
	return gtk_list_box_get_row_at_index (list, pos - 1);
}

static GtkListBoxRow *layer_selector_get_row_after (GtkListBox *list, GtkListBoxRow *row)
{
	int pos;

	pos = gtk_list_box_row_get_index(row);
	return gtk_list_box_get_row_at_index(list, pos + 1);
}

static void layer_selector_drag_data_received(GtkWidget *widget, GdkDragContext *context, gint x, gint y,
					     GtkSelectionData *selection_data, guint info, guint32 time,
					     gpointer data)
{
	GtkWidget *row_before, *row_after;
	GtkWidget *row;
	GtkWidget *source;
	int pos;

	row_before = GTK_WIDGET(g_object_get_data(G_OBJECT(widget), "row-before"));
	row_after = GTK_WIDGET(g_object_get_data(G_OBJECT(widget), "row-after"));

	g_object_set_data(G_OBJECT(widget), "row-before", NULL);
	g_object_set_data(G_OBJECT(widget), "row-after", NULL);

	if (row_before)
		gtk_style_context_remove_class(gtk_widget_get_style_context(row_before), "drag-hover-bottom");
	if (row_after)
		gtk_style_context_remove_class(gtk_widget_get_style_context(row_after), "drag-hover-top");

	row = (gpointer) *((gpointer *)gtk_selection_data_get_data(selection_data));
	source = gtk_widget_get_ancestor(row, GTK_TYPE_LIST_BOX_ROW);

	if (source == row_after)
		return;

	g_object_ref(source);
	gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(source)), source);

	if (row_after)
		pos = gtk_list_box_row_get_index(GTK_LIST_BOX_ROW(row_after));
	else
		pos = gtk_list_box_row_get_index(GTK_LIST_BOX_ROW(row_before)) + 1;

	gtk_list_box_insert(GTK_LIST_BOX(widget), source, pos);
	g_object_unref(source);
}

static gboolean layer_selector_drag_motion(GtkWidget *widget, GdkDragContext *context, int x, int y, guint time)
{
	GtkAllocation alloc;
	GtkWidget *row;
	int hover_row_y;
	int hover_row_height;
	GtkWidget *drag_row;
	GtkWidget *row_before;
	GtkWidget *row_after;

	row = GTK_WIDGET(gtk_list_box_get_row_at_y(GTK_LIST_BOX(widget), y));

	drag_row = GTK_WIDGET(g_object_get_data(G_OBJECT(widget), "drag-row"));
	row_after = GTK_WIDGET(g_object_get_data(G_OBJECT(widget), "row-after"));
	row_before = GTK_WIDGET(g_object_get_data(G_OBJECT(widget), "row-before"));

	gtk_style_context_remove_class(gtk_widget_get_style_context(drag_row), "drag-hover");
	if (row_before)
		gtk_style_context_remove_class(gtk_widget_get_style_context(row_before), "drag-hover-bottom");
	if (row_after)
		gtk_style_context_remove_class(gtk_widget_get_style_context(row_after), "drag-hover-top");

	if (row) {
		gtk_widget_get_allocation(row, &alloc);
		hover_row_y = alloc.y;
		hover_row_height = alloc.height;

		if (y < hover_row_y + hover_row_height/2) {
			row_after = row;
			row_before = GTK_WIDGET(layer_selector_get_row_before(GTK_LIST_BOX(widget), GTK_LIST_BOX_ROW(row)));
		} else {
			row_before = row;
			row_after = GTK_WIDGET(layer_selector_get_row_after(GTK_LIST_BOX(widget), GTK_LIST_BOX_ROW(row)));
		}
	} else {
		row_before = GTK_WIDGET(layer_selector_get_last_row(GTK_LIST_BOX(widget)));
		row_after = NULL;
	}

	g_object_set_data(G_OBJECT(widget), "row-before", row_before);
	g_object_set_data(G_OBJECT(widget), "row-after", row_after);

	if (drag_row == row_before || drag_row == row_after) {
		gtk_style_context_add_class(gtk_widget_get_style_context(drag_row), "drag-hover");
		return FALSE;
	}

	if (row_before)
		gtk_style_context_add_class(gtk_widget_get_style_context(row_before), "drag-hover-bottom");
	if (row_after)
		gtk_style_context_add_class(gtk_widget_get_style_context(row_after), "drag-hover-top");

	return TRUE;
}

static void layer_selector_drag_leave(GtkWidget *widget, GdkDragContext *context, guint time)
{
	GtkWidget *drag_row;
	GtkWidget *row_before;
	GtkWidget *row_after;

	drag_row = GTK_WIDGET(g_object_get_data(G_OBJECT(widget), "drag-row"));
	row_before = GTK_WIDGET(g_object_get_data(G_OBJECT(widget), "row-before"));
	row_after = GTK_WIDGET(g_object_get_data(G_OBJECT(widget), "row-after"));

	gtk_style_context_remove_class(gtk_widget_get_style_context(drag_row), "drag-hover");
	if (row_before)
		gtk_style_context_remove_class(gtk_widget_get_style_context(row_before), "drag-hover-bottom");
	if (row_after)
		gtk_style_context_remove_class(gtk_widget_get_style_context(row_after), "drag-hover-top");

}

static const char *dnd_additional_css =
  ".row:not(:first-child) { "
  "  border-top: 1px solid alpha(gray,0.5); "
  "  border-bottom: 1px solid transparent; "
  "}"
  ".row:first-child { "
  "  border-top: 1px solid transparent; "
  "  border-bottom: 1px solid transparent; "
  "}"
  ".row:last-child { "
  "  border-top: 1px solid alpha(gray,0.5); "
  "  border-bottom: 1px solid alpha(gray,0.5); "
  "}"
  ".row.drag-icon { "
  "  background: #282828; "
  "  border: 1px solid blue; "
  "}"
  ".row.drag-row { "
  "  color: gray; "
  "  background: alpha(gray,0.2); "
  "}"
  ".row.drag-row.drag-hover { "
  "  border-top: 1px solid #4e9a06; "
  "  border-bottom: 1px solid #4e9a06; "
  "}"
  ".row.drag-hover image, "
  ".row.drag-hover label { "
  "  color: #4e9a06; "
  "}"
  ".row.drag-hover-top {"
  "  border-top: 1px solid #4e9a06; "
  "}"
  ".row.drag-hover-bottom {"
  "  border-bottom: 1px solid #4e9a06; "
  "}";

void layer_selector_list_box_setup_dnd(GtkListBox *box)
{
	GtkCssProvider *provider;

	provider = gtk_css_provider_new ();
	gtk_css_provider_load_from_data (provider, dnd_additional_css, -1, NULL);
	gtk_style_context_add_provider_for_screen (gdk_screen_get_default (), GTK_STYLE_PROVIDER (provider), 800);

	gtk_drag_dest_set(GTK_WIDGET(box), GTK_DEST_DEFAULT_MOTION | GTK_DEST_DEFAULT_DROP, entries, 1, GDK_ACTION_MOVE);
	g_signal_connect(box, "drag-data-received", G_CALLBACK(layer_selector_drag_data_received), NULL);
	g_signal_connect(box, "drag-motion", G_CALLBACK(layer_selector_drag_motion), NULL);
	g_signal_connect(box, "drag-leave", G_CALLBACK(layer_selector_drag_leave), NULL);

}
