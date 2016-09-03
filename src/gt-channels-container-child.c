#include "gt-channels-container-child.h"
#include <glib/gprintf.h>
#include <glib/gi18n.h>

#define TAG "GtChannelsContainerChild"
#include "utils.h"

typedef struct
{
    GtChannel* channel;

    GtkWidget* preview_image;
    GtkWidget* name_label;
    GtkWidget* game_label;
    GtkWidget* event_box;
    GtkWidget* middle_revealer;
    GtkWidget* viewers_label;
    GtkWidget* time_label;
    GtkWidget* follow_button;
    GtkWidget* middle_stack;
    GtkWidget* play_image;
    GtkWidget* bottom_box;
} GtChannelsContainerChildPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(GtChannelsContainerChild, gt_channels_container_child, GTK_TYPE_FLOW_BOX_CHILD)

enum
{
    PROP_0,
    PROP_CHANNEL,
    NUM_PROPS
};

static GParamSpec* props[NUM_PROPS];

GtChannelsContainerChild*
gt_channels_container_child_new(GtChannel* chan)
{
    return g_object_new(GT_TYPE_CHANNELS_CONTAINER_CHILD,
                        "channel", chan,
                        NULL);
}

static gboolean
updating_converter(GBinding* bind,
                   const GValue* from,
                   GValue* to,
                   gpointer udata)
{
    if (g_value_get_boolean(from))
        g_value_set_string(to, "spinner");
    else
        g_value_set_string(to, "content");

    return TRUE;
}

static void
motion_enter_cb(GtkWidget* widget,
                GdkEvent* evt,
                gpointer udata)
{
    GtChannelsContainerChild* self = GT_CHANNELS_CONTAINER_CHILD(udata);
    GtChannelsContainerChildPrivate* priv = gt_channels_container_child_get_instance_private(self);

    gtk_revealer_set_reveal_child(GTK_REVEALER(priv->middle_revealer), TRUE);
}

static void
motion_leave_cb(GtkWidget* widget,
                GdkEvent* evt,
                gpointer udata)
{
    GtChannelsContainerChild* self = GT_CHANNELS_CONTAINER_CHILD(udata);
    GtChannelsContainerChildPrivate* priv = gt_channels_container_child_get_instance_private(self);

    gtk_revealer_set_reveal_child(GTK_REVEALER(priv->middle_revealer), FALSE);
}

static void
follow_button_cb(GtkButton* button,
                    gpointer udata)
{
    GtChannelsContainerChild* self = GT_CHANNELS_CONTAINER_CHILD(udata);
    GtChannelsContainerChildPrivate* priv = gt_channels_container_child_get_instance_private(self);

    gt_channel_toggle_followed(priv->channel);
}

static gboolean
viewers_converter(GBinding* bind,
                  const GValue* from,
                  GValue* to,
                  gpointer udata)
{
    gint64 viewers;
    gchar* label = NULL;

    if (g_value_get_int64(from) > -1)
    {
        viewers = g_value_get_int64(from);

        if (viewers > 1e4)
            // Translators: Used for when viewers >= 1000
            // Shorthand for thousands. Ex (English): 6200 = 6.2k
            label = g_strdup_printf(_("%3.1fk"), (gdouble) viewers / 1e3);
        else
            // Translators: Used for when viewers < 1000
            // No need to translate, just future-proofing
            label = g_strdup_printf(_("%ld"), viewers);
    }

    g_value_take_string(to, label);

    return TRUE;
}

