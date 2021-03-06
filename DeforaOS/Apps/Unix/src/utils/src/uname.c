/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Unix utils */
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



#include <sys/utsname.h>
#include <unistd.h>
#include <stdio.h>


/* uname */
static int _uname(int m, int n, int r, int s, int v)
{
	struct utsname buf;
	int spacing = 0;

	if(uname(&buf) != 0)
	{
		perror("uname");
		return 1;
	}
	if(s && (spacing = 1))
		printf("%s", buf.sysname);
	if(n)
		printf("%s%s", spacing++ ? " " : "", buf.nodename);
	if(r)
		printf("%s%s", spacing++ ? " " : "", buf.release);
	if(v)
		printf("%s%s", spacing++ ? " " : "", buf.version);
	if(m)
		printf("%s%s", spacing++ ? " " : "", buf.machine);
	if(spacing == 0)
		printf("%s", buf.sysname);
	putchar('\n');
	return 0;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: uname [-snrvma]\n\
  -s	Operating system name\n\
  -n	Name of this node on the network\n\
  -r	Operating system release name\n\
  -v	Operating system version\n\
  -m	Hardware type\n\
  -a	All the options above\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int flagm = 0;
	int flagn = 0;
	int flagr = 0;
	int flags = 0;
	int flagv = 0;
	int o;

	while((o = getopt(argc, argv, "amnrsv")) != -1)
		switch(o)
		{
			case 'a':
				flagm = 1;
				flagn = 1;
				flagr = 1;
				flags = 1;
				flagv = 1;
				break;
			case 'm':
				flagm = 1;
				break;
			case 'n':
				flagn = 1;
				break;
			case 'r':
				flagr = 1;
				break;
			case 's':
				flags = 1;
				break;
			case 'v':
				flagv = 1;
				break;
			default:
				return _usage();
		}
	if(optind != argc)
		return _usage();
	return _uname(flagm, flagn, flagr, flags, flagv) == 0 ? 0 : 2;
}
