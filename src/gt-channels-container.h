#ifndef GT_CHANNELS_CONTAINER_H
#define GT_CHANNELS_CONTAINER_H

#include <gtk/gtk.h>
#include "gt-channels-container-child.h"

G_BEGIN_DECLS

#define GT_TYPE_CHANNELS_CONTAINER (gt_channels_container_get_type())

G_DECLARE_DERIVABLE_TYPE(GtChannelsContainer, gt_channels_container, GT, CHANNELS_CONTAINER, GtkStack)

typedef enum
{
    GT_CHANNELS_CONTAINER_TYPE_TOP,
    GT_CHANNELS_CONTAINER_TYPE_FAVOURITE,
    GT_CHANNELS_CONTAINER_TYPE_SEARCH,
    GT_CHANNELS_CONTAINER_TYPE_GAME
} GtChannelsContainerType;

struct _GtChannelsContainerClass
{
    GtkStackClass parent_class;

    void (*check_empty) (GtChannelsContainer* self);
    void (*set_empty_info) (GtChannelsContainer* self, const gchar* icon_name, const gchar* title, const gchar* subtitle);
    void (*set_loading_info) (GtChannelsContainer* self, const gchar* title);
    void (*show_load_spinner) (GtChannelsContainer* self, gboolean show);
    void (*append_channel) (GtChannelsContainer* self, GtChannel* chan);
    void (*append_channels) (GtChannelsContainer* self, GList* channels);
    void (*remove_channel) (GtChannelsContainer* self, GtChannel* chan);
    void (*clear_channels) (GtChannelsContainer* self);
    GtkFlowBox* (*get_channels_flow) (GtChannelsContainer* self);

    void (*bottom_edge_reached) (GtChannelsContainer* self);
    void (*refresh) (GtChannelsContainer* self);
    void (*filter) (GtChannelsContainer* self, const gchar* query);
};

GtChannelsContainer* gt_channels_container_new();

void gt_channels_container_refresh(GtChannelsContainer* self);
void gt_channels_container_set_filter_query(GtChannelsContainer* self, const gchar* query);

G_END_DECLS

#endif
