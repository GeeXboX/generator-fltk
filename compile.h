/*
 *  Compliation code for GeeXboX FLTK Generator
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

#ifndef compile_h
#define compile_h

typedef enum {
  TARGET_ARCH_I386,
  TARGET_ARCH_POWERPC
} target_arch_t;

extern target_arch_t target_arch;
const char *get_target_arch_string(void);

int init_compile(GeneratorUI *ui);
int compile_iso(GeneratorUI *ui);
void cleanup_compile(void);

int find_geexbox_tree(const char *prog);
const char *find_program(const char *prog);
int tree_corrupted(void);

#endif
