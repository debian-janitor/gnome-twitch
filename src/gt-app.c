#include "gt-app.h"
#include "gt-win.h"
#include <glib/gi18n.h>
#include <glib/gstdio.h>
#include <errno.h>
#include <string.h>
#include <json-glib/json-glib.h>

#define DATA_DIR g_build_filename(g_get_user_data_dir(), "gnome-twitch", NULL)
#define CHANNEL_SETTINGS_FILE g_build_filename(g_get_user_data_dir(), "gnome-twitch", "channel_settings.json", NULL);

typedef struct
{
    GtWin* win;

    gchar* oauth_token;
    gchar* user_name;

    GMenuItem* login_item;
    GMenuModel* app_menu;
} GtAppPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(GtApp, gt_app, GTK_TYPE_APPLICATION)

enum
{
    PROP_0,
    PROP_OAUTH_TOKEN,
    PROP_USER_NAME,
    NUM_PROPS
};

static GParamSpec* props[NUM_PROPS];

enum
{
    NUM_SIGS
};

static guint sigs[NUM_SIGS];

GtApp*
gt_app_new(void)
{
    return g_object_new(GT_TYPE_APP,
                        "application-id", "com.gnome-twitch.app",
                        NULL);
}

GtChatViewSettings*
gt_chat_view_settings_new()
{
    GtChatViewSettings* ret = g_new0(GtChatViewSettings, 1);

    ret->dark_theme = TRUE;
    ret->opacity = 1.0;
    ret->visible = TRUE;
    ret->docked = TRUE;
    ret->width = 0.2;
    ret->height = 1.0;
    ret->x_pos = 0;
    ret->y_pos = 0;

    return ret;
}

static void
load_chat_settings(GtApp* self)
{
    gchar* fp = CHANNEL_SETTINGS_FILE;
    JsonParser* parse = json_parser_new();
    JsonNode* root = NULL;
    JsonArray* array = NULL;
    GError* err = NULL;

    g_message("{GtApp} Loading chat settings");

    if (!g_file_test(fp, G_FILE_TEST_EXISTS))
        goto finish;

    json_parser_load_from_file(parse, fp, &err);

    if (err)
    {
        g_warning("{GtApp} Error loading chat settings '%s'", err->message);
        goto finish;
    }

    root = json_parser_get_root(parse);
    array = json_node_get_array(root);

    for (GList* l = json_array_get_elements(array); l != NULL; l = l->next)
    {
        JsonNode* node = l->data;
        JsonObject* chan = json_node_get_object(node);
        GtChatViewSettings* settings = gt_chat_view_settings_new();
        const gchar* name;

        name = json_object_get_string_member(chan, "name");
        settings->dark_theme = json_object_get_boolean_member(chan, "dark-theme");
        settings->visible = json_object_get_boolean_member(chan, "visible");
        settings->docked = json_object_get_boolean_member(chan, "docked");
        settings->opacity = json_object_get_double_member(chan, "opacity");
        settings->width = json_object_get_double_member(chan, "width");
        settings->height = json_object_get_double_member(chan, "height");
        settings->x_pos = json_object_get_double_member(chan, "x-pos");
        settings->y_pos = json_object_get_double_member(chan, "y-pos");

        g_hash_table_insert(self->chat_settings_table, g_strdup(name), settings);
    }

finish:
    g_object_unref(parse);
    g_free(fp);
}

