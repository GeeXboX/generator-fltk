/*
 *  cURL wrapper code for GeeXboX FLTK Generator
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

#ifndef curl_h
#define curl_h

#include <FL/Fl_Progress.H>
#include <FL/Fl_Button.H>
#include <curl/curl.h>
#include <sys/types.h>
#include <stdio.h>

int init_curl(void);
int download_progress(Fl_Button *, Fl_Progress *, const char *, curl_write_callback, void *);
int download_file(Fl_Button *, Fl_Progress *, const char *, char *, const char *);

#endif
