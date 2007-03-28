/* $Id$ */
/* Copyright (c) 2007 The DeforaOS Project */



#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <errno.h>


/* types */
typedef int Prefs;
#define PREFS_f 0x01
#define PREFS_i 0x02
#define PREFS_p 0x04
#define PREFS_R 0x08
#define PREFS_H 0x10
#define PREFS_L 0x20
#define PREFS_P 0x40


/* cp */
static int _cp_error(char const * message, int ret);
static int _cp_single(Prefs * prefs, char const * src, char const * dst);
static int _cp_symlink(char const * src, char const * dst);
static int _cp_multiple(Prefs * prefs, int filec, char * const filev[]);

static int _cp(Prefs * prefs, int filec, char * filev[])
{
	struct stat st;

	if(stat(filev[filec - 1], &st) != 0)
	{
		if(errno != ENOENT)
			return _cp_error(filev[filec - 1], 1);
		if(filec > 2)
		{
			fprintf(stderr, "%s%s%s", "cp: ", filev[filec - 1],
					": Does not exist and more than two"
					" operands were given\n");
			return 1;
		}
	}
	else if(S_ISDIR(st.st_mode))
		return _cp_multiple(prefs, filec, filev);
	else if(filec > 2)
	{
		errno = ENOTDIR;
		return _cp_error(filev[filec - 1], 1);
	}
	return _cp_single(prefs, filev[0], filev[1]);
}

static int _cp_error(char const * message, int ret)
{
	fputs("cp: ", stderr);
	perror(message);
	return ret;
}

/* _cp_single */
static int _single_recurse(Prefs * prefs, char const * src, char const * dst);
static FILE * _single_open_dst(Prefs * prefs, char const * dst);

static int _cp_single(Prefs * prefs, char const * src, char const * dst)
{
	int ret = 0;
	FILE * fsrc;
	FILE * fdst;
	char buf[BUFSIZ];
	size_t size;
	int fd;
	struct stat st;

	if((fsrc = fopen(src, "r")) == NULL)
		return _cp_error(src, 1);
	if((fd = fileno(fsrc)) == -1 || fstat(fd, &st) != 0)
	{
		_cp_error(src, 1);
		fclose(fsrc);
		return 1;
	}
	if(S_ISDIR(st.st_mode))
	{
		if(*prefs & PREFS_R)
			return _single_recurse(prefs, src, dst);
		fprintf(stderr, "%s%s%s", "cp: ", src,
				": Omitting directory\n");
		return 0;
	}
	if(S_ISLNK(st.st_mode) && (*prefs & PREFS_P))
		return _cp_symlink(src, dst);
	if((fdst = _single_open_dst(prefs, dst)) == NULL)
	{
		fclose(fsrc);
		return 1;
	}
	while((size = fread(buf, sizeof(char), BUFSIZ, fsrc)) > 0)
		if(fwrite(buf, sizeof(char), size, fdst) != size)
		{
			_cp_error(dst, 1);
			fclose(fsrc);
			fclose(fdst);
			return 1;
		}
	if(!feof(fsrc))
		ret = _cp_error(src, 1);
	if(fclose(fsrc) != 0)
		ret = _cp_error(src, 1);
	if(fclose(fdst) != 0)
		return _cp_error(dst, 1);
	return ret;
}

static int _single_recurse(Prefs * prefs, char const * src, char const * dst)
{
	int ret = 0;
	Prefs prefs2 = *prefs;
	size_t srclen;
	size_t dstlen;
	DIR * dir;
	struct dirent * de;
	char * ssrc = NULL;
	char * sdst = NULL;
	char * p;
#ifndef DT_DIR
	struct stat st;
#endif

	if(mkdir(dst, 0777) != 0 && errno != EEXIST)
		return _cp_error(dst, 1);
	srclen = strlen(src);
	dstlen = strlen(dst);
	if((dir = opendir(src)) == NULL)
		return _cp_error(src, 1);
	prefs2 |= (prefs2 & PREFS_H) ? PREFS_P : 0;
	while((de = readdir(dir)) != 0)
	{
		if(de->d_name[0] == '.' && (de->d_name[1] == '\0'
					|| (de->d_name[1] == '.'
						&& de->d_name[2] == '\0')))
			continue;
		if((p = realloc(ssrc, srclen + strlen(de->d_name) + 2)) == NULL)
		{
			ret |= _cp_error(src, 1);
			continue;
		}
		ssrc = p;
		sprintf(ssrc, "%s/%s", src, de->d_name);
		if((p = realloc(sdst, dstlen + strlen(de->d_name) + 2)) == NULL)
		{
			ret |= _cp_error(ssrc, 1);
			continue;
		}
		sdst = p;
		sprintf(sdst, "%s/%s", dst, de->d_name);
#ifdef DT_DIR
		if(de->d_type == DT_DIR)
			ret |= _single_recurse(&prefs2, ssrc, sdst);
		else if(de->d_type == DT_LNK && (*prefs & PREFS_P))
#else
		if(lstat(ssrc, &st) != 0) /* XXX TOCTOU */
		{
			ret |= _cp_error(ssrc, 1);
			continue;
		}
		if(S_ISDIR(st.st_mode))
			ret |= _single_recurse(&prefs2, ssrc, sdst);
		else if(S_ISLNK(st.st_mode) && (*prefs & PREFS_P))
#endif
			ret |= _cp_symlink(ssrc, sdst);
		else
			ret |= _cp_single(&prefs2, ssrc, sdst);
	}
	closedir(dir);
	free(ssrc);
	free(sdst);
	return ret;
}

