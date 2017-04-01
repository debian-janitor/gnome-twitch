/*
 *  This file is part of GNOME Twitch - 'Enjoy Twitch on your GNU/Linux desktop'
 *  Copyright © 2017 Vincent Szolnoky <vinszent@vinszent.com>
 *
 *  GNOME Twitch is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  GNOME Twitch is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GNOME Twitch. If not, see <http://www.gnu.org/licenses/>.
 */

#include <glib/gstdio.h>
#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "config.h"
#include "gt-win.h"

#define TAG "Utils"
#include "gnome-twitch/gt-log.h"

gpointer
utils_value_ref_sink_object(const GValue* val)
{
    if (val == NULL || !G_VALUE_HOLDS_OBJECT(val) || g_value_get_object(val) == NULL)
        return NULL;
    else
        return g_object_ref_sink(g_value_get_object(val));
}

gchar*
utils_value_dup_string_allow_null(const GValue* val)
{
    if (g_value_get_string(val))
        return g_value_dup_string(val);

    return NULL;
}

void
utils_container_clear(GtkContainer* cont)
{
    for(GList* l = gtk_container_get_children(cont);
        l != NULL; l = l->next)
    {
        gtk_container_remove(cont, GTK_WIDGET(l->data));
    }
}

guint64
utils_timestamp_file(const gchar* filename, GError** error)
{
    RETURN_VAL_IF_FAIL(!utils_str_empty(filename), 0);

    GError* err = NULL; /* NOTE: Doesn't need to be freed because we propagate it */

    if (g_file_test(filename, G_FILE_TEST_EXISTS))
    {
        GTimeVal time;

        g_autoptr(GFile) file = g_file_new_for_path(filename);

        g_autoptr(GFileInfo) info = g_file_query_info(file,
            G_FILE_ATTRIBUTE_TIME_MODIFIED, G_FILE_QUERY_INFO_NONE,
            NULL, &err);

        if (err)
        {
            WARNING("Could not timestamp file because: %s", err->message);

            g_propagate_prefixed_error(error, err, "Could not timestamp file because: ");

            return 0;
        }

        g_file_info_get_modification_time(info, &time);

        return time.tv_sec;
    }

    RETURN_VAL_IF_REACHED(0);
}

gint64
utils_timestamp_now(void)
{
    gint64 timestamp;
    GDateTime* now;

    now = g_date_time_new_now_utc();
    timestamp = g_date_time_to_unix(now);
    g_date_time_unref(now);

    return timestamp;
}

void
utils_pixbuf_scale_simple(GdkPixbuf** pixbuf, gint width, gint height, GdkInterpType interp)
{
    if (!*pixbuf)
        return;

    GdkPixbuf* tmp = gdk_pixbuf_scale_simple(*pixbuf, width, height, interp);
    g_clear_object(pixbuf);
    *pixbuf = tmp;
}

gint64
utils_http_full_date_to_timestamp(const char* string)
{
    gint64 ret = G_MAXINT64;
    g_autoptr(SoupDate) date = NULL;

    date = soup_date_new_from_string(string);

    RETURN_VAL_IF_FAIL(date != NULL, ret);

    ret = soup_date_to_time_t(date);

    return ret;
}

const gchar*
utils_search_key_value_strv(gchar** strv, const gchar* key)
{
    if (!strv)
        return NULL;

    for (gchar** s = strv; *s != NULL; s += 2)
    {
        if (g_strcmp0(*s, key) == 0)
            return *(s+1);
    }

    return NULL;
}

static gboolean
utils_mouse_hover_enter_cb(GtkWidget* widget,
                           GdkEvent* evt,
                           gpointer udata)
{
    GdkWindow* win;
    GdkDisplay* disp;
    GdkCursor* cursor;

    win = ((GdkEventMotion*) evt)->window;
    disp = gdk_window_get_display(win);
    cursor = gdk_cursor_new_for_display(disp, GDK_HAND2);

    gdk_window_set_cursor(win, cursor);

    g_object_unref(cursor);

    return FALSE;
}

static gboolean
utils_mouse_hover_leave_cb(GtkWidget* widget,
                           GdkEvent* evt,
                           gpointer udata)
{
    GdkWindow* win;
    GdkDisplay* disp;
    GdkCursor* cursor;

    win = ((GdkEventMotion*) evt)->window;
    disp = gdk_window_get_display(win);
    cursor = gdk_cursor_new_for_display(disp, GDK_LEFT_PTR);

    gdk_window_set_cursor(win, cursor);

    g_object_unref(cursor);

    return FALSE;
}

