/* $Id$ */
static char const _copyright[] =
"Copyright (c) 2011 Pierre Pronchery <khorben@defora.org>";
/* This file is part of DeforaOS Desktop Editor */
static char const _license[] =
"This program is free software: you can redistribute it and/or modify\n"
"it under the terms of the GNU General Public License as published by\n"
"the Free Software Foundation, version 3 of the License.\n"
"\n"
"This program is distributed in the hope that it will be useful,\n"
"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
"GNU General Public License for more details.\n"
"\n"
"You should have received a copy of the GNU General Public License\n"
"along with this program.  If not, see <http://www.gnu.org/licenses/>.\n";



#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <libintl.h>
#include <gdk/gdkkeysyms.h>
#include <Desktop.h>
#include "callbacks.h"
#include "editor.h"
#include "../config.h"
#define _(string) gettext(string)
#define N_(string) (string)


/* Editor */
/* private */
/* constants */
#define EDITOR_CONFIG_FILE	".editor"
#define EDITOR_DEFAULT_FONT	"Monospace 9"


/* variables */
static char const * _authors[] =
{
	"Pierre Pronchery <khorben@defora.org>",
	NULL
};

#ifdef EMBEDDED
static DesktopAccel _editor_accel[] =
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
static DesktopMenu _editor_menu_file[] =
{
	{ N_("_New"), G_CALLBACK(on_file_new), GTK_STOCK_NEW, GDK_CONTROL_MASK,
		GDK_n },
	{ N_("_Open"), G_CALLBACK(on_file_open), GTK_STOCK_OPEN,
		GDK_CONTROL_MASK, GDK_o },
	{ "", NULL, NULL, 0, 0 },
	{ N_("_Save"), G_CALLBACK(on_file_save), GTK_STOCK_SAVE,
		GDK_CONTROL_MASK, GDK_s },
	{ N_("_Save as..."), G_CALLBACK(on_file_save_as), GTK_STOCK_SAVE_AS,
		GDK_CONTROL_MASK | GDK_SHIFT_MASK, GDK_S },
	{ "", NULL, NULL, 0, 0 },
	{ N_("_Close"), G_CALLBACK(on_file_close), GTK_STOCK_CLOSE, 0, 0 },
	{ NULL, NULL, NULL, 0, 0 }
};

static DesktopMenu _editor_menu_edit[] =
{
	/* FIXME implement undo and redo */
	{ N_("_Undo"), NULL, GTK_STOCK_UNDO, GDK_CONTROL_MASK,
		GDK_z },
	{ N_("_Redo"), NULL, GTK_STOCK_REDO, GDK_CONTROL_MASK, GDK_r },
	{ "", NULL, NULL, 0, 0 },
	{ N_("_Cut"), G_CALLBACK(on_edit_cut), GTK_STOCK_CUT, GDK_CONTROL_MASK,
		GDK_x },
	{ N_("_Copy"), G_CALLBACK(on_edit_copy), GTK_STOCK_COPY,
		GDK_CONTROL_MASK, GDK_c },
	{ N_("_Paste"), G_CALLBACK(on_edit_paste), GTK_STOCK_PASTE,
		GDK_CONTROL_MASK, GDK_v },
	{ "", NULL, NULL, 0, 0 },
	{ N_("Select _all"), G_CALLBACK(on_edit_select_all),
#if GTK_CHECK_VERSION(2, 10, 0)
		GTK_STOCK_SELECT_ALL,
#else
		"edit-select-all",
#endif
		GDK_CONTROL_MASK, GDK_A },
	{ N_("_Unselect all"), G_CALLBACK(on_edit_unselect_all), NULL, 0, 0 },
	{ "", NULL, NULL, 0, 0 },
	{ N_("_Find"), G_CALLBACK(on_edit_find), GTK_STOCK_FIND,
		GDK_CONTROL_MASK, GDK_F },
	{ "", NULL, NULL, 0, 0 },
	{ N_("_Preferences"), G_CALLBACK(on_edit_preferences),
		GTK_STOCK_PREFERENCES, GDK_CONTROL_MASK, GDK_p },
	{ NULL, NULL, NULL, 0, 0 }
};

