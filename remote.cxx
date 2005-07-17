/*
 *  Remote support for GeeXboX FLTK Generator
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "generatorUI.h"

#include "config.h"
#include "fs.h"
#include "remote.h"

#include <sys/types.h>
#include <string.h> /* strcmp, strncmp, strlen */
#include <stdio.h> /* sprintf */
#include <stdlib.h> /* free */

#include <FL/fl_ask.H> /* fl_alert */
#include <FL/filename.H> /* fl_filename_list */

int init_remote_tab(GeneratorUI *ui)
{
    const char *fname;
    int num_files, i;
    dirent **files;
    const Fl_Menu_Item *m;

    if ((num_files = fl_filename_list("lirc/", &files, NULL)) > 0)
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

    if ((m = ui->lirc_receiver->find_item("atiusb")))
	ui->lirc_receiver->value(m);
    else
	ui->lirc_receiver->value(0);

    if ((m = ui->lirc_remote->find_item("atiusb")))
	ui->lirc_remote->value(m);
    else
	ui->lirc_remote->value(0);

    return 1;
}

int copy_remote_files(GeneratorUI *ui)
{
    char buf[256], buf2[256];
    const char *remote, *receiver;

    if (!ui->lirc_remote->mvalue()) {
	fl_alert("Please pick remote controler.\n");
	return 0;
    }

    if (!ui->lirc_receiver->mvalue()) {
	fl_alert("Please pick remote receiver.\n");
	return 0;
    }

    remote = ui->lirc_remote->mvalue()->label();
    receiver = ui->lirc_receiver->mvalue()->label();

    sprintf(buf, "lirc/lircrc_%s", remote);
    sprintf(buf2, PATH_BASEISO "/etc/lircrc");
    copy_file(buf, buf2);

    sprintf(buf, "lirc/lircd_%s", receiver);
    sprintf(buf2, PATH_BASEISO "/etc/lircd");
    copy_file(buf, buf2);

    sprintf(buf, "lirc/lircd_%s.conf", remote);
    sprintf(buf2, PATH_BASEISO "/etc/lircd.conf");
    copy_file(buf, buf2);

    return 1;
}
