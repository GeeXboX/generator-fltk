/*
 *  MPlayer support for GeeXboX FLTK Generator
 *  Copyright (C) 2008 Mathieu Schroeter
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
#include "mplayer.h"
#include "system.h"

#include <FL/fl_ask.H> /* fl_alert */

#define yes_no(x) ((x) ? "yes" : "no")

int init_mplayer_tab(GeneratorUI *ui)
{
    char buf[256];
    config_t *config;

    config = config_open(PATH_BASEISO "/etc/mplayer/mplayer.conf", 0);
    if (!config) {
        fl_alert("Missing MPlayer configuration files.\n");
        return 0;
    }

    config_getvar(config, "visuals", buf, sizeof(buf));
    ui->goom->value(!my_strcasecmp(buf, "yes"));

    config_getvar(config, "visuals-w", buf, sizeof(buf));
    ui->goom_width->value(buf);

    config_getvar(config, "visuals-h", buf, sizeof(buf));
    ui->goom_height->value(buf);

    config_getvar(config, "visuals-fps", buf, sizeof(buf));
    ui->goom_fps->value(buf);

    config_destroy(config);

    return 1;
}

int write_mplayer_settings(GeneratorUI *ui)
{
    config_t *config;

    config = config_open(PATH_BASEISO "/etc/mplayer/mplayer.conf", 0);
    if (!config) {
        fl_alert("Failed to write MPlayer configuration.\n");
        return 0;
    }

    config_setvar(config, "visuals", yes_no(ui->goom->value()));
    config_setvar(config, "visuals-w", ui->goom_width->value());
    config_setvar(config, "visuals-h", ui->goom_height->value());
    config_setvar(config, "visuals-fps", ui->goom_fps->value());

    config_write(config, PATH_BASEISO "/etc/mplayer/mplayer.conf");
    config_destroy(config);

    return 1;
}
