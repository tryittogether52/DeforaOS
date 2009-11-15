/* $Id$ */
static char const _copyright[] =
"Copyright (c) 2009 Pierre Pronchery <khorben@defora.org>";
/* This file is part of DeforaOS Desktop Browser */
static char const _license[] =
"view is free software; you can redistribute it and/or modify it under the\n"
"terms of the GNU General Public License version 3 as published by the Free\n"
"Software Foundation.\n"
"\n"
"view is distributed in the hope that it will be useful, but WITHOUT ANY\n"
"WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS\n"
"FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more\n"
"details.\n"
"\n"
"You should have received a copy of the GNU General Public License along with\n"
"view; if not, see <http://www.gnu.org/licenses/>.\n";



#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "mime.h"
#include "../config.h"

#define COMMON_MENU
#include "common.c"


/* constants */
static char const * _authors[] =
{
	"Pierre Pronchery <khorben@defora.org>",
	NULL
};


/* View */
/* private */
/* types */
typedef struct _View
{
	char * pathname;

	/* widgets */
	GtkWidget * window;
	GtkWidget * ab_window;
} View;


/* prototypes */
static View * _view_new(char const * path);
static View * _view_new_open(void);
static void _view_delete(View * view);

/* useful */
static int _view_error(View * view, char const * message, int ret);

/* callbacks */
static gboolean _on_closex(GtkWidget * widget, GdkEvent * event, gpointer data);
static void _on_file_edit(GtkWidget * widget, gpointer data);
static void _on_file_open_with(GtkWidget * widget, gpointer data);
static void _on_file_close(GtkWidget * widget, gpointer data);
static void _on_help_about(GtkWidget * widget, gpointer data);


/* constants */
static struct _menu _view_menu_file[] =
{
	{ "Open _with...", G_CALLBACK(_on_file_open_with), NULL, 0 },
	{ "", NULL, NULL, 0 },
	{ "_Close", G_CALLBACK(_on_file_close), GTK_STOCK_CLOSE, GDK_W },
	{ NULL, NULL, NULL, 0 }
};

static struct _menu _view_menu_file_edit[] =
{
	{ "_Edit", G_CALLBACK(_on_file_edit), GTK_STOCK_EDIT, GDK_E },
	{ "Open _with...", G_CALLBACK(_on_file_open_with), NULL, 0 },
	{ "", NULL, NULL, 0 },
	{ "_Close", G_CALLBACK(_on_file_close), GTK_STOCK_CLOSE, GDK_W },
	{ NULL, NULL, NULL, 0 }
};

static struct _menu _view_menu_help[] =
{
#if GTK_CHECK_VERSION(2, 6, 0)
	{ "_About", G_CALLBACK(_on_help_about), GTK_STOCK_ABOUT, 0 },
#else
	{ "_About", G_CALLBACK(_on_help_about), NULL, 0 },
#endif
	{ NULL, NULL, NULL, 0 }
};

static struct _menubar _view_menubar[] =
{
	{ "_File", _view_menu_file },
	{ "_Help", _view_menu_help },
	{ NULL, NULL }
};

static struct _menubar _view_menubar_edit[] =
{
	{ "_File", _view_menu_file_edit },
	{ "_Help", _view_menu_help },
	{ NULL, NULL }
};


/* variables */
static Mime * _mime = NULL;
static unsigned int _view_cnt = 0;


/* functions */
/* view_new */
static GtkWidget * _new_image(View * view, char const * path);
static GtkWidget * _new_text(View * view, char const * path);

static View * _view_new(char const * pathname)
{
	View * view;
	struct stat st;
	char const * type;
	char buf[256];
	GtkWidget * vbox;
	GtkWidget * widget;

	if((view = malloc(sizeof(*view))) == NULL)
		return NULL; /* FIXME handle error */
	view->window = NULL;
	view->ab_window = NULL;
	_view_cnt++;
	if((view->pathname = strdup(pathname)) == NULL
			|| lstat(pathname, &st) != 0)
	{
		_view_error(view, strerror(errno), 2);
		return NULL;
	}
	if(_mime == NULL)
		_mime = mime_new();
	if((type = mime_type(_mime, pathname)) == NULL)
	{
		_view_error(view, "Unknown file type", 2);
		return NULL;
	}
	view->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	snprintf(buf, sizeof(buf), "%s%s", "View - ", pathname);
	gtk_window_set_title(GTK_WINDOW(view->window), buf);
	g_signal_connect(view->window, "delete-event", G_CALLBACK(_on_closex),
			view);
	vbox = gtk_vbox_new(FALSE, 0);
	widget = _common_new_menubar(GTK_WINDOW(view->window),
			mime_get_handler(_mime, type, "edit") != NULL
			? _view_menubar_edit : _view_menubar, view);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, FALSE, 0);
	if(strncmp(type, "image/", 6) == 0)
	{
		if((widget = _new_image(view, pathname)) == NULL)
			return NULL;
	}
	else if(strncmp(type, "text/", 5) == 0)
	{
		widget = _new_text(view, pathname);
		gtk_window_set_default_size(GTK_WINDOW(view->window), 600, 400);
	}
	else
	{
		_view_error(view, "Unable to view file type", 2);
		return NULL;
	}
	gtk_box_pack_start(GTK_BOX(vbox), widget, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(view->window), vbox);
	gtk_widget_show_all(view->window);
	return view;
}

