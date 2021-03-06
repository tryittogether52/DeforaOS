/* $Id$ */
/* Copyright (c) 2011-2012 Pierre Pronchery <khorben@defora.org> */
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



#include <System.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <libintl.h>
#include "Browser.h"
#define _(string) gettext(string)
#define N_(string) (string)


/* CVS */
/* private */
/* types */
typedef struct _CVSTask CVSTask;

typedef struct _BrowserPlugin
{
	BrowserPluginHelper * helper;

	char * filename;

	guint source;

	/* widgets */
	GtkWidget * widget;
	GtkWidget * name;
	GtkWidget * status;
	/* directory */
	GtkWidget * directory;
	GtkWidget * d_root;
	GtkWidget * d_repository;
	GtkWidget * d_tag;
	/* file */
	GtkWidget * file;
	GtkWidget * f_revision;
	/* additional actions */
	GtkWidget * add;
	GtkWidget * make;

	/* tasks */
	CVSTask ** tasks;
	size_t tasks_cnt;
} CVS;

struct _CVSTask
{
	CVS * cvs;

	GPid pid;
	guint source;

	/* stdout */
	gint o_fd;
	GIOChannel * o_channel;
	guint o_source;

	/* stderr */
	gint e_fd;
	GIOChannel * e_channel;
	guint e_source;

	/* widgets */
	GtkWidget * window;
	GtkWidget * view;
	GtkWidget * statusbar;
	guint statusbar_id;
};


/* prototypes */
static CVS * _cvs_init(BrowserPluginHelper * helper);
static void _cvs_destroy(CVS * cvs);
static GtkWidget * _cvs_get_widget(CVS * cvs);
static void _cvs_refresh(CVS * cvs, GList * selection);

/* accessors */
static char * _cvs_get_entries(char const * pathname);
static char * _cvs_get_repository(char const * pathname);
static char * _cvs_get_root(char const * pathname);
static char * _cvs_get_tag(char const * pathname);
static gboolean _cvs_is_managed(char const * filename, char ** revision);

/* useful */
static int _cvs_add_task(CVS * cvs, char const * title,
		char const * directory, char * argv[]);

/* tasks */
static void _cvs_task_delete(CVSTask * task);
static void _cvs_task_set_status(CVSTask * task, char const * status);
static void _cvs_task_close(CVSTask * task);
static void _cvs_task_close_channel(CVSTask * task, GIOChannel * channel);

/* callbacks */
static void _cvs_on_add(gpointer data);
static void _cvs_on_annotate(gpointer data);
static void _cvs_on_commit(gpointer data);
static void _cvs_on_diff(gpointer data);
static void _cvs_on_log(gpointer data);
static void _cvs_on_make(gpointer data);
static void _cvs_on_update(gpointer data);
/* tasks */
static gboolean _cvs_task_on_closex(gpointer data);
static void _cvs_task_on_child_watch(GPid pid, gint status, gpointer data);
static gboolean _cvs_task_on_io_can_read(GIOChannel * channel,
		GIOCondition condition, gpointer data);

static void _rtrim(char * string);


/* public */
/* variables */
/* plug-in */
BrowserPluginDefinition plugin =
{
	N_("CVS"),
	"applications-development",
	NULL,
	_cvs_init,
	_cvs_destroy,
	_cvs_get_widget,
	_cvs_refresh
};


/* private */
/* functions */
/* cvs_init */
static GtkWidget * _init_button(GtkSizeGroup * group, char const * icon,
		char const * label, GCallback callback, gpointer data);
static GtkWidget * _init_label(GtkSizeGroup * group, char const * label,
		GtkWidget ** widget);

