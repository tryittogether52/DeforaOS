/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Browser */
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



#ifndef BROWSER_DESKTOP_H
# define BROWSER_DESKTOP_H

# include <gtk/gtk.h>
# include <Desktop.h>
# include "desktopicon.h"


/* Desktop */
/* types */
# ifndef DesktopIcon
#  define DesktopIcon DesktopIcon
typedef struct _DesktopIcon DesktopIcon;
# endif

# ifndef Desktop
#  define Desktop Desktop
typedef struct _Desktop Desktop;
# endif

typedef enum _DesktopAlignment
{
	DESKTOP_ALIGNMENT_VERTICAL = 0,
	DESKTOP_ALIGNMENT_HORIZONTAL
} DesktopAlignment;

typedef enum _DesktopLayout
{
	DESKTOP_LAYOUT_NONE = 0,
	DESKTOP_LAYOUT_APPLICATIONS,
	DESKTOP_LAYOUT_CATEGORIES,
	DESKTOP_LAYOUT_FILES,
	DESKTOP_LAYOUT_HOMESCREEN
} DesktopLayout;
# define DESKTOP_LAYOUT_LAST DESKTOP_LAYOUT_HOMESCREEN
# define DESKTOP_LAYOUT_COUNT (DESKTOP_LAYOUT_LAST + 1)

typedef enum _DesktopMessage
{
	DESKTOP_MESSAGE_SET_ALIGNMENT = 0,
	DESKTOP_MESSAGE_SET_LAYOUT,
	DESKTOP_MESSAGE_SHOW
} DesktopMessage;

typedef enum _DesktopShow
{
	DESKTOP_SHOW_SETTINGS = 0
} DesktopMessageShow;

typedef struct _DesktopPrefs
{
	DesktopAlignment alignment;
	DesktopLayout layout;
	int monitor;
} DesktopPrefs;


/* constants */
# define DESKTOP_CLIENT_MESSAGE	"DEFORAOS_DESKTOP_DESKTOP_CLIENT"


/* functions */
Desktop * desktop_new(DesktopPrefs * prefs);
void desktop_delete(Desktop * desktop);

/* accessors */
/* XXX most of these accessors expose internal structures somehow */
int desktop_get_drag_data(Desktop * desktop, GtkSelectionData * seldata);
GdkPixbuf * desktop_get_file(Desktop * desktop);
GdkPixbuf * desktop_get_folder(Desktop * desktop);
Mime * desktop_get_mime(Desktop * desktop);
GtkIconTheme * desktop_get_theme(Desktop * desktop);

void desktop_set_alignment(Desktop * desktop, DesktopAlignment alignment);
void desktop_set_layout(Desktop * desktop, DesktopLayout layout);

/* useful */
int desktop_error(Desktop * desktop, char const * message, int ret);

void desktop_refresh(Desktop * desktop);

void desktop_icon_add(Desktop * desktop, DesktopIcon * icon);
void desktop_icon_remove(Desktop * desktop, DesktopIcon * icon);

void desktop_icons_align(Desktop * desktop);
void desktop_icons_sort(Desktop * desktop);

void desktop_select_all(Desktop * desktop);
void desktop_select_above(Desktop * desktop, DesktopIcon * icon);
void desktop_select_under(Desktop * desktop, DesktopIcon * icon);
void desktop_unselect_all(Desktop * desktop);

#endif /* !BROWSER_DESKTOP_H */
