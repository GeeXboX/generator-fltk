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
#include "configparser.h"
#include "dvdnav.h"
#include "system.h"

#include <FL/fl_ask.H> /* fl_alert */

#define yes_no(x) ((x) ? "yes" : "no")

int init_dvdnav_tab(GeneratorUI *ui)
{
    char buf[256];
    config_t *config;

    config = config_open(PATH_BASEISO "/etc/dvd", 1);
    if (!config) {
        fl_alert("Missing dvd configuration files.\n");
        return 0;
    }

    config_getvar(config, "DVDNAV", buf, sizeof(buf));
    ui->dvdnav_menu->value(!my_strcasecmp(buf, "yes"));
    ui->dvdnav_direct->value(my_strcasecmp(buf, "yes"));

    config_destroy(config);

    return 1;
}

int write_dvdnav_settings(GeneratorUI *ui)
{
    config_t *config;

    config = config_open(PATH_BASEISO "/etc/dvd", 1);
    if (!config) {
        fl_alert("Failed to write dvd configuration.\n");
        return 0;
    }

    config_setvar(config, "DVDNAV", yes_no(ui->dvdnav_menu->value()));

    config_write(config, PATH_BASEISO "/etc/dvd");
    config_destroy(config);

    return 1;
}