static CVS * _cvs_init(BrowserPluginHelper * helper)
{
	CVS * cvs;
	PangoFontDescription * font;
	GtkSizeGroup * group;
	GtkSizeGroup * bgroup;
	GtkWidget * widget;

	if((cvs = object_new(sizeof(*cvs))) == NULL)
		return NULL;
	cvs->helper = helper;
	cvs->filename = NULL;
	cvs->source = 0;
	/* widgets */
	cvs->widget = gtk_vbox_new(FALSE, 4);
	font = pango_font_description_new();
	pango_font_description_set_weight(font, PANGO_WEIGHT_BOLD);
	group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	bgroup = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	/* label */
	cvs->name = gtk_label_new("");
	gtk_label_set_ellipsize(GTK_LABEL(cvs->name), PANGO_ELLIPSIZE_MIDDLE);
	gtk_misc_set_alignment(GTK_MISC(cvs->name), 0.0, 0.0);
	gtk_widget_modify_font(cvs->name, font);
	gtk_box_pack_start(GTK_BOX(cvs->widget), cvs->name, FALSE, TRUE, 0);
	cvs->status = gtk_label_new("");
	gtk_label_set_ellipsize(GTK_LABEL(cvs->status), PANGO_ELLIPSIZE_END);
	gtk_misc_set_alignment(GTK_MISC(cvs->status), 0.0, 0.0);
	gtk_box_pack_start(GTK_BOX(cvs->widget), cvs->status, FALSE, TRUE, 0);
	/* directory */
	cvs->directory = gtk_vbox_new(FALSE, 4);
	widget = _init_label(group, _("Root:"), &cvs->d_root);
	gtk_box_pack_start(GTK_BOX(cvs->directory), widget, FALSE, TRUE, 0);
	widget = _init_label(group, _("Repository:"), &cvs->d_repository);
	gtk_box_pack_start(GTK_BOX(cvs->directory), widget, FALSE, TRUE, 0);
	widget = _init_label(group, _("Tag:"), &cvs->d_tag);
	gtk_box_pack_start(GTK_BOX(cvs->directory), widget, FALSE, TRUE, 0);
	widget = _init_button(bgroup, GTK_STOCK_INDEX, _("Request diff"),
			G_CALLBACK(_cvs_on_diff), cvs);
	gtk_box_pack_start(GTK_BOX(cvs->directory), widget, FALSE, TRUE, 0);
	widget = _init_button(bgroup, GTK_STOCK_INDEX, _("Annotate"),
			G_CALLBACK(_cvs_on_annotate), cvs);
	gtk_box_pack_start(GTK_BOX(cvs->directory), widget, FALSE, TRUE, 0);
	widget = _init_button(bgroup, GTK_STOCK_INDEX, _("View log"),
			G_CALLBACK(_cvs_on_log), cvs);
	gtk_box_pack_start(GTK_BOX(cvs->directory), widget, FALSE, TRUE, 0);
	widget = _init_button(bgroup, GTK_STOCK_REFRESH, _("Update"),
			G_CALLBACK(_cvs_on_update), cvs);
	gtk_box_pack_start(GTK_BOX(cvs->directory), widget, FALSE, TRUE, 0);
	widget = _init_button(bgroup, GTK_STOCK_JUMP_TO, _("Commit"),
			G_CALLBACK(_cvs_on_commit), cvs);
	gtk_box_pack_start(GTK_BOX(cvs->directory), widget, FALSE, TRUE, 0);
	gtk_widget_show_all(cvs->directory);
	gtk_widget_set_no_show_all(cvs->directory, TRUE);
	gtk_box_pack_start(GTK_BOX(cvs->widget), cvs->directory, FALSE, TRUE,
			0);
	/* file */
	cvs->file = gtk_vbox_new(FALSE, 4);
	widget = _init_label(group, _("Revision:"), &cvs->f_revision);
	gtk_box_pack_start(GTK_BOX(cvs->file), widget, FALSE, TRUE, 0);
	widget = _init_button(bgroup, GTK_STOCK_INDEX, _("Request diff"),
			G_CALLBACK(_cvs_on_diff), cvs);
	gtk_box_pack_start(GTK_BOX(cvs->file), widget, FALSE, TRUE, 0);
	widget = _init_button(bgroup, GTK_STOCK_INDEX, _("Annotate"),
			G_CALLBACK(_cvs_on_annotate), cvs);
	gtk_box_pack_start(GTK_BOX(cvs->file), widget, FALSE, TRUE, 0);
	widget = _init_button(bgroup, GTK_STOCK_INDEX, _("View log"),
			G_CALLBACK(_cvs_on_log), cvs);
	gtk_box_pack_start(GTK_BOX(cvs->file), widget, FALSE, TRUE, 0);
	widget = _init_button(bgroup, GTK_STOCK_REFRESH, _("Update"),
			G_CALLBACK(_cvs_on_update), cvs);
	gtk_box_pack_start(GTK_BOX(cvs->file), widget, FALSE, TRUE, 0);
	widget = _init_button(bgroup, GTK_STOCK_JUMP_TO, _("Commit"),
			G_CALLBACK(_cvs_on_commit), cvs);
	gtk_box_pack_start(GTK_BOX(cvs->file), widget, FALSE, TRUE, 0);
	gtk_widget_show_all(cvs->file);
	gtk_widget_set_no_show_all(cvs->file, TRUE);
	gtk_box_pack_start(GTK_BOX(cvs->widget), cvs->file, FALSE, TRUE, 0);
	/* additional actions */
	cvs->add = _init_button(bgroup, GTK_STOCK_ADD, _("Add to repository"),
			G_CALLBACK(_cvs_on_add), cvs);
	gtk_box_pack_start(GTK_BOX(cvs->widget), cvs->add, FALSE, TRUE, 0);
	cvs->make = _init_button(bgroup, GTK_STOCK_EXECUTE, _("Run make"),
			G_CALLBACK(_cvs_on_make), cvs);
	gtk_box_pack_start(GTK_BOX(cvs->widget), cvs->make, FALSE, TRUE, 0);
	gtk_widget_show_all(cvs->widget);
	pango_font_description_free(font);
	/* tasks */
	cvs->tasks = NULL;
	cvs->tasks_cnt = 0;
	return cvs;
}

