#ifndef GT_WIN_H
#define GT_WIN_H

#include <gtk/gtk.h>
#include "gt-app.h"
#include "gt-channel.h"
#include "gt-games-view.h"
#include "gt-channels-view.h"

#define GT_WIN_TOPLEVEL(w) GT_WIN(gtk_widget_get_toplevel(GTK_WIDGET(w)))

G_BEGIN_DECLS

#define GT_TYPE_WIN (gt_win_get_type())

G_DECLARE_FINAL_TYPE(GtWin, gt_win, GT, WIN, GtkApplicationWindow)

struct _GtWin
{
    GtkApplicationWindow parent_instance;

    GtkWidget* player;
    //   GtChannel* open_channel;
};

GtWin* gt_win_new(GtApp* app);
void gt_win_open_channel(GtWin* self, GtChannel* chan);
void gt_win_browse_view(GtWin* self);
void gt_win_browse_channels_view(GtWin* self);
void gt_win_browse_games_view(GtWin* self);
void gt_win_start_search(GtWin* self);
void gt_win_stop_search(GtWin* self);
void gt_win_refresh_view(GtWin* self);
void gt_win_show_favourites(GtWin* self);
GtGamesView* gt_win_get_games_view(GtWin* self);
GtChannelsView* gt_win_get_channels_view(GtWin* self);
gboolean gt_win_get_fullscreen(GtWin* self);
void gt_win_show_info_message(GtWin* self, const gchar* msg);

G_END_DECLS

#endif
