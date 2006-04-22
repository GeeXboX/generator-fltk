/*
 *  Autoplay support for GeeXboX FLTK Generator
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
#include "autoplay.h"
#include "system.h"
#include "utils.h"

#include <FL/fl_ask.H> /* fl_alert */

#define yes_no(x) ((x) ? "yes" : "no")

int init_autoplay_tab(GeneratorUI *ui)
{
    char buf[256];
    config_t *config;

    config = config_open(PATH_BASEISO "/etc/autoplay", 1);
    if (!config) {
        fl_alert("Missing autoplay configuration files.\n");
        return 0;
    }

    config_getvar(config, "AUTOPLAY", buf, sizeof(buf));
    ui->media_autoplay->value(!my_strcasecmp(buf, "yes"));

    config_destroy(config);

    return 1;
}

int write_autoplay_settings(GeneratorUI *ui)
{
    config_t *config;

    config = config_open(PATH_BASEISO "/etc/autoplay", 1);
    if (!config) {
        fl_alert("Failed to write autoplay configuration.\n");
        return 0;
    }

    config_setvar(config, "AUTOPLAY", yes_no(ui->media_autoplay->value()));

    config_write(config, PATH_BASEISO "/etc/autoplay");
    config_destroy(config);

    return 1;
}
