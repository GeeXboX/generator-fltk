/*
 *  Filesystem code for GeeXboX FLTK Generator
 *  Copyright (C) 2005-2006  Amir Shalem
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef fs_h
#define fs_h

extern int copy_errors;

int file_exists(const char *file);

int _copy_file(const char *src, const char *dst, int append);
int copy_file(const char *src, const char *dst);

int multi_copy(const char *srcdir, const char *dstdir, const char *exclude);
void multi_delete(const char *dir, const char *prefix, const char *suffix, int recursive);

#endif