static gboolean
utils_mouse_clicked_link_cb(GtkWidget* widget,
                            GdkEventButton* evt,
                            gpointer udata)
{
    if (evt->button == 1 && evt->type == GDK_BUTTON_PRESS)
    {
        GtWin* parent = GT_WIN_TOPLEVEL(widget);

        gtk_show_uri_on_window(GTK_WINDOW(parent), (gchar*) udata, GDK_CURRENT_TIME, NULL);
    }

    return FALSE;
}

void
utils_connect_mouse_hover(GtkWidget* widget)
{
    g_signal_connect(widget, "enter-notify-event", G_CALLBACK(utils_mouse_hover_enter_cb), NULL);
    g_signal_connect(widget, "leave-notify-event", G_CALLBACK(utils_mouse_hover_leave_cb), NULL);
}

void
utils_connect_link(GtkWidget* widget, const gchar* link)
{
    gchar* tmp = g_strdup(link); //TODO: Free this
    utils_connect_mouse_hover(widget);
    g_signal_connect(widget, "button-press-event", G_CALLBACK(utils_mouse_clicked_link_cb), tmp);
}

gboolean
utils_str_empty(const gchar* str)
{
    return !(str && strlen(str) > 0);
}

gchar*
utils_str_capitalise(const gchar* str)
{
    g_assert_false(utils_str_empty(str));

    gchar* ret = g_strdup_printf("%c%s", g_ascii_toupper(*str), str+1);

    return ret;
}

typedef struct
{
    gpointer instance;
    GCallback cb;
    gpointer udata;
} OneshotData;

static void
oneshot_cb(OneshotData* data)
{
    g_signal_handlers_disconnect_by_func(data->instance,
                                         data->cb,
                                         data->udata);
    g_signal_handlers_disconnect_by_func(data->instance,
                                         oneshot_cb,
                                         data);
}

void
utils_signal_connect_oneshot(gpointer instance,
                             const gchar* signal,
                             GCallback cb,
                             gpointer udata)
{
    OneshotData* data = g_new(OneshotData, 1);

    data->instance = instance;
    data->cb = cb;
    data->udata = udata;

    g_signal_connect(instance, signal, cb, udata);

    g_signal_connect_data(instance,
                          signal,
                          G_CALLBACK(oneshot_cb),
                          data,
                          (GClosureNotify) g_free,
                          G_CONNECT_AFTER | G_CONNECT_SWAPPED);
}

void
utils_signal_connect_oneshot_swapped(gpointer instance,
    const gchar* signal, GCallback cb, gpointer udata)
{

    OneshotData* data = g_new(OneshotData, 1);

    data->instance = instance;
    data->cb = cb;
    data->udata = udata;

    g_signal_connect_swapped(instance, signal, cb, udata);

    g_signal_connect_data(instance,
                          signal,
                          G_CALLBACK(oneshot_cb),
                          data,
                          (GClosureNotify) g_free,
                          G_CONNECT_AFTER | G_CONNECT_SWAPPED);
}

inline void
utils_refresh_cancellable(GCancellable** cancel)
{
    if (*cancel && !g_cancellable_is_cancelled(*cancel))
        g_cancellable_cancel(*cancel);
    g_clear_object(cancel);
    *cancel = g_cancellable_new();
}

GWeakRef*
utils_create_weak_ref(gpointer obj)
{
    GWeakRef* ref = g_malloc(sizeof(GWeakRef));

    g_weak_ref_init(ref, obj);

    return ref;
}

void
utils_free_weak_ref(GWeakRef* ref)
{
    g_weak_ref_clear(ref);

    g_clear_pointer(&ref, g_free);
}

GDateTime*
utils_parse_time_iso_8601(const gchar* time, GError** error)
{
    GDateTime* ret = NULL;

    gint year, month, day, hour, min, sec;

    gint scanned = sscanf(time, "%d-%d-%dT%d:%d:%dZ", &year, &month, &day, &hour, &min, &sec);

    if (scanned != 6)
    {
        g_set_error(error, GT_UTILS_ERROR, GT_UTILS_ERROR_PARSING_TIME,
            "Unable to parse time from input '%s'", time);
    }
    else
        ret = g_date_time_new_utc(year, month, day, hour, min, sec);

    return ret;
}

GenericTaskData*
generic_task_data_new()
{
    return g_slice_new0(GenericTaskData);
}

void
generic_task_data_free(GenericTaskData* data)
{
    g_free(data->str_1);
    g_free(data->str_2);
    g_free(data->str_3);

    g_slice_free(GenericTaskData, data);
}
