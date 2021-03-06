/* $Id$ */
/* Copyright (c) 2011-2012 Pierre Pronchery <khorben@defora.org> */
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



#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <libintl.h>
#include <System.h>
#include "Panel.h"
#include "../../config.h"
#define _(string) gettext(string)

/* XXX to avoid pointless warnings with GCC */
#define main _main


/* Main */
/* private */
/* types */
typedef struct _PanelApplet
{
	PanelAppletHelper * helper;
	GSList * apps;
	guint idle;
	time_t refresh_mti;
} Main;

typedef struct _MainMenu
{
	char const * category;
	char const * label;
	char const * stock;
} MainMenu;


/* constants */
#ifndef PREFIX
# define PREFIX "/usr/local"
#endif

static const MainMenu _main_menus[] =
{
	{ "Audio;",	"Audio",	"gnome-mime-audio",		},
	{ "Development;","Development",	"applications-development",	},
	{ "Education;",	"Education",	"applications-science",		},
	{ "Game;",	"Games",	"applications-games",		},
	{ "Graphics;",	"Graphics",	"applications-graphics",	},
	{ "AudioVideo;","Multimedia",	"applications-multimedia",	},
	{ "Network;",	"Network",	"applications-internet",	},
	{ "Office;",	"Office",	"applications-office",		},
	{ "Settings;",	"Settings",	"gnome-settings",		},
	{ "System;",	"System",	"applications-system",		},
	{ "Utility;",	"Utilities",	"applications-utilities",	},
	{ "Video;",	"Video",	"video",			},
	{ NULL,		NULL,		NULL,				}
};
#define MAIN_MENUS_COUNT (sizeof(_main_menus) / sizeof(*_main_menus))


/* prototypes */
static Main * _main_init(PanelAppletHelper * helper, GtkWidget ** widget);
static void _main_destroy(Main * main);

/* helpers */
static GtkWidget * _main_applications(Main * main);
static GtkWidget * _main_image(char const * name);
static GtkWidget * _main_menuitem(char const * label, char const * stock);

/* callbacks */
static void _on_about(gpointer data);
static void _on_clicked(gpointer data);
static gboolean _on_idle(gpointer data);
static void _on_lock(gpointer data);
static void _on_logout(gpointer data);
#ifdef EMBEDDED
static void _on_rotate(gpointer data);
#endif
static void _on_run(gpointer data);
static void _on_shutdown(gpointer data);
static void _on_suspend(gpointer data);
static gboolean _on_timeout(gpointer data);


/* public */
/* variables */
PanelAppletDefinition applet =
{
	"Main menu",
	"gnome-main-menu",
	NULL,
	_main_init,
	_main_destroy,
	NULL,
	FALSE,
	TRUE
};


/* private */
/* functions */
/* main_init */
static Main * _main_init(PanelAppletHelper * helper, GtkWidget ** widget)
{
	Main * main;
	GtkWidget * ret;
	GtkWidget * image;

	if((main = malloc(sizeof(*main))) == NULL)
		return NULL;
	main->helper = helper;
	main->apps = NULL;
	main->idle = g_idle_add(_on_idle, main);
	main->refresh_mti = 0;
	ret = gtk_button_new();
	image = gtk_image_new_from_icon_name("gnome-main-menu",
			helper->icon_size);
	gtk_button_set_image(GTK_BUTTON(ret), image);
	gtk_button_set_relief(GTK_BUTTON(ret), GTK_RELIEF_NONE);
#if GTK_CHECK_VERSION(2, 12, 0)
	gtk_widget_set_tooltip_text(ret, _("Main menu"));
#endif
	g_signal_connect_swapped(G_OBJECT(ret), "clicked", G_CALLBACK(
				_on_clicked), main);
	gtk_widget_show_all(ret);
	*widget = ret;
	return main;
}


