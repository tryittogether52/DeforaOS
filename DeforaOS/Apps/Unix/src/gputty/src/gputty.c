/* gputty.c */
/* Copyright (C) 2004 Pierre Pronchery */
/* This file is part of GPuTTY. */
/* GPuTTY is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GPuTTY is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GPuTTY; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */



#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <err.h>
#include <gtk/gtk.h>
#include "gputty.h"


/* GPuTTY */
/* callbacks */
static void gputty_on_about(GtkWidget * widget, gpointer data);
static void gputty_on_about_close(GtkWidget * widget, gpointer data);
static void gputty_on_about_closex(GtkWidget * widget, GdkEvent * event, gpointer data);
static void gputty_on_connect(GtkWidget * widget, gpointer data);
static void gputty_on_load(GtkWidget * widget, gpointer data);
static void gputty_on_quit(GtkWidget * widget, gpointer data);
static void gputty_on_quitx(GtkWidget * widget, GdkEvent * event, gpointer data);
static void gputty_on_save(GtkWidget * widget, gpointer data);
static void gputty_on_select(GtkWidget * widget, gint row, gint column, GdkEventButton *event, gpointer data);
static void gputty_on_unselect(GtkWidget * widget, gint row, gint column, GdkEventButton *event, gpointer data);
GPuTTY * gputty_new(void)
{
	GPuTTY * g;
	char * p;

	if((g = malloc(sizeof(GPuTTY))) == NULL)
	{
		perror("malloc");
		return NULL;
	}

	/* Config */
	if((g->config = config_new()) == NULL)
	{
		free(g);
		return NULL;
	}
	config_set(g->config, "", "ssh", SSH);
	config_set(g->config, "", "xterm", XTERM);
	config_load(g->config, "/etc/gputty");
	config_load(g->config, ".gputty");

	g->selection = -1;

	/* widgets */
	g->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(g->window), "GPuTTY");
	gtk_container_set_border_width(GTK_CONTAINER(g->window), 4);
	g_signal_connect(G_OBJECT(g->window), "delete_event",
			G_CALLBACK(gputty_on_quitx), g);
	g->vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(g->window), g->vbox);
	/* hostname */
	g->hn_frame = gtk_frame_new("Specify your connection");
	gtk_box_pack_start(GTK_BOX(g->vbox), g->hn_frame, FALSE, FALSE, 0);
	g->hn_vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(g->hn_vbox), 4);
	gtk_container_add(GTK_CONTAINER(g->hn_frame), g->hn_vbox);
	g->hn_hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(g->hn_vbox), g->hn_hbox, TRUE, TRUE, 0);
	/* hostname: hostname */
	g->hn_vbox1 = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(g->hn_hbox), g->hn_vbox1, TRUE, TRUE, 0);
	g->hn_lhostname = gtk_label_new("Host name");
	gtk_box_pack_start(GTK_BOX(g->hn_vbox1), g->hn_lhostname, TRUE, TRUE, 0);
	g->hn_ehostname = gtk_entry_new();
	if((p = config_get(g->config, "", "hostname")) != NULL)
		gtk_entry_set_text(GTK_ENTRY(g->hn_ehostname), p);
	gtk_box_pack_start(GTK_BOX(g->hn_vbox1), g->hn_ehostname, TRUE, TRUE, 0);
	/* hostname: port */
	g->hn_vbox2 = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(g->hn_hbox), g->hn_vbox2, TRUE, TRUE, 4);
	g->hn_lport = gtk_label_new("Port");
	gtk_box_pack_start(GTK_BOX(g->hn_vbox2), g->hn_lport, TRUE, TRUE, 0);
	g->hn_sport_adj = (GtkAdjustment *)gtk_adjustment_new(22, 0, 65535, 1, 4, 4);
	g->hn_sport = gtk_spin_button_new(g->hn_sport_adj, 1, 0);
	gtk_box_pack_start(GTK_BOX(g->hn_vbox2), g->hn_sport, TRUE, TRUE, 0);
	/* hostname: username */
	g->hn_vbox3 = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(g->hn_hbox), g->hn_vbox3, TRUE, TRUE, 0);
	g->hn_lusername = gtk_label_new("User name");
	gtk_box_pack_start(GTK_BOX(g->hn_vbox3), g->hn_lusername, TRUE, TRUE, 0);
	g->hn_eusername = gtk_entry_new();
	if((p = config_get(g->config, "", "username")) != NULL)
		gtk_entry_set_text(GTK_ENTRY(g->hn_eusername), p);
	gtk_box_pack_start(GTK_BOX(g->hn_vbox3), g->hn_eusername, TRUE, TRUE, 0);
	/* sessions */
	g->sn_frame = gtk_frame_new("Manage sessions");
	g->sn_vbox1 = gtk_vbox_new(FALSE, 0);
	g->sn_hbox = gtk_hbox_new(FALSE, 0);
	/* sessions: list */
	g->sn_vbox2 = gtk_vbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(g->sn_vbox2), 4);
	g->sn_esessions = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(g->sn_vbox2), g->sn_esessions, FALSE, FALSE, 0);
	g->sn_swsessions = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(g->sn_swsessions), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	g->sn_clsessions = gtk_clist_new(1);
	gtk_clist_set_selection_mode(GTK_CLIST(g->sn_clsessions),
			GTK_SELECTION_SINGLE);
	g_signal_connect(G_OBJECT(g->sn_clsessions), "select-row",
			G_CALLBACK(gputty_on_select), g);
	g_signal_connect(G_OBJECT(g->sn_clsessions), "unselect-row",
			G_CALLBACK(gputty_on_unselect), g);
	gtk_container_add(GTK_CONTAINER(g->sn_swsessions), g->sn_clsessions);
	gtk_box_pack_start(GTK_BOX(g->sn_vbox2), g->sn_swsessions, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(g->sn_hbox), g->sn_vbox2, TRUE, TRUE, 0);
	/* sessions: buttons */
	g->sn_vbox3 = gtk_vbox_new(FALSE, 0);
	g->sn_load = gtk_button_new_from_stock(GTK_STOCK_OPEN);
	g_signal_connect(G_OBJECT(g->sn_load), "clicked",
			G_CALLBACK(gputty_on_load), g);
	gtk_box_pack_start(GTK_BOX(g->sn_vbox3), g->sn_load, FALSE, FALSE, 4);
	g->sn_save = gtk_button_new_from_stock(GTK_STOCK_SAVE);
	g_signal_connect(G_OBJECT(g->sn_save), "clicked",
			G_CALLBACK(gputty_on_save), g);
	gtk_box_pack_start(GTK_BOX(g->sn_vbox3), g->sn_save, FALSE, FALSE, 0);
	g->sn_delete = gtk_button_new_from_stock(GTK_STOCK_DELETE);
	gtk_box_pack_start(GTK_BOX(g->sn_vbox3), g->sn_delete, FALSE, FALSE, 4);
	gtk_box_pack_start(GTK_BOX(g->sn_hbox), g->sn_vbox3, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(g->sn_vbox1), g->sn_hbox, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(g->sn_frame), g->sn_vbox1);
	gtk_box_pack_start(GTK_BOX(g->vbox), g->sn_frame, TRUE, TRUE, 4);
	/* actions */
	g->ac_hbox = gtk_hbox_new(FALSE, 0);
	g->ac_about = gtk_button_new_with_label("About");
	gtk_box_pack_start(GTK_BOX(g->ac_hbox), g->ac_about, FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT(g->ac_about), "clicked",
			G_CALLBACK(gputty_on_about), g);
	g->ac_connect = gtk_button_new_with_label("Connect");
	g_signal_connect(G_OBJECT(g->ac_connect), "clicked",
			G_CALLBACK(gputty_on_connect), g);
	gtk_box_pack_end(GTK_BOX(g->ac_hbox), g->ac_connect, FALSE, FALSE, 0);
	g->ac_quit = gtk_button_new_from_stock(GTK_STOCK_QUIT);
	gtk_box_pack_end(GTK_BOX(g->ac_hbox), g->ac_quit, FALSE, FALSE, 4);
	g_signal_connect(G_OBJECT(g->ac_quit), "clicked",
			G_CALLBACK(gputty_on_quit), g);
	gtk_box_pack_start(GTK_BOX(g->vbox), g->ac_hbox, FALSE, FALSE, 0);
	/* about */
	g->ab_window = NULL;
	gtk_widget_show_all(g->window);
	return g;
}


