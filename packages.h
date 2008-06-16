/*
 *  External packages support for GeeXboX FLTK Generator
 *  Copyright (C) 2005-2006  Amir Shalem
 *  Copyright (C) 2006-2008  Mathieu Schroeter
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

#ifndef packages_h
#define packages_h

void package_license_agree(GeneratorUI *ui, int);
void package_license_exit(GeneratorUI *ui);
void package_download(GeneratorUI *ui, int run_compile);

int init_packages_tab(GeneratorUI *ui);
int write_packages_settings(GeneratorUI *ui);
int is_package_downloaded(Flu_Tree_Browser::Node *n);

#endif
