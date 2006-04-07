/*
 *  DVDNav support for GeeXboX FLTK Generator
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
#include "dvdnav.h"
#include "fs.h"
#include "system.h"

#include <FL/fl_ask.H> /* fl_alert */

int init_dvdnav_tab(GeneratorUI *ui)
{
    if (!file_exists(PATH_BASEISO "/var/dvdnav")) {
        ui->dvdnav_direct->value(1);
        ui->dvdnav_menu->value(0);
    }
    else {
        ui->dvdnav_direct->value(0);
        ui->dvdnav_menu->value(1);
    }

    return 1;
}

int check_dvdnav_file(GeneratorUI *ui)
{
    FILE *fp;

    if (ui->dvdnav_menu->value()) {
        // write a void file
        fp = fopen(PATH_BASEISO "/var/dvdnav", "wb");
        if (!fp) {
            fl_alert("Failed to write dvdnav file.\n");
            return 0;
        }
        fclose(fp);
    }
    else
        unlink(PATH_BASEISO "/var/dvdnav");

    return 1;
}