static gboolean
time_converter(GBinding* bind,
               const GValue* from,
               GValue* to,
               gpointer udata)
{
    gchar* label = NULL;
    GDateTime* now_time;
    GDateTime* stream_started_time;
    GTimeSpan dif;

    if (g_value_get_pointer(from) != NULL)
    {
        now_time = g_date_time_new_now_utc();
        stream_started_time = (GDateTime*) g_value_get_pointer(from);

        dif = g_date_time_difference(now_time, stream_started_time);

        if (dif > G_TIME_SPAN_HOUR)
            // Translators: Used for when stream time > 60 min
            // Ex (English): 3 hours and 45 minutes = 3.75h
            label = g_strdup_printf(_("%2.1fh"), (gdouble) dif / G_TIME_SPAN_HOUR);
        else
            // Translators: Used when stream time <= 60min
            // Ex (English): 45 minutes = 45m
            label  = g_strdup_printf(_("%ldm"), dif / G_TIME_SPAN_MINUTE);

        g_date_time_unref(now_time);
    }

    g_value_take_string(to, label);

    return TRUE;
}

static void
online_cb(GObject* source,
          GParamSpec* pspec,
          gpointer udata)
{
    GtChannelsContainerChild* self = GT_CHANNELS_CONTAINER_CHILD(udata);
    GtChannelsContainerChildPrivate* priv = gt_channels_container_child_get_instance_private(self);
    gboolean online;

    g_object_get(priv->channel, "online", &online, NULL);

    if (online)
        REMOVE_STYLE_CLASS(self, "gt-channels-container-child-offline");
    else
        ADD_STYLE_CLASS(self, "gt-channels-container-child-offline");
}

static void
finalize(GObject* object)
{
    GtChannelsContainerChild* self = (GtChannelsContainerChild*) object;
    GtChannelsContainerChildPrivate* priv = gt_channels_container_child_get_instance_private(self);

    g_object_unref(priv->channel);

    G_OBJECT_CLASS(gt_channels_container_child_parent_class)->finalize(object);
}