static GtkWidget * _init_button(GtkSizeGroup * group, char const * icon,
		char const * label, GCallback callback, gpointer data)
{
	GtkWidget * hbox;
	GtkWidget * image;
	GtkWidget * widget;
	char const stock[] = "gtk-";

	hbox = gtk_hbox_new(FALSE, 4);
	widget = gtk_button_new_with_label(label);
	gtk_size_group_add_widget(group, widget);
	if(icon != NULL)
	{
		if(strncmp(icon, stock, sizeof(stock) - 1) == 0)
			image = gtk_image_new_from_stock(icon,
					GTK_ICON_SIZE_BUTTON);
		else
			image = gtk_image_new_from_icon_name(icon,
					GTK_ICON_SIZE_BUTTON);
		gtk_button_set_image(GTK_BUTTON(widget), image);
	}
	g_signal_connect_swapped(widget, "clicked", callback, data);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	return hbox;
}

static GtkWidget * _init_label(GtkSizeGroup * group, char const * label,
		GtkWidget ** widget)
{
	GtkWidget * hbox;

	hbox = gtk_hbox_new(FALSE, 4);
	*widget = gtk_label_new(label);
	gtk_misc_set_alignment(GTK_MISC(*widget), 0.0, 0.0);
	gtk_size_group_add_widget(group, *widget);
	gtk_box_pack_start(GTK_BOX(hbox), *widget, FALSE, TRUE, 0);
	*widget = gtk_label_new("");
	gtk_label_set_ellipsize(GTK_LABEL(*widget), PANGO_ELLIPSIZE_MIDDLE);
	gtk_misc_set_alignment(GTK_MISC(*widget), 0.0, 0.0);
	gtk_box_pack_start(GTK_BOX(hbox), *widget, TRUE, TRUE, 0);
	return hbox;
}


/* cvs_destroy */
static void _cvs_destroy(CVS * cvs)
{
	size_t i;

	for(i = 0; i < cvs->tasks_cnt; i++)
		_cvs_task_delete(cvs->tasks[i]);
	free(cvs->tasks);
	if(cvs->source != 0)
		g_source_remove(cvs->source);
	object_delete(cvs);
}


/* cvs_get_widget */
static GtkWidget * _cvs_get_widget(CVS * cvs)
{
	return cvs->widget;
}


/* cvs_refresh */
static void _refresh_dir(CVS * cvs);
static void _refresh_file(CVS * cvs);
static void _refresh_make(CVS * cvs, struct stat * st);
static void _refresh_status(CVS * cvs, char const * status);

