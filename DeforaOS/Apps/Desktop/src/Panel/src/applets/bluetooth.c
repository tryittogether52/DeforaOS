/* $Id$ */
/* Copyright (c) 2010-2012 Pierre Pronchery <khorben@defora.org> */
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



#if defined(__NetBSD__)
# include <sys/types.h>
# include <sys/ioctl.h>
# include <bluetooth.h>
# include <unistd.h>
# include <stdio.h>
# include <string.h>
# include <errno.h>
#elif defined(__linux__)
# include <fcntl.h>
# include <unistd.h>
# include <string.h>
# include <errno.h>
#endif
#include <stdlib.h>
#include <libintl.h>
#include <System.h>
#include "Panel.h"
#define _(string) gettext(string)


/* Bluetooth */
/* private */
/* types */
typedef struct _PanelApplet
{
	PanelAppletHelper * helper;
	GtkWidget * image;
	guint timeout;
#if defined(__NetBSD__) || defined(__linux__)
	int fd;
#endif
} Bluetooth;


/* prototypes */
static Bluetooth * _bluetooth_init(PanelAppletHelper * helper,
		GtkWidget ** widget);
static void _bluetooth_destroy(Bluetooth * bluetooth);

static gboolean _bluetooth_get(Bluetooth * bluetooth);
static void _bluetooth_set(Bluetooth * bluetooth, gboolean on);

/* callbacks */
static gboolean _on_timeout(gpointer data);


/* public */
/* variables */
PanelAppletDefinition applet =
{
	"Bluetooth",
	"panel-applet-bluetooth",
	NULL,
	_bluetooth_init,
	_bluetooth_destroy,
	NULL,
	FALSE,
	TRUE
};


/* private */
/* functions */
/* bluetooth_init */
static Bluetooth * _bluetooth_init(PanelAppletHelper * helper,
		GtkWidget ** widget)
{
	Bluetooth * bluetooth;

	if((bluetooth = malloc(sizeof(*bluetooth))) == NULL)
		return NULL;
	bluetooth->helper = helper;
	bluetooth->timeout = 0;
#if defined(__NetBSD__) || defined(__linux__)
	bluetooth->fd = -1;
#endif
	bluetooth->image = gtk_image_new_from_icon_name(
			"panel-applet-bluetooth", helper->icon_size);
#if GTK_CHECK_VERSION(2, 12, 0)
	gtk_widget_set_tooltip_text(bluetooth->image,
			_("Bluetooth is enabled"));
#endif
	bluetooth->timeout = g_timeout_add(1000, _on_timeout, bluetooth);
	_on_timeout(bluetooth);
	*widget = bluetooth->image;
	return bluetooth;
}


/* bluetooth_destroy */
static void _bluetooth_destroy(Bluetooth * bluetooth)
{
	if(bluetooth->timeout > 0)
		g_source_remove(bluetooth->timeout);
#if defined(__NetBSD__) || defined(__linux__)
	if(bluetooth->fd >= 0)
		close(bluetooth->fd);
#endif
	free(bluetooth);
}


/* bluetooth_set */
static void _bluetooth_set(Bluetooth * bluetooth, gboolean on)
{
	if(on == TRUE)
		gtk_widget_show(bluetooth->image);
	else
		gtk_widget_hide(bluetooth->image);
}


/* callbacks */
/* on_timeout */
#if defined(__NetBSD__)
static gboolean _bluetooth_get(Bluetooth * bluetooth)
{
	struct btreq btr;
	const char name[] = "ubt0";

	if(bluetooth->fd < 0 && (bluetooth->fd = socket(AF_BLUETOOTH,
					SOCK_RAW, BTPROTO_HCI)) < 0)
	{
		error_set("%s: %s", "socket", strerror(errno));
		return FALSE;
	}
	memset(&btr, 0, sizeof(btr));
	strncpy(btr.btr_name, name, sizeof(name));
	if(ioctl(bluetooth->fd, SIOCGBTINFO, &btr) == -1)
	{
		error_set("%s: %s", name, strerror(errno));
		close(bluetooth->fd);
		bluetooth->fd = -1;
		return FALSE;
	}
	/* XXX should not be necessary but EBADF happens once otherwise */
	close(bluetooth->fd);
	bluetooth->fd = -1;
	return TRUE;
}
#elif defined(__linux__)
static gboolean _bluetooth_get(Bluetooth * bluetooth)
{
	/* XXX currently hard-coded for the Openmoko Freerunner */
	char const dv1[] = "/sys/bus/platform/devices/gta02-pm-bt.0/power_on";
	char const dv2[] = "/sys/bus/platform/devices/neo1973-pm-bt.0/power_on";
	char const * dev = dv1;
	char on;

	if(bluetooth->fd < 0)
	{
		if((bluetooth->fd = open(dev, O_RDONLY)) < 0)
		{
			dev = dv2;
			bluetooth->fd = open(dev, O_RDONLY);
		}
		if(bluetooth->fd < 0)
		{
			error_set("%s: %s", dev, strerror(errno));
			return FALSE;
		}
	}
	errno = ENODATA; /* in case the pseudo-file is empty */
	if(lseek(bluetooth->fd, 0, SEEK_SET) != 0
			|| read(bluetooth->fd, &on, sizeof(on)) != 1)
	{
		error_set("%s: %s", dev, strerror(errno));
		close(bluetooth->fd);
		bluetooth->fd = -1;
		return FALSE;
	}
	return (on == '1') ? TRUE : FALSE;
}
#else
static gboolean _bluetooth_get(Bluetooth * bluetooth)
{
	/* FIXME not supported */
	return FALSE;
}
#endif


/* callbacks */
/* on_timeout */
static gboolean _on_timeout(gpointer data)
{
	Bluetooth * bluetooth = data;

	_bluetooth_set(bluetooth, _bluetooth_get(bluetooth));
	return TRUE;
}