static DesktopMenu _editor_menu_help[] =
{
	{ N_("_About"), G_CALLBACK(on_help_about),
#if GTK_CHECK_VERSION(2, 6, 0)
		GTK_STOCK_ABOUT, 0, 0 },
#else
		NULL, 0, 0 },
#endif
	{ NULL, NULL, NULL, 0, 0 }
};

static DesktopMenubar _editor_menubar[] =
{
	{ N_("_File"), _editor_menu_file },
	{ N_("_Edit"), _editor_menu_edit },
	{ N_("_Help"), _editor_menu_help },
	{ NULL, NULL }
};
#endif

static DesktopToolbar _editor_toolbar[] =
{
	{ N_("New"), G_CALLBACK(on_new), GTK_STOCK_NEW, 0, 0, NULL },
	{ N_("Open"), G_CALLBACK(on_open), GTK_STOCK_OPEN, 0, 0, NULL },
	{ "", NULL, NULL, 0, 0, NULL },
	{ N_("Save"), G_CALLBACK(on_save), GTK_STOCK_SAVE, 0, 0, NULL },
	{ N_("Save as"), G_CALLBACK(on_save_as), GTK_STOCK_SAVE_AS, 0, 0,
		NULL },
#ifdef EMBEDDED
	{ "", NULL, NULL, 0, 0, NULL },
	{ N_("Preferences"), G_CALLBACK(on_preferences), GTK_STOCK_PREFERENCES,
		0, 0, NULL },
#endif
	{ NULL, NULL, NULL, 0, 0, NULL }
};


/* prototypes */
static char * _editor_config_filename(void);
static gboolean _editor_find(Editor * editor, char const * text,
		gboolean sensitive, gboolean wrap);


/* public */
/* functions */
/* editor_new */
static void _new_set_title(Editor * editor);

Editor * editor_new(void)
{
	Editor * editor;
	GtkAccelGroup * group;
	GtkWidget * vbox;
	GtkWidget * widget;
	PangoFontDescription * desc;

	if((editor = malloc(sizeof(*editor))) == NULL)
		return NULL;
	editor->config = config_new();
	if(editor->config == NULL)
	{
		editor_delete(editor);
		return NULL;
	}
	editor->filename = NULL;
	editor->search = 0;
	editor_config_load(editor);
	/* widgets */
	group = gtk_accel_group_new();
	editor->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_add_accel_group(GTK_WINDOW(editor->window), group);
	gtk_window_set_default_size(GTK_WINDOW(editor->window), 600, 400);
	_new_set_title(editor);
#if GTK_CHECK_VERSION(2, 6, 0)
	gtk_window_set_icon_name(GTK_WINDOW(editor->window), "text-editor");
#endif
	g_signal_connect_swapped(G_OBJECT(editor->window), "delete-event",
			G_CALLBACK(on_closex), editor);
	vbox = gtk_vbox_new(FALSE, 0);
	/* menubar */
#ifndef EMBEDDED
	widget = desktop_menubar_create(_editor_menubar, editor, group);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, FALSE, 0);
#else
	desktop_accel_create(_editor_accel, editor, group);
#endif
	/* toolbar */
	widget = desktop_toolbar_create(_editor_toolbar, editor, group);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, FALSE, 0);
	/* view */
	widget = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	editor->view = gtk_text_view_new();
	/* FIXME make it an option */
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(editor->view),
			GTK_WRAP_WORD_CHAR);
	desc = pango_font_description_from_string(editor_get_font(editor));
	editor_set_font(editor, pango_font_description_to_string(desc));
	pango_font_description_free(desc);
	gtk_container_add(GTK_CONTAINER(widget), editor->view);
	gtk_box_pack_start(GTK_BOX(vbox), widget, TRUE, TRUE, 0);
	/* statusbar */
	editor->statusbar = gtk_statusbar_new();
	gtk_box_pack_start(GTK_BOX(vbox), editor->statusbar, FALSE, FALSE, 0);
	/* preferences */
	editor->pr_window = NULL;
	/* find */
	editor->fi_dialog = NULL;
	/* about */
	editor->ab_window = NULL;
	gtk_container_add(GTK_CONTAINER(editor->window), vbox);
	gtk_window_set_focus(GTK_WINDOW(editor->window), editor->view);
	gtk_widget_show_all(editor->window);
	return editor;
}

