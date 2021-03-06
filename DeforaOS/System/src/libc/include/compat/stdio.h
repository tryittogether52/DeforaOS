/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS System libc */
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



#ifndef LIBC_COMPAT_STDIO_H
# define LIBC_COMPAT_STDIO_H


# if defined(__linux__)
#  include "kernel/linux/stdio.h"
# elif defined(__FreeBSD__)
#  include "kernel/freebsd/stdio.h"
# elif defined(__NetBSD__)
#  include "kernel/netbsd/stdio.h"
# elif defined(__OpenBSD__)
#  include "kernel/openbsd/stdio.h"
# elif defined(__sun__)
#  include "kernel/solaris/stdio.h"
# elif defined(__Whitix__)
#  include "kernel/whitix/stdio.h"
# else
#  warning Unsupported platform
# endif

#endif /* !LIBC_COMPAT_STDIO_H */