void gputty_delete(GPuTTY * gputty)
{
	config_delete(gputty->config);
	free(gputty);
}


/* callbacks */
static void gputty_on_about(GtkWidget * widget, gpointer data)
{
	GPuTTY * g;

	g = data;
	if(g->ab_window == NULL)
	{
		g->ab_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_container_set_border_width(GTK_CONTAINER(g->ab_window), 4);
		gtk_window_set_title(GTK_WINDOW(g->ab_window), "About GPuTTY");
		g_signal_connect(G_OBJECT(g->ab_window), "delete_event",
				G_CALLBACK(gputty_on_about_closex), g);
		g->ab_vbox = gtk_vbox_new(FALSE, 0);
		gtk_container_add(GTK_CONTAINER(g->ab_window), g->ab_vbox);
		g->ab_label = gtk_label_new("GPuTTY is a clone of PuTTY for Open Source desktops.\n\
Project homepage is found at:\n\
http://www.defora.org/index.php?page=gputty\n\
This software has been written by:\n\
- Pierre Pronchery <khorben@defora.org>\n\
and mainly relies on the following software:\n\
- Glib\n\
- Gtk+\n\
This project is released under the terms of the\n\
GNU General Public License.\n\
Credits go to all Free Software contributors.");
		gtk_box_pack_start(GTK_BOX(g->ab_vbox), g->ab_label, TRUE, TRUE, 0);
		g->ab_hsep = gtk_hseparator_new();
		gtk_box_pack_start(GTK_BOX(g->ab_vbox), g->ab_hsep, TRUE, TRUE, 4);
		g->ab_close = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
		g_signal_connect(G_OBJECT(g->ab_close), "clicked",
				G_CALLBACK(gputty_on_about_close), g);
		gtk_box_pack_start(GTK_BOX(g->ab_vbox), g->ab_close, FALSE, FALSE, 0);
	}
	gtk_widget_show_all(g->ab_window);
}