static void _new_set_title(Editor * editor)
{
	char buf[256];

	snprintf(buf, sizeof(buf), "%s%s", _("Text editor - "), editor->filename
		       	== NULL ? _("(Untitled)") : editor->filename);
	gtk_window_set_title(GTK_WINDOW(editor->window), buf);
}


/* editor_delete */
void editor_delete(Editor * editor)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(editor->config != NULL)
		config_delete(editor->config);
	free(editor);
}


/* accessors */
/* editor_get_font */
char const * editor_get_font(Editor * editor)
{
	char const * p;
	char * q;
	GtkSettings * settings;
	PangoFontDescription * desc;

	if((p = config_get(editor->config, NULL, "font")) != NULL)
		return p;
	settings = gtk_settings_get_default();
	g_object_get(G_OBJECT(settings), "gtk-font-name", &q, NULL);
	if(q != NULL)
	{
		desc = pango_font_description_from_string(q);
		g_free(q);
		pango_font_description_set_family(desc, "monospace");
		q = pango_font_description_to_string(desc);
		config_set(editor->config, NULL, "font", q);
		g_free(q);
		pango_font_description_free(desc);
		if((p = config_get(editor->config, NULL, "font")) != NULL)
			return p;
	}
	return EDITOR_DEFAULT_FONT;
}


/* editor_set_font */
void editor_set_font(Editor * editor, char const * font)
{
	PangoFontDescription * desc;

	desc = pango_font_description_from_string(font);
	gtk_widget_modify_font(editor->view, desc);
	pango_font_description_free(desc);
	config_set(editor->config, NULL, "font", font);
}


/* useful */
/* editor_about */
static gboolean _about_on_closex(gpointer data);

void editor_about(Editor * editor)
{
	if(editor->ab_window != NULL)
	{
		gtk_widget_show(editor->ab_window);
		return;
	}
	editor->ab_window = desktop_about_dialog_new();
	gtk_window_set_transient_for(GTK_WINDOW(editor->ab_window),
			GTK_WINDOW(editor->window));
	desktop_about_dialog_set_authors(editor->ab_window, _authors);
	desktop_about_dialog_set_copyright(editor->ab_window, _copyright);
	desktop_about_dialog_set_logo_icon_name(editor->ab_window,
			"text-editor");
	desktop_about_dialog_set_license(editor->ab_window, _license);
	desktop_about_dialog_set_name(editor->ab_window, PACKAGE);
	desktop_about_dialog_set_version(editor->ab_window, VERSION);
	g_signal_connect_swapped(G_OBJECT(editor->ab_window), "delete-event",
			G_CALLBACK(_about_on_closex), editor);
	gtk_widget_show(editor->ab_window);
}

static gboolean _about_on_closex(gpointer data)
{
	Editor * editor = data;

	gtk_widget_hide(editor->ab_window);
	return TRUE;
}


/* editor_config_load */
void editor_config_load(Editor * editor)
{
	char * filename;

	if((filename = _editor_config_filename()) == NULL)
		return;
	config_load(editor->config, filename); /* we can ignore errors */
	free(filename);
}


/* editor_config_save */
void editor_config_save(Editor * editor)
{
	char * filename;

	if((filename = _editor_config_filename()) == NULL)
		return;
	if(config_save(editor->config, filename) != 0)
		editor_error(editor, _("Could not save configuration"), 0);
	free(filename);
}


