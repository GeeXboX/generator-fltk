/*
 *  Isolinux support for GeeXboX FLTK Generator
 *  Copyright (C) 2008 Mathieu Schroeter
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

#ifndef isolinux_h
#define isolinux_h

#include <string>

typedef struct isolinux_s isolinux_t;

isolinux_t *isolinux_load(const char *path);
void isolinux_unload(isolinux_t *isolinux);
void isolinux_write(isolinux_t *isolinux, const char *path);

std::string isolinux_get_default(isolinux_t *isolinux);
void isolinux_set_default(isolinux_t *isolinux, std::string label);

#endif