static void
save_chat_settings(GtApp* self)
{
    gchar* fp = CHANNEL_SETTINGS_FILE;
    JsonArray* array = json_array_new();
    JsonGenerator* generator = json_generator_new();
    JsonNode* root = json_node_new(JSON_NODE_ARRAY);
    GList* keys = g_hash_table_get_keys(self->chat_settings_table);
    GError* err = NULL;

    g_message("{GtApp} Saving chat settings");

    for (GList* l = keys; l != NULL; l = l->next)
    {
        JsonObject* obj = json_object_new();
        JsonNode* node = json_node_new(JSON_NODE_OBJECT);
        const gchar* key = l->data;
        GtChatViewSettings* settings = g_hash_table_lookup(self->chat_settings_table, key);

        json_object_set_string_member(obj, "name", key);
        json_object_set_boolean_member(obj, "dark-theme", settings->dark_theme);
        json_object_set_boolean_member(obj, "visible", settings->visible);
        json_object_set_boolean_member(obj, "docked", settings->docked);
        json_object_set_double_member(obj, "opacity", settings->opacity);
        json_object_set_double_member(obj, "width", settings->width);
        json_object_set_double_member(obj, "height", settings->height);
        json_object_set_double_member(obj, "x-pos", settings->x_pos);
        json_object_set_double_member(obj, "y-pos", settings->y_pos);

        json_node_take_object(node, obj);
        json_array_add_element(array, node);
    }

    json_node_take_array(root, array);

    json_generator_set_root(generator, root);
    json_generator_to_file(generator, fp, &err);

    if (err)
        g_warning("{GtApp} Error saving chat settings '%s'", err->message);

    json_node_free(root);
    g_object_unref(generator);
    g_list_free(keys);
    g_free(fp);
}

static void
oauth_token_set_cb(GObject* src,
                   GParamSpec* pspec,
                   gpointer udata)
{
    GtApp* self = GT_APP(udata);
    GtAppPrivate* priv = gt_app_get_instance_private(self);

    if (priv->oauth_token && strlen(priv->oauth_token) > 0)
    {
        g_menu_remove(G_MENU(priv->app_menu), 0);
        g_menu_item_set_label(priv->login_item, _("Refresh login"));
        g_menu_prepend_item(G_MENU(priv->app_menu), priv->login_item);
    }
}

static void
init_dirs()
{
    gchar* fp = DATA_DIR;

    int err = g_mkdir(fp, 0777);

    if (err != 0 && g_file_error_from_errno(errno) != G_FILE_ERROR_EXIST)
    {
        g_warning("{GtApp} Error creating data dir");
    }

    g_free(fp);
}


static void
quit_cb(GSimpleAction* action,
        GVariant* par,
        gpointer udata)
{
    g_message("{%s} Quitting", "GtApp");

    GtApp* self = GT_APP(udata);

    g_application_quit(G_APPLICATION(self));
}

static GActionEntry app_actions[] =
{
    {"quit", quit_cb, NULL, NULL, NULL}
};

static void
activate(GApplication* app)
{
    g_message("{%s} Activate", "GtApp");

    GtApp* self = GT_APP(app);
    GtAppPrivate* priv = gt_app_get_instance_private(self);
    GtkBuilder* menu_bld;

    init_dirs();

    g_action_map_add_action_entries(G_ACTION_MAP(self),
                                    app_actions,
                                    G_N_ELEMENTS(app_actions),
                                    self);

    menu_bld = gtk_builder_new_from_resource("/com/gnome-twitch/ui/app-menu.ui");
    priv->app_menu = G_MENU_MODEL(gtk_builder_get_object(menu_bld, "app_menu"));
    g_menu_prepend_item(G_MENU(priv->app_menu), priv->login_item);

    gtk_application_set_app_menu(GTK_APPLICATION(app), G_MENU_MODEL(priv->app_menu));
    g_object_unref(menu_bld);

    g_settings_bind(self->settings, "user-name",
                    self, "user-name",
                    G_SETTINGS_BIND_DEFAULT);
    g_settings_bind(self->settings, "oauth-token",
                    self, "oauth-token",
                    G_SETTINGS_BIND_DEFAULT);

    priv->win = gt_win_new(self);

    gtk_window_present(GTK_WINDOW(priv->win));
}

static void
gt_app_prefer_dark_theme_changed_cb(GSettings *settings,
                                    const char* key,
                                    GtkSettings *gtk_settings)
{
    gboolean prefer_dark_theme = g_settings_get_boolean(settings, key);

    g_object_set(gtk_settings,
                 "gtk-application-prefer-dark-theme",
                 prefer_dark_theme,
                 NULL);
}

