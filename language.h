/*
 *  Language support for GeeXboX FLTK Generator
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

#ifndef language_h
#define language_h

struct charset_info {
    char *name;
    char *codename;
    char *menu_font;
    char *sub_font;
};

struct lang_info {
    char *shortname;
    struct charset_info *c;
};

void change_font(Fl_Output *o, Fl_Check_Button *b);
int init_language_tab(GeneratorUI *ui);
int copy_language_files(GeneratorUI *ui);

#endif /* language_h */
