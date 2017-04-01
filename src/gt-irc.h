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

#ifndef GT_IRC_H
#define GT_IRC_H

#include <gtk/gtk.h>
#include "gt-channel.h"

G_BEGIN_DECLS

#define GT_TYPE_IRC gt_irc_get_type()

G_DECLARE_FINAL_TYPE(GtIrc, gt_irc, GT, IRC, GObject);

typedef enum
{
    GT_IRC_STATE_DISCONNECTED,
    GT_IRC_STATE_CONNECTING,
    GT_IRC_STATE_CONNECTED,
    GT_IRC_STATE_LOGGED_IN,
    GT_IRC_STATE_JOINED,
} GtIrcState;

#define GT_TYPE_IRC_STATE gt_irc_state_get_type()

GType gt_irc_state_get_type();

typedef enum
{
    GT_IRC_COMMAND_NOTICE,
    GT_IRC_COMMAND_PRIVMSG,
    GT_IRC_COMMAND_PING,
    GT_IRC_COMMAND_REPLY,
    GT_IRC_COMMAND_CHANNEL_MODE,
    GT_IRC_COMMAND_CAP,
    GT_IRC_COMMAND_JOIN,
    GT_IRC_COMMAND_PART,
    GT_IRC_COMMAND_USERSTATE,
    GT_IRC_COMMAND_ROOMSTATE,
    GT_IRC_COMMAND_CLEARCHAT,
} GtIrcCommandType;

typedef enum
{
    GT_CHAT_REPLY_WELCOME    = 1,
    GT_CHAT_REPLY_YOURHOST   = 2,
    GT_CHAT_REPLY_CREATED    = 3,
    GT_CHAT_REPLY_MYINFO     = 4,
    GT_CHAT_REPLY_MOTDSTART  = 375,
    GT_CHAT_REPLY_MOTD       = 372,
    GT_CHAT_REPLY_ENDOFMOTD  = 376,
    GT_CHAT_REPLY_NAMEREPLY  = 353,
    GT_CHAT_REPLY_ENDOFNAMES = 366,
} GtChatReplyType;

typedef struct
{
    gchar* target;
    gchar* msg;
} GtIrcCommandNotice;

#define IRC_USER_MODE_GLOBAL_MOD  1
#define IRC_USER_MODE_ADMIN       1 << 2
#define IRC_USER_MODE_BROADCASTER 1 << 3
#define IRC_USER_MODE_MOD         1 << 4
#define IRC_USER_MODE_STAFF       1 << 5
#define IRC_USER_MODE_TURBO       1 << 6
#define IRC_USER_MODE_SUBSCRIBER  1 << 7

enum
{
    IRC_BADGE_SUBSCRIBER_0 = 1 << 0,
    IRC_BADGE_SUBSCRIBER_3 = 1 << 1,
    IRC_BADGE_SUBSCRIBER_6 = 1 << 2,
    IRC_BADGE_SUBSCRIBER_12 = 1 << 3,
    IRC_BADGE_SUBSCRIBER_24 = 1 << 4,
    IRC_BADGE_STAFF = 1 << 5,
    IRC_BADGE_ADMIN = 1 << 6,
    IRC_BADGE_GLOBAL_MOD = 1 << 7,
    IRC_BADGE_MOD = 1 << 8,
    IRC_BADGE_TURBO = 1 << 9,
    IRC_BADGE_PRIME = 1 << 10
};

typedef struct
{
    gchar* target;
    gchar* msg;
    gint user_modes;
    gchar* display_name;
    GList* badges;
    GList* emotes;
    gchar* colour;
} GtIrcCommandPrivmsg;

typedef struct
{
    gchar* server;
} GtIrcCommandPing;

typedef struct
{
    gchar* channel;
} GtIrcCommandJoin;

typedef struct
{
    gchar* channel;
} GtIrcCommandPart;

typedef enum
{
    GT_CHAT_CAP_SUB_COMMAND_ACK,
    GT_CHAT_CAP_SUB_COMMAND_LS,
    GT_CHAT_CAP_SUB_COMMAND_LIST,
    GT_CHAT_CAP_SUB_COMMAND_NAK,
    GT_CHAT_CAP_SUB_COMMAND_END,
    GT_CHAT_CAP_SUB_COMMAND_REQ
} GtChatCapSubCommandType;

typedef struct
{
    gchar* target;
    gchar* sub_command; //TODO: Change this to enum
    gchar* parameter;
} GtIrcCommandCap;

typedef struct
{
    GtChatReplyType type;
    gchar* reply; // Might need another union for more complex replies
} GtIrcCommandReply;

typedef struct
{
    gchar* channel;
    gchar* modes; //TODO: Turn this into bitmask of modes
    gchar* nick;
} GtIrcCommandChannelMode;

typedef struct
{
    gchar* channel;
} GtIrcCommandUserstate;

typedef struct
{
    gchar* channel;
} GtIrcCommandRoomstate;

typedef struct
{
    gchar* channel;
    gchar* target;
} GtIrcCommandClearchat;

typedef struct
{
    gchar* nick;
    gchar* user;
    gchar* host;
    GtIrcCommandType cmd_type;
    gchar** tags;
    union
    {
        GtIrcCommandNotice* notice;
        GtIrcCommandPrivmsg* privmsg;
        GtIrcCommandPing* ping;
        GtIrcCommandJoin* join;
        GtIrcCommandPart* part;
        GtIrcCommandCap* cap;
        GtIrcCommandReply* reply;
        GtIrcCommandChannelMode* chan_mode;
        GtIrcCommandUserstate* userstate;
        GtIrcCommandRoomstate* roomstate;
        GtIrcCommandClearchat* clearchat;
    } cmd;
} GtIrcMessage;

typedef gboolean (*GtTwitchChatSourceFunc) (GtIrcMessage* msg, gpointer udata);

typedef struct _GtTwitchChatSource GtTwitchChatSource;

struct _GtIrc
{
    GObject parent_instance;

    GtTwitchChatSource* source;
};

GtIrc*     gt_irc_new();
void       gt_irc_connect(GtIrc* self, const gchar* host, int port, const gchar* oauth_token, const gchar* nick);
void       gt_irc_disconnect(GtIrc* self);
void       gt_irc_join(GtIrc* self, const gchar* channel);
void       gt_irc_connect_and_join_channel(GtIrc* self, GtChannel* chan);
void       gt_irc_connect_and_join_channel_async(GtIrc* self, GtChannel* chan, GCancellable* cancel, GAsyncReadyCallback cb, gpointer udata);
void       gt_irc_part(GtIrc* self);
void       gt_irc_privmsg(GtIrc* self, const gchar* msg);
GtIrcState gt_irc_get_state(GtIrc* self);
void       gt_irc_message_free(GtIrcMessage* msg);

G_END_DECLS

#endif
