/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS System libc */
/* libc is not free software; you can redistribute it and/or modify it under
 * the terms of the Creative Commons Attribution-NonCommercial-ShareAlike 3.0
 * Unported as published by the Creative Commons organization.
 *
 * libc is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with libc; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



#ifndef LIBC_COMPAT_SYS_STAT_H
# define LIBC_COMPAT_SYS_STAT_H


# if defined(__linux__)
#  include "kernel/linux/sys/stat.h"
# elif defined(__NetBSD__)
#  include "kernel/netbsd/sys/stat.h"
# elif defined(__sun__)
#  include "kernel/solaris/sys/stat.h"
# else
#  warning Unsupported platform
# endif

#endif /* !LIBC_COMPAT_SYS_STAT_H */