/* editor_error */
int editor_error(Editor * editor, char const * message, int ret)
{
	GtkWidget * dialog;

	dialog = gtk_message_dialog_new(GTK_WINDOW(editor->window),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "%s",
#if GTK_CHECK_VERSION(2, 6, 0)
			_("Error"));
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
			"%s",
#endif
			message);
	gtk_window_set_title(GTK_WINDOW(dialog), _("Error"));
	g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(
				gtk_widget_destroy), NULL);
	gtk_widget_show(dialog);
	return ret;
}


/* editor_close */
gboolean editor_close(Editor * editor)
{
	GtkWidget * dialog;
	int res;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(gtk_text_buffer_get_modified(gtk_text_view_get_buffer(GTK_TEXT_VIEW(
						editor->view))) == FALSE)
	{
		gtk_main_quit();
		return FALSE;
	}
	dialog = gtk_message_dialog_new(GTK_WINDOW(editor->window),
			GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_WARNING, GTK_BUTTONS_NONE, "%s",
#if GTK_CHECK_VERSION(2, 6, 0)
			_("Warning"));
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
			"%s",
#endif
			_("There are unsaved changes.\n"
				"Discard or save them?"));
	gtk_dialog_add_buttons(GTK_DIALOG(dialog), GTK_STOCK_CANCEL,
			GTK_RESPONSE_CANCEL, GTK_STOCK_DISCARD,
			GTK_RESPONSE_REJECT, GTK_STOCK_SAVE,
			GTK_RESPONSE_ACCEPT, NULL);
	gtk_window_set_title(GTK_WINDOW(dialog), _("Warning"));
	res = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	if(res == GTK_RESPONSE_CANCEL)
		return TRUE;
	else if(res == GTK_RESPONSE_ACCEPT && editor_save(editor) != TRUE)
		return TRUE;
	gtk_main_quit();
	return FALSE;
}


/* editor_copy */
void editor_copy(Editor * editor)
{
	GtkTextBuffer * buffer;
	GtkClipboard * clipboard;

	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor->view));
	clipboard = gtk_widget_get_clipboard(editor->view,
			GDK_SELECTION_CLIPBOARD);
	gtk_text_buffer_copy_clipboard(buffer, clipboard);
}


/* editor_cut */
void editor_cut(Editor * editor)
{
	GtkTextBuffer * buffer;
	GtkClipboard * clipboard;

	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor->view));
	clipboard = gtk_widget_get_clipboard(editor->view,
			GDK_SELECTION_CLIPBOARD);
	gtk_text_buffer_cut_clipboard(buffer, clipboard, TRUE);
}


/* editor_find */
static void _find_dialog(Editor * editor);
static void _on_find_activate(GtkWidget * widget, gpointer data);
static void _on_find_response(GtkWidget * widget, gint response, gpointer data);

void editor_find(Editor * editor, char const * text)
{
	if(editor->fi_dialog == NULL)
		_find_dialog(editor);
	if(text != NULL)
		gtk_entry_set_text(GTK_ENTRY(editor->fi_text), text);
	gtk_window_present(GTK_WINDOW(editor->fi_dialog));
}

static void _find_dialog(Editor * editor)
{
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * widget;

	editor->fi_dialog = gtk_dialog_new_with_buttons(_("Find text"),
			GTK_WINDOW(editor->window),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
			GTK_STOCK_FIND, GTK_RESPONSE_ACCEPT, NULL);
	vbox = GTK_DIALOG(editor->fi_dialog)->vbox;
	hbox = gtk_hbox_new(FALSE, 0);
	widget = gtk_label_new(_("Text:"));
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	editor->fi_text = gtk_entry_new();
	g_signal_connect(G_OBJECT(editor->fi_text), "activate", G_CALLBACK(
				_on_find_activate), editor);
	gtk_box_pack_start(GTK_BOX(hbox), editor->fi_text, TRUE, TRUE, 4);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 4);
	editor->fi_case = gtk_check_button_new_with_label(_("Case-sensitive"));
	gtk_box_pack_start(GTK_BOX(vbox), editor->fi_case, TRUE, TRUE, 4);
	editor->fi_wrap = gtk_check_button_new_with_label(_("Wrap"));
	gtk_box_pack_start(GTK_BOX(vbox), editor->fi_wrap, TRUE, TRUE, 4);
	gtk_widget_show_all(vbox);
	g_signal_connect(G_OBJECT(editor->fi_dialog), "response", G_CALLBACK(
				_on_find_response), editor);
}