static void _cvs_refresh(CVS * cvs, GList * selection)
{
	char * path = (selection != NULL) ? selection->data : NULL;
	struct stat st;
	gchar * p;

	if(cvs->source != 0)
		g_source_remove(cvs->source);
	free(cvs->filename);
	cvs->filename = NULL;
	if(lstat(path, &st) != 0)
		return;
	if((cvs->filename = strdup(path)) == NULL)
		return;
	p = g_filename_display_basename(path);
	gtk_label_set_text(GTK_LABEL(cvs->name), p);
	g_free(p);
	_refresh_status(cvs, NULL);
	gtk_widget_hide(cvs->directory);
	gtk_widget_hide(cvs->file);
	gtk_widget_hide(cvs->add);
	gtk_widget_hide(cvs->make);
	if(S_ISDIR(st.st_mode))
		_refresh_dir(cvs);
	else
		_refresh_file(cvs);
	_refresh_make(cvs, &st);
}

static void _refresh_dir(CVS * cvs)
{
	BrowserPluginHelper * helper = cvs->helper;
	char const dir[] = "CVS";
	size_t len = strlen(cvs->filename);
	char * p = NULL;
	struct stat st;
	gchar * q;
	char const * filename = cvs->filename;

	/* reset the interface */
	gtk_label_set_text(GTK_LABEL(cvs->d_root), NULL);
	gtk_label_set_text(GTK_LABEL(cvs->d_repository), NULL);
	gtk_label_set_text(GTK_LABEL(cvs->d_tag), NULL);
	/* consider "CVS" folders as managed */
	if((len = strlen(filename)) >= 4
			&& strcmp(&filename[len - 4], "/CVS") == 0)
	{
		if((p = strdup(filename)) != NULL)
		{
			p[len - 4] = '\0';
			filename = p;
		}
	}
	/* check if it is a CVS repository */
	else
	{
		len = strlen(filename) + sizeof(dir) + 1;
		if((p = malloc(len)) == NULL)
		{
			helper->error(helper->browser, strerror(errno), 1);
			return;
		}
		snprintf(p, len, "%s/%s", filename, dir);
		if(lstat(p, &st) != 0)
		{
			/* check if the parent folder is managed */
			if(_cvs_is_managed(filename, NULL) == FALSE)
				_refresh_status(cvs, _("Not a CVS repository"));
			else
			{
				_refresh_status(cvs, _("Not managed by CVS"));
				gtk_widget_show(cvs->add);
			}
			free(p);
			return;
		}
	}
	/* this folder is managed */
	gtk_widget_show(cvs->directory);
	/* obtain the CVS root */
	if((q = _cvs_get_root(filename)) != NULL)
	{
		gtk_label_set_text(GTK_LABEL(cvs->d_root), q);
		free(q);
	}
	/* obtain the CVS repository */
	if((q = _cvs_get_repository(filename)) != NULL)
	{
		gtk_label_set_text(GTK_LABEL(cvs->d_repository), q);
		free(q);
	}
	/* obtain the default CVS tag (if set) */
	if((q = _cvs_get_tag(filename)) != NULL)
	{
		if(q[0] == 'T' && q[1] != '\0')
			gtk_label_set_text(GTK_LABEL(cvs->d_tag), &q[1]);
		g_free(q);
	}
	free(p);
}

static void _refresh_file(CVS * cvs)
{
	char * revision = NULL;

	/* reset the interface */
	gtk_label_set_text(GTK_LABEL(cvs->f_revision), NULL);
	/* check if it is managed */
	if(_cvs_is_managed(cvs->filename, &revision) == FALSE)
		_refresh_status(cvs, _("Not a CVS repository"));
	else if(revision == NULL)
	{
		gtk_widget_show(cvs->add);
		_refresh_status(cvs, _("Not managed by CVS"));
	}
	else
	{
		gtk_widget_show(cvs->file);
		if(revision != NULL)
		{
			gtk_label_set_text(GTK_LABEL(cvs->f_revision),
					revision);
			free(revision);
		}
	}
}

static void _refresh_make(CVS * cvs, struct stat * st)
{
	gboolean show = FALSE;
	gchar * dirname;
	char const * makefile[] = { "Makefile", "makefile", "GNUmakefile" };
	size_t i;
	gchar * p;

	dirname = S_ISDIR(st->st_mode) ? g_strdup(cvs->filename)
		: g_path_get_dirname(cvs->filename);
	for(i = 0; show == FALSE && i < sizeof(makefile) / sizeof(*makefile);
			i++)
	{
		p = g_strdup_printf("%s/%s", dirname, makefile[i]);
		show = (lstat(p, st) == 0) ? TRUE : FALSE;
		g_free(p);
	}
	g_free(dirname);
	if(show)
		gtk_widget_show(cvs->make);
}

