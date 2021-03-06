/* $Id$ */
/* Copyright (c) 2009-2012 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop libDesktop */
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



#ifndef LIBDESKTOP_DESKTOP_ACCEL_H
# define LIBDESKTOP_DESKTOP_ACCEL_H


/* Accel */
/* types */
typedef struct _DesktopAccel
{
	GCallback callback;
	GdkModifierType modifier;
	unsigned int accel;
} DesktopAccel;


/* functions */
void desktop_accel_create(DesktopAccel const * accel, gpointer data,
		GtkAccelGroup * group);

#endif /* !LIBDESKTOP_DESKTOP_ACCEL_H */