static void _on_find_activate(GtkWidget * widget, gpointer data)
{
	Editor * editor = data;
	char const * text;
	gboolean sensitive;
	gboolean wrap;

	if((text = gtk_entry_get_text(GTK_ENTRY(widget))) == NULL
			|| strlen(text) == 0)
		return;
	sensitive = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
				editor->fi_case));
	wrap = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
				editor->fi_wrap));
	if(_editor_find(editor, text, sensitive, wrap) == TRUE)
		return;
	editor_error(editor, _("Text not found"), 0);
}

static void _on_find_response(GtkWidget * widget, gint response, gpointer data)
{
	Editor * editor = data;

	if(response != GTK_RESPONSE_ACCEPT)
	{
		gtk_widget_hide(widget);
		if(response == GTK_RESPONSE_DELETE_EVENT)
			editor->fi_dialog = NULL;
		return;
	}
	_on_find_activate(editor->fi_text, editor);
}


/* editor_open */
void editor_open(Editor * editor, char const * filename)
{
	GtkWidget * dialog;
	int res;
	FILE * fp;
	GtkTextBuffer * tbuf;
	GtkTextIter iter;
	char buf[BUFSIZ];
	size_t len;
	char * p;
	size_t rlen;
	size_t wlen;
	GError * error = NULL;

	if(gtk_text_buffer_get_modified(gtk_text_view_get_buffer(GTK_TEXT_VIEW(
						editor->view))) == TRUE)
	{
		dialog = gtk_message_dialog_new(GTK_WINDOW(editor->window),
				GTK_DIALOG_MODAL
				| GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_WARNING, GTK_BUTTONS_NONE, "%s",
#if GTK_CHECK_VERSION(2, 6, 0)
				_("Warning"));
		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(
					dialog), "%s",
#endif
				_("There are unsaved changes.\n"
				"Are you sure you want to discard them?"));
		gtk_dialog_add_buttons(GTK_DIALOG(dialog), GTK_STOCK_CANCEL,
				GTK_RESPONSE_CANCEL,
#if GTK_CHECK_VERSION(2, 12, 0)
				GTK_STOCK_DISCARD,
#else
				_("Discard"),
#endif
				GTK_RESPONSE_CLOSE, NULL);
		gtk_window_set_title(GTK_WINDOW(dialog), _("Warning"));
		res = gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		if(res != GTK_RESPONSE_CLOSE)
			return;
	}
	tbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor->view));
	gtk_text_buffer_set_text(tbuf, "", 0);
	editor->search = 0;
	if(filename == NULL)
	{
		gtk_text_buffer_set_modified(tbuf, FALSE);
		return;
	}
	if((fp = fopen(filename, "r")) == NULL)
	{
		snprintf(buf, sizeof(buf), "%s: %s", filename, strerror(errno));
		editor_error(editor, buf, 1);
		return;
	}
	tbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor->view));
	gtk_text_buffer_set_text(tbuf, "", 0);
	while((len = fread(buf, sizeof(char), sizeof(buf), fp)) > 0)
	{
		gtk_text_buffer_get_end_iter(tbuf, &iter);
#if 0
		if((p = g_convert(buf, len, "UTF-8", "ISO-8859-15", &rlen, &wlen, NULL)) != NULL)
		{
			gtk_text_buffer_insert(tbuf, &iter, p, wlen);
			g_free(p);
		}
		else
			gtk_text_buffer_insert(tbuf, &iter, buf, len);
#else
		if((p = g_locale_to_utf8(buf, len, &rlen, &wlen, &error))
				!= NULL)
			/* FIXME may lose characters */
			gtk_text_buffer_insert(tbuf, &iter, p, wlen);
		else
			gtk_text_buffer_insert(tbuf, &iter, buf, len);
#endif
	}
	fclose(fp);
	gtk_text_buffer_set_modified(GTK_TEXT_BUFFER(gtk_text_view_get_buffer(
					GTK_TEXT_VIEW(editor->view))), FALSE);
	editor->filename = g_strdup(filename);
	_new_set_title(editor); /* XXX make it a generic private function */
}