static void _refresh_status(CVS * cvs, char const * status)
{
	if(status == NULL)
		gtk_widget_hide(cvs->status);
	else
	{
		gtk_label_set_text(GTK_LABEL(cvs->status), status);
		gtk_widget_show(cvs->status);
	}
}


/* accessors */
/* cvs_get_entries */
static char * _cvs_get_entries(char const * pathname)
{
	char * ret = NULL;
	char const entries[] = "CVS/Entries";
	gchar * dirname;
	size_t len;
	char * p;

	dirname = g_path_get_dirname(pathname);
	len = strlen(dirname) + sizeof(entries) + 1;
	if((p = malloc(len)) == NULL)
		return NULL;
	snprintf(p, len, "%s/%s", dirname, entries);
	g_file_get_contents(p, &ret, NULL, NULL);
	free(p);
	g_free(dirname);
	return ret;
}


/* cvs_get_repository */
static char * _cvs_get_repository(char const * pathname)
{
	char * ret = NULL;
	char const repository[] = "CVS/Repository";
	size_t len;
	char * p;

	len = strlen(pathname) + sizeof(repository) + 1;
	if((p = malloc(len)) == NULL)
		return NULL;
	snprintf(p, len, "%s/%s", pathname, repository);
	if(g_file_get_contents(p, &ret, NULL, NULL) == TRUE)
		_rtrim(ret);
	free(p);
	return ret;
}


/* cvs_get_root */
static char * _cvs_get_root(char const * pathname)
{
	char * ret = NULL;
	char const root[] = "CVS/Root";
	size_t len;
	char * p;

	len = strlen(pathname) + sizeof(root) + 1;
	if((p = malloc(len)) == NULL)
		return NULL;
	snprintf(p, len, "%s/%s", pathname, root);
	if(g_file_get_contents(p, &ret, NULL, NULL) == TRUE)
		_rtrim(ret);
	free(p);
	return ret;
}


/* cvs_get_tag */
static char * _cvs_get_tag(char const * pathname)
{
	char * ret = NULL;
	char const tag[] = "CVS/Tag";
	char * p;
	size_t len;

	len = strlen(pathname) + sizeof(tag) + 1;
	if((p = malloc(len)) == NULL)
		return NULL;
	snprintf(p, len, "%s/%s", pathname, tag);
	if(g_file_get_contents(p, &ret, NULL, NULL) == TRUE)
		_rtrim(ret);
	free(p);
	return ret;
}


/* cvs_is_managed */
/* XXX returns if the directory is managed, set *revision if the file is */
static gboolean _cvs_is_managed(char const * pathname, char ** revision)
{
	char * entries;
	gchar * basename;
	size_t len;
	char const * s;
	char buf[256];

	/* obtain the CVS entries */
	if((entries = _cvs_get_entries(pathname)) == NULL)
		return FALSE;
	/* lookup the filename within the entries */
	basename = g_path_get_basename(pathname);
	len = strlen(basename);
	for(s = entries; s != NULL && s[0] != '\0'; s = strchr(s, '\n'))
	{
		if((s = strchr(s, '/')) == NULL)
			break;
		if(strncmp(++s, basename, len) != 0 || s[len] != '/')
			continue;
		s += len;
		if(sscanf(s, "/%255[^/]/", buf) != 1)
			break;
		buf[sizeof(buf) - 1] = '\0';
		break;
	}
	g_free(basename);
	g_free(entries);
	if(s != NULL && revision != NULL)
		*revision = strdup(buf);
	return TRUE;
}


/* useful */
/* cvs_add_task */
static int _cvs_add_task(CVS * cvs, char const * title,
		char const * directory, char * argv[])
{
	BrowserPluginHelper * helper = cvs->helper;
	CVSTask ** p;
	CVSTask * task;
	GSpawnFlags flags = G_SPAWN_SEARCH_PATH | G_SPAWN_DO_NOT_REAP_CHILD;
	gboolean res;
	GError * error = NULL;
	PangoFontDescription * font;
	char buf[256];
	GtkWidget * vbox;
	GtkWidget * widget;

	if((p = realloc(cvs->tasks, sizeof(*p) * (cvs->tasks_cnt + 1))) == NULL)
		return -helper->error(helper->browser, strerror(errno), 1);
	cvs->tasks = p;
	if((task = object_new(sizeof(*task))) == NULL)
		return -helper->error(helper->browser, error_get(), 1);
	task->cvs = cvs;
#ifdef DEBUG
	argv[0] = "echo";
#endif
	res = g_spawn_async_with_pipes(directory, argv, NULL, flags, NULL, NULL,
			&task->pid, NULL, &task->o_fd, &task->e_fd, &error);
	if(res != TRUE)
	{
		helper->error(helper->browser, error->message, 1);
		g_error_free(error);
		object_delete(task);
		return -1;
	}
	cvs->tasks[cvs->tasks_cnt++] = task;
	/* widgets */
	font = pango_font_description_new();
	pango_font_description_set_family(font, "monospace");
	task->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(task->window), 600, 400);
