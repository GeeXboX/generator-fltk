/*
 *  Extra files support for GeeXboX FLTK Generator
 *  Copyright (C) 2006-2007 Mathieu Schroeter
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

#ifndef extrafiles_h
#define extrafiles_h

typedef struct {
    char *path;
    unsigned int size;
} Extrafile;

void add_files(Flu_Tree_Browser *tree, GeneratorUI *ui);
void add_folders(Flu_Tree_Browser *tree, GeneratorUI *ui);
void remove_nodes(Flu_Tree_Browser *tree, GeneratorUI *ui);
int init_extrafiles_tab(GeneratorUI *ui);

#endif
