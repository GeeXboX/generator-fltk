/*
 *  System specfic code for GeeXboX FLTK Generator
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

#ifndef system_h
#define system_h

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h> /* mkdir */

#ifdef _WIN32
#include <windows.h> /* Sleep */
#endif

static inline int my_mkdir (const char *path, int mode)
{
#ifdef _WIN32
  return mkdir(path);
#else
  return mkdir(path, mode);
#endif
}

static inline void my_msleep (unsigned int mseconds)
{
#ifdef _WIN32
  Sleep(mseconds);
#else
  usleep(mseconds*1000);
#endif
}

int execute_bg_program(char *string);
void destroy_bg_program(void);

#endif