#if GTK_CHECK_VERSION(2, 6, 0)
	gtk_window_set_icon_name(GTK_WINDOW(task->window), plugin.icon);
#endif
	snprintf(buf, sizeof(buf), "%s - %s (%s)", _("CVS"), title, directory);
	gtk_window_set_title(GTK_WINDOW(task->window), buf);
	g_signal_connect_swapped(task->window, "delete-event", G_CALLBACK(
				_cvs_task_on_closex), task);
	vbox = gtk_vbox_new(FALSE, 0);
	widget = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	task->view = gtk_text_view_new();
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(task->view), FALSE);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(task->view), FALSE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(task->view),
			GTK_WRAP_WORD_CHAR);
	gtk_widget_modify_font(task->view, font);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(widget),
			task->view);
	gtk_box_pack_start(GTK_BOX(vbox), widget, TRUE, TRUE, 0);
	task->statusbar = gtk_statusbar_new();
	task->statusbar_id = 0;
	gtk_box_pack_start(GTK_BOX(vbox), task->statusbar, FALSE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(task->window), vbox);
	gtk_widget_show_all(task->window);
	pango_font_description_free(font);
	/* events */
	task->source = g_child_watch_add(task->pid, _cvs_task_on_child_watch,
			task);
	task->o_channel = g_io_channel_unix_new(task->o_fd);
	if((g_io_channel_set_encoding(task->o_channel, NULL, &error))
			!= G_IO_STATUS_NORMAL)
	{
		helper->error(helper->browser, error->message, 1);
		g_error_free(error);
	}
	task->o_source = g_io_add_watch(task->o_channel, G_IO_IN,
			_cvs_task_on_io_can_read, task);
	task->e_channel = g_io_channel_unix_new(task->e_fd);
	if((g_io_channel_set_encoding(task->e_channel, NULL, &error))
			!= G_IO_STATUS_NORMAL)
	{
		helper->error(helper->browser, error->message, 1);
		g_error_free(error);
	}
	task->e_source = g_io_add_watch(task->e_channel, G_IO_IN,
			_cvs_task_on_io_can_read, task);
	_cvs_task_set_status(task, _("Running command..."));
	return 0;
}


/* tasks */
/* cvs_task_delete */
static void _cvs_task_delete(CVSTask * task)
{
	_cvs_task_close(task);
	if(task->source != 0)
		g_source_remove(task->source);
	task->source = 0;
	gtk_widget_destroy(task->window);
	object_delete(task);
}


/* cvs_task_set_status */
static void _cvs_task_set_status(CVSTask * task, char const * status)
{
	GtkStatusbar * sb = GTK_STATUSBAR(task->statusbar);

	if(task->statusbar_id != 0)
		gtk_statusbar_remove(sb, gtk_statusbar_get_context_id(sb, ""),
				task->statusbar_id);
	task->statusbar_id = gtk_statusbar_push(sb,
			gtk_statusbar_get_context_id(sb, ""), status);
}


/* cvs_task_close */
static void _cvs_task_close(CVSTask * task)
{
	_cvs_task_close_channel(task, task->o_channel);
	_cvs_task_close_channel(task, task->e_channel);
}


/* cvs_task_close */
static void _cvs_task_close_channel(CVSTask * task, GIOChannel * channel)
{
	if(channel != NULL && channel == task->o_channel)
	{
		if(task->o_source != 0)
			g_source_remove(task->o_source);
		task->o_source = 0;
		g_io_channel_shutdown(task->o_channel, FALSE, NULL);
		g_io_channel_unref(task->o_channel);
		task->o_channel = NULL;
	}
	if(channel != NULL && task->e_channel != NULL)
	{
		if(task->e_source != 0)
			g_source_remove(task->e_source);
		task->e_source = 0;
		g_io_channel_shutdown(task->e_channel, FALSE, NULL);
		g_io_channel_unref(task->e_channel);
		task->e_channel = NULL;
	}
}