/* editor_open_dialog */
void editor_open_dialog(Editor * editor)
{
	GtkWidget * dialog;
	char * filename = NULL;

	dialog = gtk_file_chooser_dialog_new(_("Open file..."),
			GTK_WINDOW(editor->window),
			GTK_FILE_CHOOSER_ACTION_OPEN,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(
					dialog));
	gtk_widget_destroy(dialog);
	if(filename == NULL)
		return;
	editor_open(editor, filename);
	g_free(filename);
}


/* editor_paste */
void editor_paste(Editor * editor)
{
	GtkTextBuffer * buffer;
	GtkClipboard * clipboard;

	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor->view));
	clipboard = gtk_widget_get_clipboard(editor->view,
			GDK_SELECTION_CLIPBOARD);
	gtk_text_buffer_paste_clipboard(buffer, clipboard, NULL, TRUE);
}


/* editor_save */
gboolean editor_save(Editor * editor)
{
	FILE * fp;
	GtkTextBuffer * tbuf;
	GtkTextIter start;
	GtkTextIter end;
	char * buf;
	size_t len;

	if(editor->filename == NULL)
		return editor_save_as_dialog(editor);
	if((fp = fopen(editor->filename, "w")) == NULL)
	{
		buf = g_strdup_printf("%s: %s", editor->filename, strerror(
					errno));
		editor_error(editor, buf, 1);
		g_free(buf);
		return FALSE;
	}
	tbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor->view));
	/* FIXME allocating the complete file is not optimal */
	gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(tbuf), &start);
	gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(tbuf), &end);
	buf = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(tbuf), &start, &end,
			FALSE);
	len = strlen(buf);
	if(fwrite(buf, sizeof(char), len, fp) != len)
	{
		g_free(buf);
		fclose(fp);
		editor_error(editor, _("Partial write"), 0);
		return FALSE;
	}
	g_free(buf);
	fclose(fp);
	gtk_text_buffer_set_modified(GTK_TEXT_BUFFER(tbuf), FALSE);
	return TRUE;
}


/* editor_save_as */
gboolean editor_save_as(Editor * editor, char const * filename)
{
	struct stat st;
	GtkWidget * dialog;
	int ret;

	if(stat(filename, &st) == 0)
	{
		dialog = gtk_message_dialog_new(GTK_WINDOW(editor->window),
				GTK_DIALOG_MODAL
				| GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO, "%s",
#if GTK_CHECK_VERSION(2, 6, 0)
				_("Warning"));
		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(
					dialog), "%s",
#endif
				_("This file already exists. Overwrite?"));
		gtk_window_set_title(GTK_WINDOW(dialog), _("Warning"));
		ret = gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		if(ret == GTK_RESPONSE_NO)
			return FALSE;
	}
	g_free(editor->filename);
	if((editor->filename = g_strdup(filename)) == NULL)
		return editor_error(editor, _("Allocation error"), FALSE);
	if(editor_save(editor) != TRUE)
		return FALSE;
	_new_set_title(editor);
	return TRUE;
}


/* editor_save_as_dialog */
gboolean editor_save_as_dialog(Editor * editor)
{
	GtkWidget * dialog;
	char * filename = NULL;
	gboolean ret;

	dialog = gtk_file_chooser_dialog_new(_("Save as..."),
			GTK_WINDOW(editor->window),
			GTK_FILE_CHOOSER_ACTION_SAVE,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(
					dialog));
	gtk_widget_destroy(dialog);
	if(filename == NULL)
		return FALSE;
	ret = editor_save_as(editor, filename);
	g_free(filename);
	return ret;
}


