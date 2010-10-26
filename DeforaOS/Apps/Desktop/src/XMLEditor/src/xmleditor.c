/* $Id$ */
static char const _copyright[] =
"Copyright (c) 2010 Pierre Pronchery <khorben@defora.org>";
/* This file is part of DeforaOS Desktop XMLEditor */
static char const _license[] =
"This program is free software; you can redistribute it and/or modify\n"
"it under the terms of the GNU General Public License as published by the\n"
"Free Software Foundation, version 3 of the License.\n"
"\n"
"This program is distributed in the hope that it will be useful,\n"
"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
"GNU General Public License for more details.\n"
"\n"
"You should have received a copy of the GNU General Public License\n"
"along with this program. If not, see <http://www.gnu.org/licenses/>.\n";



#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <gdk/gdkkeysyms.h>
#include <System/Parser.h>
#include <Desktop.h>
#include "callbacks.h"
#include "xmleditor.h"
#include "../config.h"


/* XMLEditor */
/* private */
/* types */
struct _XMLEditor
{
	XML * xml;

	/* widgets */
	GtkWidget * window;
	GtkTreeStore * store;
	GtkWidget * view;
	GtkWidget * statusbar;
	/* about */
	GtkWidget * ab_window;
};


/* variables */
static char const * _authors[] =
{
	"Pierre Pronchery <khorben@defora.org>",
	NULL
};

#ifdef EMBEDDED
static DesktopAccel _xmleditor_accel[] =
{
	{ G_CALLBACK(on_close), GDK_CONTROL_MASK, GDK_w },
	{ G_CALLBACK(on_new), GDK_CONTROL_MASK, GDK_n },
	{ G_CALLBACK(on_open), GDK_CONTROL_MASK, GDK_o },
	{ G_CALLBACK(on_preferences), GDK_CONTROL_MASK, GDK_p },
	{ G_CALLBACK(on_save), GDK_CONTROL_MASK, GDK_s },
	{ G_CALLBACK(on_save_as), GDK_CONTROL_MASK, GDK_S },
	{ NULL, 0, 0 }
};
#endif

#ifndef EMBEDDED
static DesktopMenu _xmleditor_menu_file[] =
{
	{ "_New", G_CALLBACK(on_file_new), GTK_STOCK_NEW, GDK_CONTROL_MASK,
		GDK_n },
	{ "_Open", G_CALLBACK(on_file_open), GTK_STOCK_OPEN,
		GDK_CONTROL_MASK, GDK_o },
	{ "", NULL, NULL, 0, 0 },
	{ "_Save", G_CALLBACK(on_file_save), GTK_STOCK_SAVE,
		GDK_CONTROL_MASK, GDK_s },
	{ "_Save as...", G_CALLBACK(on_file_save_as), GTK_STOCK_SAVE_AS,
		GDK_CONTROL_MASK | GDK_SHIFT_MASK, GDK_S },
	{ "", NULL, NULL, 0, 0 },
	{ "_Close", G_CALLBACK(on_file_close), GTK_STOCK_CLOSE, 0, 0 },
	{ NULL, NULL, NULL, 0, 0 }
};

static DesktopMenu _xmleditor_menu_edit[] =
{
	{ "_Preferences", G_CALLBACK(on_edit_preferences),
		GTK_STOCK_PREFERENCES, GDK_CONTROL_MASK, GDK_p },
	{ NULL, NULL, NULL, 0, 0 }
};

static DesktopMenu _xmleditor_menu_help[] =
{
	{ "_About", G_CALLBACK(on_help_about),
#if GTK_CHECK_VERSION(2, 6, 0)
		GTK_STOCK_ABOUT, 0, 0 },
#else
		NULL, 0, 0 },
#endif
	{ NULL, NULL, NULL, 0, 0 }
};

static DesktopMenubar _xmleditor_menubar[] =
{
	{ "_File", _xmleditor_menu_file },
	{ "_Edit", _xmleditor_menu_edit },
	{ "_Help", _xmleditor_menu_help },
	{ NULL, NULL }
};
#endif

static DesktopToolbar _xmleditor_toolbar[] =
{
	{ "New", G_CALLBACK(on_new), GTK_STOCK_NEW, 0, 0, NULL },
	{ "Open", G_CALLBACK(on_open), GTK_STOCK_OPEN, 0, 0, NULL },
	{ "", NULL, NULL, 0, 0, NULL },
	{ "Save", G_CALLBACK(on_save), GTK_STOCK_SAVE, 0, 0, NULL },
	{ "Save as", G_CALLBACK(on_save_as), GTK_STOCK_SAVE_AS, 0, 0,
		NULL },
#ifdef EMBEDDED
	{ "", NULL, NULL, 0, 0, NULL },
	{ "Preferences", G_CALLBACK(on_preferences), GTK_STOCK_PREFERENCES,
		0, 0, NULL },
#endif
	{ NULL, NULL, NULL, 0, 0, NULL }
};