/* callbacks */
/* cvs_on_add */
static gboolean _add_is_binary(char const * type);

static void _cvs_on_add(gpointer data)
{
	CVS * cvs = data;
	gchar * dirname;
	gchar * basename;
	char * argv[] = { "cvs", "add", "--", NULL, NULL, NULL };
	Mime * mime;
	char const * type;

	if(cvs->filename == NULL)
		return;
	dirname = g_path_get_dirname(cvs->filename);
	basename = g_path_get_basename(cvs->filename);
	argv[3] = basename;
	mime = cvs->helper->get_mime(cvs->helper->browser);
	type = mime_type(mime, cvs->filename);
	if(_add_is_binary(type))
	{
		argv[4] = argv[3];
		argv[3] = argv[2];
		argv[2] = "-kb";
	}
	_cvs_add_task(cvs, "cvs add", dirname, argv);
	g_free(basename);
	g_free(dirname);
}

static gboolean _add_is_binary(char const * type)
{
	char const text[] = "text/";
	char const * types[] = { "application/x-perl",
		"application/x-shellscript",
		"application/xml",
		"application/xslt+xml" };
	size_t i;

	if(type == NULL)
		return TRUE;
	if(strncmp(text, type, sizeof(text) - 1) == 0)
		return FALSE;
	for(i = 0; i < sizeof(types) / sizeof(*types); i++)
		if(strcmp(types[i], type) == 0)
			return FALSE;
	return TRUE;
}


/* cvs_on_annotate */
static void _cvs_on_annotate(gpointer data)
{
	CVS * cvs = data;
	struct stat st;
	gchar * dirname;
	gchar * basename;
	char * argv[] = { "cvs", "annotate", "--", NULL, NULL };

	if(cvs->filename == NULL || lstat(cvs->filename, &st) != 0)
		return;
	dirname = S_ISDIR(st.st_mode) ? g_strdup(cvs->filename)
		: g_path_get_dirname(cvs->filename);
	basename = S_ISDIR(st.st_mode) ? NULL
		: g_path_get_basename(cvs->filename);
	argv[3] = basename;
	_cvs_add_task(cvs, "cvs annotate", dirname, argv);
	g_free(basename);
	g_free(dirname);
}


/* cvs_on_commit */
static void _cvs_on_commit(gpointer data)
{
	CVS * cvs = data;
	struct stat st;
	gchar * dirname;
	gchar * basename;
	char * argv[] = { "cvs", "commit", "--", NULL, NULL };

	if(cvs->filename == NULL || lstat(cvs->filename, &st) != 0)
		return;
	dirname = S_ISDIR(st.st_mode) ? g_strdup(cvs->filename)
		: g_path_get_dirname(cvs->filename);
	basename = S_ISDIR(st.st_mode) ? NULL
		: g_path_get_basename(cvs->filename);
	argv[3] = basename;
	_cvs_add_task(cvs, "cvs commit", dirname, argv);
	g_free(basename);
	g_free(dirname);
}


/* cvs_on_diff */
static void _cvs_on_diff(gpointer data)
{
	CVS * cvs = data;
	struct stat st;
	gchar * dirname;
	gchar * basename;
	char * argv[] = { "cvs", "diff", "--", NULL, NULL };

	if(cvs->filename == NULL || lstat(cvs->filename, &st) != 0)
		return;
	dirname = S_ISDIR(st.st_mode) ? g_strdup(cvs->filename)
		: g_path_get_dirname(cvs->filename);
	basename = S_ISDIR(st.st_mode) ? NULL
		: g_path_get_basename(cvs->filename);
	argv[3] = basename;
	_cvs_add_task(cvs, "cvs diff", dirname, argv);
	g_free(basename);
	g_free(dirname);
}