static int _single_confirm(char const * message);
static FILE * _single_open_dst(Prefs * prefs, char const * dst)
{
	FILE * fp;
	int fd;

	if(*prefs & PREFS_f)
	{
		if((fp = fopen(dst, "w")) == NULL)
			_cp_error(dst, 1);
		return fp;
	}
	if((fd = open(dst, O_WRONLY | O_CREAT | O_EXCL, 0666)) < 0)
	{
		if(errno != EEXIST)
		{
			_cp_error(dst, 1);
			return NULL;
		}
		if(!_single_confirm(dst))
			return NULL;
		/* XXX TOCTOU */
		if((fd = open(dst, O_WRONLY | O_CREAT | O_TRUNC, 0666)) < 0)
		{
			_cp_error(dst, 1);
			return NULL;
		}
	}
	if((fp = fdopen(fd, "w")) == NULL)
		_cp_error(dst, 1);
	return fp;
}

static int _single_confirm(char const * message)
{
	int c;
	int tmp;

	fprintf(stderr, "%s%s%s", "cp: ", message, ": Overwrite? ");
	if((c = fgetc(stdin)) == EOF)
		return 0;
	while(c != '\n' && (tmp = fgetc(stdin)) != EOF && tmp != '\n');
	return c == 'y';
}

static int _cp_symlink(char const * src, char const * dst)
{
	char buf[PATH_MAX + 1];
	ssize_t len;

	if((len = readlink(src, buf, sizeof(buf) - 1)) == -1)
		return _cp_error(src, 1);
	buf[len] = '\0';
	if(symlink(buf, dst) != 0) /* FIXME fails if dst already exists */
		return _cp_error(dst, 1);
	return 0;
}

static int _cp_multiple(Prefs * prefs, int filec, char * const filev[])
{
	int ret = 0;
	int i;
	char * dst;
	size_t len;
	char * sdst = NULL;
	char * p;

	for(i = 0; i < filec - 1; i++)
	{
		dst = basename(filev[i]);
		len = strlen(filev[i]) + strlen(dst) + 2;
		if((p = realloc(sdst, len * sizeof(char))) == NULL)
		{
			ret |= _cp_error(filev[filec - 1], 1);
			continue;
		}
		sdst = p;
		sprintf(sdst, "%s/%s", filev[filec - 1], dst);
		ret |= _cp_single(prefs, filev[i], sdst);
	}
	free(sdst);
	return ret;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: cp [-fip] source_file target_file\n\
       cp [-fip] source_file ... target\n\
       cp -R [-H | -L | -P][-fip] source_file ... target\n\
       cp -r [-H | -L | -P][-fip] source_file ... target\n\
  -f    attempt to remove destination file before a copy if necessary\n\
  -i    prompt before a copy to an existing file\n\
  -p    duplicate characteristics of the source files\n\
  -R    copy file hierarchies\n\
  -r    copy file hierarchies\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	Prefs prefs;
	int o;

	memset(&prefs, 0, sizeof(Prefs));
	prefs |= PREFS_i | PREFS_H;
	while((o = getopt(argc, argv, "fipRrHLP")) != -1)
		switch(o)
		{
			case 'f':
				prefs -= prefs & PREFS_i;
				prefs |= PREFS_f;
				break;
			case 'i':
				prefs -= prefs & PREFS_f;
				prefs |= PREFS_i;
				break;
			case 'p':
				prefs |= PREFS_p;
				break;
			case 'R':
			case 'r':
				prefs |= PREFS_R;
				break;
			case 'H':
				prefs -= prefs & (PREFS_L | PREFS_P);
				prefs |= PREFS_H;
				break;
			case 'L':
				prefs -= prefs & (PREFS_H | PREFS_P);
				prefs |= PREFS_L;
				break;
			case 'P':
				prefs -= prefs & (PREFS_H | PREFS_L);
				prefs |= PREFS_P;
				break;
			default:
				return _usage();
		}
	if(argc - optind < 2)
		return _usage();
	return _cp(&prefs, argc - optind, &argv[optind]) == 0 ? 0 : 2;
}