static GtkWidget * _new_image(View * view, char const * path)
{
	GtkWidget * window;
	GError * error = NULL;
	GdkPixbuf * pixbuf;
	GtkWidget * widget;
	int pw;
	int ph;
	GdkScreen * screen;
	gint monitor;
	GdkRectangle rect;

	window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(window),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	if((pixbuf = gdk_pixbuf_new_from_file_at_size(path, -1, -1, &error))
			== NULL)
	{
		_view_error(view, error->message, 1);
		return NULL;
	}
	widget = gtk_image_new_from_pixbuf(pixbuf);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(window),
			widget);
	pw = gdk_pixbuf_get_width(pixbuf) + 4;
	ph = gdk_pixbuf_get_height(pixbuf) + 4;
	/* get the current monitor size */
	screen = gdk_screen_get_default();
#if GTK_CHECK_VERSION(2, 14, 0)
	gtk_widget_realize(view->window);
	monitor = gdk_screen_get_monitor_at_window(screen,
			gtk_widget_get_window(view->window));
#else
	monitor = 0; /* FIXME hard-coded */
#endif
	gdk_screen_get_monitor_geometry(screen, monitor, &rect);
	/* set an upper bound to the size of the window */
	gtk_widget_set_size_request(window, min(pw, rect.width), min(ph,
				rect.height));
	return window;
}

static GtkWidget * _new_text(View * view, char const * path)
{
	GtkWidget * widget;
	GtkWidget * text;
	PangoFontDescription * desc;
	FILE * fp;
	GtkTextBuffer * tbuf;
	GtkTextIter iter;
	char buf[BUFSIZ];
	size_t len;

	widget = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	text = gtk_text_view_new();
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(text), FALSE);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(text), FALSE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text), GTK_WRAP_WORD_CHAR);
	desc = pango_font_description_new();
	pango_font_description_set_family(desc, "monospace");
	gtk_widget_modify_font(text, desc);
	pango_font_description_free(desc);
	gtk_container_add(GTK_CONTAINER(widget), text);
	if((fp = fopen(path, "r")) == NULL)
	{
		_view_error(view, strerror(errno), 0);
		return widget;
	}
	tbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text));
	while((len = fread(buf, sizeof(char), sizeof(buf), fp)) > 0)
	{
		gtk_text_buffer_get_end_iter(tbuf, &iter);
		gtk_text_buffer_insert(tbuf, &iter, buf, len);
	}
	fclose(fp);
	return widget;
}


/* view_new_open */
static View * _view_new_open(void)
{
	View * ret;
	GtkWidget * dialog;
	char * pathname = NULL;

	dialog = gtk_file_chooser_dialog_new("View file...", NULL,
			GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL,
			GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN,
			GTK_RESPONSE_ACCEPT, NULL);
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
		pathname = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(
					dialog));
	gtk_widget_destroy(dialog);
	if(pathname == NULL)
		return NULL;
	ret = _view_new(pathname);
	free(pathname);
	return ret;
}


/* view_delete */
static void _view_delete(View * view)
{
	free(view->pathname);
	if(view->ab_window != NULL)
		gtk_widget_destroy(view->ab_window);
	if(view->window != NULL)
		gtk_widget_destroy(view->window);
	free(view);
	if(--_view_cnt == 0)
		gtk_main_quit();
}


/* useful */
/* view_error
 * POST	view is deleted if ret != 0 */
static void _error_response(GtkWidget * widget, gint arg, gpointer data);

