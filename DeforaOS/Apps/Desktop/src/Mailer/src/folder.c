/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Mailer */
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



#include <stdlib.h>
#ifdef DEBUG
# include <stdio.h>
#endif
#include <string.h>
#include <libintl.h>
#include <System.h>
#include "mailer.h"
#include "folder.h"
#define _(string) gettext(string)
#define N_(string) (string)


/* Folder */
/* private */
/* types */
struct _MailerFolder
{
	FolderType type;
	char * name;
	GtkTreeStore * store;
	GtkTreeRowReference * row;

	GtkListStore * messages;

	AccountFolder * data;		/* for account plug-ins */
};


/* prototypes */
static gboolean _folder_set(Folder * folder, MailerFolderColumn column,
		void * value);

static char const * _get_local_name(FolderType type, char const * name);


/* functions */
/* folder_new */
Folder * folder_new(AccountFolder * folder, FolderType type, char const * name,
		GtkTreeStore * store, GtkTreeIter * iter)
{
	Folder * ret;
	GtkTreePath * path;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, name);
#endif
	if((ret = object_new(sizeof(*ret))) == NULL)
		return NULL;
	name = _get_local_name(type, name);
	ret->name = string_new(name);
	ret->store = store;
	path = gtk_tree_model_get_path(GTK_TREE_MODEL(store), iter);
	ret->row = gtk_tree_row_reference_new(GTK_TREE_MODEL(store), path);
	gtk_tree_path_free(path);
	gtk_tree_store_set(store, iter, MFC_FOLDER, ret, MFC_NAME, name, -1);
	folder_set_type(ret, type);
	ret->messages = gtk_list_store_new(MHC_COUNT, G_TYPE_POINTER,
			G_TYPE_POINTER, G_TYPE_POINTER, GDK_TYPE_PIXBUF,
			G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
			G_TYPE_STRING, G_TYPE_STRING, G_TYPE_UINT,
			G_TYPE_STRING, G_TYPE_BOOLEAN, G_TYPE_UINT);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(ret->messages),
			MHC_DATE, GTK_SORT_DESCENDING);
	ret->data = folder;
	if(ret->name == NULL)
	{
		folder_delete(ret);
		return NULL;
	}
	return ret;
}


/* folder_delete */
void folder_delete(Folder * folder)
{
	gtk_tree_row_reference_free(folder->row);
	string_delete(folder->name);
	object_delete(folder);
}


/* accessors */
/* folder_get_data */
AccountFolder * folder_get_data(Folder * folder)
{
	return folder->data;
}


/* folder_get_iter */
gboolean folder_get_iter(Folder * folder, GtkTreeIter * iter)
{
	GtkTreePath * path;

	if((path = gtk_tree_row_reference_get_path(folder->row)) == NULL)
		return FALSE;
	return gtk_tree_model_get_iter(GTK_TREE_MODEL(folder->store), iter,
			path);
}


/* folder_get_messages */
GtkListStore * folder_get_messages(Folder * folder)
{
	return folder->messages;
}


/* folder_get_name */
char const * folder_get_name(Folder * folder)
{
	return folder->name;
}


/* folder_get_type */
FolderType folder_get_type(Folder * folder)
{
	return folder->type;
}


/* folder_set_type */
void folder_set_type(Folder * folder, FolderType type)
{
	GtkTreeIter iter;
	struct
	{
		FolderType type;
		char const * icon;
	} icons[FT_COUNT] =
	{
		{ FT_INBOX,	"mailer-inbox"		},
		{ FT_DRAFTS,	"mailer-drafts"		},
		{ FT_SENT,	"mailer-sent"		},
		{ FT_TRASH,	"gnome-stock-trash"	},
#if GTK_CHECK_VERSION(2, 6, 0)
		{ FT_FOLDER,	GTK_STOCK_DIRECTORY	}
#else
		{ FT_FOLDER,	"stock_directory"	}
#endif
	};
	size_t i;
	GtkIconTheme * theme;
	GdkPixbuf * pixbuf;

	folder->type = type;
	if(folder_get_iter(folder, &iter) != TRUE)
		return;
	for(i = 0; i < FT_LAST; i++)
		if(icons[i].type == type)
			break;
	theme = gtk_icon_theme_get_default();
	pixbuf = gtk_icon_theme_load_icon(theme, icons[i].icon, 16, 0, NULL);
	_folder_set(folder, MFC_ICON, pixbuf);
}


/* private */
/* folder_set */
static gboolean _folder_set(Folder * folder, MailerFolderColumn column,
		void * value)
{
	GtkTreeIter iter;

	if(folder_get_iter(folder, &iter) != TRUE)
		return FALSE;
	gtk_tree_store_set(folder->store, &iter, column, value, -1);
	return TRUE;
}


/* get_local_name */
static char const * _get_local_name(FolderType type, char const * name)
{
	struct
	{
		FolderType type;
		char const * name;
		char const * lname;
	} names[] =
	{
		{ FT_INBOX,	"Inbox",	N_("Inbox")	},
		{ FT_DRAFTS,	"Drafts",	N_("Drafts")	},
		{ FT_SENT,	"Sent",		N_("Sent")	},
		{ FT_TRASH,	"Trash",	N_("Trash")	}
	};
	size_t i;

	for(i = 0; i < sizeof(names) / sizeof(*names); i++)
		if(names[i].type == type && strcasecmp(names[i].name, name)
				== 0)
			return _(names[i].lname);
	return name;
}