static void gputty_on_about_close(GtkWidget * widget, gpointer data)
{
	GPuTTY * g;

	g = data;
	gtk_widget_hide(g->ab_window);
}

static void gputty_on_about_closex(GtkWidget * widget, GdkEvent * event, gpointer data)
{
	gputty_on_about_close(widget, data);
}

static void gputty_on_connect(GtkWidget * widget, gpointer data)
{
	GPuTTY * g = data;
	pid_t pid;
	char * xterm = NULL;
	char * ssh = NULL;
	char const * hostname;
	char port[6];
	char * useropt = NULL;
	char const * username = NULL;

	xterm = config_get(g->config, "", "xterm");
	ssh = config_get(g->config, "", "ssh");
	hostname = gtk_entry_get_text(GTK_ENTRY(g->hn_ehostname));
	if(snprintf(port, 6, "%d", gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(g->hn_sport))) >= 6)
		port[5] = '\0';
	if(hostname[0] == '\0')
		return;
	if((pid = fork()) == -1)
	{
		warn("fork");
		return;
	}
	else if(pid == 0)
	{
		username = gtk_entry_get_text(GTK_ENTRY(g->hn_eusername));
		if(username[0] != '\0')
			useropt = "-l";
		execlp(xterm, xterm, "-e",
				ssh, hostname,
				"-p", port,
				useropt, username,
				NULL);
		err(2, xterm);
	}
}

static void gputty_on_load(GtkWidget * widget, gpointer data)
{
	/* FIXME */
	GPuTTY * g;

	g = data;
}

static void gputty_on_quit(GtkWidget * widget, gpointer data)
{
	GPuTTY * g = data;

	config_set(g->config, "", "hostname", (void*)gtk_entry_get_text(GTK_ENTRY(g->hn_ehostname)));
	config_set(g->config, "", "username", (void*)gtk_entry_get_text(GTK_ENTRY(g->hn_eusername)));
	if(g->config == NULL)
		fprintf(stderr, "gputty: not saving configuration\n");
	else if(config_save(g->config, ".gputty") != 0)
		fprintf(stderr, "gputty: an error occured while saving configuration\n");
	gtk_main_quit();
}

static void gputty_on_quitx(GtkWidget * widget, GdkEvent * event, gpointer data)
{
	gputty_on_quit(widget, data);
}

static void gputty_on_save(GtkWidget * widget, gpointer data)
{
	/* FIXME
	 * - remember saved sessions
	 * - when 2 sessions have the same name, update it */
	GPuTTY * g;
	char const * session;
	char const * hostname;
	int port;
	char const * username;

	g = data;
	session = gtk_entry_get_text(GTK_ENTRY(g->sn_esessions));
	if(session[0] == '\0')
		return;
	hostname = gtk_entry_get_text(GTK_ENTRY(g->hn_ehostname));
	if(hostname[0] == '\0')
		return;
	username = gtk_entry_get_text(GTK_ENTRY(g->hn_eusername));
	port = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(g->hn_sport));
	gtk_clist_append(GTK_CLIST(g->sn_clsessions), (char**)&session);
}

static void gputty_on_select(GtkWidget * widget, gint row, gint column, GdkEventButton *event, gpointer data)
{
	GPuTTY * g = data;

	g->selection = row;
}

static void gputty_on_unselect(GtkWidget * widget, gint row, gint column, GdkEventButton *event, gpointer data)
{
	GPuTTY * g = data;

	g->selection = -1;
}

/* main */
int main(int argc, char * argv[])
{
	GPuTTY * gputty;

	gtk_init(&argc, &argv);
	if((gputty = gputty_new()) == NULL)
		return 1;
	gtk_main();
	gputty_delete(gputty);
	return 0;
}
