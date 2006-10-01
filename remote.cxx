/*
 *  Remote support for GeeXboX FLTK Generator
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

#include "generatorUI.h"

#include "config.h"
#include "configparser.h"
#include "compile.h"
#include "fs.h"
#include "remote.h"

#include <stdlib.h> /* free */

#include <FL/fl_ask.H> /* fl_alert */
#include <FL/filename.H> /* fl_filename_list */

int init_remote_tab(GeneratorUI *ui)
{
    char remote[256], receiver[256];
    const char *fname;
    int num_files, i;
    struct dirent **files;
    const Fl_Menu_Item *m;

    if ((num_files = fl_filename_list(PATH_BASEISO "/etc/lirc/", &files, NULL)) > 0)
    {
	for (i = 0; i < num_files; i++)
	{
	    fname = files[i]->d_name;
	    if (!strncmp("lircd_", fname, 6) &&
		    strcmp(".conf", &fname[strlen(fname)-5]))
		ui->lirc_receiver->add(&fname[6]);
	    else if (!strncmp("lircrc_", fname, 7))
		ui->lirc_remote->add(&fname[7]);
	    free((void*)files[i]);
	}
	free((void*)files);
    }

    if (ui->lirc_receiver->size() < 1 || ui->lirc_remote->size() < 1) {
	fl_alert("Missing remote/receiver configuration files.\n");
	return 0;
    }

    if (target_arch == TARGET_ARCH_I386) {
        config_t *config;

        config = config_open(PATH_BASEISO "/boot/isolinux.cfg", 0);
        if (!config) {
            fl_alert("Missing isolinux configuration files.\n");
            return 0;
        }

        config_getvar(config, "receiver", receiver, sizeof(receiver));
        config_getvar(config, "remote", remote, sizeof(remote));

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

        config_getvar(config, "receiver", receiver, sizeof(receiver));
        config_getvar(config, "remote", remote, sizeof(remote));

        config_destroy(config);
    }

    if ((m = ui->lirc_receiver->find_item(receiver)))
	ui->lirc_receiver->value(m);
    else if ((m = ui->lirc_receiver->find_item("atiusb")))
	ui->lirc_receiver->value(m);
    else
	ui->lirc_receiver->value(0);

    if ((m = ui->lirc_remote->find_item(remote)))
	ui->lirc_remote->value(m);
    else if ((m = ui->lirc_remote->find_item("atiusb")))
	ui->lirc_remote->value(m);
    else
	ui->lirc_remote->value(0);

    return 1;
}

int write_remote_settings(GeneratorUI *ui)
{
    if (target_arch == TARGET_ARCH_I386) {
        config_t *config, *config2;

        config = config_open(PATH_BASEISO "/boot/isolinux.cfg", 0);
        config2 = config_open(PATH_BASEISO "/boot/pxelinux.cfg/default", 0);
        if (!config || !config2) {
            fl_alert("Failed to open for write isolinux configuration.\n");
            return 0;
        }

        config_setvar(config, "receiver", ui->lirc_receiver->mvalue()->label());
        config_setvar(config2, "receiver", ui->lirc_receiver->mvalue()->label());
        config_setvar(config, "remote", ui->lirc_remote->mvalue()->label());
        config_setvar(config2, "remote", ui->lirc_remote->mvalue()->label());

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

        config_setvar(config, "receiver", ui->lirc_receiver->mvalue()->label());
        config_setvar(config2, "receiver", ui->lirc_receiver->mvalue()->label());
        config_setvar(config, "remote", ui->lirc_remote->mvalue()->label());
        config_setvar(config2, "remote", ui->lirc_remote->mvalue()->label());

        config_write(config, PATH_BASEISO "/boot/yaboot.conf");
        config_write(config2, PATH_BASEISO "/boot/netboot/yaboot.conf");
        config_destroy(config);
        config_destroy(config2);
    }

    return 1;
}

