/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
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



#ifndef LIBC_KERNEL_LINUX_ERRNO_H
# define LIBC_KERNEL_LINUX_ERRNO_H


/* constants */
# define EPERM		1
# define ENOENT		2
# define ESRCH		3
# define EINTR		4
# define EIO		5
# define ENXIO		6
# define E2BIG		7
# define ENOEXEC	8
# define EBADF		9
# define ECHILD		10
# define EAGAIN		11
# define ENOMEM		12
# define EACCES		13
# define EFAULT		14
# define EBUSY		16
# define EEXIST		17
# define EXDEV		18
# define ENODEV		19
# define ENOTDIR	20
# define EISDIR		21
# define EINVAL		22
# define ENOTTY		25
# define ENOSPC		28
# define ESPIPE		29
# define EROFS		30
# define EPIPE		32
# define EDOM		33
# define ERANGE		34
# define ENOSYS		38
# define ENOTEMPTY	39
# define ELOOP		40
# define EPROTO		71
# define ENOTSUP	95
# define EADDRINUSE	98
# define EADDRNOTAVAIL	99
# define ENOBUFS	105
# define ETIMEDOUT	110
# define ECONNREFUSED	111

#endif /* !LIBC_KERNEL_LINUX_ERRNO_H */
