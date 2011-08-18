/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Phone */
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
/* This code is inspired by existing work by Sebastian Reichel <sre@debian.org>,
 * see https://elektranox.org/n900/libisi */
/* TODO:
 * - test on actual hardware */



#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <gtk/gtk.h>
#include <System.h>
#include "Phone.h"


/* N900 */
/* prototypes */
/* plug-in */
static int _n900_event(PhonePlugin * plugin, PhoneEvent * event);


/* public */
/* variables */
PhonePlugin plugin =
{
	NULL,
	"Nokia N900",
	NULL,
	NULL,
	NULL,
	_n900_event,
	NULL,
	NULL
};


/* private */
/* n900_event */
static int _event_power_on(PhonePlugin * plugin, gboolean power);

static int _n900_event(PhonePlugin * plugin, PhoneEvent * event)
{
	switch(event->type)
	{
		case PHONE_EVENT_TYPE_OFFLINE:
			_event_power_on(plugin, FALSE);
			break;
		case PHONE_EVENT_TYPE_ONLINE:
			_event_power_on(plugin, TRUE);
			break;
		default:
			break;
	}
	return 0;
}

static int _event_power_on(PhonePlugin * plugin, gboolean power)
{
	int ret = 0;
	char const root[] = "/sys/devices/platform/gpio-switch";
	struct
	{
		char const * name;
		int enable;
		int disable;
		unsigned int delay;
	} commands[] = {
		{ "cmt_apeslpx",	0,	0,	0	},
		{ "cmt_rst_rq",		0,	0,	0	},
		{ "cmt_bsi",		0,	-1,	0	},
		{ "cmt_rst",		0,	0,	0	},
		{ "cmt_en",		1,	0,	0	},
		{ "cmt_rst",		1,	1,	0	},
		{ "cmt_rst_eq",		1,	-1,	0	},
		{ "cmt_en",		1,	-1,	5	},
	};
	size_t i;
	int fd;
	char path[256];
	char buf[256];
	int len;

	for(i = 0; i < sizeof(commands) / sizeof(*commands) && ret == 0; i++)
	{
		if((power ? commands[i].enable : commands[i].disable) < 0)
			continue;
		if(commands[i].delay > 0)
			/* FIXME freezes the application for as long */
			sleep(commands[i].delay);
		snprintf(path, sizeof(path), "%s/%s/%s", root, commands[i].name,
				"state");
		if((fd = open(path, O_WRONLY)) < 0)
		{
			snprintf(buf, sizeof(buf), "%s: %s", path, strerror(
						errno));
			ret = plugin->helper->error(NULL, buf, 1);
			break;
		}
		len = snprintf(buf, sizeof(buf), "%s", power ? "active"
				: "inactive");
		if(write(fd, buf, len) != len)
		{
			snprintf(buf, sizeof(buf), "%s: %s", path,
					strerror(errno));
			ret = plugin->helper->error(NULL, buf, 1);
		}
		close(fd);
	}
	return ret;
}
