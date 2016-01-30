#ifndef _UTILS_H
#define _UTILS_H

#include <gtk/gtk.h>
#include <libsoup/soup.h>

#define REMOVE_STYLE_CLASS(w, n) gtk_style_context_remove_class(gtk_widget_get_style_context(GTK_WIDGET(w)), n)
#define ADD_STYLE_CLASS(w, n) gtk_style_context_add_class(gtk_widget_get_style_context(GTK_WIDGET(w)), n)
#define UROUND(x) ((guint) ((x) + 0.5))

gpointer utils_value_ref_sink_object(const GValue* val);
gchar* utils_value_dup_string_allow_null(const GValue* val);
void utils_container_clear(GtkContainer* cont);
void utils_pixbuf_scale_simple(GdkPixbuf** pixbuf, gint width, gint height, GdkInterpType interp);
GdkPixbuf* utils_download_picture(SoupSession* soup, const gchar* url);
gchar* utils_search_key_value_strv(gchar** strv, const gchar* key);
void utils_connect_mouse_hover(GtkWidget* widget);
void utils_connect_link(GtkWidget* widget, const gchar* link);

#endif
