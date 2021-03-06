/* $Id$ */
/* Copyright (c) 2004-2012 Pierre Pronchery <khorben@defora.org> */
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



#ifndef LIBC_UNISTD_H
# define LIBC_UNISTD_H

# include "compat/unistd.h"


/* constants */
/* for access() */
enum { F_OK, R_OK, W_OK, X_OK };

/* file streams */
# define STDIN_FILENO	0
# define STDOUT_FILENO	1
# define STDERR_FILENO	2


/* types */
# ifndef intptr_t
#  define intptr_t intptr_t
typedef signed long intptr_t;
# endif
# ifndef pid_t
#  define pid_t pid_t
typedef signed int pid_t;
# endif
# ifndef size_t
#  define size_t size_t
typedef unsigned long size_t;
# endif
# ifndef ssize_t
#  define ssize_t ssize_t
typedef signed int ssize_t;
# endif

# ifndef id_t
#  define id_t id_t
typedef unsigned int id_t;
# endif
# ifndef gid_t
#  define gid_t gid_t
typedef id_t gid_t;
# endif
# ifndef uid_t
#  define uid_t uid_t
typedef id_t uid_t;
# endif


/* variables */
extern char * optarg;
extern int optind, opterr, optopt;


/* functions */
int access(char const * filename, int mode);
unsigned int alarm(unsigned int seconds);
int brk(void * addr);
int chdir(char const * filename);
int chown(char const * filename, uid_t owner, gid_t group);
int close(int fildes);
int dup(int fildes);
int dup2(int fildes1, int fildes2);
int execl(char const * filename, char * const arg, ...);
int execle(char const * filename, char * const arg, ...);
int execlp(char const * filename, char * const arg, ...);
int execv(char const * filename, char * const argv[]);
int execve(char const * filename, char * const argv[], char * const envp[]);
int execvp(char const * filename, char * const argv[]);
void _exit(int status);
int fchdir(int fildes);
int fchown(int fildes, uid_t uid, gid_t gid);
pid_t fork(void);
int fsync(int fildes);
int ftruncate(int fildes, off_t offset);
char * getcwd(char * buf, size_t size);
gid_t getegid(void);
uid_t geteuid(void);
gid_t getgid(void);
int gethostname(char * name, size_t size);
char * getlogin(void);
int getlogin_r(char * buf, size_t size);
int getopt(int argc, char * const argv[], char const * optstring);
pid_t getpgrp(void);
pid_t getpid(void);
pid_t getppid(void);
uid_t getuid(void);
char * getwd(char * buf);
int isatty(int fildes);
int lchown(char const * filename, uid_t owner, gid_t group);
int link(char const * from, char const * to);
off_t lseek(int fildes, off_t offset, int whence);
int nice(int inc);
int pipe(int fildes[2]);
ssize_t read(int fildes, void * buf, size_t count);
ssize_t readlink(char const * filename, char * buf, size_t bufsiz);
int rmdir(char const * filename);
void * sbrk(intptr_t increment);
int setgid(gid_t gid);
int sethostname(char const * name, size_t size);
int setpgid(pid_t pid, pid_t pgid);
int setpgrp(void);
int setregid(gid_t rgid, gid_t egid);
int setreuid(uid_t ruid, uid_t euid);
pid_t setsid(void);
int setuid(uid_t uid);
unsigned int sleep(unsigned int seconds);
int symlink(char const * from, char const * to);
void sync(void);
long sysconf(int name);
int truncate(char const * filename, off_t offset);
char * ttyname(int fildes);
int unlink(char const * filename);
int usleep(useconds_t useconds);
pid_t vfork(void);
ssize_t write(int fildes, const void * buf, size_t count);

#endif /* !LIBC_UNISTD_H */
