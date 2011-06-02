/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Panel */
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



#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <libintl.h>
#include <gdk/gdkx.h>
#include <X11/Xatom.h>
#include "Panel.h"


/* Close */
/* private */
/* types */
typedef struct _Close
{
	PanelAppletHelper * helper;
	GtkWidget * widget;

	GdkDisplay * display;
	GdkScreen * screen;
	GdkWindow * root;

	Atom atom_active;
	Atom atom_close;
	Window window;
} Close;


/* prototypes */
/* close */
static GtkWidget * _close_init(PanelApplet * applet);
static void _close_destroy(PanelApplet * applet);

/* useful */
static void _close_do(Close * close);

/* callbacks */
static void _on_close(gpointer data);
static GdkFilterReturn _on_filter(GdkXEvent * xevent, GdkEvent * event,
		gpointer data);
static void _on_screen_changed(GtkWidget * widget, GdkScreen * previous,
		gpointer data);


/* public */
/* variables */
PanelApplet applet =
{
	NULL,
	"Close",
	NULL,
	_close_init,
	_close_destroy,
	NULL,
	PANEL_APPLET_POSITION_END,
	FALSE,
	TRUE,
	NULL
};


/* private */
/* functions */
/* close_init */
static GtkWidget * _close_init(PanelApplet * applet)
{
	Close * close;
	GtkWidget * image;

	if((close = malloc(sizeof(*close))) == NULL)
		return NULL;
	applet->priv = close;
	close->helper = applet->helper;
	close->widget = gtk_button_new();
	gtk_button_set_relief(GTK_BUTTON(close->widget), GTK_RELIEF_NONE);
	image = gtk_image_new_from_stock(GTK_STOCK_CLOSE, GTK_ICON_SIZE_BUTTON);
	gtk_button_set_image(GTK_BUTTON(close->widget), image);
	g_signal_connect_swapped(G_OBJECT(close->widget), "clicked", G_CALLBACK(
				_on_close), close);
	g_signal_connect(G_OBJECT(close->widget), "screen-changed", G_CALLBACK(
				_on_screen_changed), close);
	close->display = NULL;
	close->screen = NULL;
	close->root = NULL;
	close->atom_active = 0;
	close->atom_close = 0;
	close->window = None;
	gtk_widget_show(close->widget);
	return close->widget;
}


/* close_destroy */
static void _close_destroy(PanelApplet * applet)
{
	Close * close = applet->priv;

	free(close);
}


/* accessors */
/* close_get_window_property */
static int _close_get_window_property(Close * close, Window window,
		Atom property, Atom atom, unsigned long * cnt,
		unsigned char ** ret)
{
	int res;
	Atom type;
	int format;
	unsigned long bytes;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(close, window, %lu, %lu)\n", __func__,
			property, atom);
#endif
	gdk_error_trap_push();
	res = XGetWindowProperty(GDK_DISPLAY_XDISPLAY(close->display), window,
			property, 0, G_MAXLONG, False, atom, &type, &format,
			cnt, &bytes, ret);
	if(gdk_error_trap_pop() != 0 || res != Success)
		return 1;
	if(type != atom)
	{
		if(*ret != NULL)
			XFree(*ret);
		*ret = NULL;
		return 1;
	}
	return 0;
}


/* close_do */
static void _close_do(Close * close)
{
	unsigned long cnt = 0;
	Window * window;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(_close_get_window_property(close, GDK_WINDOW_XWINDOW(close->root),
				close->atom_active, XA_WINDOW, &cnt,
				(void*)&window) != 0 || cnt != 1)
		return;
	close->window = *window;
	XFree(window);
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() %u\n", __func__, close->window);
#endif
}


/* callbacks */
/* on_close */
static void _on_close(gpointer data)
{
	Close * close = data;
	GdkDisplay * display;
	XEvent xev;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() %u\n", __func__, close->window);
#endif
	if(close->window == None)
		return;
	display = close->display;
	memset(&xev, 0, sizeof(xev));
	xev.xclient.type = ClientMessage;
	xev.xclient.window = close->window;
	xev.xclient.message_type = close->atom_close;
	xev.xclient.format = 32;
	xev.xclient.data.l[0] = gdk_x11_display_get_user_time(display);
	xev.xclient.data.l[1] = 2;
	gdk_error_trap_push();
	XSendEvent(GDK_DISPLAY_XDISPLAY(display),
			GDK_WINDOW_XWINDOW(close->root), False,
			SubstructureNotifyMask | SubstructureRedirectMask,
			&xev);
	gdk_error_trap_pop();
}


/* on_filter */
static GdkFilterReturn _on_filter(GdkXEvent * xevent, GdkEvent * event,
		gpointer data)
{
	Close * close = data;
	XEvent * xev = xevent;

	if(xev->type != PropertyNotify)
		return GDK_FILTER_CONTINUE;
	if(xev->xproperty.atom != close->atom_active)
		return GDK_FILTER_CONTINUE;
	_close_do(close);
	return GDK_FILTER_CONTINUE;
}


/* on_screen_changed */
static void _on_screen_changed(GtkWidget * widget, GdkScreen * previous,
		gpointer data)
{
	Close * close = data;
	GdkEventMask events;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	close->screen = gtk_widget_get_screen(widget);
	close->display = gdk_screen_get_display(close->screen);
	close->root = gdk_screen_get_root_window(close->screen);
	events = gdk_window_get_events(close->root);
	gdk_window_set_events(close->root, events
			| GDK_PROPERTY_CHANGE_MASK);
	gdk_window_add_filter(close->root, _on_filter, close);
	close->atom_active = gdk_x11_get_xatom_by_name_for_display(
			close->display, "_NET_ACTIVE_WINDOW");
	close->atom_close = gdk_x11_get_xatom_by_name_for_display(
			close->display, "_NET_CLOSE_WINDOW");
	_close_do(close);
}