static int _view_error(View * view, char const * message, int ret)
{
	GtkWidget * dialog;

	dialog = gtk_message_dialog_new(view != NULL && view->window != NULL
			? GTK_WINDOW(view->window) : NULL,
			GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR,
			GTK_BUTTONS_CLOSE, "%s", "Error");
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
			"%s", message);
	gtk_window_set_title(GTK_WINDOW(dialog), "Error");
	g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(
				_error_response), ret != 0 ? view : NULL);
	gtk_widget_show(dialog);
	return ret;
}

static void _error_response(GtkWidget * widget, gint arg, gpointer data)
{
	View * view = data;

	if(view != NULL)
		_view_delete(view);
	gtk_widget_destroy(widget);
}


/* callbacks */
static gboolean _on_closex(GtkWidget * widget, GdkEvent * event, gpointer data)
{
	View * view = data;

	_view_delete(view);
	if(_view_cnt == 0)
		gtk_main_quit();
	return FALSE;
}


static void _on_file_edit(GtkWidget * widget, gpointer data)
{
	View * view = data;

	if(mime_action(_mime, "edit", view->pathname) != 0)
		_view_error(view, "Could not edit file", 0);
}


static void _on_file_open_with(GtkWidget * widget, gpointer data)
{
	View * view = data;
	GtkWidget * dialog;
	char * filename = NULL;
	pid_t pid;

	dialog = gtk_file_chooser_dialog_new("Open with...",
			GTK_WINDOW(view->window),
			GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL,
			GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN,
			GTK_RESPONSE_ACCEPT, NULL);
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(
					dialog));
	gtk_widget_destroy(dialog);
	if(filename == NULL)
		return;
	if((pid = fork()) == -1)
		_view_error(view, "fork", 0);
	else if(pid == 0)
	{
		if(close(0) != 0)
			_view_error(NULL, "stdin", 0);
		execlp(filename, filename, view->pathname, NULL);
		_view_error(NULL, filename, 0);
		exit(2);
	}
	g_free(filename);
}


static void _on_file_close(GtkWidget * widget, gpointer data)
{
	View * view = data;

	_view_delete(view);
	if(_view_cnt == 0)
		gtk_main_quit();
}


/* on_help_about */
static gboolean _about_on_closex(GtkWidget * widget, GdkEvent * event,
		gpointer data);
#if !GTK_CHECK_VERSION(2, 6, 0)
static void _about_on_close(GtkWidget * widget, gpointer data);
static void _about_on_credits(GtkWidget * widget, gpointer data);
static void _about_on_license(GtkWidget * widget, gpointer data);
#endif

static void _on_help_about(GtkWidget * widget, gpointer data)
{
	View * view = data;
	GtkWidget * window;
#if GTK_CHECK_VERSION(2, 6, 0)
	gsize cnt = 65536;
	gchar * buf;

	if(view->ab_window != NULL)
	{
		gtk_widget_show(view->ab_window);
		return;
	}
	view->ab_window = gtk_about_dialog_new();
	window = view->ab_window;
	gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(
				view->window));
	gtk_about_dialog_set_name(GTK_ABOUT_DIALOG(window), "View");
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(window), VERSION);
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(window), _copyright);
	gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(window), _authors);
	if(g_file_get_contents("/usr/share/common-licenses/GPL-2", &buf, &cnt,
				NULL) == TRUE)
		gtk_about_dialog_set_license(GTK_ABOUT_DIALOG(window), buf);
	else
		gtk_about_dialog_set_license(GTK_ABOUT_DIALOG(window),
				_license);
	free(buf);
	g_signal_connect(G_OBJECT(window), "delete-event", G_CALLBACK(
				_about_on_closex), window);
	g_signal_connect(G_OBJECT(window), "response", G_CALLBACK(
				gtk_widget_hide), NULL);
	gtk_widget_show(window);
}
#else /* !GTK_CHECK_VERSION(2, 6, 0) */
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * button;

	if(browser->ab_window != NULL)
	{
		gtk_widget_show(browser->ab_window);
		return;
	}
	browser->ab_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	window = browser->ab_window;
	gtk_container_set_border_width(GTK_CONTAINER(window), 4);
	gtk_window_set_title(GTK_WINDOW(window), "About Browser");
	gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(
				browser->window));
	g_signal_connect(G_OBJECT(window), "delete-event", G_CALLBACK(
				_about_on_closex), window);
	vbox = gtk_vbox_new(FALSE, 2);
	gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new(PACKAGE " " VERSION),
			FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new(_copyright), FALSE,
			FALSE, 2);
	hbox = gtk_hbox_new(TRUE, 4);
	button = gtk_button_new_with_mnemonic("C_redits");
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(
				_about_on_credits), window);
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, TRUE, 4);
	button = gtk_button_new_with_mnemonic("_License");
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(
				_about_on_license), window);
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, TRUE, 4);
	button = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(
				_about_on_close), window);
	gtk_box_pack_end(GTK_BOX(hbox), button, FALSE, TRUE, 4);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 4);
	gtk_container_add(GTK_CONTAINER(window), vbox);
	gtk_widget_show_all(window);
}
#endif