static void
startup(GApplication* app)
{
    GtApp* self = GT_APP(app);
    GtAppPrivate* priv = gt_app_get_instance_private(self);
    GtkSettings *gtk_settings = gtk_settings_get_default();

    self->fav_mgr = gt_favourites_manager_new();
    gt_favourites_manager_load(self->fav_mgr);

    gt_app_prefer_dark_theme_changed_cb(self->settings,
                                        "prefer-dark-theme",
                                        gtk_settings);
    g_signal_connect(self->settings,
                     "changed::prefer-dark-theme",
                     G_CALLBACK(gt_app_prefer_dark_theme_changed_cb),
                     gtk_settings);

    G_APPLICATION_CLASS(gt_app_parent_class)->startup(app);
}

static void
shutdown(GApplication* app)
{
    GtApp* self = GT_APP(app);

    g_message("{GtApp} Shutting down");

    save_chat_settings(self);

    G_APPLICATION_CLASS(gt_app_parent_class)->shutdown(app);
}


static void
finalize(GObject* object)
{
    g_message("{GtApp} Finalise");

    GtApp* self = (GtApp*) object;
    GtAppPrivate* priv = gt_app_get_instance_private(self);

    G_OBJECT_CLASS(gt_app_parent_class)->finalize(object);
}

static void
get_property (GObject*    obj,
              guint       prop,
              GValue*     val,
              GParamSpec* pspec)
{
    GtApp* self = GT_APP(obj);
    GtAppPrivate* priv = gt_app_get_instance_private(self);

    switch (prop)
    {
        case PROP_OAUTH_TOKEN:
            g_value_set_string(val, priv->oauth_token);
            break;
        case PROP_USER_NAME:
            g_value_set_string(val, priv->user_name);
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
    GtApp* self = GT_APP(obj);
    GtAppPrivate* priv = gt_app_get_instance_private(self);

    switch (prop)
    {
        case PROP_OAUTH_TOKEN:
            g_free(priv->oauth_token);
            priv->oauth_token = g_value_dup_string(val);
            break;
        case PROP_USER_NAME:
            g_free(priv->user_name);
            priv->user_name = g_value_dup_string(val);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop, pspec);
    }
}

static void
gt_app_class_init(GtAppClass* klass)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    G_APPLICATION_CLASS(klass)->activate = activate;
    G_APPLICATION_CLASS(klass)->startup = startup;
    G_APPLICATION_CLASS(klass)->shutdown = shutdown;

    object_class->finalize = finalize;
    object_class->get_property = get_property;
    object_class->set_property = set_property;

    props[PROP_OAUTH_TOKEN] = g_param_spec_string("oauth-token",
                                                  "Oauth token",
                                                  "Twitch Oauth token",
                                                  NULL,
                                                  G_PARAM_READWRITE);
    props[PROP_USER_NAME] = g_param_spec_string("user-name",
                                                "User name",
                                                "User name",
                                                NULL,
                                                G_PARAM_READWRITE);

    g_object_class_install_properties(object_class, NUM_PROPS, props);
}

static void
gt_app_init(GtApp* self)
{
    GtAppPrivate* priv = gt_app_get_instance_private(self);

    self->chat_settings_table = g_hash_table_new(g_str_hash, g_str_equal);

    priv->login_item = g_menu_item_new(_("Login to Twitch"), "win.show_twitch_login");

    self->twitch = gt_twitch_new();
    self->settings = g_settings_new("com.gnome-twitch.app");

    load_chat_settings(self);
    //  self->chat = gt_irc_new();

    g_signal_connect(self, "notify::oauth-token", G_CALLBACK(oauth_token_set_cb), self);

    /* g_signal_connect(self, "activate", G_CALLBACK(activate), NULL); */
    /* g_signal_connect(self, "startup", G_CALLBACK(startup), NULL); */
}

const gchar*
gt_app_get_user_name(GtApp* self)
{
    GtAppPrivate* priv = gt_app_get_instance_private(self);

    return priv->user_name;
}

const gchar*
gt_app_get_oauth_token(GtApp* self)
{
    GtAppPrivate* priv = gt_app_get_instance_private(self);

    return priv->oauth_token;
}

gboolean
gt_app_credentials_valid(GtApp* self)
{
    GtAppPrivate* priv = gt_app_get_instance_private(self);

    return
        priv->oauth_token             &&
        priv->user_name               &&
        strlen(priv->oauth_token) > 1 &&
        strlen(priv->user_name) > 1;
}
