/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Player */
/* This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. */



#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <errno.h>
#include <libintl.h>
#include "player.h"
#include "callbacks.h"
#define _(string) gettext(string)


/* macros */
#define max(a, b) (((a) > (b)) ? (a) : (b))


/* functions */
/* callbacks */
/* on_player_closex */
gboolean on_player_closex(gpointer data)
{
	Player * player = data;

	player_stop(player);
	gtk_main_quit();
	return TRUE;
}


/* on_player_removed */
void on_player_removed(gpointer data)
{
	/* FIXME implement */
}


/* on_file_open */
void on_file_open(gpointer data)
{
	Player * player = data;

	player_open_dialog(player);
}


/* on_file_properties */
void on_file_properties(gpointer data)
{
	Player * player = data;
	GtkWidget * window;
	char * p;
	char buf[256];

	if(player->filename == NULL)
		return;
	if((p = strdup(player->filename)) == NULL)
	{
		player_error(player, strerror(errno), 0);
		return;
	}
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(window), "delete-event", G_CALLBACK(
				gtk_widget_destroy), NULL);
	snprintf(buf, sizeof(buf), "%s%s", _("Properties of "), basename(p));
	free(p);
	gtk_window_set_title(GTK_WINDOW(window), buf);
	/* FIXME implement */
	gtk_widget_show_all(window);
}


/* on_file_close */
void on_file_close(gpointer data)
{
	Player * player = data;

	on_player_closex(player);
}


/* on_edit_preferences */
void on_edit_preferences(gpointer data)
{
	Player * player = data;

	player_view_preferences(player);
}


/* view menu */
/* on_view_fullscreen */
void on_view_fullscreen(gpointer data)
{
	Player * player = data;

	player_set_fullscreen(player, !player_get_fullscreen(player));
}


/* on_view_playlist */
void on_view_playlist(gpointer data)
{
	Player * player = data;

	gtk_widget_show(player->pl_window);
}


/* on_help_about */
void on_help_about(gpointer data)
{
	Player * player = data;

	player_about(player);
}


/* toolbar */
void on_previous(gpointer data)
{
	Player * player = data;

	player_previous(player);
}


void on_rewind(gpointer data)
{
	Player * player = data;

	player_rewind(player);
}


void on_play(gpointer data)
{
	Player * player = data;

	player_play(player);
}


void on_pause(gpointer data)
{
	Player * player = data;

	player_pause(player);
}


void on_stop(gpointer data)
{
	Player * player = data;

	player_stop(player);
}


void on_forward(gpointer data)
{
	Player * player = data;

	player_forward(player);
}


void on_next(gpointer data)
{
	Player * player = data;

	player_next(player);
}


void on_fullscreen(gpointer data)
{
	Player * player = data;

	player_set_fullscreen(player, !player_get_fullscreen(player));
}


/* view */
/* playlist */
/* on_playlist_add */
void on_playlist_add(gpointer data)
{
	/* FIXME implement */
}


/* on_playlist_closex */
gboolean on_playlist_closex(gpointer data)
{
	Player * player = data;

	gtk_widget_hide(player->pl_window);
	return TRUE;
}


/* on_playlist_load */
void on_playlist_load(gpointer data)
{
	/* FIXME implement */
}


/* on_playlist_remove */
void on_playlist_remove(gpointer data)
{
	/* FIXME implement */
}


/* on_playlist_save */
void on_playlist_save(gpointer data)
{
	/* FIXME implement */
}
