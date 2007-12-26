/*
 *  Ndiswrapper support for GeeXboX FLTK Generator
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

#include "compile.h"
#include "config.h"
#include "configparser.h"
#include "fs.h"
#include "ndiswrapper.h"
#include "system.h"

#include <FL/fl_ask.H> /* fl_alert */
#include <FL/Fl_File_Chooser.H> /* fl_file_chooser */

#include <string>

const char *path_ndiswrapper;

static void load_driver_node(Flu_Tree_Browser *tree, std::string inf, int copy)
{
    char buf[512];
    std::string src, dst, dir;
    std::string drivername, filename;
    std::string dst_dir = PATH_BASEISO "/etc/ndiswrapper";
    int size;
    Flu_Tree_Browser::Node *n;

    size = inf.find_last_of("/", inf.length() - 1);
    drivername = inf.substr(size + 1, inf.length() - size - 5);
    dir = inf.substr(0, size + 1);

    if (tree->find(drivername.c_str()) == NULL) {
        if (!fl_filename_isdir(dst_dir.c_str()))
            my_mkdir(dst_dir.c_str());

        if (copy) {
            snprintf(buf, sizeof(buf), "%s -i \"%s\" -o \"%s\" -a", path_ndiswrapper, inf.c_str(), dst_dir.c_str());
            if (execute_bg_program(buf) != 0) {
                fl_alert("Error with the INF file processing!\n");
                return;
            }
        }
        // add node to the browser tree
        n = tree->add(drivername.c_str());
        n->auto_label_color(true);
        Fl::redraw();
    }
    else
        fl_alert("This driver is already loaded!\n");
}

static void unload_driver_node(Flu_Tree_Browser *tree, Flu_Tree_Browser::Node *n)
{
    multi_delete(PATH_BASEISO "/etc/ndiswrapper/", NULL, NULL, 1);
    tree->remove(n);
    Fl::redraw();
}

void load_drvwin32(Flu_Tree_Browser *tree)
{
    const char *new_driver;
    Flu_Tree_Browser::Node *n;

    n = tree->get_root();
    if (n->children() == 0) {
        new_driver = fl_file_chooser("Choose win32 (Windows XP) driver?", "*.inf", "");
        if (new_driver) {
            if (strstr(new_driver, ".inf") != NULL || strstr(new_driver, ".INF") != NULL)
                load_driver_node(tree, new_driver, 1);
            else
                fl_alert("You haven't chosen a .inf file!\n");
        }
    }
    else
        fl_alert("Only one driver can be loaded!\n");
}

void unload_drvwin32(Flu_Tree_Browser *tree)
{
    Flu_Tree_Browser::Node *n;

    n = tree->get_root();
    if (n->children() != 0) {
        n = tree->first_leaf();
        unload_driver_node(tree, n);
    }
}

static std::string search_driver(const char *path)
{
    const char *fname;
    int i, num_files;
    std::string ret = "";
    struct dirent **files;

    if (fl_filename_isdir(path)) {
        num_files = fl_filename_list(path, &files, NULL);
        for (i = 0; i < num_files; i++) {
            fname = files[i]->d_name;
            if (fl_filename_isdir(fname) &&
                strcmp(fname, ".") != 0 &&
                strcmp(fname, "./") != 0 &&
                strcmp(fname, "..") != 0 &&
                strcmp(fname, "../") != 0)
            {
                ret = fname;
                ret.resize(ret.length() - 1);
                break;
            }
        }
        for (i = num_files; i > 0;)
            free((void*)(files[--i]));
        free((void*)files);
    }

    return ret;
}

int init_ndiswrapper_tab(GeneratorUI *ui)
{
    std::string inf;
    std::string driver_name;

    Flu_Tree_Browser *tree = ui->drvwin32_tree;

    if (target_arch == TARGET_ARCH_I386) {
        path_ndiswrapper = find_program("ndiswrapper");

        // Init browser tree for win32 drivers
        tree->get_root()->always_open(true);
        tree->when(FL_WHEN_NOT_CHANGED);
        tree->show_root(true);
        tree->label("Windows driver");
        tree->animate(false);
        tree->selection_mode(FLU_SINGLE_SELECT);
        tree->insertion_mode(FLU_INSERT_BACK);

        driver_name = search_driver(PATH_BASEISO "/etc/ndiswrapper");
        if (driver_name != "") {
            inf = PATH_BASEISO "/etc/ndiswrapper/" + driver_name + "/" + driver_name + ".inf";
            load_driver_node(tree, inf, 0);
        }
    }
    else
        ui->ndiswrapper_subtab->deactivate();

    return 1;
}
