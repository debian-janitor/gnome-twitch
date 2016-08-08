#ifndef GT_PLAYER_H
#define GT_PLAYER_H

#include <gtk/gtk.h>
#include "gt-channel.h"

G_BEGIN_DECLS

#define GT_TYPE_PLAYER (gt_player_get_type())

G_DECLARE_DERIVABLE_TYPE(GtPlayer, gt_player, GT, PLAYER, GtkBin)

struct _GtPlayerClass
{
    GtkBinClass parent_class;

    void (*set_uri) (GtPlayer* self, const gchar* uri);
    void (*play) (GtPlayer* self);
    void (*stop) (GtPlayer* self);
};

void gt_player_open_channel(GtPlayer* self, GtChannel* chan);
void gt_player_close_channel(GtPlayer* self);
void gt_player_play(GtPlayer* self);
void gt_player_stop(GtPlayer* self);
void gt_player_set_quality(GtPlayer* self, GtTwitchStreamQuality q);
void gt_player_toggle_muted(GtPlayer* self);

G_END_DECLS

#endif
