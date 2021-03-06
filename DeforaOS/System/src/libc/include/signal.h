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



#ifndef LIBC_SIGNAL_H
# define LIBC_SIGNAL_H

# include "compat/signal.h"


/* types */
# ifndef id_t
#  define id_t id_t
typedef unsigned int id_t;
# endif
# ifndef pid_t
#  define pid_t pid_t
typedef signed int pid_t;
# endif
# ifndef sig_atomic_t
#  define sig_atomic_t sig_atomic_t
typedef int sig_atomic_t;
# endif
# ifndef uid_t
#  define uid_t uid_t
typedef id_t uid_t;
# endif


/* functions */
int kill(pid_t pid, int sig);
int raise(int sig);
int sigaction(int sig, const struct sigaction * act, struct sigaction * oact);
int sigismember(sigset_t * set, int sig);
int sigprocmask(int how, const sigset_t * set, sigset_t * oset);
void (*signal(int sig, void (*func)(int)));

#endif /* !LIBC_SIGNAL_H */
