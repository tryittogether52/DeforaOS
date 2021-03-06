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



#include "errno.h"
#include "../syscalls.h"
#include "sys/stat.h"


/* chmod */
#ifndef SYS_chmod
# warning Unsupported platform: chmod() is missing
int chmod(char const * name, mode_t mode)
{
	errno = ENOSYS;
	return -1;
}
#endif


/* fchmod */
#ifndef SYS_fchmod
# warning Unsupported platform: fchmod() is missing
int fchmod(int fildes, mode_t mode)
{
	errno = ENOSYS;
	return -1;
}
#endif


/* fstat */
#ifndef SYS_fstat
# warning Unsupported platform: fstat() is missing
int fstat(int fildes, struct stat * st)
{
	errno = ENOSYS;
	return -1;
}
#endif


/* lstat */
#ifndef SYS_lstat
# warning Unsupported platform: lstat() is missing
int lstat(char const * pathname, struct stat * st)
{
	errno = ENOSYS;
	return -1;
}
#endif


/* mkdir */
#ifndef SYS_mkdir
# warning Unsupported platform: mkdir() is missing
int mkdir(char const * name, mode_t mode)
{
	errno = ENOSYS;
	return -1;
}
#endif


/* mkfifo */
#if !defined(SYS_mkfifo)
# if defined(SYS_mknod)
int mkfifo(char const * path, mode_t mode)
{
	return mknod(path, mode | S_IFIFO, 0);
}
# else /* !SYS_mkfifo && !SYS_mknod */
#  warning Unsupported platform: mkfifo() is missing
int mkfifo(char const * path, mode_t mode)
{
	errno = ENOSYS;
	return -1;
}
# endif
#endif /* !SYS_mkfifo */


/* mknod */
#ifndef SYS_mknod
# warning Unsupported platform: mknod() is missing
int mknod(char const * name, mode_t mode, dev_t dev)
{
	errno = ENOSYS;
	return -1;
}
#endif


/* stat */
#ifndef SYS_stat
# warning Unsupported platform: stat() is missing
int stat(char const * pathname, struct stat * st)
{
	errno = ENOSYS;
	return -1;
}
#endif


/* umask */
#ifndef SYS_umask
# warning Unsupported platform: umask() is missing
#endif