/* main_destroy */
static void _main_destroy(Main * main)
{
	if(main->idle != 0)
		g_source_remove(main->idle);
	g_slist_foreach(main->apps, (GFunc)config_delete, NULL);
	g_slist_free(main->apps);
	free(main);
}


/* helpers */
/* main_applications */
static void _applications_on_activate(gpointer data);
static void _applications_categories(GtkWidget * menu, GtkWidget ** menus);

static GtkWidget * _main_applications(Main * main)
{
	GtkWidget * menus[MAIN_MENUS_COUNT];
	GSList * p;
	GtkWidget * menu;
	GtkWidget * menuitem;
	Config * config;
	const char section[] = "Desktop Entry";
	char const * q;
	size_t i;

	_on_idle(main); /* just in case */
	memset(menus, 0, sizeof(menus));
	menu = gtk_menu_new();
	for(p = main->apps; p != NULL; p = p->next)
	{
		config = p->data;
		q = config_get(config, section, "Name"); /* should not fail */
		menuitem = _main_menuitem(q, config_get(config, section,
					"Icon"));
		if((q = config_get(config, section, "Comment")) != NULL)
			gtk_widget_set_tooltip_text(menuitem, q);
		if((q = config_get(config, section, "Exec")) != NULL)
			g_signal_connect_swapped(G_OBJECT(menuitem), "activate",
					G_CALLBACK(_applications_on_activate),
					(gpointer)q);
		else
			gtk_widget_set_sensitive(menuitem, FALSE);
		if((q = config_get(config, section, "Categories")) == NULL)
		{
			gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
			continue;
		}
		for(i = 0; _main_menus[i].category != NULL && string_find(q,
					_main_menus[i].category) == NULL; i++);
		if(_main_menus[i].category == NULL)
		{
			gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
			continue;
		}
		if(menus[i] == NULL)
			menus[i] = gtk_menu_new();
		gtk_menu_shell_append(GTK_MENU_SHELL(menus[i]), menuitem);
	}
	_applications_categories(menu, menus);
	return menu;
}

static void _applications_on_activate(gpointer data)
{
	char const * program = data;

	if(program == NULL)
		return;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() \"%s\"", __func__, program);
#endif
	g_spawn_command_line_async(program, NULL);
}

static void _applications_categories(GtkWidget * menu, GtkWidget ** menus)
{
	size_t i;
	MainMenu const * m;
	GtkWidget * menuitem;
	size_t pos = 0;

	for(i = 0; _main_menus[i].category != NULL; i++)
	{
		if(menus[i] == NULL)
			continue;
		m = &_main_menus[i];
		menuitem = _main_menuitem(m->label, m->stock);
		gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem), menus[i]);
		gtk_menu_shell_insert(GTK_MENU_SHELL(menu), menuitem, pos++);
	}
}


/* main_image */
static GtkWidget * _main_image(char const * name)
{
	int width;
	int height;
	size_t len;
	String * buf;
	GError * error = NULL;
	GdkPixbuf * pixbuf = NULL;

	if(gtk_icon_size_lookup(GTK_ICON_SIZE_MENU, &width, &height) == TRUE
			&& (name != NULL && (len = strlen(name)) > 4)
			&& (strcmp(&name[len - 4], ".png") == 0
				|| strcmp(&name[len - 4], ".xpm") == 0)
			&& (buf = string_new_append(PREFIX, "/share/pixmaps/",
					name, NULL)) != NULL)
	{
		pixbuf = gdk_pixbuf_new_from_file_at_size(buf, width, height,
				&error);
		string_delete(buf);
	}
	if(pixbuf != NULL)
		return gtk_image_new_from_pixbuf(pixbuf);
	return gtk_image_new_from_icon_name(name, GTK_ICON_SIZE_MENU);
}


/* main_menuitem */
static GtkWidget * _main_menuitem(char const * label, char const * stock)
{
	GtkWidget * ret;
	GtkWidget * image;

	ret = gtk_image_menu_item_new_with_label(label);
	if(stock != NULL)
	{
		image = _main_image(stock);
		gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(ret), image);
	}
	return ret;
}