static void
realise_cb(GtkWidget* widget,
           gpointer udata)
{
    GtChannelsContainerChild* self = GT_CHANNELS_CONTAINER_CHILD(widget);
    GtChannelsContainerChildPrivate* priv = gt_channels_container_child_get_instance_private(self);

    g_object_bind_property(priv->channel, "online",
                           priv->viewers_label, "visible",
                           G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
    g_object_bind_property(priv->channel, "online",
                           priv->play_image, "visible",
                           G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
    g_object_bind_property(priv->channel, "online",
                           priv->bottom_box, "visible",
                           G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
}

static void
get_property (GObject*    obj,
              guint       prop,
              GValue*     val,
              GParamSpec* pspec)
{
    GtChannelsContainerChild* self = GT_CHANNELS_CONTAINER_CHILD(obj);
    GtChannelsContainerChildPrivate* priv = gt_channels_container_child_get_instance_private(self);

    switch (prop)
    {
        case PROP_CHANNEL:
            g_value_set_object(val, priv->channel);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop, pspec);
    }
}

static void
set_property(GObject*      obj,
             guint         prop,
             const GValue* val,
             GParamSpec*   pspec)
{
    GtChannelsContainerChild* self = GT_CHANNELS_CONTAINER_CHILD(obj);
    GtChannelsContainerChildPrivate* priv = gt_channels_container_child_get_instance_private(self);

    switch (prop)
    {
        case PROP_CHANNEL:
            g_clear_object(&priv->channel);
            priv->channel = utils_value_ref_sink_object(val);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop, pspec);
    }
}

static void
constructed(GObject* obj)
{
    GtChannelsContainerChild* self = GT_CHANNELS_CONTAINER_CHILD(obj);
    GtChannelsContainerChildPrivate* priv = gt_channels_container_child_get_instance_private(self);

    g_object_bind_property(priv->channel, "display-name",
                           priv->name_label, "label",
                           G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
    g_object_bind_property(priv->channel, "game",
                           priv->game_label, "label",
                           G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
    g_object_bind_property(priv->channel, "followed",
                           priv->follow_button, "active",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(priv->channel, "preview",
                           priv->preview_image, "pixbuf",
                           G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
    g_object_bind_property_full(priv->channel, "viewers",
                                priv->viewers_label, "label",
                                G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE,
                                (GBindingTransformFunc) viewers_converter,
                                NULL, NULL, NULL);
    g_object_bind_property_full(priv->channel, "stream-started-time",
                                priv->time_label, "label",
                                G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE,
                                (GBindingTransformFunc) time_converter,
                                NULL, NULL, NULL);
    g_object_bind_property_full(priv->channel, "updating",
                                priv->middle_stack, "visible-child-name",
                                G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE,
                                (GBindingTransformFunc) updating_converter,
                                NULL, NULL, NULL);

    g_signal_connect(priv->channel, "notify::online", G_CALLBACK(online_cb), self);

    online_cb(NULL, NULL, self);

    G_OBJECT_CLASS(gt_channels_container_child_parent_class)->constructed(obj);
}

static void
gt_channels_container_child_class_init(GtChannelsContainerChildClass* klass)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->finalize = finalize;
    object_class->get_property = get_property;
    object_class->set_property = set_property;
    object_class->constructed = constructed;

    gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(klass),
                                                "/com/vinszent/GnomeTwitch/ui/gt-channels-container-child.ui");

    props[PROP_CHANNEL] = g_param_spec_object("channel",
                                              "Channel",
                                              "Associated channel",
                                              GT_TYPE_CHANNEL,
                                              G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

    g_object_class_install_properties(object_class,
                                      NUM_PROPS,
                                      props);

    gtk_widget_class_bind_template_callback(GTK_WIDGET_CLASS(klass), motion_enter_cb);
    gtk_widget_class_bind_template_callback(GTK_WIDGET_CLASS(klass), motion_leave_cb);
    gtk_widget_class_bind_template_callback(GTK_WIDGET_CLASS(klass), follow_button_cb);
    gtk_widget_class_bind_template_child_private(GTK_WIDGET_CLASS(klass), GtChannelsContainerChild, preview_image);
    gtk_widget_class_bind_template_child_private(GTK_WIDGET_CLASS(klass), GtChannelsContainerChild, name_label);
    gtk_widget_class_bind_template_child_private(GTK_WIDGET_CLASS(klass), GtChannelsContainerChild, game_label);
    gtk_widget_class_bind_template_child_private(GTK_WIDGET_CLASS(klass), GtChannelsContainerChild, event_box);
    gtk_widget_class_bind_template_child_private(GTK_WIDGET_CLASS(klass), GtChannelsContainerChild, middle_revealer);
    gtk_widget_class_bind_template_child_private(GTK_WIDGET_CLASS(klass), GtChannelsContainerChild, viewers_label);
    gtk_widget_class_bind_template_child_private(GTK_WIDGET_CLASS(klass), GtChannelsContainerChild, time_label);
    gtk_widget_class_bind_template_child_private(GTK_WIDGET_CLASS(klass), GtChannelsContainerChild, follow_button);
    gtk_widget_class_bind_template_child_private(GTK_WIDGET_CLASS(klass), GtChannelsContainerChild, middle_stack);
    gtk_widget_class_bind_template_child_private(GTK_WIDGET_CLASS(klass), GtChannelsContainerChild, play_image);
    gtk_widget_class_bind_template_child_private(GTK_WIDGET_CLASS(klass), GtChannelsContainerChild, bottom_box);
}

static void
gt_channels_container_child_init(GtChannelsContainerChild* self)
{
    GtChannelsContainerChildPrivate* priv = gt_channels_container_child_get_instance_private(self);

    g_signal_connect(self, "realize", G_CALLBACK(realise_cb), self);

    gtk_widget_init_template(GTK_WIDGET(self));
}

void
gt_channels_container_child_hide_overlay(GtChannelsContainerChild* self)
{
    GtChannelsContainerChildPrivate* priv = gt_channels_container_child_get_instance_private(self);

    gtk_revealer_set_reveal_child(GTK_REVEALER(priv->middle_revealer), FALSE);
}
