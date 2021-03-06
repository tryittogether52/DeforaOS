/* $Id$ */
/* Copyright (c) 2011-2012 Pierre Pronchery <khorben@defora.org> */
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



#ifndef MAILER_SRC_FOLDER_H
# define MAILER_SRC_FOLDER_H

# include <gtk/gtk.h>
# include "../include/Mailer/plugin.h"
# include "../include/Mailer/folder.h"


/* Folder */
/* types */


/* functions */
Folder * folder_new(AccountFolder * folder, FolderType type, char const * name,
		GtkTreeStore * store, GtkTreeIter * iter);
void folder_delete(Folder * folder);

/* accessors */
AccountFolder * folder_get_data(Folder * folder);
gboolean folder_get_iter(Folder * folder, GtkTreeIter * iter);
GtkListStore * folder_get_messages(Folder * folder);

#endif /* !MAILER_SRC_MAILER_H */