/* callbacks */
/* on_about */
static void _on_about(gpointer data)
{
	Main * main = data;

	main->helper->about_dialog(main->helper->panel);
}


/* on_clicked */
static void _clicked_position_menu(GtkMenu * menu, gint * x, gint * y,
		gboolean * push_in, gpointer data);

static void _on_clicked(gpointer data)
{
	Main * main = data;
	GtkWidget * menu;
	GtkWidget * menuitem;

	menu = gtk_menu_new();
	menuitem = _main_menuitem(_("Applications"), "gnome-applications");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem), _main_applications(
				main));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	menuitem = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	menuitem = _main_menuitem(_("Run..."), GTK_STOCK_EXECUTE);
	g_signal_connect_swapped(G_OBJECT(menuitem), "activate", G_CALLBACK(
				_on_run), main);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	menuitem = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_ABOUT, NULL);
	g_signal_connect_swapped(G_OBJECT(menuitem), "activate", G_CALLBACK(
				_on_about), main);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	/* lock screen */
	menuitem = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	menuitem = _main_menuitem(_("Lock screen"), "gnome-lockscreen");
	g_signal_connect_swapped(G_OBJECT(menuitem), "activate", G_CALLBACK(
				_on_lock), main);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
#ifdef EMBEDDED
	/* rotate screen */
	menuitem = _main_menuitem(_("Rotate"), GTK_STOCK_REFRESH); /* XXX */
	g_signal_connect_swapped(G_OBJECT(menuitem), "activate",
			G_CALLBACK(_on_rotate), data);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
#endif
	/* logout */
	if(main->helper->logout_dialog != NULL)
	{
		menuitem = _main_menuitem(_("Logout..."), "gnome-logout");
		g_signal_connect_swapped(G_OBJECT(menuitem), "activate",
				G_CALLBACK(_on_logout), data);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	}
	/* suspend */
	if(main->helper->suspend != NULL)
	{
		menuitem = _main_menuitem(_("Suspend"), "gtk-media-pause");
		g_signal_connect_swapped(G_OBJECT(menuitem), "activate",
				G_CALLBACK(_on_suspend), data);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	}
	/* shutdown */
	menuitem = _main_menuitem(_("Shutdown..."), "gnome-shutdown");
	g_signal_connect_swapped(G_OBJECT(menuitem), "activate", G_CALLBACK(
				_on_shutdown), data);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	gtk_widget_show_all(menu);
	gtk_menu_popup(GTK_MENU(menu), NULL, NULL, _clicked_position_menu,
			main, 0, gtk_get_current_event_time());
}

static void _clicked_position_menu(GtkMenu * menu, gint * x, gint * y,
		gboolean * push_in, gpointer data)
{
	Main * main = data;

	main->helper->position_menu(main->helper->panel, menu, x, y, push_in);
}


/* on_idle */
static gint _idle_apps_compare(gconstpointer a, gconstpointer b);

