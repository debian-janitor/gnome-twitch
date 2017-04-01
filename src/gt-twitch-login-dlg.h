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

#ifndef GT_TWITCH_LOGIN_DLG_H
#define GT_TWITCH_LOGIN_DLG_H

#include <gtk/gtk.h>
#include "gt-win.h"

G_BEGIN_DECLS

#define GT_TYPE_TWITCH_LOGIN_DLG gt_twitch_login_dlg_get_type()

G_DECLARE_FINAL_TYPE(GtTwitchLoginDlg, gt_twitch_login_dlg, GT, TWITCH_LOGIN_DLG, GtkDialog)

struct _GtTwitchLoginDlg
{
    GtkDialog parent_instance;
};

GtTwitchLoginDlg* gt_twitch_login_dlg_new(GtkWindow* win);

G_END_DECLS

#endif