static gboolean _about_on_closex(GtkWidget * widget, GdkEvent * event,
		gpointer data)
{
	gtk_widget_hide(widget);
	return TRUE;
}

#if !GTK_CHECK_VERSION(2, 6, 0)
static void _about_on_close(GtkWidget * widget, gpointer data)
{
	GtkWidget * window = data;

	gtk_widget_hide(window);
}

static void _about_on_credits(GtkWidget * widget, gpointer data)
{
	static GtkWidget * window = NULL;
	GtkWidget * about = data;
	GtkWidget * vbox;
	GtkWidget * notebook;
	GtkWidget * textview;
	GtkTextBuffer * tbuf;
	GtkTextIter iter;
	GtkWidget * hbox;
	size_t i;

	if(window != NULL)
	{
		gtk_widget_show(window);
		return;
	}
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(window), 200, 200);
	gtk_container_set_border_width(GTK_CONTAINER(window), 4);
	gtk_window_set_title(GTK_WINDOW(window), "Credits");
	gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(about));
	g_signal_connect(G_OBJECT(window), "delete-event", G_CALLBACK(
				_about_on_closex), NULL);
	vbox = gtk_vbox_new(FALSE, 0);
	textview = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(textview), FALSE);
	tbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	gtk_text_buffer_set_text(tbuf, "", 0);
	for(i = 0; _authors[i] != NULL; i++)
	{
		gtk_text_buffer_get_end_iter(tbuf, &iter);
		gtk_text_buffer_insert(tbuf, &iter, _authors[i], strlen(
					_authors[i]));
	}
	widget = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(widget),
			GTK_SHADOW_IN);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(widget), textview);
	notebook = gtk_notebook_new();
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), widget,
			gtk_label_new("Written by"));
	gtk_box_pack_start(GTK_BOX(vbox), notebook, TRUE, TRUE, 4);
	hbox = gtk_hbox_new(FALSE, 0);
	widget = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
	g_signal_connect(G_OBJECT(widget), "clicked",
			G_CALLBACK(_about_on_close), window);
	gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, TRUE, 4);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 4);
	gtk_container_add(GTK_CONTAINER(window), vbox);
	gtk_widget_show_all(window);
}

static void _about_on_license(GtkWidget * widget, gpointer data)
{
	static GtkWidget * window = NULL;
	GtkWidget * about = data;
	GtkWidget * vbox;
	GtkWidget * textview;
	GtkTextBuffer * tbuf;
	GtkWidget * hbox;

	if(window != NULL)
	{
		gtk_widget_show(window);
		return;
	}
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(window), 200, 200);
	gtk_container_set_border_width(GTK_CONTAINER(window), 4);
	gtk_window_set_title(GTK_WINDOW(window), "License");
	gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(about));
	g_signal_connect(G_OBJECT(window), "delete-event", G_CALLBACK(
				_about_on_closex), NULL);
	vbox = gtk_vbox_new(FALSE, 0);
	textview = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(textview), FALSE);
	tbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	gtk_text_buffer_set_text(tbuf, _license, strlen(_license));
	widget = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(widget),
			GTK_SHADOW_IN);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(widget), textview);
	gtk_box_pack_start(GTK_BOX(vbox), widget, TRUE, TRUE, 4);
	hbox = gtk_hbox_new(FALSE, 0);
	widget = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
	g_signal_connect(G_OBJECT(widget), "clicked",
			G_CALLBACK(_about_on_close), window);
	gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, TRUE, 4);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 4);
	gtk_container_add(GTK_CONTAINER(window), vbox);
	gtk_widget_show_all(window);
}
#endif /* !GTK_CHECK_VERSION(2, 6, 0) */


/* usage */
static int _usage(void)
{
	fputs("Usage: view file...\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	int i;

	gtk_init(&argc, &argv);
	while((o = getopt(argc, argv, "")) != -1)
		switch(o)
		{
			default:
				return _usage();
		}
	if(optind == argc)
		_view_new_open();
	else
		for(i = optind; i < argc; i++)
			_view_new(argv[i]);
	if(_view_cnt)
		gtk_main();
	return 0;
}