static gboolean _on_idle(gpointer data)
{
	Main * main = data;
	const char path[] = PREFIX "/share/applications";
	DIR * dir;
	int fd;
	struct stat st;
	struct dirent * de;
	size_t len;
	const char ext[] = ".desktop";
	const char section[] = "Desktop Entry";
	char * name = NULL;
	char * p;
	Config * config = NULL;
	String const * q;

	if(main->apps != NULL)
		return FALSE;
#if defined(__sun__)
	if((fd = open(path, O_RDONLY)) < 0
			|| fstat(fd, &st) != 0
			|| (dir = fdopendir(fd)) == NULL)
#else
	if((dir = opendir(path)) == NULL
			|| (fd = dirfd(dir)) < 0
			|| fstat(fd, &st) != 0)
#endif
		return main->helper->error(NULL, path, FALSE);
	main->refresh_mti = st.st_mtime;
	while((de = readdir(dir)) != NULL)
	{
		if(de->d_name[0] == '.')
			if(de->d_name[1] == '\0' || (de->d_name[1] == '.'
						&& de->d_name[2] == '\0'))
				continue;
		len = strlen(de->d_name);
		if(len < sizeof(ext) || strncmp(&de->d_name[len - sizeof(ext)
					+ 1], ext, sizeof(ext)) != 0)
			continue;
		if((p = realloc(name, sizeof(path) + len + 1)) == NULL)
		{
			main->helper->error(NULL, path, 1);
			continue;
		}
		name = p;
		snprintf(name, sizeof(path) + len + 1, "%s/%s", path,
				de->d_name);
#ifdef DEBUG
		fprintf(stderr, "DEBUG: %s() \"%s\"\n", __func__, name);
#endif
		if(config == NULL && (config = config_new()) == NULL)
			continue; /* XXX report error */
		config_reset(config);
		if(config_load(config, name) != 0)
		{
			error_print("Panel"); /* XXX use the error helper */
			continue;
		}
		q = config_get(config, section, "Name");
		if(q == NULL)
			continue;
		main->apps = g_slist_insert_sorted(main->apps, config,
				_idle_apps_compare);
		config = NULL;
	}
	free(name);
	closedir(dir);
	if(config != NULL)
		config_delete(config);
	g_timeout_add(1000, _on_timeout, main);
	return FALSE;
}

static gint _idle_apps_compare(gconstpointer a, gconstpointer b)
{
	Config * ca = (Config *)a;
	Config * cb = (Config *)b;
	char const * cap;
	char const * cbp;
	const char section[] = "Desktop Entry";
	const char variable[] = "Name";

	/* these should not fail */
	cap = config_get(ca, section, variable);
	cbp = config_get(cb, section, variable);
	return string_compare(cap, cbp);
}


/* on_lock */
static void _on_lock(gpointer data)
{
	Main * main = data;

	main->helper->lock(main->helper->panel);
}


/* on_logout */
static void _on_logout(gpointer data)
{
	Main * main = data;

	main->helper->logout_dialog(main->helper->panel);
}


#ifdef EMBEDDED
/* on_rotate */
static void _on_rotate(gpointer data)
{
	Main * main = data;

	main->helper->rotate_screen(main->helper->panel);
}
#endif


/* on_run */
static void _on_run(gpointer data)
{
	Main * main = data;
	char * argv[] = { "run", NULL };
	GSpawnFlags flags = G_SPAWN_SEARCH_PATH
		| G_SPAWN_STDOUT_TO_DEV_NULL
		| G_SPAWN_STDERR_TO_DEV_NULL;
	GError * error = NULL;

	if(g_spawn_async(NULL, argv, NULL, flags, NULL, NULL, NULL, &error)
			!= TRUE)
	{
		main->helper->error(main->helper->panel, error->message, 1);
		g_error_free(error);
	}
}


/* on_shutdown */
static void _on_shutdown(gpointer data)
{
	Main * main = data;

	main->helper->shutdown_dialog(main->helper->panel);
}


/* on_suspend */
static void _on_suspend(gpointer data)
{
	Main * main = data;

	main->helper->suspend(main->helper->panel);
}


/* on_timeout */
static gboolean _on_timeout(gpointer data)
{
	Main * main = data;
	const char path[] = PREFIX "/share/applications";
	struct stat st;

	if(stat(path, &st) != 0)
		return TRUE;
	if(st.st_mtime == main->refresh_mti)
		return TRUE;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() resetting the menu\n", __func__);
#endif
	g_slist_foreach(main->apps, (GFunc)config_delete, NULL);
	g_slist_free(main->apps);
	main->apps = NULL;
	g_idle_add(_on_idle, main);
	return FALSE;
}