/* editor_select_all */
void editor_select_all(Editor * editor)
{
#if GTK_CHECK_VERSION(2, 4, 0)
	GtkTextBuffer * tbuf;
	GtkTextIter start;
	GtkTextIter end;

	tbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor->view));
	gtk_text_buffer_get_start_iter(tbuf, &start);
	gtk_text_buffer_get_end_iter(tbuf, &end);
	gtk_text_buffer_select_range(tbuf, &start, &end);
#endif
}


/* editor_unselect_all */
void editor_unselect_all(Editor * editor)
{
#if GTK_CHECK_VERSION(2, 4, 0)
	GtkTextBuffer * tbuf;
	GtkTextMark * mark;
	GtkTextIter iter;

	tbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor->view));
	mark = gtk_text_buffer_get_mark(tbuf, "insert");
	gtk_text_buffer_get_iter_at_mark(tbuf, &iter, mark);
	gtk_text_buffer_select_range(tbuf, &iter, &iter);
#endif
}


/* private */
/* functions */
/* editor_config_filename */
static char * _editor_config_filename(void)
{
	char const * homedir;
	size_t len;
	char * filename;

	if((homedir = getenv("HOME")) == NULL)
		homedir = g_get_home_dir();
	len = strlen(homedir) + 1 + sizeof(EDITOR_CONFIG_FILE);
	if((filename = malloc(len)) == NULL)
		return NULL;
	snprintf(filename, len, "%s/%s", homedir, EDITOR_CONFIG_FILE);
	return filename;
}


/* editor_find */
static char const * _find_string(char const * big, char const * little,
		gboolean sensitive);
static gboolean _find_match(Editor * editor, GtkTextBuffer * buffer,
		char const * buf, char const * str, size_t len);

static gboolean _editor_find(Editor * editor, char const * text,
		gboolean sensitive, gboolean wrap)
{
	gboolean ret = FALSE;
	size_t tlen;
	GtkTextBuffer * buffer;
	GtkTextIter start;
	GtkTextIter end;
	gchar * buf;
	size_t blen;
	char const * str;

	if(text == NULL || (tlen = strlen(text)) == 0)
		return ret;
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor->view));
	/* XXX highly inefficient */
	gtk_text_buffer_get_start_iter(buffer, &start);
	gtk_text_buffer_get_end_iter(buffer, &end);
	buf = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
	if(buf == NULL || (blen = strlen(buf)) == 0)
		return ret;
	if(editor->search >= blen)
		editor->search = 0;
	if((str = _find_string(&buf[editor->search], text, sensitive)) != NULL)
		ret = _find_match(editor, buffer, buf, str, tlen);
	else if(wrap && editor->search != 0) /* wrap around */
	{
		buf[editor->search] = '\0';
		if((str = _find_string(buf, text, sensitive)) != NULL)
			ret = _find_match(editor, buffer, buf, str, tlen);
	}
	g_free(buf);
	return ret;
}

static char const * _find_string(char const * big, char const * little,
		gboolean sensitive)
{
	return sensitive ? strstr(big, little) : strcasestr(big, little);
}

static gboolean _find_match(Editor * editor, GtkTextBuffer * buffer,
		char const * buf, char const * str, size_t len)
{
	size_t offset;
	GtkTextIter start;
	GtkTextIter end;

	offset = str - buf;
	editor->search = offset + 1;
	gtk_text_buffer_get_iter_at_offset(buffer, &start, offset);
	gtk_text_buffer_get_iter_at_offset(buffer, &end, offset + len);
	gtk_text_buffer_select_range(buffer, &start, &end);
	gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(editor->view), &start, 0.0,
			FALSE, 0.0, 0.0);
	return TRUE;
}
