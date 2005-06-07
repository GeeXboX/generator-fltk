/*
 *  utils.h : GeeXboX Win32 generator shell utilities.
 *  Copyright (C) 2004-2005  Amir Shalem
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

#ifndef UTILS_H_
#define UTILS_H_

#include <stdio.h>

extern int copy_errors;

const char *find_basename(const char *str);
int file_exists(const char *file);

int _copy_file(const char *src, const char *dst, int append);
int copy_file(const char *src, const char *dst);

int multi_copy(const char *srcdir, const char *dstdir, const char *exclude);
void multi_delete(const char *dir, const char *prefix, const char *suffix, int recursive);

void replace_char (char *str, char o, char n);

int my_strcasecmp (const char *s1, const char *s2);
char *my_strdup (const char *str);
int my_mkdir(const char *path, int mode);

int nget_shvar_value (FILE *fp, const char *var, char *dst, size_t dstlen);

#define get_shvar_value(fp, var, dst) \
  nget_shvar_value ((fp), (var), (dst), sizeof(dst))

#endif