/* cvs_on_log */
static void _cvs_on_log(gpointer data)
{
	CVS * cvs = data;
	struct stat st;
	gchar * dirname;
	gchar * basename;
	char * argv[] = { "cvs", "log", "--", NULL, NULL };

	if(cvs->filename == NULL || lstat(cvs->filename, &st) != 0)
		return;
	dirname = S_ISDIR(st.st_mode) ? g_strdup(cvs->filename)
		: g_path_get_dirname(cvs->filename);
	basename = S_ISDIR(st.st_mode) ? NULL
		: g_path_get_basename(cvs->filename);
	argv[3] = basename;
	_cvs_add_task(cvs, "cvs log", dirname, argv);
	g_free(basename);
	g_free(dirname);
}


/* cvs_on_make */
static void _cvs_on_make(gpointer data)
{
	CVS * cvs = data;
	struct stat st;
	gchar * dirname;
	char * argv[] = { "make", NULL };

	if(cvs->filename == NULL || lstat(cvs->filename, &st) != 0)
		return;
	dirname = S_ISDIR(st.st_mode) ? g_strdup(cvs->filename)
		: g_path_get_dirname(cvs->filename);
	_cvs_add_task(cvs, "make", dirname, argv);
	g_free(dirname);
}


/* cvs_on_update */
static void _cvs_on_update(gpointer data)
{
	CVS * cvs = data;
	struct stat st;
	gchar * dirname;
	gchar * basename;
	char * argv[] = { "cvs", "update", "--", NULL, NULL };

	if(cvs->filename == NULL || lstat(cvs->filename, &st) != 0)
		return;
	dirname = S_ISDIR(st.st_mode) ? g_strdup(cvs->filename)
		: g_path_get_dirname(cvs->filename);
	basename = S_ISDIR(st.st_mode) ? NULL
		: g_path_get_basename(cvs->filename);
	argv[3] = basename;
	_cvs_add_task(cvs, "cvs update", dirname, argv);
	g_free(basename);
	g_free(dirname);
}


/* cvs_task_on_closex */
static gboolean _cvs_task_on_closex(gpointer data)
{
	CVSTask * task = data;

	gtk_widget_hide(task->window);
	_cvs_task_close(task);
	/* FIXME really implement */
	return TRUE;
}


/* cvs_task_on_child_watch */
static void _cvs_task_on_child_watch(GPid pid, gint status, gpointer data)
{
	CVSTask * task = data;
	char buf[256];

	task->source = 0;
	if(WIFEXITED(status))
	{
		snprintf(buf, sizeof(buf),
				_("Command exited with error code %d"),
				WEXITSTATUS(status));
		_cvs_task_set_status(task, buf);
	}
	else if(WIFSIGNALED(status))
	{
		snprintf(buf, sizeof(buf), _("Command exited with signal %d"),
				WTERMSIG(status));
		_cvs_task_set_status(task, buf);
	}
	g_spawn_close_pid(pid);
}


/* cvs_task_on_io_can_read */
static gboolean _cvs_task_on_io_can_read(GIOChannel * channel,
		GIOCondition condition, gpointer data)
{
	CVSTask * task = data;
	CVS * cvs = task->cvs;
	BrowserPluginHelper * helper = cvs->helper;
	char buf[256];
	gsize cnt = 0;
	GError * error = NULL;
	GIOStatus status;
	GtkTextBuffer * tbuf;
	GtkTextIter iter;

	if(condition != G_IO_IN)
		return FALSE;
	if(channel != task->o_channel && channel != task->e_channel)
		return FALSE;
	status = g_io_channel_read_chars(channel, buf, sizeof(buf), &cnt,
			&error);
	if(cnt > 0)
	{
		tbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(task->view));
		gtk_text_buffer_get_end_iter(tbuf, &iter);
		gtk_text_buffer_insert(tbuf, &iter, buf, cnt);
	}
	switch(status)
	{
		case G_IO_STATUS_NORMAL:
			break;
		case G_IO_STATUS_ERROR:
			helper->error(helper->browser, error->message, 1);
			g_error_free(error);
		case G_IO_STATUS_EOF:
		default: /* should not happen... */
			_cvs_task_close_channel(task, channel);
			return FALSE;
	}
	return TRUE;
}


/* rtrim */
static void _rtrim(char * string)
{
	size_t i;
	int c;

	if(string == NULL || (i = strlen(string)) == 0)
		return;
	for(i--; i > 0 && (c = string[i]) != '\0' && isspace(c); i--)
		string[i] = '\0';
}
