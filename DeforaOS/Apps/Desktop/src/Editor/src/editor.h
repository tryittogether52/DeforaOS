/* $Id$ */
/* Copyright (c) 2006-2012 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Editor */
/* This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>. */



#ifndef EDITOR_EDITOR_H
# define EDITOR_EDITOR_H

# include <System.h>
# include <gtk/gtk.h>


/* Editor */
/* types */
typedef struct _Editor Editor;


/* functions */
Editor * editor_new(void);
void editor_delete(Editor * editor);

/* accessors */
char const * editor_get_font(Editor * editor);
GtkWrapMode editor_get_wrap_mode(Editor * editor);
void editor_set_font(Editor * editor, char const * font);
void editor_set_wrap_mode(Editor * editor, GtkWrapMode wrap);

/* useful */
void editor_about(Editor * editor);

int editor_config_load(Editor * editor);
int editor_config_save(Editor * editor);

int editor_confirm(Editor * editor, char const * message, ...);
int editor_error(Editor * editor, char const * message, int ret);

gboolean editor_close(Editor * editor);
void editor_open(Editor * editor, char const * filename);
void editor_open_dialog(Editor * editor);

void editor_copy(Editor * editor);
void editor_cut(Editor * editor);
void editor_paste(Editor * editor);

int editor_insert_file(Editor * editor, char const * filename);
int editor_insert_file_dialog(Editor * editor);

gboolean editor_save(Editor * editor);
gboolean editor_save_as(Editor * editor, char const * filename);
gboolean editor_save_as_dialog(Editor * editor);

void editor_select_all(Editor * editor);
void editor_unselect_all(Editor * editor);

void editor_find(Editor * editor, char const * text);

void editor_show_preferences(Editor * editor, gboolean show);

#endif /* !EDITOR_EDITOR_H */
