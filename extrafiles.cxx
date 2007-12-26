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

#include "generatorUI.h"

#include "config.h"
#include "generator.h"
#include "extrafiles.h"

#include <FL/fl_ask.H> /* fl_alert */
#include <FL/Fl_File_Chooser.H> /* fl_file_chooser fl_dir_chooser */

#include <string>
#include <sys/stat.h> /* stat */

int init_extrafiles_tab(GeneratorUI *ui)
{
    Flu_Tree_Browser *tree = ui->extrafiles_tree;

    tree->get_root()->always_open(true);
    tree->show_root(true);
    tree->label("iso");
    tree->animate(false);
    tree->selection_mode(FLU_SINGLE_SELECT);
    tree->insertion_mode(FLU_INSERT_SORTED);

    return 1;
}

static void add_file_node(Flu_Tree_Browser *tree, std::string file)
{
    int size;
    std::string filename, dir;
    Flu_Tree_Browser::Node *n;
    Extrafile *e = NULL;
    struct stat st;

    size = file.find_last_of("/", file.length() - 1);
    filename = file.substr(size + 1, file.length() - size - 1);

    if (tree->find(filename.c_str()) == NULL) {
        n = tree->add(filename.c_str());
        n->widget(new Fl_Check_Button(0, 0, 20, 20));
        n->swap_label_and_widget(true);
        n->auto_label_color(true);
        stat(file.c_str(), &st);
        e = new Extrafile;
        e->path = strdup(file.c_str());
        e->size = st.st_size;
        n->user_data(e);
    }
    else
        fl_alert("This file already exists!\n");
}

static void add_folder_node(Flu_Tree_Browser *tree, std::string dir)
{
    int i, size, num_files;
    std::string dirname, new_dir, dir_node;
    struct dirent **files;
    Flu_Tree_Browser::Node *n;
    Extrafile *e = NULL;
    struct stat st;

    if (dir.substr(dir.length() - 1) != "/")
        dir.append("/");

    size = dir.find_last_of("/", dir.length() - 2);
    dirname = dir.substr(size + 1, dir.length() - size);

    if (tree->find(dirname.c_str()) == NULL) {
        n = tree->add(dirname.c_str());
        n->widget(new Fl_Check_Button(0, 0, 20, 20));
        n->swap_label_and_widget(true);
        n->auto_label_color(true);
        e = new Extrafile;
        e->path = strdup(dir.c_str());
        *(e->path + strlen(e->path) - 1) = '\0';
        e->size = 0;
        n->user_data(e);
        num_files = fl_filename_list(dir.c_str(), &files, NULL);
        for (i = 0; i < num_files; i++) {
            new_dir = dir + files[i]->d_name;
            if (!fl_filename_isdir(new_dir.c_str())) {
                dir_node = dirname + files[i]->d_name;
                n = tree->add(dir_node.c_str());
                n->widget(new Fl_Check_Button(0, 0, 20, 20));
                n->swap_label_and_widget(true);
                n->auto_label_color(true);
                stat(new_dir.c_str(), &st);
                e = new Extrafile;
                e->path = NULL;
                e->size = st.st_size;
                n->user_data(e);
            }
            free((void*)files[i]);
        }
        free((void*)files);
    }
    else
        fl_alert("This directory already exists!\n");
}

static void refresh_size(GeneratorUI *ui)
{
    Flu_Tree_Browser::Node *n;
    Extrafile *e;
    float size = 0;
    char t_size[16];

    for (n = ui->extrafiles_tree->first(); n; n = n->next()) {
        e = (Extrafile*)n->user_data();
        if (e)
            size += e->size;
    }
    if (!size)
        t_size[0] = '\0';
    else
        snprintf(t_size, sizeof(t_size), "%0.3f MB", size / (1024 * 1024));
    ui->extrafiles_size->static_value(strdup(t_size));
}

void add_files(Flu_Tree_Browser *tree, GeneratorUI *ui)
{
    const char *new_file;
    Flu_Tree_Browser::Node *n;

    n = tree->get_root();
    new_file = fl_file_chooser("Choose a file?", "", "");
    if (new_file) {
        add_file_node(tree, new_file);
        Fl::redraw();
        refresh_size(ui);
    }
}

void add_folders(Flu_Tree_Browser *tree, GeneratorUI *ui)
{
    const char *new_dir;
    Flu_Tree_Browser::Node *n;

    n = tree->get_root();
    new_dir = fl_dir_chooser("Choose a directory?", "");
    if (new_dir) {
        add_folder_node(tree, new_dir);
        Fl::redraw();
        refresh_size(ui);
    }
}

void remove_nodes(Flu_Tree_Browser *tree, GeneratorUI *ui)
{
    Flu_Tree_Browser::Node *n;
    Fl_Button *b;

    n = tree->first();
    while (n) {
        b = (Fl_Button*)n->widget();
        if (b && b->value()) {
            tree->remove(n);
            n = tree->first();
            Fl::redraw();
            refresh_size(ui);
        }
        else
            n = n->next();
    }
}
