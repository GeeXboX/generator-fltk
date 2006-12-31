/*
 *  Keymap support for GeeXboX FLTK Generator
 *  Copyright (C) 2006 Mathieu Schroeter
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
#include "configparser.h"
#include "compile.h"
#include "fs.h"
#include "keymap.h"

#include <stdlib.h> /* free */

#include <FL/fl_ask.H> /* fl_alert */
#include <FL/filename.H> /* fl_filename_list */

int init_keymap_tab(GeneratorUI *ui)
{
    char keymap[256];
    int num_files, i;
    struct dirent **files;
    const Fl_Menu_Item *m;

    ui->keymap->add("querty");
    if ((num_files = fl_filename_list(PATH_BASEISO "/etc/keymaps/", &files, NULL)) > 0)
    {
        for (i = 0; i < num_files; i++)
        {
            if (strcmp(".", files[i]->d_name) && strcmp("./", files[i]->d_name) &&
                strcmp("..", files[i]->d_name) && strcmp("../", files[i]->d_name))
            {
                ui->keymap->add(files[i]->d_name);
            }
            free((void*)files[i]);
        }
        free((void*)files);
    }

    if (target_arch == TARGET_ARCH_I386) {
        config_t *config;

        config = config_open(PATH_BASEISO "/boot/isolinux.cfg", 0);
        if (!config) {
            fl_alert("Missing isolinux configuration files.\n");
            return 0;
        }

        config_getvar(config, "keymap", keymap, sizeof(keymap));

        config_destroy(config);
    }
    else if (target_arch == TARGET_ARCH_PPC)
    {
        config_t *config;

        config = config_open(PATH_BASEISO "/boot/yaboot.conf", 0);
        if (!config) {
            fl_alert("Missing yaboot configuration files.\n");
            return 0;
        }

        config_getvar(config, "keymap", keymap, sizeof(keymap));

        config_destroy(config);
    }

    if ((m = ui->keymap->find_item(keymap)))
        ui->keymap->value(m);
    else if ((m = ui->keymap->find_item("querty")))
        ui->keymap->value(m);
    else
        ui->keymap->value(0);

    return 1;
}

int write_keymap_settings(GeneratorUI *ui)
{
    if (target_arch == TARGET_ARCH_I386) {
        config_t *config, *config2;

        config = config_open(PATH_BASEISO "/boot/isolinux.cfg", 0);
        config2 = config_open(PATH_BASEISO "/boot/pxelinux.cfg/default", 0);
        if (!config || !config2) {
            fl_alert("Failed to open for write isolinux configuration.\n");
            return 0;
        }

        config_setvar(config, "keymap", ui->keymap->mvalue()->label());
        config_setvar(config2, "keymap", ui->keymap->mvalue()->label());

        config_write(config, PATH_BASEISO "/boot/isolinux.cfg");
        config_write(config2, PATH_BASEISO "/boot/pxelinux.cfg/default");
        config_destroy(config);
        config_destroy(config2);
    }
    else if (target_arch == TARGET_ARCH_PPC)
    {
        config_t *config, *config2;

        config = config_open(PATH_BASEISO "/boot/yaboot.conf", 0);
        config2 = config_open(PATH_BASEISO "/boot/netboot/yaboot.conf", 0);
        if (!config || !config2) {
            fl_alert("Failed to open for write yaboot configuration.\n");
            return 0;
        }

        config_setvar(config, "keymap", ui->keymap->mvalue()->label());
        config_setvar(config2, "keymap", ui->keymap->mvalue()->label());

        config_write(config, PATH_BASEISO "/boot/yaboot.conf");
        config_write(config2, PATH_BASEISO "/boot/netboot/yaboot.conf");
        config_destroy(config);
        config_destroy(config2);
    }

    return 1;
}