/* public */
/* functions */
/* xmleditor_new */
static void _new_set_title(XMLEditor * xmleditor);

XMLEditor * xmleditor_new(void)
{
	XMLEditor * xmleditor;
	GtkAccelGroup * group;
	GtkSettings * settings;
	GtkWidget * vbox;
	GtkWidget * widget;
	GtkCellRenderer * renderer;
	GtkTreeViewColumn * column;

	if((xmleditor = malloc(sizeof(*xmleditor))) == NULL)
		return NULL;
	settings = gtk_settings_get_default();
	xmleditor->xml = NULL;
	/* widgets */
	group = gtk_accel_group_new();
	xmleditor->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_add_accel_group(GTK_WINDOW(xmleditor->window), group);
	gtk_window_set_default_size(GTK_WINDOW(xmleditor->window), 600, 400);
	_new_set_title(xmleditor);
#if GTK_CHECK_VERSION(2, 6, 0)
	gtk_window_set_icon_name(GTK_WINDOW(xmleditor->window), "text-editor");
#endif
	g_signal_connect_swapped(G_OBJECT(xmleditor->window), "delete-event",
			G_CALLBACK(on_closex), xmleditor);
	vbox = gtk_vbox_new(FALSE, 0);
	/* menubar */
#ifndef EMBEDDED
	widget = desktop_menubar_create(_xmleditor_menubar, xmleditor, group);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, FALSE, 0);
#else
	desktop_accel_create(_xmleditor_accel, xmleditor, group);
#endif
	/* toolbar */
	widget = desktop_toolbar_create(_xmleditor_toolbar, xmleditor, group);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, FALSE, 0);
	/* view */
	widget = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	xmleditor->store = gtk_tree_store_new(1, G_TYPE_STRING);
	xmleditor->view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(
				xmleditor->store));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(xmleditor->view),
			FALSE);
	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Name", renderer,
			"text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(xmleditor->view), column);
	gtk_container_add(GTK_CONTAINER(widget), xmleditor->view);
	gtk_box_pack_start(GTK_BOX(vbox), widget, TRUE, TRUE, 0);
	/* statusbar */
	xmleditor->statusbar = gtk_statusbar_new();
	gtk_box_pack_start(GTK_BOX(vbox), xmleditor->statusbar, FALSE, FALSE,
			0);
	/* about */
	xmleditor->ab_window = NULL;
	gtk_container_add(GTK_CONTAINER(xmleditor->window), vbox);
	gtk_window_set_focus(GTK_WINDOW(xmleditor->window), xmleditor->view);
	gtk_widget_show_all(xmleditor->window);
	return xmleditor;
}

static void _new_set_title(XMLEditor * xmleditor)
{
	char buf[256];

	snprintf(buf, sizeof(buf), "%s%s", "XML editor - ",
			(xmleditor->xml == NULL) ? "(Untitled)"
			: xml_get_filename(xmleditor->xml));
	gtk_window_set_title(GTK_WINDOW(xmleditor->window), buf);
}


/* xmleditor_delete */
void xmleditor_delete(XMLEditor * xmleditor)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(xmleditor->xml != NULL)
		xml_delete(xmleditor->xml);
	free(xmleditor);
}


/* useful */
/* xmleditor_about */
static gboolean _about_on_closex(GtkWidget * widget);

void xmleditor_about(XMLEditor * xmleditor)
{
	if(xmleditor->ab_window != NULL)
	{
		gtk_widget_show(xmleditor->ab_window);
		return;
	}
	xmleditor->ab_window = desktop_about_dialog_new();
	gtk_window_set_transient_for(GTK_WINDOW(xmleditor->ab_window),
			GTK_WINDOW(xmleditor->window));
	g_signal_connect(G_OBJECT(xmleditor->ab_window), "delete-event",
			G_CALLBACK(_about_on_closex), NULL);
	desktop_about_dialog_set_authors(xmleditor->ab_window, _authors);
	desktop_about_dialog_set_copyright(xmleditor->ab_window, _copyright);
	desktop_about_dialog_set_license(xmleditor->ab_window, _license);
	desktop_about_dialog_set_logo_icon_name(xmleditor->ab_window,
			"text-editor");
	desktop_about_dialog_set_name(xmleditor->ab_window, PACKAGE);
	desktop_about_dialog_set_version(xmleditor->ab_window, VERSION);
	gtk_widget_show(xmleditor->ab_window);
}

static gboolean _about_on_closex(GtkWidget * widget)
{
	gtk_widget_hide(widget);
	return TRUE;
}


