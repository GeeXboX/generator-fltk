/*
 *  Utilities for GeeXboX FLTK Generator
 *  Copyright (C) 2005  Amir Shalem
 *
 *   This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *   This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *   You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#include "utils.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h> /* open creat */
#include <unistd.h> /* fstat */
#include <ctype.h> /* isspace */
#include <string.h> /* strchr strcmp strncpy strrchr */
#include <dirent.h> /* opendir */
#include <stdlib.h> /* malloc */

#include <FL/fl_ask.H> /* fl_alert */

/* this is required on windows */
#ifndef O_BINARY
#define O_BINARY (0)
#endif

int copy_errors = 0;

int my_strcasecmp (const char *s1, const char *s2)
{
    const unsigned char
	*us1 = (const unsigned char *)s1,
	*us2 = (const unsigned char *)s2;

    while (tolower(*us1) == tolower(*us2++))
	if (*us1++ == '\0')
	    return (0);
    return (tolower(*us1) - tolower(*--us2));
}

char *my_strdup (const char *str)
{
    size_t len;
    char *copy;

    if ((copy = (char*)malloc((len = strlen(str) + 1))))
	memcpy(copy, str, len);
    return copy;
}

int my_mkdir (const char *path, int mode)
{
#ifdef __WIN32__
  return mkdir(path);
#else
  return mkdir(path, mode);
#endif
}

void replace_char (char *str, const char o, const char n)
{
    for (; *str; str++)
	if (*str == o)
	    *str = n;
}

int
nget_shvar_value (FILE *fp, const char *var, char *dst, size_t dstlen)
{
  static char buf[512];
  char *line, *value, *end;

  rewind (fp);
  while ((line = fgets (buf, sizeof (buf), fp)))
    {
      while (isspace (*line))
        line++;

      if (*line == '#' || (value = strchr (line, '=')) == NULL)
        continue;

      *value++ = '\0';

      if (strcmp (line, var))
        continue;

      if (*value == '\"' || *value == '\'')
        {
	  char strtype = *value++;
          end = strchr(value, strtype);
        }
      else
          for (end = value; !isspace(*end); end++)
	    ;

      *end = '\0';

      strncpy (dst, value, --dstlen);
      return 1;
    }

  *dst = '\0';
  return 0;
}

int file_exists(const char *file)
{
    struct stat st;
    if (stat(file, &st) < 0)
	return 0;
    return 1;
}

const char *find_basename(const char *file)
{
    const char *fname;

    fname = strrchr(file, '/');
    if (!fname)
	fname = strrchr(file, '\\');

    if (fname)
	fname++;
    else
	fname = file;

    return file;
}

int _copy_file(const char *src, const char *dst, int append)
{
    struct stat st;
    char buf[512];
    int fi, fo, x;

    fi = open(src, O_RDONLY | O_BINARY, 0);
    if (fi < 0)
	return 1;
    if (fstat(fi, &st) < 0) {
	close(fi);
	return 1;
    }
    fo = open(dst, append ? (O_APPEND | O_WRONLY | O_BINARY) : (O_CREAT | O_TRUNC | O_WRONLY | O_BINARY),
                        (int) (st.st_mode & 0777));
    if (fo < 0) {
	close(fi);
	return 1;
    }

    for (x = 1; x > 0;) 
    {
	x = read(fi, buf, 512);
	if (x > 0 && write(fo, buf, x) < x)
	{
	    close(fo);
	    close(fi);
	    unlink(dst);
	    return 1;
	}
    }

    close(fo);
    close(fi);

    return 0;
}

int copy_file(const char *src, const char *dst)
{
    int ret = _copy_file(src, dst, 0);
    if (ret && copy_errors++ < 3)
	fl_alert("Failed to copy file '%s'\n to '%s'.\n", src, dst);
    return ret;
}

int multi_copy(const char *srcdir, const char *dstdir, const char *exclude)
{
    struct stat st;
    char srcfile[100], dstfile[100];
    DIR *dirp;
    struct dirent *dp;
    int errors = 0;

    dirp = opendir(srcdir);
    if (!dirp)
	return 0;

    while ((dp = readdir(dirp)) != NULL)
    {
	if (strcmp(dp->d_name, ".")
	    && strcmp(dp->d_name, "..") 
	    && strcmp(dp->d_name, exclude))
	{ 
	    strcpy(srcfile, srcdir);
	    strcat(srcfile, dp->d_name);
	    strcpy(dstfile, dstdir);
	    strcat(dstfile, dp->d_name);

	    if (stat(srcfile, &st) < 0)
		continue;

	    if (S_ISDIR(st.st_mode)) {
		my_mkdir(dstfile, st.st_mode);
		strcat(srcfile, "/");
		strcat(dstfile, "/");
		errors += multi_copy(srcfile, dstfile, exclude);
	    } else {
		errors += copy_file(srcfile, dstfile);
	    }
	}
    }
    closedir(dirp);

    return errors;
}

void multi_delete(const char *dir, const char *prefix, const char *suffix, const int recursive)
{
    struct stat st;
    char file[100];
    DIR *dirp;
    struct dirent *dp;

    dirp = opendir(dir);
    if (!dirp)
	return;

    while ((dp = readdir(dirp)) != NULL)
    {
	if (strcmp(dp->d_name, ".")
	    && strcmp(dp->d_name, "..")
	    && (!prefix || !strncmp(prefix, dp->d_name, strlen(prefix)))
            && (!suffix || !strcmp(suffix, &dp->d_name[strlen(dp->d_name)-strlen(suffix)]))
	    )
	{ 
	    strcpy(file, dir);
	    strcat(file, dp->d_name);
	    if (stat(file, &st) < 0)
		continue;
	    if (S_ISDIR(st.st_mode)) {
		if (recursive) {
		    strcat(file, "/");
		    multi_delete(file, prefix, suffix, recursive);
		    rmdir(file);
		}
	    } else {
		unlink(file);
	    }
	}
    }
    closedir(dirp);
}
