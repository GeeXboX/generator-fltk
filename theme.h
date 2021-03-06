/*
 *  Theme support for GeeXboX FLTK Generator
 *  Copyright (C) 2005-2006  Amir Shalem
 *  Copyright (C) 2006  Mathieu Schroeter
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

#ifndef theme_h
#define theme_h

int init_theme_tab(GeneratorUI *ui);
void update_theme_tab(GeneratorUI *ui);
char *valid_theme_font(const char *theme_name, struct charset_info *c);
int copy_theme_files(GeneratorUI *ui);
int copy_theme_boot_files(GeneratorUI *ui);

#endif