/* xmleditor_error */
int xmleditor_error(XMLEditor * xmleditor, char const * message, int ret)
{
	GtkWidget * dialog;

	dialog = gtk_message_dialog_new(GTK_WINDOW(xmleditor->window),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "%s",
#if GTK_CHECK_VERSION(2, 6, 0)
			"Error");
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
			"%s",
#endif
			message);
	gtk_window_set_title(GTK_WINDOW(dialog), "Error");
	g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(
				gtk_widget_destroy), NULL);
	gtk_widget_show(dialog);
	return ret;
}


/* xmleditor_close */
gboolean xmleditor_close(XMLEditor * xmleditor)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	/* FIXME implement */
	gtk_main_quit();
	return FALSE;
}


/* xmleditor_open */
static void _open_document_node(XMLEditor * xmleditor, XMLNode * node,
		GtkTreeIter * parent);

void xmleditor_open(XMLEditor * xmleditor, char const * filename)
{
	XMLDocument * doc;

	gtk_tree_store_clear(xmleditor->store);
	if(filename == NULL)
		return;
	if(xmleditor->xml != NULL)
		xml_delete(xmleditor->xml);
	if((xmleditor->xml = xml_new(NULL, filename)) == NULL)
		return;
	if((doc = xml_get_document(xmleditor->xml)) != NULL)
		_open_document_node(xmleditor, doc->root, NULL);
	_new_set_title(xmleditor); /* XXX make it a generic private function */
}

static void _open_document_node(XMLEditor * xmleditor, XMLNode * node,
		GtkTreeIter * parent)
{
	size_t i;
	GtkTreeIter iter;

	if(node == NULL)
		return;
	gtk_tree_store_append(xmleditor->store, &iter, parent);
	switch(node->type)
	{
		case XML_NODE_TYPE_DATA:
			gtk_tree_store_set(xmleditor->store, &iter, 0,
					node->data.buffer, -1);
			break;
		case XML_NODE_TYPE_TAG:
			gtk_tree_store_set(xmleditor->store, &iter, 0,
					node->tag.name, -1);
			for(i = 0; i < node->tag.childs_cnt; i++)
				_open_document_node(xmleditor,
						node->tag.childs[i], &iter);
			break;
	}
}


/* xmleditor_open_dialog */
void xmleditor_open_dialog(XMLEditor * xmleditor)
{
	GtkWidget * dialog;
	char * filename = NULL;

	dialog = gtk_file_chooser_dialog_new("Open file...",
			GTK_WINDOW(xmleditor->window),
			GTK_FILE_CHOOSER_ACTION_OPEN,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(
					dialog));
	gtk_widget_destroy(dialog);
	if(filename == NULL)
		return;
	xmleditor_open(xmleditor, filename);
	g_free(filename);
}


/* xmleditor_save */
gboolean xmleditor_save(XMLEditor * xmleditor)
{
	char const * filename;
	char * buf;
	FILE * fp;

	if(xmleditor->xml == NULL)
	{
		xmleditor_save_as_dialog(xmleditor);
		return FALSE;
	}
	filename = xml_get_filename(xmleditor->xml);
	if((fp = fopen(filename, "w")) == NULL)
	{
		buf = g_strdup_printf("%s: %s", filename, strerror(errno));
		xmleditor_error(xmleditor, buf, 1);
		g_free(buf);
		return FALSE;
	}
	/* FIXME implement */
	fclose(fp);
	return TRUE;
}


/* xmleditor_save_as */
gboolean xmleditor_save_as(XMLEditor * xmleditor, char const * filename)
{
	struct stat st;
	GtkWidget * dialog;
	int ret;

	if(stat(filename, &st) == 0)
	{
		dialog = gtk_message_dialog_new(GTK_WINDOW(xmleditor->window),
				GTK_DIALOG_MODAL
				| GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO, "%s",
#if GTK_CHECK_VERSION(2, 6, 0)
				"Warning");
		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(
					dialog), "%s",
#endif
				"File exists. Overwrite?");
		gtk_window_set_title(GTK_WINDOW(dialog), "Warning");
		ret = gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		if(ret == GTK_RESPONSE_NO)
			return FALSE;
	}
	/* FIXME implement */
	return TRUE;
}


/* xmleditor_save_as_dialog */
gboolean xmleditor_save_as_dialog(XMLEditor * xmleditor)
{
	GtkWidget * dialog;
	char * filename = NULL;
	gboolean ret;

	dialog = gtk_file_chooser_dialog_new("Save as...",
			GTK_WINDOW(xmleditor->window),
			GTK_FILE_CHOOSER_ACTION_SAVE,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(
					dialog));
	gtk_widget_destroy(dialog);
	if(filename == NULL)
		return FALSE;
	ret = xmleditor_save_as(xmleditor, filename);
	g_free(filename);
	return ret;
}
