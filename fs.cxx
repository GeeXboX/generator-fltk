/*
 *  Filesystem code for GeeXboX FLTK Generator
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

#include "fs.h"
#include "system.h"

#include <sys/types.h>
#include <sys/stat.h> /* fstat */
#include <fcntl.h> /* open creat */
#include <ctype.h> /* isspace */
#include <string.h> /* strchr strcmp strncpy strrchr */
#include <stdlib.h> /* free */

#ifdef _WIN32
#include <io.h> /* open read write close */
#else
#include <unistd.h> /* open read write close */
#endif

#include <FL/fl_ask.H> /* fl_alert */
#include <FL/filename.H> /* fl_filename_list */

/* this is required on windows */
#ifndef O_BINARY
#define O_BINARY (0)
#endif

int copy_errors = 0;

int file_exists(const char *file)
{
    struct stat st;
    if (stat(file, &st) < 0)
	return 0;
    return 1;
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
    char srcfile[100], dstfile[100];
    const char *fname;
    int num_files, i;
    dirent **files;
    int errors = 0;

    if ((num_files = fl_filename_list(srcdir, &files, NULL)) <= 0)
	return 0;

    for (i = 0; i < num_files; i++)
    {
	fname = files[i]->d_name;
	if (strcmp(fname, ".") && strcmp(fname, "./")
	    && strcmp(fname, "..") && strcmp(fname, "../")
	    && strcmp(fname, exclude))
	{ 
	    strcpy(srcfile, srcdir);
	    strcat(srcfile, fname);
	    strcpy(dstfile, dstdir);
	    strcat(dstfile, fname);

	    if (fl_filename_isdir(srcfile)) {
		my_mkdir(dstfile);
		strcat(srcfile, "/");
		strcat(dstfile, "/");
		errors += multi_copy(srcfile, dstfile, exclude);
	    } else {
		errors += copy_file(srcfile, dstfile);
	    }
	}
	free((void*)files[i]);
    }
    free((void*)files);

    return errors;
}

void multi_delete(const char *dir, const char *prefix, const char *suffix, const int recursive)
{
    char file[100];
    const char *fname;
    int num_files, i;
    dirent **files;

    if ((num_files = fl_filename_list(dir, &files, NULL)) <= 0)
	return;

    for (i = 0; i < num_files; i++)
    {
	fname = files[i]->d_name;
	if (strcmp(fname, ".") && strcmp(fname, "./")
	    && strcmp(fname, "..") && strcmp(fname, "../")
	    && (!prefix || !strncmp(prefix, fname, strlen(prefix)))
            && (!suffix || !strcmp(suffix, &fname[strlen(fname)-strlen(suffix)]))
	    )
	{ 
	    strcpy(file, dir);
	    strcat(file, fname);
	    if (fl_filename_isdir(file)) {
		if (recursive) {
		    strcat(file, "/");
		    multi_delete(file, prefix, suffix, recursive);
		    rmdir(file);
		}
	    } else {
		unlink(file);
	    }
	}
	free((void*)files[i]);
    }
    free((void*)files);
}
